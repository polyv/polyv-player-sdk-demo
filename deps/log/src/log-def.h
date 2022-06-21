#pragma once

const int kLogSize = 4096;
const int kLogMinMemory = 2 * 1024 * 1024;
const int kLogDefaultMemory = -1;///> unlimited

#include <thread>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void do_log_with_param(int lvl,
                       const char *msg,
                       const std::thread::id &id,
                       va_list args /* = NULL*/,
                       struct tm *time /* = NULL*/,
                       const char *function /* = NULL*/,
                       void *param /* = NULL*/);
