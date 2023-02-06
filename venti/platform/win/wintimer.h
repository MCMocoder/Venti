/**
 * @file wintimer.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <chrono>
#include <future>
#include <thread>

#include "venti/context.h"

namespace mocoder::venti {

using namespace std;

class WinSteadyTimer {
 public:
  Context* ctx_;

  chrono::nanoseconds nsec_;

  function<void(int)> cb_;

  WinSteadyTimer(Context* ctx) : ctx_(ctx) {}

  void SetExpire(chrono::nanoseconds ns) { nsec_ = ns; }

  void Wait(function<void(int)> cb) {
    cb_ = cb;
    auto t = async([this]() {
      this_thread::sleep_for(this->nsec_);
      this->ctx_->SubmitComplete(Event([this](int, int) { this->cb_(0); }));
    });
  }
};

}  // namespace mocoder::venti
