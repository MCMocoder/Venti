/**
 * @file event.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <functional>
#include <queue>

namespace mocoder::venti {

using namespace std;

enum EvStat { WAITING, COMPLETED, ERR };

class Event {
 public:
  typedef function<void(int, int)> Callback;
  Callback cb_, errcb_;
  int status_ = WAITING;
  int error_ = 0;
  int res_ = 0;

  Event(Callback cb,Callback errcb) : cb_(cb),errcb_(errcb) {}

  Event(Callback cb) : cb_(cb), errcb_([](int,int) {}) {}

  Event() : cb_([](int, int) {}) {}

  Event(const Event& ev)
      : cb_(ev.cb_),
        errcb_(ev.errcb_),
        status_(ev.status_),
        error_(ev.error_),
        res_(ev.res_) {}

  void Complete() { cb_(res_, error_); }
};

}  // namespace mocoder::venti
