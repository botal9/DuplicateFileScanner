//
// Created by vitalya on 10.12.18.
//

#ifndef DIRDEMO_WORKER_H
#define DIRDEMO_WORKER_H

#include "DirectoryScanner.h"
#include "FileComparator.h"

#include <QtCore/QObject>


class Worker : public QObject {

    Q_OBJECT

public:
    Worker();


private:
    SpscQueue Queue;
    QString WorkingDirectory;
};


#endif //DIRDEMO_WORKER_H
