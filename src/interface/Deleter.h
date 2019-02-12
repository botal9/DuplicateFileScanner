//
// Created by vitalya on 12.02.19.
//

#ifndef DUPLICATEFILESCANNER_DELETER_H
#define DUPLICATEFILESCANNER_DELETER_H

#include "util.h"

#include <atomic>

#include <QObject>
#include <QTreeWidgetItem>


class Deleter : public QObject {
    Q_OBJECT

public:
    Deleter(QVector<QTreeWidgetItem*>&& itemsToDelete, const QString& filePrefix);

public slots:
    void DeleteItems();
    void Stop();

signals:
    void Finished(const FileList& deletedFiles, const FileList& skippedFiles);
    void Finished();

private:
    const QVector<QTreeWidgetItem*> ItemsToDelete;
    const QString FilePrefix;

    std::atomic_bool NeedStop = false;
};


#endif //DUPLICATEFILESCANNER_DELETER_H
