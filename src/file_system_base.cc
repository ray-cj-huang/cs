#include "file_system_base.h"

FileSystem::FileSystem(std::mutex& mutex_fs): mutex_fs_(mutex_fs) {}