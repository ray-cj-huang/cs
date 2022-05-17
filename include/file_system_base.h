#ifndef FILE_SYSTEM_BASE_H
#define FILE_SYSTEM_BASE_H

// https://www.boost.org/doc/libs/1_79_0/libs/filesystem/doc/index.htm
// https://en.cppreference.com/w/cpp/filesystem/path
#include <boost/filesystem/path.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <mutex>

class FileSystem {
    public:
        FileSystem(std::mutex& mutex_fs);

        virtual bool exists( const boost::filesystem::path& path ) const = 0;
        virtual bool is_directory( const boost::filesystem::path& path ) const = 0;
        virtual bool remove( const boost::filesystem::path& path ) = 0;
        virtual bool is_empty( const boost::filesystem::path& path ) const = 0;
        virtual bool create_directories( const boost::filesystem::path& path ) = 0;
        virtual bool upload_file( const boost::filesystem::path& path, const std::string& body) = 0;
        
        // thread_unsafe:
        virtual boost::filesystem::path current_path() const = 0;
        virtual void current_path(const boost::filesystem::path& path, boost::system::error_code& ec) = 0;
        
        boost::filesystem::path absolute( const boost::filesystem::path& path ) const;
    
    protected:
        std::mutex& mutex_fs_; // saved for filesystem multithread safety
};

#endif