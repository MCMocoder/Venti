/**
 * @file winsignalimpl.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <csignal>

#include "venti/context.h"
#include "venti/platform/win/winsignal.h"

namespace mocoder::venti {

void SigCallback(int signal) { WinSigManager::Get().sigmap_[signal](signal); }

void WinSigHandler::Wait(function<void(int)> fn) {
  WinSigManager& m = WinSigManager::Get();
  for (auto i : signals_) {
    m.sigmap_[i] = fn;
  }
  for (auto i : m.sigmap_) {
    signal(i.first, SigCallback);
  }
}

}  // namespace mocoder::venti
