/*
 *  cp_log_cmn.h - Common functions declarations for the CP log and dump
 *                 program.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2015-2-16 Zhang Ziyi
 *  Initial version.
 */
#ifndef _CP_LOG_CMN_H_
#define _CP_LOG_CMN_H_

#include <cstddef>
#include <cstdint>
#include <cstring>

#ifdef USE_STD_CPP_LIB_
#include <list>
#include <vector>
#include <string>

#define LogList std::list
#define LogVector std::vector
#define LogString std::string

inline const char* ls2cstring(const LogString& str) { return str.c_str(); }

inline void str_assign(LogString& str, const char* s, size_t n) {
  str.assign(s, n);
}

inline bool str_empty(const LogString& s) { return s.empty(); }

template <typename C>
typename C::value_type& item(C& c, int index) {
  return c[index];
}

template <typename C>
void remove_at(C& c, size_t index) {
  typename C::iterator it = c.begin();
  size_t i = 0;

  while (i < index) {
    ++it;
    ++i;
  }
  c.erase(it);
}

template <typename C, typename V>
void ll_remove(C& c, const V& v) {
  c.remove(v);
}

template <typename T>
T rm_last(LogList<T>& c) {
  T t = c.back();
  c.pop_back();
  return t;
}

#else  // !USE_STD_CPP_LIB_

#include <utils/List.h>
#include <utils/Vector.h>
#include <utils/String8.h>

#define LogList android::List
#define LogVector android::Vector
#define LogString android::String8

inline const char* ls2cstring(const LogString& str) { return str.string(); }

inline void str_assign(LogString& str, const char* s, size_t n) {
  str.setTo(s, n);
}

inline bool str_empty(const LogString& s) { return s.isEmpty(); }

template <typename C>
typename C::value_type& item(C& c, int index) {
  return c.editItemAt(index);
}

template <typename C>
void remove_at(C& c, size_t index) {
  c.removeAt(index);
}

template <typename C, typename V>
void ll_remove(C& c, const V& v) {
  typename C::iterator it;
  for (it = c.begin(); it != c.end(); ++it) {
    if (*it == v) {
      c.erase(it);
      break;
    }
  }
}

template <typename T>
T rm_last(LogList<T>& c) {
  typename LogList<T>::iterator it1;
  typename LogList<T>::iterator it2;
  T e;

  for (it1 = c.begin(); it1 != c.end(); ++it1) {
    it2 = it1;
  }
  e = *it2;
  c.erase(it2);
  return e;
}
#endif  // USE_STD_CPP_LIB_

inline bool str_starts_with(const LogString& s1, const char* s2, size_t len) {
  return s1.length() >= len && !memcmp(ls2cstring(s1), s2, len);
}

inline bool str_ends_with(const LogString& s1, const char* s2, size_t len) {
  return s1.length() >= len &&
         !memcmp(ls2cstring(s1) + s1.length() - len, s2, len);
}

template <typename C>
void clear_ptr_container(C& c) {
  for (auto it = c.begin(); it != c.end(); ++it) {
    delete *it;
  }
  c.clear();
}

#include <errno.h>

#ifdef HOST_TEST_
// GCC that does not support C++11
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
#define override
#define nullptr 0
#endif

#include <cstdio>
#define ALOGE printf
#define ALOGD printf
#else
#include <cutils/log.h>
#endif

#define LOG_LEVEL 3

#if LOG_LEVEL >= 1
#define err_log(fmt, arg...) \
  ALOGE("%s: " fmt " [%d(%s)]\n", __func__, ##arg, errno, strerror(errno))
#else
#define err_log(fmt, arg...)
#endif

#if (LOG_LEVEL >= 2)
#define warn_log(fmt, arg...) ALOGE("%s: " fmt "\n", __func__, ##arg)
#else
#define warn_log(fmt, arg...)
#endif

#if (LOG_LEVEL >= 3)
#define info_log(fmt, arg...) ALOGE("%s: " fmt "\n", __func__, ##arg)
#else
#define info_log(fmt, arg...)
#endif

enum CpType {
  CT_UNKNOWN = -1,
  CT_WCDMA,
  CT_TD,
  CT_3MODE,
  CT_4MODE,
  CT_5MODE,
  CT_WCN,
  CT_GNSS,
  CT_AGDSP,
  CT_PM_SH,
  CT_NUMBER,
  CT_WANMODEM
};

enum SystemRunMode {
  SRM_NORMAL,
  SRM_AUTOTEST,
  SRM_CALIBRATION
};

enum IqType { IT_UNKNOWN = -1, IT_GSM, IT_WCDMA, IT_ALL };

struct time_sync {
  uint32_t sys_cnt;
  uint32_t uptime;
}__attribute__((__packed__));

struct modem_timestamp {
  uint32_t magic_number; /* magic number for verify the structure */
  uint32_t tv_sec;       /* clock time, seconds since 1970.01.01 */
  uint32_t tv_usec;      /* clock time, microeseconds part */
  uint32_t sys_cnt;      /* modem's time */
}__attribute__((__packed__));

int copy_file(int src_fd, int dest_fd);
/*  get_timezone_diff - calculate the UTC time offset of the local time
 *  @tnow: the current time encoded in time_t
 *
 *  Return the time offset in second.
 */
int get_timezone_diff(time_t tnow);
int copy_file_seg(int src_fd, int dest_fd, size_t m);
int copy_file(const char* src, const char* dest);
int set_nonblock(int fd);
void data2HexString(uint8_t* out_buf, const uint8_t* data, size_t len);
int get_sys_run_mode(SystemRunMode& sr);

int operator - (const timeval& t1, const timeval& t2);

int connect_tcp_server(int protocol_family,
                       const char* address, unsigned short port);

#endif  // !_CP_LOG_CMN_H_
