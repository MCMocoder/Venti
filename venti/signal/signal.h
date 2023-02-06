/**
 * @file signal.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <coroutine>
#include <initializer_list>
#include <set>

#include "venti/platform/pltsignal.h"
#include "venti/platform/pltsignalimpl.h"


namespace mocoder::venti {

using namespace std;

class SignalHandler {
 public:
  PltSigHandler handler_;

  SignalHandler(Context& ctx) : handler_(&ctx) {}

  void AddSignal(initializer_list<int> signals) { handler_.AddSig(signals); }

  void AsyncWaitcb(function<void(int)> cb) { handler_.Wait(cb); }
};

struct SignalAwait {
  SignalAwait(SignalHandler& handleri) : handler(handleri) {}

  bool await_ready() { return false; }

  void await_suspend(coroutine_handle<> handlei) {
    handle = handlei;
    handler.AsyncWaitcb([this](int signal) { this->Resume(signal); });
  }

  int await_resume() { return retsig; }

  void Resume(int signal) {
    retsig = signal;
    handle.resume();
  }

  SignalHandler& handler;
  int retsig;
  coroutine_handle<> handle;
};

SignalAwait AsyncWait(SignalHandler& sigh) { return SignalAwait(sigh); }

}  // namespace mocoder::venti
