//
// Created by vitalya on 11.12.18.
//

#ifndef DIRECTORYSCANNER_QUEUE_H
#define DIRECTORYSCANNER_QUEUE_H

#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVector>

static inline constexpr std::size_t QUEUE_SIZE = 64 * 1024 - 2;

using FileList = QVector<QString>;

using FileMap = QHash<uint64_t, FileList>;

using SpscQueue = boost::lockfree::spsc_queue<FileList, boost::lockfree::capacity<QUEUE_SIZE>>;

#endif //DIRECTORYSCANNER_QUEUE_H
