//
// Created by vitalya on 11.12.18.
//

#ifndef DIRECTORYSCANNER_PRODUCER_H
#define DIRECTORYSCANNER_PRODUCER_H

#include "util.h"

#include <atomic>

#include <QObject>

class FileComparator : public QObject {
    Q_OBJECT

public:
    FileComparator() = default;
    FileComparator(const FileMap& fileMap, std::atomic_bool* needStop);
    ~FileComparator() = default;

public slots:
    void Process();

signals:
    void SendDuplicates(const FileList& files);
    void ProcessedFiles(int filesNumber);
    void Finished();

private:
    void ProcessFileList(const FileList& fileList);

    bool Compare(const QString& lhs, const QString& rhs);

    std::size_t Hash(const QString& file);

    std::size_t FastHash(const QString& file);

private:
    FileMap Hash2FileList;
    std::atomic_bool* NeedStop = nullptr;
};


#endif //DIRECTORYSCANNER_PRODUCER_H
