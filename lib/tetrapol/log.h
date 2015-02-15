#pragma once

#include <stdio.h>

/**
  This file provides trivial logging facility.

  Configuration is done trought defining some macros before log.h is included.

  #define LOG_PREFIX "some_prefix"  // prefix used for logging (optional)
  #define LOG_LVL DBG               // override log level for this file

  */

#define WTF 0
#define ERR 20
#define INFO 40
#define DBG 60

extern int log_global_lvl;

// define LOG_LVL to override log level for single file
#ifndef LOG_LVL
#define LOG_LOCAL_LVL(lvl) false
#else
#define LOG_LOCAL_LVL(lvl) (lvl <= LOG_LVL)
#endif

#ifndef LOG_PREFIX
#define LOG_PREFIX "_"
#endif

#define LOG_STR_(s) #s

#define LOG__(line, msg, ...) \
    printf(LOG_PREFIX ":" LOG_STR_(line) " " msg , ##__VA_ARGS__)

#define LOG_(msg, ...) \
    LOG__(__LINE__, msg , ##__VA_ARGS__)

#define IF_LOG(lvl) \
    if (LOG_LOCAL_LVL(lvl) || lvl <= log_global_lvl)

#define LOG(lvl, msg, ...) \
    do { \
    if (LOG_LOCAL_LVL(lvl) || lvl <= log_global_lvl) { \
            LOG_(msg "\n", ##__VA_ARGS__); \
        } \
    } while(false)

inline void log_set_lvl(int lvl)
{
    log_global_lvl = lvl;
}
