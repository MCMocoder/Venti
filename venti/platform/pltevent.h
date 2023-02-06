/**
 * @file pltevent.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#if defined(_WIN32)

#include "venti/platform/win/winiocp.h"

#elif defined(__linux__) 

#include "venti/platform/linux/epoll.h"

#endif

namespace mocoder::venti {

#if defined(_WIN32)

typedef WinIocp PltEvent;

#elif defined(__linux__)

typedef Epoll PltEvent;

#endif

}  // namespace mocoder::venti

