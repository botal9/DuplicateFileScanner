//
// Created by vitalya on 11.12.18.
//

#ifndef DIRECTORYSCANNER_QUEUE_H
#define DIRECTORYSCANNER_QUEUE_H

#include <QHash>
#include <QMap>
#include <QString>
#include <QVector>
#include <QMetaType>
#include <QMetaClassInfo>
#include <QFile>

using FileList = QVector<QString>;

Q_DECLARE_METATYPE(FileList);
static inline int idFileList = qRegisterMetaType<FileList>("Filelist");

using FileMap = QHash<uint64_t, FileList>;

static inline int idUint64 = qRegisterMetaType<uint64_t>("uint64_t");

static inline int idQVectorInt = qRegisterMetaType<uint64_t>("QVector<int>");

#endif //DIRECTORYSCANNER_QUEUE_H
