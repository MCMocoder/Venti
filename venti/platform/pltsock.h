/**
 * @file pltsock.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#if defined(_WIN32)

#include "venti/platform/win/winsock.h"

#elif defined(__linux__) 

#include "venti/platform/linux/linuxsock.h"

#endif

namespace mocoder::venti {

#if defined(_WIN32)

typedef WinSock PltSock;

#elif defined(__linux__)

typedef LinuxSock PltSock;

#endif

}  // namespace mocoder::venti
