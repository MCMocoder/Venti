/**
 * @file rawsocket.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-28
 *
 * @copyright Copyright (c) 2022 Mocoder Studio
 *
 */

#pragma once

#include <coroutine>

#include "venti/context.h"
#include "venti/socket/socket.h"

namespace mocoder::venti {

class NetLayerSocket : public Socket {
 public:
  typedef function<void(size_t, int)> Callback;

  Context* ctx_;

  NetLayerSocket() {}

  NetLayerSocket(Context& ctx, int type):ctx_(&ctx) {
   if (type & IPAddr::Type::IPV4) {
      socket_.RawSocket4();
    } else {
      socket_.RawSocket6();
    }
  }
  
};

}
