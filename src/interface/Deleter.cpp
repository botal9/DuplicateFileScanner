//
// Created by vitalya on 12.02.19.
//

#include "Deleter.h"

#include "TreeWidgetItem.h"

#include <QDir>

Deleter::Deleter(QVector<QTreeWidgetItem*>&& itemsToDelete, const QString& filePrefix)
    : ItemsToDelete(std::move(itemsToDelete))
    , FilePrefix(filePrefix + QDir::separator())
{
}

void Deleter::DeleteItems() {
    QMap<QString, QTreeWidgetItem*> deletedFilesMap;
    FileList skippedFiles;
    FileList deletedFiles;

    for (const auto& item : ItemsToDelete) {
        if (NeedStop) {
            break;
        }
        QFile file(FilePrefix + item->text(0));
        if (file.remove()) {
            deletedFilesMap.insert(file.fileName(), item);
            deletedFiles.push_back(file.fileName());
        } else {
            skippedFiles.push_back(file.fileName());
        }
    }


    for (const auto& item : deletedFilesMap) {
        auto* parent = item->parent();
        parent->removeChild(item);
        dynamic_cast<TreeWidgetItem*>(parent)->SetNumber(0, parent->childCount());
        if (parent->childCount() == 0) {
            delete parent;
        }
    }

    emit Finished(deletedFiles, skippedFiles);
    emit Finished();
}

void Deleter::Stop() {
    NeedStop = true;
}
