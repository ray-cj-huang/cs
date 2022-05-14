#ifndef FILE_SYSTEM_REAL_H
#define FILE_SYSTEM_REAL_H

#include "file_system_base.h"

class RealFileSystem: public FileSystem {
    public:
        RealFileSystem(std::mutex& mutex_fs);

        bool exists( const boost::filesystem::path& path ) override;
        bool is_directory( const boost::filesystem::path& path ) override;
        bool remove( const boost::filesystem::path& path ) override;
        bool is_empty( const boost::filesystem::path& path ) override;
        bool create_directories( const boost::filesystem::path& path ) override;
        bool upload_file( const boost::filesystem::path& path, const std::string& body) override;
};

#endif