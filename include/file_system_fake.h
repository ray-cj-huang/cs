#ifndef FILE_SYSTEM_FAKE_H
#define FILE_SYSTEM_FAKE_H

#include "file_system_base.h"

class FakeEntry {
    public:
        enum EntryType {
            ENTRY_TYPE_DIR = 0,
            ENTRY_TYPE_FILE = 1,
            ENTRY_TYPE_NOT_FOUND = 2
        };
        FakeEntry(std::string name): name_(name), type_(ENTRY_TYPE_NOT_FOUND) {}
        std::string name_;
        EntryType type_;
};

class FakeDirectory: public FakeEntry {
    public:
        FakeDirectory(std::string name): FakeEntry(name) {
            type_ = ENTRY_TYPE_DIR;
        }
        std::string ToString(boost::filesystem::path root = boost::filesystem::path("")) const {
            std::string dir_str = "";
            root /= boost::filesystem::path(name_) / boost::filesystem::path("//");
            root = root.lexically_normal();
            for (auto entry : child_entries_) {
                boost::filesystem::path new_root = (root / boost::filesystem::path(entry->name_)).lexically_normal();
                if (entry->type_ == ENTRY_TYPE_FILE) {
                    dir_str += new_root.string() + "\n";
                } else if (entry->type_ == ENTRY_TYPE_DIR) {
                    dir_str += new_root.string() + "/\n";
                    dir_str += ((FakeDirectory*)entry.get())->ToString(root);
                }
            }
            return dir_str;
        }
        std::vector<std::shared_ptr<FakeEntry>> child_entries_;
};

class FakeFile: public FakeEntry {
    public:
        FakeFile(std::string name, std::string content): FakeEntry(name), file_content_(content) {
            type_ = ENTRY_TYPE_FILE;
        }
        std::string file_content_;
};

class FakeFileSystem: public FileSystem {
    public:
        FakeFileSystem(std::mutex& mutex_fs);

        bool exists( const boost::filesystem::path& path ) override;
        bool is_directory( const boost::filesystem::path& path ) override;
        bool remove( const boost::filesystem::path& path ) override;
        bool is_empty( const boost::filesystem::path& path ) override;
        bool create_directories( const boost::filesystem::path& path ) override;
        bool upload_file( const boost::filesystem::path& path, const std::string& body) override;

        std::shared_ptr<FakeEntry> get_entry( const boost::filesystem::path& path );

        std::string ToString() const { return root_->ToString(); };
    
    private:
        std::shared_ptr<FakeDirectory> root_;
};

#endif