/**
 * @file linuxtimer.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <sys/timerfd.h>

#include <chrono>
#include <ctime>
#include <future>
#include <thread>

#include "venti/context.h"
#include "venti/platform/linux/linuxsock.h"

namespace mocoder::venti {

using namespace std;
using namespace std::chrono;

class LinuxSteadyTimer {
 public:
  Context* ctx_;

  LinuxSock sock_;

  chrono::nanoseconds nsec_;

  function<void(int)> cb_;

  LinuxSteadyTimer(Context* ctx) : ctx_(ctx) {
    sock_.Timer();
    ctx_->handler_.RegSock(sock_);
  }

  void SetExpire(chrono::nanoseconds ns) { nsec_ = ns; }

  void Wait(function<void(int)> cb) {
    sock_.SetTime(nsec_);
    ctx_->handler_.SubmitTimer(sock_, Event([cb](int len, int ec) {
                                 cb(ec);
                               }));
  }
};

}
