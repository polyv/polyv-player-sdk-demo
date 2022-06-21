#include "log/log.h"
#include "log-def.h"
#include "log-internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <ratio>
#include <mutex>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#pragma warning(disable:4996)
#else
#include <libgen.h>
#include <sys/stat.h>
#include <limits.h>
#endif
#ifdef USE_LOG_STREAM

namespace slogstream {

#define CPP_DELETE(obj)\
    if(obj){\
        delete obj;\
        obj = NULL;\
    }\

#define CPP_DELETE_ARRAY(obj)\
    if(obj){\
        delete[] obj;\
        obj = NULL;\
    }\

    struct LogStream::Impl {
    public:
        Impl(int level, const char* function) :
            level_(level),
            function_(function ? function : ""),
            isExtend_(false) {
            time_t now = time(0);
            time_ = *localtime(&now);
            threadId_ = std::this_thread::get_id();
            Reset();
        }
        virtual ~Impl(void) {
            if (begin_) {
                do_log_with_param(level_, begin_, threadId_, NULL, &time_, function_.c_str(), NULL);
            }
            if (isExtend_) {
                CPP_DELETE_ARRAY(begin_);
            }
        }
    public:
        void Reset(void) {
            if (isExtend_) {
                CPP_DELETE_ARRAY(begin_);
                isExtend_ = false;
            }
            size_ = kLogSize;
            memset(buffer_, 0, size_);
            begin_ = buffer_;
            end_ = begin_ + size_;
            current_ = begin_;
        }
        void Set(int level) {
            level_ = level;
        }
        int Level(void) const {
            return level_;
        }
        std::thread::id ThreadId(void) const {
            return threadId_;
        }
        const char* Log(void) const {
            return begin_;
        }
        bool IsExtend(void) const {
            return isExtend_;
        }
        const struct tm Time(void) const {
            return time_;
        }
        template <typename T>
        LogStream& Write(LogStream& serialize, const char* format, T value) {
            if (current_ < end_) {
                int len = 0;
                int count = (int)(end_ - current_);
                do
                {
                    auto current = current_;
                    auto size = count - 1;

                    len = snprintf(current, size, format, value);
                    if (ERANGE == errno) {// 如果超过范围，则申请
                        goto newMemory;
                    }
                    else if (len >= size) {
                        goto newMemory;
                    }
                    else if (len < 0) {
                        *current_ = '\0';
                        len = 0;
                    }
                    break;
                newMemory:
                    if (!NewMemory(len)) {
                        break;
                    }
                    count = (int)(end_ - current_);
                    continue;
                } while (true);

                current_ += len;
            }
            return serialize;
        }

        LogStream& Write(LogStream& serialize, long value) {
            return Write(serialize, (long long)value);
        }
        LogStream& Write(LogStream& serialize, unsigned long value) {
            return Write(serialize, (unsigned long long)value);
        }
        LogStream& Write(LogStream& serialize, long long value) {
#ifdef _WIN32
            return Write(serialize, "%I64d", value);
#else
            return Write(serialize, "%lld", value);
#endif
        }
        LogStream& Write(LogStream& serialize, unsigned long long value) {
#ifdef _WIN32
            return Write(serialize, "%I64u", value);
#else
            return Write(serialize, "%llu", value);
#endif
        }

        LogStream& Write(LogStream& serialize, const void* value) {
#ifdef _WIN32
            return sizeof(value) == 8 ? Write(serialize, "%016I64x", (unsigned long long)value) : Write(serialize, "%08I64x", (unsigned long long)value);
#else
            return sizeof(value) == 8 ? Write(serialize, "%016llx", (unsigned long long)value) : Write(serialize, "%08llx", (unsigned long long)value);
#endif
        }


    private:
        bool NewMemory(int count) {
            int size = size_ + kLogSize;
            if (count > kLogSize) {
                size = (count / kLogSize + 1) * kLogSize;
            }
            auto temp = new char[size];
            if (!temp) {
                return false;
            }
            auto pos = current_ - begin_;
            memset(temp, 0, size);
            auto begin = temp;
            if (pos > 0) {
                memcpy(begin, begin_, pos);
            }
            if (isExtend_) {// 如果是扩展，则要先delete
                CPP_DELETE_ARRAY(begin_);
            }
            isExtend_ = true;
            size_ = size;
            begin_ = begin;
            end_ = begin_ + size_;
            current_ = begin_ + pos;

            return true;
        }

    private:
        char buffer_[kLogSize] = { 0 };
        char *begin_ = NULL, *end_ = NULL, *current_ = NULL;
        int level_ = 0;

        int size_ = 0;
        struct tm time_;
        std::string function_;
        std::thread::id threadId_;
        bool isExtend_ = false;
    };


    LogStream::LogStream(int level, const char* function)
        : impl_(new Impl(level, function))
    {
    }

    LogStream::~LogStream(void)
    {
        delete impl_;
    }

    LogStream& LogStream::operator <<(const void* value) { return impl_->Write(*this, value); }
    LogStream& LogStream::operator <<(const char* value) { return impl_->Write(*this, "%s", value); }
    LogStream& LogStream::operator <<(bool value) { return (value ? impl_->Write(*this, "%s", "true") : impl_->Write(*this, "%s", "false")); }
    LogStream& LogStream::operator <<(char value) { return impl_->Write(*this, "%c", value); }
    LogStream& LogStream::operator <<(unsigned char value) { return impl_->Write(*this, "%u", (unsigned int)value); }
    LogStream& LogStream::operator <<(short value) { return impl_->Write(*this, "%d", (int)value); }
    LogStream& LogStream::operator <<(unsigned short value) { return impl_->Write(*this, "%u", (unsigned int)value); }
    LogStream& LogStream::operator <<(int value) { return impl_->Write(*this, "%d", value); }
    LogStream& LogStream::operator <<(unsigned int value) { return impl_->Write(*this, "%u", value); }
    LogStream& LogStream::operator <<(long value) { return impl_->Write(*this, value); }
    LogStream& LogStream::operator <<(unsigned long value) { return impl_->Write(*this, value); }
    LogStream& LogStream::operator <<(long long value) { return impl_->Write(*this, value); }
    LogStream& LogStream::operator <<(unsigned long long value) { return impl_->Write(*this, value); }
    LogStream& LogStream::operator <<(float value) { return impl_->Write(*this, "%.4f", value); }
    LogStream& LogStream::operator <<(double value) { return impl_->Write(*this, "%.4lf", value); }

}// end log
#endif// end USE_LOG_STREAM
