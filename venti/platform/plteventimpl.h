/**
 * @file plteventimpl.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-26
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#if defined(_WIN32)

#include "venti/platform/win/winiocpimpl.h"

#elif defined(__linux__)

#include "venti/platform/linux/epollimpl.h"

#endif