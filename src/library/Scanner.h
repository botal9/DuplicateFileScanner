//
// Created by vitalya on 10.12.18.
//

#ifndef DIRDEMO_WORKER_H
#define DIRDEMO_WORKER_H

#include "DirectoryScanner.h"
#include "FileComparator.h"
#include "util.h"

#include <atomic>
#include <thread>

#include <QtCore/QDir>
#include <QtCore/QObject>


class Worker : public QObject {

    Q_OBJECT

public:
    Worker() = default;
    Worker(const QString& directory);
    virtual ~Worker() = default;

    void Scan();

    void Stop();

private:
    SpscQueue Queue;
    QDir WorkingDirectory;
    std::atomic_bool NeedStop;
};


#endif //DIRDEMO_WORKER_H
