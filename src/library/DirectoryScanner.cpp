//
// Created by vitalya on 07.12.18.
//

#include <QDirIterator>
#include "DirectoryScanner.h"

DirectoryScanner::DirectoryScanner(const QString& directory, std::atomic_bool* needStop)
    : WorkingDirectory(directory)
    , NeedStop(needStop)
{
}

DirectoryScanner::DirectoryScanner(const QDir& dir, std::atomic_bool* needStop)
    : WorkingDirectory(dir)
    , NeedStop(needStop)
{
}

void DirectoryScanner::RecursiveScanReduceBySize(FileMap& fileMap, uint64_t& filesNumber) {
    QDirIterator it(WorkingDirectory, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (it.hasNext()) {
        if (NeedStop->load()) {
            break;
        }
        QFileInfo file(it.next());
        if (!file.permission(QFile::ReadUser)) {
            continue;
        }
        if (file.isFile()) {
            ++filesNumber;
            fileMap[file.size()].push_back(file.absoluteFilePath());
        }
    }
    emit Finished();
}

void DirectoryScanner::setWorkingDirectory(const QDir& dir) {
    WorkingDirectory = dir;
}

void DirectoryScanner::setWorkingDirectory(const QString& directory) {
    WorkingDirectory = directory;
}
