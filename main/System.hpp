#pragma once

#include <Preferences.h>
#include <RTClib.h>
#include <StreamString.h>


extern StreamString eventLog;


constexpr char PREFS_NAMESPACE[] = "Wallee";
static_assert(sizeof(PREFS_NAMESPACE) <= 16);


extern DateTime getUptime();
extern DateTime getSystemTime();

extern bool setSystemTime(DateTime systemTime);
