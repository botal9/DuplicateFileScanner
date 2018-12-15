//
// Created by vitalya on 11.12.18.
//

#include "Consumer.h"

#include <thread>

Consumer::Consumer(SpscQueue &queue, std::atomic_bool &needStop)
    : QRunnable()
    , Queue(queue)
    , NeedStop(needStop)
{
}

static constexpr int WAIT_TIME = 20; // milliseconds

void Consumer::run() {
    while (!NeedStop) {
        Queue.consume_all(ToUI);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    }
}

void Consumer::ToUI() {
    // TODO
}
