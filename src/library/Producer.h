//
// Created by vitalya on 11.12.18.
//

#ifndef DIRECTORYSCANNER_PRODUCER_H
#define DIRECTORYSCANNER_PRODUCER_H

#include "util.h"

#include <atomic>

#include <QtCore/QRunnable>

class Producer : public QRunnable {
public:
    Producer() = delete;
    Producer(const FileMap& fileMap, SpscQueue& queue, std::atomic_bool& needStop);
    ~Producer() override;

    void run() override;

private:
    void ProcessFileList(const FileList& fileList);

    bool Compare(const QString& lhs, const QString& rhs);

    std::size_t Hash(const QString& file);

    std::size_t FastHash(const QString& file);

private:
    FileMap Hash2FileList;
    SpscQueue& Queue;
    std::atomic_bool& NeedStop;
};


#endif //DIRECTORYSCANNER_PRODUCER_H
