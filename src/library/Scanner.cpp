//
// Created by vitalya on 10.12.18.
//

#include "Scanner.h"


Worker::Worker(const QString &directory)
    : WorkingDirectory(directory)
    , NeedStop(false)
{
}

void Worker::Stop() {
    NeedStop = true;
}

void Worker::Scan() {
    //TODO
}
