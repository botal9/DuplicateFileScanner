//
// Created by vitalya on 07.12.18.
//

#ifndef DIRDEMO_DIRECTORYSCANNER_H
#define DIRDEMO_DIRECTORYSCANNER_H

#include "util.h"

#include <atomic>

#include <QtCore/QObject>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

class DirectoryScanner {
public:
    DirectoryScanner() = delete;
    DirectoryScanner(const QString& directory);
    ~DirectoryScanner() = default;

    FileList RecursiveScan();
    FileMap RecursiveScanReduceBySize();

    void Stop();

private:
    void RecursiveScan(QDir& tmpDir, FileList& result);
    void RecursiveScanReduceBySize(QDir& tmpDir, FileMap& result);

private:
    const QDir WorkingDirectory;
    std::atomic_bool NeedStop;
};


#endif //DIRDEMO_DIRECTORYSCANNER_H
