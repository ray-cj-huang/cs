#include <vector>
#include <iostream>

#include "file_system_fake.h"

namespace fs = boost::filesystem;

FakeFileSystem::FakeFileSystem(std::mutex& mutex_fs, fs::path pwd): FileSystem(mutex_fs), root_(new FakeDirectory("/")), pwd_(pwd) {}

FakeEntry::EntryType FakeFileSystem::traverse(FakeDirectory* &directory, fs::path& path, bool if_curr_dir) const {
    if (if_curr_dir) {
        path = FileSystem::absolute(path);
        path = path.lexically_normal();
    }
    auto it_curr_entry = path.begin();
    if (it_curr_entry == path.end() || it_curr_entry->string() == ".") {
        // reached the end of path with all the entries matched
        return FakeEntry::ENTRY_TYPE_DIR;
    } else if (it_curr_entry->string() != directory->name_) {
        // name mismatch
        return FakeEntry::ENTRY_TYPE_NOT_FOUND;
    } else { // entry name == directory->name_
        path = path.lexically_relative(*it_curr_entry);
        auto it_next_entry = path.begin();
        if (it_next_entry == path.end() || it_next_entry->string() == ".") {
            // current directory is the end
            return FakeEntry::ENTRY_TYPE_DIR;
        }
        // not the end of path, searching among child entries of directory
        for (auto it_child_entry = directory->child_entries_.begin(); it_child_entry != directory->child_entries_.end(); it_child_entry++) {
            if ( (*it_child_entry)->name_ != (*it_next_entry).string() ) {
                continue; // if name mismatch, skip
            }
            switch ((*it_child_entry)->type_) {
                case FakeEntry::ENTRY_TYPE_FILE: {
                    auto it_next_next_entry = it_next_entry;
                    it_next_next_entry++;
                    if ((*it_child_entry)->name_ == it_next_entry->string() && it_next_next_entry == path.end()) {
                        // end of path, and a matched file is found
                        return FakeEntry::ENTRY_TYPE_FILE;
                    }
                } break;
                case FakeEntry::ENTRY_TYPE_DIR:
                    directory = (FakeDirectory*)it_child_entry->get();
                    return traverse(directory, path, false /* recursive call, no longer in current directory */); 
                break;
                case FakeEntry::ENTRY_TYPE_NOT_FOUND:
                default:
                    // error, skip and continue searching
                    break;
            }
        }
    }
    
    return FakeEntry::ENTRY_TYPE_NOT_FOUND;
}

bool FakeFileSystem::exists( const fs::path& path ) const {
    bool if_exist = false;
    fs::path curr_path = path;
    mutex_fs_.lock();  /**** atomic start ****/
    FakeDirectory* directory = root_.get();
    switch (traverse(directory, curr_path)) {
        case FakeEntry::ENTRY_TYPE_FILE:
        case FakeEntry::ENTRY_TYPE_DIR:
            if_exist = true;
            break;
        case FakeEntry::ENTRY_TYPE_NOT_FOUND:
        default:
            if_exist = false;
            break;
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    return if_exist;
}

bool FakeFileSystem::is_directory( const fs::path& path ) const {
    bool if_dir = false;
    fs::path curr_path = path;
    mutex_fs_.lock();  /**** atomic start ****/
    FakeDirectory* directory = root_.get();
    switch (traverse(directory, curr_path)) {
        case FakeEntry::ENTRY_TYPE_DIR:
            if_dir = true;
            break;
        case FakeEntry::ENTRY_TYPE_FILE:
        case FakeEntry::ENTRY_TYPE_NOT_FOUND:
        default:
            if_dir = false;
            break;
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    return if_dir;
}

// returns if existing files/directories are removed
bool FakeFileSystem::remove( const fs::path& path ) {
    bool if_removed = false;
    fs::path curr_path =  FileSystem::absolute(path).lexically_normal();
    if (curr_path.filename() == ".") {
        // if directory, get the directory name instead of "."
        curr_path = curr_path.parent_path();
    }
    fs::path parent_path = curr_path.parent_path();
    mutex_fs_.lock();  /**** atomic start ****/
    FakeDirectory* directory = root_.get();
    if (traverse(directory, parent_path) != FakeEntry::ENTRY_TYPE_DIR) {
        // no further file/directory to remove
        return false;
    }
    if (directory->name_ == "/") {
        // remove the root directory
        directory->child_entries_.clear();
        if_removed = true;
    } else {
        // the removed file/directory is inside directory->child_entries_
        for (auto it = directory->child_entries_.begin(); it != directory->child_entries_.end(); it++) {
            if ((*it)->name_ == curr_path.filename()) {
                directory->child_entries_.erase(it);
                if_removed = true;
                break;
            }
        }
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    // if pwd_ is removed, retrieve to the first existing parent directory
    while (!is_directory(pwd_)) {
        pwd_ = pwd_.parent_path();
    }
    return if_removed;
}

bool FakeFileSystem::is_empty( const fs::path& path ) const {
    bool if_empty = false;
    fs::path curr_path = path;
    mutex_fs_.lock();  /**** atomic start ****/
    FakeDirectory* directory = root_.get();
    if_empty = (
        traverse(directory, curr_path) == FakeEntry::ENTRY_TYPE_DIR && // path is an existing directory
        directory->child_entries_.size() == 0 // no entries in the directory
    );
    mutex_fs_.unlock();  /**** atomic end ****/
    return if_empty;
}

// returns if new directories are created
bool FakeFileSystem::create_directories( const fs::path& path ) {
    bool if_new_created = false;
    fs::path curr_path = path.lexically_normal();
    if (curr_path.filename() == ".") {
        // if directory, get the directory name instead of "."
        curr_path = curr_path.parent_path();
    }
    mutex_fs_.lock();  /**** atomic start ****/
    FakeDirectory* directory = root_.get();
    switch (traverse(directory, curr_path)) {
        case FakeEntry::ENTRY_TYPE_DIR:
            // directory already exists
            mutex_fs_.unlock();  /**** atomic end ****/
            return false;
        default:
            // now path is non-empty
            std::string entry_name = curr_path.begin()->string();
            directory->child_entries_.emplace_back(new FakeDirectory(entry_name));
            mutex_fs_.unlock();  /**** atomic end ****/
            create_directories(path);
            return true;
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    return false;
}

bool FakeFileSystem::upload_file( const fs::path& path, const std::string& body ) {
    fs::path curr_path = path.lexically_normal();
    if (curr_path.filename() == "." || curr_path.filename() == "/") {
        // path is a directory, not a file
        return false;
    }
    create_directories(curr_path.parent_path()); // atomic operation
    mutex_fs_.lock();  /**** atomic start ****/
    FakeDirectory* directory = root_.get();
    traverse(directory, curr_path);
    bool if_exist = false;
    for (auto entry: directory->child_entries_) {
        if (entry->name_ == path.filename().string() && entry->type_ == FakeEntry::ENTRY_TYPE_FILE) {
            // file already exists, update the content
            ((FakeFile*)entry.get())->file_content_ = body;
            if_exist = true;
            break;
        }
    }
    if (!if_exist) {
        // file doesn't exist, create a new entry with content
        directory->child_entries_.emplace_back(new FakeFile(curr_path.filename().string(), body));
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    return true;
}

std::shared_ptr<FakeEntry> FakeFileSystem::get_entry( const fs::path& path ) {
    std::shared_ptr<FakeEntry> entry_wanted(nullptr);
    fs::path curr_path = path.lexically_normal();
    if (curr_path.filename() == ".") {
        // if directory, get the directory name instead of "."
        curr_path = curr_path.parent_path();
    }
    mutex_fs_.lock();  /**** atomic start ****/
    FakeDirectory* directory = root_.get();
    switch (traverse(directory, curr_path)) {
        case FakeEntry::ENTRY_TYPE_DIR:
            entry_wanted.reset(directory);
            break;
        case FakeEntry::ENTRY_TYPE_FILE:
            for (auto entry : directory->child_entries_) {
                if (entry->name_ == path.filename().string()) {
                    entry_wanted = entry;
                    break;
                }
            }
            break;
        case FakeEntry::ENTRY_TYPE_NOT_FOUND:
        default:
            break;
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    return entry_wanted;
}

fs::path FakeFileSystem::current_path() const {
    fs::path pwd;
    pwd = pwd_;
    return pwd;
}

void FakeFileSystem::current_path(const fs::path& path, boost::system::error_code& ec) {
    if (!exists(path)) {
        ec.assign(boost::system::errc::no_such_file_or_directory, boost::system::generic_category());
    } else if (!is_directory(path)) {
        ec.assign(boost::system::errc::not_a_directory, boost::system::generic_category());
    } else {
        ec.assign(boost::system::errc::success, boost::system::generic_category()); // success
        pwd_ = path.lexically_normal();
    }
}