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

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string FileSystem::path_cat(
  boost::beast::string_view base,
  boost::beast::string_view path
) {
    if(base.empty()) {
      return std::string(path);
    }
    std::string result(base);
    char constexpr path_separator = '/';
    if(result.back() == path_separator) {
        result.resize(result.size() - 1);
    }
    result.push_back(path_separator);
    if (!path.empty() && path[0] == path_separator) {
      result.append(path.data() + 1, path.size() - 1);
    } 
    else {
      result.append(path.data(), path.size());
    }
  return result;
}
