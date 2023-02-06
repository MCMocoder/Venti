/**
 * @file pltlock.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <mutex>

#ifdef WIN32

std::mutex pltmutex;

#define PLT_LOCK std::lock_guard<std::mutex> lk(pltmutex)

#else

#define PLT_LOCK ;

#endif
