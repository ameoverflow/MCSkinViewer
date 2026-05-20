//
// Created by void on 20/05/2026.
//

#ifndef SKINVIEWER_FILESYSTEM_H
#define SKINVIEWER_FILESYSTEM_H
#include <string>

namespace FileSystem {
    std::string GetSystemTempPath();
    void ChangeWatchedFile(const std::string& path);
    void StopWatching();
}

#endif //SKINVIEWER_FILESYSTEM_H
