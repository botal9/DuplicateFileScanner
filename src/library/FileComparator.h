//
// Created by vitalya on 08.12.18.
//

#ifndef DIRDEMO_FILECOMPARATOR_H
#define DIRDEMO_FILECOMPARATOR_H

#include "DirectoryScanner.h"

#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/policies.hpp>

#include <QtCore/QObject>
#include <QtCore/QRunnable>
#include <QtCore/QString>
#include <QtCore/QVector>

using SpscQueue = boost::lockfree::spsc_queue<FileList>;

static constexpr std::size_t BUFFER_SIZE = 128 * 1024;

class FileComparator : public QRunnable {
public:
    FileComparator(const FileList& files, SpscQueue& queue);
    virtual ~FileComparator() = default;

    void run() override;

private:
    bool Compare(const QString& lhs, const QString& rhs);

    std::size_t Hash(const QString& file);

private:
    QMultiHash<std::size_t, QVector<QString>> hash2file;

    FileList Files;
    SpscQueue& Queue;
};



#endif //DIRDEMO_FILECOMPARATOR_H
