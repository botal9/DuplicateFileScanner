//
// Created by vitalya on 11.12.18.
//

#ifndef DIRECTORYSCANNER_CONSUMER_H
#define DIRECTORYSCANNER_CONSUMER_H

#include "util.h"

#include <atomic>

#include <QtCore/QRunnable>


class Consumer : public QRunnable {
public:
    Consumer() = delete;
    Consumer(SpscQueue& queue, std::atomic_bool& needStop);
    virtual ~Consumer() = default;

    void run() override;

    static void ToUI();

private:
    SpscQueue& Queue;
    std::atomic_bool& NeedStop;
};


#endif //DIRECTORYSCANNER_CONSUMER_H
