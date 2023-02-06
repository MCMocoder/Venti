/**
 * @file timer.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <chrono>
#include <coroutine>

#include "venti/context.h"
#include "venti/platform/plttimer.h"
#include "venti/socket/socket.h"

namespace mocoder::venti {

using namespace std;

class SteadyTimer {
 public:
  typedef function<void()> Callback;

  Context* ctx_;

  PltSteadyTimer plt_;

  SteadyTimer(Context& ctx) : ctx_(&ctx), plt_(&ctx) {}

  SteadyTimer(Context& ctx, chrono::nanoseconds ns) : ctx_(&ctx), plt_(&ctx) {
    plt_.SetExpire(ns);
  }

  void SetExpire(chrono::nanoseconds ns) { plt_.SetExpire(ns); }

  void AsyncWaitcb(function<void(int)> cb) { plt_.Wait(cb); }

  struct SteadyTimerAwait {
    SteadyTimerAwait(SteadyTimer* timeri) : timer(timeri) {}

    bool await_ready() { return false; }

    int await_resume() { return retval; }

    void await_suspend(coroutine_handle<> handlei) {
      handle = handlei;
      timer->AsyncWaitcb([this](int ec) { this->Resume(ec); });
    }

    void Resume(int ec) {
      retval = ec;
      handle.resume();
    }

    SteadyTimer* timer;
    int retval;
    coroutine_handle<> handle;
  };

  SteadyTimerAwait AsyncWait() {
    return SteadyTimerAwait(this);
  }
};

}  // namespace mocoder::venti
