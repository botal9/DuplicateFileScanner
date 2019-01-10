//
// Created by vitalya on 15.12.18.
//

#include "Worker.h"

Worker::Worker(const QString &directory, QObject* parent, std::atomic_bool* needStop)
    : WorkingDirectory(directory)
    , Parent(parent)
    , NeedStop(needStop)
{
}

Worker::Worker(QObject* parent, std::atomic_bool* needStop)
    : Parent(parent)
    , NeedStop(needStop)
{
}

void Worker::setWorkingDirectory(const QString &directory) {
    WorkingDirectory = directory;
}

Worker::~Worker() {
    delete Scanner;
    delete Comparator;
}

void Worker::Process() {
    qDebug() << "Start scanning directory";

    Scanner = new DirectoryScanner(WorkingDirectory, NeedStop);

    uint64_t filesNumber = 0;
    FileMap fileMap;
    Scanner->RecursiveScanReduceBySize(fileMap, filesNumber);

    emit SetupFilesNumber(filesNumber);
    if (NeedStop->load()) {
        emit Aborted();
        return;
    }

    Comparator = new FileComparator(fileMap, NeedStop);

    connect(Comparator, SIGNAL(SendDuplicates(const FileList&)), Parent, SLOT(AddDuplicatesList(const FileList&)));
    connect(Comparator, SIGNAL(ProcessedFiles(int)), Parent, SLOT(UpdateProgressBar(int)));
    connect(Comparator, SIGNAL(Finished()), this, SLOT(Finish()));

    Comparator->Process();
}

void Worker::Stop() {
    qDebug() << "Stop scanning";
    NeedStop->store(true);
}

void Worker::Finish() {
    if (NeedStop->load()) {
        emit Aborted();
    } else {
        emit Finished();
    }
}
