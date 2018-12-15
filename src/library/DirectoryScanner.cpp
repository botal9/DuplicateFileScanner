//
// Created by vitalya on 07.12.18.
//

#include "DirectoryScanner.h"

DirectoryScanner::DirectoryScanner(const QString& directory)
    : WorkingDirectory(directory)
    , NeedStop(false)
{
}

void DirectoryScanner::RecursiveScan(QDir& tmpDir, FileList& result) {
    QFileInfoList list = tmpDir.entryInfoList();
    for (const QFileInfo& file : list) {
        if (NeedStop) {
            break;
        }
        if (file.fileName() == "." || file.fileName() == "..") {
            continue;
        }
        if (file.isSymLink()){
            continue;
        }
        if (!file.permission(QFile::ReadUser)) {
            continue;
        }
        if (file.isDir()) {
            tmpDir.cd(file.fileName());
            RecursiveScan(tmpDir, result);
            tmpDir.cdUp();
        }
        if (file.isFile()) {
            result.push_back(file.absoluteFilePath());
        }
    }
}

FileList DirectoryScanner::RecursiveScan() {
    FileList result;
    QDir tmpDir = WorkingDirectory;
    RecursiveScan(tmpDir, result);
    return result;
}

void DirectoryScanner::RecursiveScanReduceBySize(QDir &tmpDir, FileMap &result) {
    QFileInfoList list = tmpDir.entryInfoList();
    for (const QFileInfo& file : list) {
        if (NeedStop) {
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
            RecursiveScanReduceBySize(tmpDir, result);
            tmpDir.cdUp();
        }
        if (file.isFile()) {
            result[file.size()].push_back(file.absoluteFilePath());
        }
    }
}

FileMap DirectoryScanner::RecursiveScanReduceBySize() {
    FileMap result;
    QDir tmpDir = WorkingDirectory;
    RecursiveScanReduceBySize(tmpDir, result);
    return result;
}

void DirectoryScanner::Stop() {
    NeedStop = true;
}
