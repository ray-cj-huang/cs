#include "file_system_base.h"

namespace fs = boost::filesystem;

FileSystem::FileSystem(std::mutex& mutex_fs): mutex_fs_(mutex_fs) {}

fs::path FileSystem::absolute( const fs::path& path ) const {
    if (path.is_absolute()) {
        return path;
    } else {
        return current_path() / path;
    }
}