/**
 * @file winsignal.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <signal.h>

#include <functional>
#include <initializer_list>
#include <map>
#include <set>

#include "venti/ctxdef.h"
#include "venti/event.h"

namespace mocoder::venti {

using namespace std;

class WinSigManager {
 public:
  static WinSigManager& Get() {
    static WinSigManager m;
    return m;
  }

  void SetContext(Context* ctx) { ctx_ = ctx; }

  Context* ctx_;

  map<int, function<void(int)>> sigmap_;
};

void SigCallback(int signal);

class WinSigHandler {
 public:
  set<int> signals_;

  WinSigHandler(Context* ctx) {
    WinSigManager::Get().SetContext(ctx);
  }

  void AddSig(initializer_list<int> sigs) { signals_.insert(sigs); }

  void Wait(function<void(int)> fn);
};

}  // namespace mocoder::venti
