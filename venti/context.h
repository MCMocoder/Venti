/**
 * @file context.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <list>
#include <memory>
#include <set>

#include "venti/event.h"
#include "venti/platform/pltevent.h"
#include "venti/platform/pltlock.h"

namespace mocoder::venti {

using namespace std;

class Context {
 public:
  list<Event> waitset_;

  PltEvent handler_;

  bool can_continue_ = true;

  Context() : handler_(this) {}

  void SubmitComplete(Event&& event) {
    PLT_LOCK;
    if (event.status_ == WAITING) {
      event.status_ = COMPLETED;
    }
    waitset_.push_back(move(event));
  }

  void End() { can_continue_ = false; }

  void Run() {
    while (can_continue_) {
      for (auto i = waitset_.begin(); i != waitset_.end();) {
        if (i->status_ == COMPLETED || i->status_ == ERR) {
          i->Complete();
          i = waitset_.erase(i);
        } else {
          ++i;
        }
      }
      handler_.Wait();
    }
  }
};

}  // namespace mocoder::venti
