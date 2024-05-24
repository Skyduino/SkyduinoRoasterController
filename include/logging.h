#ifndef __SW_LOGGING_H
#define __SW_LOGGING_H

#ifdef __DEBUG__
#define DEBUG(...) Serial.print(__VA_ARGS__)
#define DEBUGLN(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG(...)
#define DEBUGLN(...)
#endif
#ifdef __WARN__
#define WARN(...) Serial.print(__VA_ARGS__)
#define WARNLN(...) Serial.println(__VA_ARGS__)
#else
#define WARN(...)
#define WARNLN(...)
#endif

#endif