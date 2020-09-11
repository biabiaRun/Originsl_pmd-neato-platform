#ifndef COMMON_H_
#define COMMON_H_

#include <condition_variable>
#include <mutex>

extern std::mutex g_ptcloud_mutex;
extern std::condition_variable g_ptcloud_cv;
extern bool g_newDataAvailable;

#endif