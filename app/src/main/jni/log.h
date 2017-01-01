#pragma once

#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "MCPEAddon"
#endif
#define LOGUNK(...) ((void) __android_log_print(ANDROID_LOG_UNKNOWN, LOG_TAG, __VA_ARGS__))
#define LOGDEF(...) ((void) __android_log_print(ANDROID_LOG_DEFAULT, LOG_TAG, __VA_ARGS__))
#define LOGV(...) ((void) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#define LOGD(...) ((void) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((void) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGF(...) ((void) __android_log_print(ANDROID_FATAL_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGS(...) ((void) __android_log_print(ANDROID_SILENT_ERROR, LOG_TAG, __VA_ARGS__))