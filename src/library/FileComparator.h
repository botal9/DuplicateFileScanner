//
// Created by vitalya on 11.12.18.
//

#ifndef DIRECTORYSCANNER_PRODUCER_H
#define DIRECTORYSCANNER_PRODUCER_H

#include "util.h"

#include <atomic>

#include <QByteArray>
#include <QObject>

class FileComparator : public QObject {
    Q_OBJECT

public:
    FileComparator() = default;
    FileComparator(std::atomic_bool* needStop);
    FileComparator(const FileMap& fileMap, std::atomic_bool* needStop);
    ~FileComparator() = default;

    void setFileMap(const FileMap& fileMap);

public slots:
    void Process();

signals:
    void SendDuplicates(const FileList& files);
    void ProcessedFiles(int filesNumber);
    void Finished();

private:
    void ProcessFileList(const FileList& fileList);

    bool Compare(QFile& lhs, QFile& rhs);

    QByteArray Hash(QFile& file);

    QByteArray FastHash(QFile& file);

private:
    FileMap Size2FileList;
    std::atomic_bool* NeedStop = nullptr;
};


#endif //DIRECTORYSCANNER_PRODUCER_H
