//
// Created by vitalya on 07.12.18.
//

#ifndef DIRDEMO_DIRECTORYSCANNER_H
#define DIRDEMO_DIRECTORYSCANNER_H


#include <QtCore/QObject>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QDir>
#include <stdatomic.h>

using FileList = QVector<QString>;
using FileMap = QHash<uint64_t, FileList>;

class DirectoryScanner {
public:
    DirectoryScanner() = delete;
    DirectoryScanner(const QString& directory);
    virtual ~DirectoryScanner() = default;

    FileList RecursiveScan();
    FileMap RecursiveScanReduceBySize();

    void Stop();

private:
    void RecursiveScan(QDir& tmpDir, FileList& result);
    void RecursiveScanReduceBySize(QDir& tmpDir, FileMap& result);

private:
    const QDir WorkingDirectory;
    atomic_bool NeedStop;
};


#endif //DIRDEMO_DIRECTORYSCANNER_H
