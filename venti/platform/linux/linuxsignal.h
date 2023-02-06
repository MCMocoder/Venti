/**
 * @file linuxsignal.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-01-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <bits/types/sigset_t.h>
#include <sys/signalfd.h>

#include <csignal>
#include <functional>
#include <initializer_list>

#include "venti/context.h"
#include "venti/platform/linux/linuxsock.h"

namespace mocoder::venti {

using namespace std;

class LinuxSignalHandler {
 public:
  sigset_t mask_;

  LinuxSock sock_;

  Context* ctx_;

  LinuxSignalHandler(Context* ctx) : ctx_(ctx) {}

  void AddSig(initializer_list<int> sigs) {
    for (auto i : sigs) {
      sigaddset(&mask_, i);
    }
  }

  void Wait(function<void(int)> cb) {
    sigprocmask(SIG_SETMASK, &mask_, NULL);
    sock_.sock_ = signalfd(-1, &mask_, O_NONBLOCK);
    ctx_->handler_.RegSock(sock_);
    ctx_->handler_.SubmitSignal(sock_,
                                Event([cb](int len, int ec) { cb(len); }));
  }
};

}  // namespace mocoder::venti
