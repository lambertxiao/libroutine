#ifndef LIBROUTINE_LOGGER_H_
#define LIBROUTINE_LOGGER_H_

#include <stdio.h>
#include <time.h>
#include <cstring>

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 3

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_RESET "\x1b[0m"

static int log_level = LOG_LEVEL_DEBUG;

#define LOG_INTERNAL(level, fmt, ...)                                                                                                                                                            \
  do {                                                                                                                                                                                           \
    if (level >= log_level) {                                                                                                                                                                    \
      time_t t = time(NULL);                                                                                                                                                                     \
      struct tm* tm = localtime(&t);                                                                                                                                                             \
      const char* prefix = NULL;                                                                                                                                                                 \
      const char* color = NULL;                                                                                                                                                                  \
      switch (level) {                                                                                                                                                                           \
        case LOG_LEVEL_DEBUG:                                                                                                                                                                    \
          prefix = "DEBUG";                                                                                                                                                                      \
          color = COLOR_RESET;                                                                                                                                                                   \
          break;                                                                                                                                                                                 \
        case LOG_LEVEL_INFO:                                                                                                                                                                     \
          color = COLOR_GREEN;                                                                                                                                                                   \
          prefix = "INFO";                                                                                                                                                                      \
          break;                                                                                                                                                                                 \
        case LOG_LEVEL_WARN:                                                                                                                                                                     \
          prefix = "WARN";                                                                                                                                                                      \
          color = COLOR_YELLOW;                                                                                                                                                                  \
          break;                                                                                                                                                                                 \
        case LOG_LEVEL_ERROR:                                                                                                                                                                    \
          prefix = "ERROR";                                                                                                                                                                      \
          color = COLOR_RED;                                                                                                                                                                     \
          break;                                                                                                                                                                                 \
        default:                                                                                                                                                                                 \
          prefix = "UNKNW";                                                                                                                                                                      \
          color = COLOR_MAGENTA;                                                                                                                                                                 \
          break;                                                                                                                                                                                 \
      }                                                                                                                                                                                          \
      const char* fpath = std::strrchr(__FILE__, '/') + 1;                                                                                                                                       \
      printf("%s [%d/%02d/%02d %02d:%02d:%02d] [%s] [RT/%s:%d] " fmt "\n", color, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, prefix, fpath, __LINE__, \
             ##__VA_ARGS__);                                                                                                                                                                     \
    }                                                                                                                                                                                            \
  } while (0)

#define LOG_DEBUG(fmt, ...) LOG_INTERNAL(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_INTERNAL(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_INTERNAL(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_INTERNAL(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

#endif
