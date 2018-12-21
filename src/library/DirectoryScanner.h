//
// Created by vitalya on 07.12.18.
//

#ifndef DIRDEMO_DIRECTORYSCANNER_H
#define DIRDEMO_DIRECTORYSCANNER_H

#include "util.h"

#include <atomic>

#include <QObject>
#include <QFileInfo>
#include <QDir>

class DirectoryScanner : public QObject {
    Q_OBJECT

public:
    DirectoryScanner() = default;
    DirectoryScanner(const QString& directory, std::atomic_bool* needStop);
    DirectoryScanner(const QDir& dir, std::atomic_bool* needStop);
    ~DirectoryScanner() = default;

    void setWorkingDirectory(const QDir& directory);
    void setWorkingDirectory(const QString& directory);

    void RecursiveScanReduceBySize(FileMap& fileMap, uint64_t& filesNumber);

private:
    void RecursiveScanReduceBySize(QDir& tmpDir, FileMap& result, uint64_t& filesNumber);

private:
    QDir WorkingDirectory;
    std::atomic_bool* NeedStop = nullptr;
};


#endif //DIRDEMO_DIRECTORYSCANNER_H
