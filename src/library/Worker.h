//
// Created by vitalya on 15.12.18.
//

#ifndef DUPLICATEFILESCANNER_WORKER_H
#define DUPLICATEFILESCANNER_WORKER_H

#include "DirectoryScanner.h"
#include "FileComparator.h"

#include <atomic>

#include <QDebug>
#include <QThreadPool>

class Worker : public QObject {
    Q_OBJECT

public:
    Worker() = default;
    Worker(QObject* parent, std::atomic_bool* needStop);
    Worker(const QString& directory, QObject* parent, std::atomic_bool* needStop);
    ~Worker();

    void setWorkingDirectory(const QString& directory);

public slots:
    void Process();
    void Finish();
    void Stop();

signals:
    void SetupFilesNumber(int filesNumber);
    void Finished();
    void Aborted();

private:
    QDir WorkingDirectory;
    QObject* Parent = nullptr;
    std::atomic_bool* NeedStop = nullptr;

    DirectoryScanner* Scanner = nullptr;
    FileComparator* Comparator = nullptr;
};


#endif //DUPLICATEFILESCANNER_WORKER_H
