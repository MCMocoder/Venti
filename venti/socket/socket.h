/**
 * @file socket.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <memory>

#include "venti/buffer.h"
#include "venti/context.h"
#include "venti/event.h"
#include "venti/platform/pltsock.h"

namespace mocoder::venti {

class Socket {
 public:
  PltSock socket_;
  Context* ctx_;

  bool operator==(const Socket& another) const {
    return socket_ == another.socket_;
  }

  int GetErr() { return socket_.GetSockErr(); }
};

}  // namespace mocoder::venti
