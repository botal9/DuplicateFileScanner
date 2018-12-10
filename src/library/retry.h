//
// Created by vitalya on 08.12.18.
//

#ifndef DIRDEMO_RETRY_H
#define DIRDEMO_RETRY_H

#include <thread>

struct TimeOptions {
    int Count = 0;
    int Hours = 0;
    int Minutes = 0;
    int Seconds = 0;
    int Milliseconds = 0;
    int Microseconds = 0;

    TimeOptions(int count, int hours, int minutes, int seconds, int milliseconds, int microseconds)
        : Count(count)
        , Hours(hours)
        , Minutes(minutes)
        , Seconds(seconds)
        , Milliseconds(milliseconds)
        , Microseconds(microseconds)
    {}
};

template <typename Function, typename... Args>
bool DoWithRetry(TimeOptions options, const Function& function, Args&&... args) {
    bool ok = false;
    for (int i = 0; i < options.Count; ++i) {
        try {
            function(std::forward<Args>(args)...);
            ok = true;
            break;
        } catch (...) {
            auto sleepDuration = std::chrono::hours(options.Hours) +
                    std::chrono::minutes(options.Minutes) +
                    std::chrono::seconds(options.Seconds) +
                    std::chrono::milliseconds(options.Milliseconds) +
                    std::chrono::microseconds(options.Microseconds);

            std::this_thread::sleep_for(sleepDuration);
        }
    }
    return ok;
};


#endif //DIRDEMO_RETRY_H
