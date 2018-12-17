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

    Scanner = new DirectoryScanner(WorkingDirectory, NeedStop);

    int filesNumber = 0;
    FileMap fileMap;
    Scanner->RecursiveScanReduceBySize(fileMap, filesNumber);

    emit SetupFilesNumber(filesNumber);

    Comparator = new FileComparator(fileMap, NeedStop);

    connect(Comparator, SIGNAL(SendDuplicates(const FileList&)), Parent, SLOT(AddDuplicatesList(const FileList&)));
    connect(Comparator, SIGNAL(ProcessedFiles(int)), Parent, SLOT(UpdateProgressBar(int)));

    Comparator->Process();
}

void Worker::Stop() {
    qDebug() << "Scanning has stopped";
    NeedStop->store(true);
}
