#pragma once
#include <Print.h>
#include <StreamString.h>

#include <RTClib.h>


struct DatePart { DateTime dt; };
struct TimePart { DateTime dt; };

using StreamFormatter = void(Print&);


inline Print& operator<< (Print& out, StreamFormatter f)
{
  f(out);
  return out;
}

inline void endl(Print& out) {
  out.println();
  out.flush();
}

inline char digit(int number)
{
  return '0' + (number % 10);
}

template <class T>
inline Print& operator<< (Print& out, T value)
{
  out.print(value);
  return out;
}

inline Print& operator<< (Print& out, DatePart date)
{
  return out
    << date.dt.year() / 1000
    << digit(date.dt.year() / 100)
    << digit(date.dt.year() / 10)
    << digit(date.dt.year())
    << '-'
    << digit(date.dt.month() / 10)
    << digit(date.dt.month())
    << '-'
    << digit(date.dt.day() / 10)
    << digit(date.dt.day());
}

inline Print& operator<< (Print& out, TimePart time)
{
  return out
    << digit(time.dt.hour() / 10)
    << digit(time.dt.hour())
    << ':'
    << digit(time.dt.minute() / 10)
    << digit(time.dt.minute())
    << ':'
    << digit(time.dt.second() / 10)
    << digit(time.dt.second());
}

inline Print& operator<< (Print& out, DateTime datetime)
{
  return out
      << DatePart{datetime} << ' '
      << TimePart{datetime};
}

template <class T>
String to_str(const T& obj)
{
  StreamString result;
  result << obj;
  return result;
}

template <class T>
String hex(const T& value)
{
  StreamString result;
  result.print(value, HEX);
  return result;
}

inline const char* checkSuccess(bool success)
{
  return success ? "SUCCESS!" : "FAILED!";
}
