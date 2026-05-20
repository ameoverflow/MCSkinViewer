//
// Created by void on 20/05/2026.
//

#include "FileSystem.h"

#include "State.h"

std::string FileSystem::GetSystemTempPath() {
#ifdef _WIN32
    char tempFolder[MAX_PATH];
    GetTempPathA(MAX_PATH, tempFolder);
    return std::string(tempFolder);
#else
    return "/tmp/";
#endif
}

void FileSystem::ChangeWatchedFile(const std::string& path) {
    if (State.watch != nullptr) {
        State.watch.reset();
    }
    TraceLog(LOG_INFO, "Adding a file watch on %s", path.c_str());
    State.watch = std::make_unique<filewatch::FileWatch<std::string>>(
        path,
        [](const std::string& changedPath, const filewatch::Event change_type) {
            switch (change_type) {
                case filewatch::Event::modified:
                    State.needsReload = true;
                    break;
                default:
                    break;
            }
        }
    );
}

void FileSystem::StopWatching() {
    TraceLog(LOG_INFO, "Removing file watch");
    if (State.watch != nullptr) {
        State.watch.reset();
    }
}
