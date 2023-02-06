/**
 * @file plterr.h
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

#include "venti/platform/win/winerr.h"

#elif defined(__linux__)

#include "venti/platform/linux/linuxerror.h"

#endif
