#include <vector>
#include <iostream>

#include "file_system_fake.h"

namespace fs = boost::filesystem;

FakeFileSystem::FakeFileSystem(std::mutex& mutex_fs): FileSystem(mutex_fs), root_(new FakeDirectory("/")) {}

FakeEntry::EntryType traverse(std::vector<std::shared_ptr<FakeEntry>>* &entries, fs::path& path) {
    auto it = path.begin();
    if (it == path.end()) {
        return FakeEntry::ENTRY_TYPE_DIR;
    }
    std::string entry_name = it->string();
    if (entry_name == ".") {
        return FakeEntry::ENTRY_TYPE_DIR;
    }
    for (auto it_child_entry = entries->begin(); it_child_entry != entries->end(); it_child_entry++) {
        if ((*it_child_entry)->name_ == entry_name) {
            auto it_next = it;
            it_next++;
            if ((*it_child_entry)->type_ == FakeEntry::ENTRY_TYPE_FILE && it_next == path.end()) {
                return FakeEntry::ENTRY_TYPE_FILE;
            } else if ((*it_child_entry)->type_ == FakeEntry::ENTRY_TYPE_DIR) {
                path = path.lexically_relative(*it);
                entries = & ((FakeDirectory*)(*it_child_entry).get())->child_entries_;
                return traverse(entries, path); // go to next level
            } else /* if (type_ == FakeEntry::ENTRY_TYPE_NOT_FOUND)*/ {
                continue;
                // a bad entry; continue searching for good entries
            }
        }
    }
    return FakeEntry::ENTRY_TYPE_NOT_FOUND;
}

bool FakeFileSystem::exists( const fs::path& path ) {
    bool if_exist = false;
    fs::path curr_path = path.lexically_relative(fs::path("/"));
    mutex_fs_.lock();  /**** atomic start ****/
    std::vector<std::shared_ptr<FakeEntry>>* entries = &root_->child_entries_;
    switch (traverse(entries, curr_path)) {
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

bool FakeFileSystem::is_directory( const fs::path& path ) {
    bool if_dir = false;
    fs::path curr_path = path.lexically_relative(fs::path("/"));
    mutex_fs_.lock();  /**** atomic start ****/
    std::vector<std::shared_ptr<FakeEntry>>* entries = &root_->child_entries_;
    switch (traverse(entries, curr_path)) {
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
    fs::path parent_path = path.parent_path().lexically_relative(fs::path("/"));
    mutex_fs_.lock();  /**** atomic start ****/
    std::vector<std::shared_ptr<FakeEntry>>* entries = &root_->child_entries_;
    traverse(entries, parent_path);
    for (auto it = entries->begin(); it != entries->end(); it++) {
        if ((*it)->name_ == path.filename()) {
            it = entries->erase(it);
            if_removed = true;
            break;
        }
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    return if_removed;
}

bool FakeFileSystem::is_empty( const fs::path& path ) {
    bool if_empty = false;
    fs::path curr_path = path.lexically_relative(fs::path("/"));
    mutex_fs_.lock();  /**** atomic start ****/
    std::vector<std::shared_ptr<FakeEntry>>* entries = &root_->child_entries_;
    if_empty = (
        traverse(entries, curr_path) == FakeEntry::ENTRY_TYPE_DIR && // path is an existing directory
        entries->size() == 0 // no entries in the directory
    );
    mutex_fs_.unlock();  /**** atomic end ****/
    return if_empty;
}

// returns if new directories are created
bool FakeFileSystem::create_directories( const fs::path& path ) {
    bool if_new_created = false;
    fs::path curr_path = path.lexically_relative(fs::path("/"));
    mutex_fs_.lock();  /**** atomic start ****/
    std::vector<std::shared_ptr<FakeEntry>>* entries = &root_->child_entries_;
    switch (traverse(entries, curr_path)) {
        case FakeEntry::ENTRY_TYPE_DIR:
            mutex_fs_.unlock();  /**** atomic end ****/
            return false;
        default:
            // now path is non-empty
            std::string entry_name = curr_path.begin()->string();
            entries->emplace_back(new FakeDirectory(entry_name));
            mutex_fs_.unlock();  /**** atomic end ****/
            create_directories(path);
            return true;
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    return false;
}

bool FakeFileSystem::upload_file( const fs::path& path, const std::string& body ) {
    create_directories(path.parent_path()); // atomic operation
    fs::path curr_path = path.parent_path().lexically_relative(fs::path("/"));
    mutex_fs_.lock();  /**** atomic start ****/
    std::vector<std::shared_ptr<FakeEntry>>* entries = &root_->child_entries_;
    traverse(entries, curr_path);
    bool if_exist = false;
    for (auto entry: (*entries)) {
        if (entry->name_ == path.filename().string() && entry->type_ == FakeEntry::ENTRY_TYPE_FILE) {
            // file already exists, update the content
            ((FakeFile*)entry.get())->file_content_ = body;
            if_exist = true;
            break;
        }
    }
    if (!if_exist) {
        // file doesn't exist, create a new entry with content
        entries->emplace_back(new FakeFile(path.filename().string(), body));
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    return true;
}

std::shared_ptr<FakeEntry> FakeFileSystem::get_entry( const fs::path& path ) {
    std::shared_ptr<FakeEntry> entry_wanted(nullptr);
    fs::path curr_path = path.parent_path().lexically_relative(fs::path("/"));
    mutex_fs_.lock();  /**** atomic start ****/
    std::vector<std::shared_ptr<FakeEntry>>* entries = &root_->child_entries_;
    if (traverse(entries, curr_path) != FakeEntry::ENTRY_TYPE_DIR) {
        mutex_fs_.unlock();  /**** atomic end ****/
        return entry_wanted; // nullptr
    }
    for (auto entry : (*entries)) {
        if (entry->name_ == path.filename().string()) {
            entry_wanted = entry;
            break;
        }
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    return entry_wanted;
}