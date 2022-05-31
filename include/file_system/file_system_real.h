#ifndef FILE_SYSTEM_REAL_H
#define FILE_SYSTEM_REAL_H

#include "file_system_base.h"

class RealFileSystem: public FileSystem {
    public:
        RealFileSystem(std::mutex& mutex_fs);

        bool exists( const boost::filesystem::path& path ) const override;
        int get_next_id (const boost::filesystem::path& path) const override;
        bool is_directory( const boost::filesystem::path& path ) const override;
        bool remove( const boost::filesystem::path& path ) override;
        bool is_empty( const boost::filesystem::path& path ) const override;
        bool create_directories( const boost::filesystem::path& path ) override;
        bool upload_file( const boost::filesystem::path& path, const std::string& body) override;
        bool read( const boost::filesystem::path& path, std::string& data ) const override;
        bool list_directory( const boost::filesystem::path& path, std::string& list_str ) const override;

        // thread-unsafe
        boost::filesystem::path current_path() const override;
        void current_path(const boost::filesystem::path& path, boost::system::error_code& ec) override;
        // wrapped by exists() and get_next_id()
        bool exists_( const boost::filesystem::path& path ) const override;

};

#endif