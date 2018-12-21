//
// Created by vitalya on 07.12.18.
//

#include "DirectoryScanner.h"

DirectoryScanner::DirectoryScanner(const QString& directory, std::atomic_bool* needStop)
    : WorkingDirectory(directory)
    , NeedStop(needStop)
{
}

DirectoryScanner::DirectoryScanner(const QDir &dir, std::atomic_bool* needStop)
    : WorkingDirectory(dir)
    , NeedStop(needStop)
{
}

void DirectoryScanner::RecursiveScanReduceBySize(QDir &tmpDir, FileMap &result, uint64_t& filesNumber) {
    QFileInfoList list = tmpDir.entryInfoList();
    for (const QFileInfo& file : list) {
        if (NeedStop->load()) {
            break;
        }
        if (file.fileName() == "." || file.fileName() == "..") {
            continue;
        }
        if (file.isSymLink()) {
            continue;
        }
        if (!file.permission(QFile::ReadUser)) {
            continue;
        }
        if (file.isDir()) {
            tmpDir.cd(file.fileName());
            RecursiveScanReduceBySize(tmpDir, result, filesNumber);
            tmpDir.cdUp();
        }
        if (file.isFile()) {
            ++filesNumber;
            result[file.size()].push_back(file.absoluteFilePath());
        }
    }
}

void DirectoryScanner::RecursiveScanReduceBySize(FileMap& fileMap, uint64_t& filesNumber) {
    if (!WorkingDirectory.isEmpty()) {
        QDir tmpDir = WorkingDirectory;
        RecursiveScanReduceBySize(tmpDir, fileMap, filesNumber);
    }
}

void DirectoryScanner::setWorkingDirectory(const QDir& dir) {
    WorkingDirectory = dir;
}

void DirectoryScanner::setWorkingDirectory(const QString& directory) {
    WorkingDirectory = directory;
}
