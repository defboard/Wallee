#include "System.hpp"


StreamString eventLog;


DateTime getSystemTime()
{
    uint32_t unixtime = time(NULL);
    return DateTime(unixtime);
}

DateTime getBootTime()
{
    uint32_t unixtime = time(NULL);
    uint64_t uptime = esp_timer_get_time() / 1'000'000;
    return DateTime(unixtime - uptime);
}

bool setSystemTime(DateTime newTime)
{
    if (not newTime.isValid()) {
        return false;
    }

    timeval tv;
    tv.tv_sec = newTime.unixtime();
    tv.tv_usec = 0;

    int result = settimeofday(&tv, NULL);

    return result == 0;
}
