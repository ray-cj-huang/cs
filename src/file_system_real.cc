#include <boost/filesystem.hpp>
#include <fstream>

#include "file_system_real.h"

namespace fs = boost::filesystem;

RealFileSystem::RealFileSystem(std::mutex& mutex_fs): FileSystem(mutex_fs) {}

bool RealFileSystem::exists( const fs::path& path ) {
    bool flag;
    mutex_fs_.lock();  /**** atomic start ****/
    flag = fs::exists(path);
    mutex_fs_.unlock();  /**** atomic end ****/
    return flag;
}

bool RealFileSystem::is_directory( const fs::path& path ) {
    bool flag;
    mutex_fs_.lock();  /**** atomic start ****/
    flag = fs::is_directory(path);
    mutex_fs_.unlock();  /**** atomic end ****/
    return flag;
}

bool RealFileSystem::remove( const fs::path& path ) {
    bool flag;
    mutex_fs_.lock();  /**** atomic start ****/
    flag = fs::remove(path);
    mutex_fs_.unlock();  /**** atomic end ****/
    return flag;
}

bool RealFileSystem::is_empty( const fs::path& path ) {
    bool flag;
    mutex_fs_.lock();  /**** atomic start ****/
    flag = fs::is_empty(path);
    mutex_fs_.unlock();  /**** atomic end ****/
    return flag;
}

bool RealFileSystem::create_directories( const fs::path& path ) {
    bool flag;
    mutex_fs_.lock();  /**** atomic start ****/
    flag = fs::create_directories(path);
    mutex_fs_.unlock();  /**** atomic end ****/
    return flag;
}

bool RealFileSystem::upload_file( const fs::path& path, const std::string& body ) {
    std::string path_str = path.lexically_normal().generic_string();
    std::fstream fstr;
    mutex_fs_.lock();  /**** atomic start ****/
    fstr.open(path_str.c_str(), std::fstream::out);
    if (!fstr.is_open()) {
        return false;
    }
    fstr << body;
    fstr.close();
    mutex_fs_.unlock();  /**** atomic end ****/
    return true;
}