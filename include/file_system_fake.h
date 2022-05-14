#ifndef FILE_SYSTEM_FAKE_H
#define FILE_SYSTEM_FAKE_H

#include "file_system_base.h"

class FakeFileSystem: public FileSystem {
    // TODO: a fake filesystem that doesn't involve the disk
    public:
        FakeFileSystem(std::mutex& mutex_fs);

        bool exists( const boost::filesystem::path& path ) override;
        bool is_directory( const boost::filesystem::path& path ) override;
        bool remove( const boost::filesystem::path& path ) override;
        bool is_empty( const boost::filesystem::path& path ) override;
        bool create_directories( const boost::filesystem::path& path ) override;
        bool upload_file( const boost::filesystem::path& path, const std::string& body) override;
};

#endif