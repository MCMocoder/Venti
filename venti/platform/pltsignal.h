/**
 * @file pltsignal.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#if defined(_WIN32)

#include "venti/platform/win/winsignal.h"

#elif defined(__linux__) 

#include "venti/platform/linux/linuxsignal.h"

#endif

namespace mocoder::venti {

#if defined(_WIN32)

typedef WinSigHandler PltSigHandler;

#elif defined(__linux__)

typedef LinuxSignalHandler PltSigHandler;

#endif

}
