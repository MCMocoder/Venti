/**
 * @file winerr.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <errno.h>
#include <winerror.h>

#undef EINTR
#undef EBADF
#undef EACCES
#undef EFAULT
#undef EINVAL
#undef EMFILE
#undef EWOULDBLOCK 
#undef EINPROGRESS 
#undef EALREADY
#undef ENOTSOCK
#undef EDESTADDRREQ
#undef EMSGSIZE
#undef EPROTOTYPE
#undef ENOPROTOOPT 
#undef EOPNOTSUPP
#undef EAFNOSUPPORT
#undef EADDRINUSE
#undef ENETDOWN
#undef ENETUNREACH 
#undef ENETRESET
#undef ECONNABORTED
#undef ECONNRESET
#undef ENOBUFS
#undef EISCONN
#undef ENOTCONN
#undef ETIMEDOUT
#undef ECONNREFUSED
#undef ELOOP
#undef ENAMETOOLONG
#undef EHOSTUNREACH
#undef ENOTEMPTY

#define EINTR WSAEINTR
#define EBADF WSAEBADF
#define EACCES WSAEACCES
#define EFAULT WSAEFAULT
#define EINVAL WSAEINVAL
#define EMFILE WSAEMFILE
#define EWOULDBLOCK  WSAEWOULDBLOCK 
#define EINPROGRESS  WSAEINPROGRESS 
#define EALREADY WSAEALREADY
#define ENOTSOCK WSAENOTSOCK
#define EDESTADDRREQ WSAEDESTADDRREQ
#define EMSGSIZE WSAEMSGSIZE
#define EPROTOTYPE WSAEPROTOTYPE
#define ENOPROTOOPT  WSAENOPROTOOPT 
#define EOPNOTSUPP WSAEOPNOTSUPP
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#define EADDRINUSE WSAEADDRINUSE
#define ENETDOWN WSAENETDOWN
#define ENETUNREACH  WSAENETUNREACH 
#define ENETRESET WSAENETRESET
#define ECONNABORTED WSAECONNABORTED
#define ECONNRESET WSAECONNRESET
#define ENOBUFS WSAENOBUFS
#define EISCONN WSAEISCONN
#define ENOTCONN WSAENOTCONN
#define ETIMEDOUT WSAETIMEDOUT
#define ECONNREFUSED WSAECONNREFUSED
#define ELOOP WSAELOOP
#define ENAMETOOLONG WSAENAMETOOLONG
#define EHOSTUNREACH WSAEHOSTUNREACH
#define ENOTEMPTY WSAENOTEMPTY
