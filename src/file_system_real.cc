#include <boost/filesystem.hpp>
#include <fstream>

#include "file_system_real.h"

namespace fs = boost::filesystem;

RealFileSystem::RealFileSystem(std::mutex& mutex_fs): FileSystem(mutex_fs) {}

bool RealFileSystem::exists( const fs::path& path ) const {
    bool flag;
    mutex_fs_.lock();  /**** atomic start ****/
    flag = fs::exists(path);
    mutex_fs_.unlock();  /**** atomic end ****/
    return flag;
}

bool RealFileSystem::is_directory( const fs::path& path ) const {
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

bool RealFileSystem::is_empty( const fs::path& path ) const {
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
    std::string path_str = path.lexically_normal().string();
    std::fstream fstr;
    mutex_fs_.lock();  /**** atomic start ****/
    fstr.open(path_str.c_str(), std::fstream::out);
    if (!fstr.is_open()) {
        mutex_fs_.unlock();  /**** atomic end ****/
        return false;
    }
    fstr << body;
    fstr.close();
    mutex_fs_.unlock();  /**** atomic end ****/
    return true;
}

bool RealFileSystem::read( const boost::filesystem::path& path, std::string& data ) const {
    std::string path_str = path.lexically_normal().string();
    std::fstream fstr;
    mutex_fs_.lock();  /**** atomic start ****/
    fstr.open(path_str.c_str(), std::fstream::in);
    if (!fstr.is_open()) {
        mutex_fs_.unlock();  /**** atomic end ****/
        return false;
    }
    data = "";
    std::string line;
    while (std::getline(fstr, line)) {
        data += line;
        data += '\n';
    }
    fstr.close();
    mutex_fs_.unlock();  /**** atomic end ****/
    return true;
}

bool RealFileSystem::list_directory( const boost::filesystem::path& path, std::string& list_str ) const {  
    list_str = ""; 
    if (!is_directory(path)) {
        return false;
    }
    mutex_fs_.lock();  /**** atomic start ****/
    for (fs::directory_iterator it(path); it!=fs::directory_iterator(); it++) {
        list_str += it->path().filename().string();
        list_str += ", ";
    }
    mutex_fs_.unlock();  /**** atomic end ****/
    if (list_str != "") {
        list_str = list_str.substr(0, list_str.size()-2);
    }
    list_str = "[" + list_str + "]";
    return true;
}

fs::path RealFileSystem::current_path() const {
    return fs::current_path();
}

void RealFileSystem::current_path(const fs::path& path, boost::system::error_code& ec) {
    fs::current_path(path, ec);
}