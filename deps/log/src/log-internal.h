#pragma once
#include <thread>

#include "log/log.h"

extern void do_log_with_param(int lvl, const char *msg, const std::thread::id& id, va_list args = NULL,
    struct tm* time = NULL, const char* function = NULL, void* param = NULL);
