/**
 * @file udpsocket.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <coroutine>

#include "venti/socket/socket.h"

namespace mocoder::venti {

using namespace std;

class UDPSocket : public Socket {
 public:
  typedef function<void(size_t, int)> Callback;

  UDPSocket(Context& ctx, IPAddr::Type type) {
    ctx_ = &ctx;
    if (type == IPAddr::Type::IPV6) {
      socket_.UDPSocket6();
    } else {
      socket_.UDPSocket4();
    }
    ctx_->handler_.RegSock(socket_);
  }

  int SyncRecvfrom(IPAddr&& peer, Buffers&& bufs) {
    return socket_.Recvfrom(bufs, &peer);
  }

  int SyncSendto(IPAddr&& peer, Buffers&& bufs) {
    return socket_.Sendto(bufs, &peer);
  }

  int Connect(IPAddr&& addr) { return socket_.Connect(&addr); }

  void AsyncRecvfromcb(Buffers&& bufs, IPAddr& addr, Callback cb) {
    ctx_->handler_.SubmitRecvfrom(socket_, &addr, bufs,
                                  Event([cb](int len, int ec) {
                                    if (ec != 0) {
                                      cb(-1, ec);
                                    } else {
                                      cb(len, 0);
                                    }
                                  }));
  }

  void AsyncConnectedRecvfromcb(Buffers&& bufs, Callback cb) {
    ctx_->handler_.SubmitRecvfrom(socket_, nullptr, bufs,
                                  Event([cb](int len, int ec) {
                                    if (ec != 0) {
                                      cb(-1, ec);
                                    } else {
                                      cb(len, 0);
                                    }
                                  }));
  }

  void AsyncSendtocb(Buffers&& bufs, IPAddr& addr, Callback cb) {
    ctx_->handler_.SubmitSendto(socket_, &addr, bufs,
                                Event([cb](int len, int ec) {
                                  if (ec != 0) {
                                    cb(-1, ec);
                                  } else {
                                    cb(len, 0);
                                  }
                                }));
  }

  void AsyncConnectedSendtocb(Buffers&& bufs, Callback cb) {
    ctx_->handler_.SubmitSendto(socket_, nullptr, bufs,
                                Event([cb](int len, int ec) {
                                  if (ec != 0) {
                                    cb(-1, ec);
                                  } else {
                                    cb(len, 0);
                                  }
                                }));
  }

  int Bind(IPAddr&& addr) { return socket_.Bind(addr); }

  int Close() { return ctx_->handler_.CloseSock(socket_); }

  struct RecvAwait {
    RecvAwait(UDPSocket* socketi, Buffers& bufsi, IPAddr& addri)
        : socket(socketi), bufs(bufsi), addr(addri) {}

    bool await_ready() { return false; }

    void await_suspend(coroutine_handle<> handlei) {
      handle = handlei;
      socket->AsyncRecvfromcb(
          move(bufs), addr, [this](int len, int ec) { this->Resume(len, ec); });
    }

    int await_resume() { return retlen; }

    void Resume(int len, int ec) {
      if (ec != 0) {
        retlen = -ec;
      } else {
        retlen = len;
      }
      handle.resume();
    }

    UDPSocket* socket;
    IPAddr& addr;
    Buffers bufs;
    int retlen;
    coroutine_handle<> handle;
  };

  RecvAwait Recvfrom(IPAddr& addr, Buffers&& bufs) {
    return RecvAwait(this, bufs, addr);
  }

  struct ConnectedRecvAwait {
    ConnectedRecvAwait(UDPSocket* socketi, Buffers& bufsi)
        : socket(socketi), bufs(bufsi) {}

    bool await_ready() { return false; }

    void await_suspend(coroutine_handle<> handlei) {
      handle = handlei;
      socket->AsyncConnectedRecvfromcb(
          move(bufs), [this](int len, int ec) { this->Resume(len, ec); });
    }

    int await_resume() { return retlen; }

    void Resume(int len, int ec) {
      if (ec != 0) {
        retlen = -ec;
      } else {
        retlen = len;
      }
      handle.resume();
    }

    UDPSocket* socket;
    Buffers bufs;
    int retlen;
    coroutine_handle<> handle;
  };

  ConnectedRecvAwait Recvfrom(Buffers&& bufs) {
    return ConnectedRecvAwait(this, bufs);
  }

  struct SendAwait {
    SendAwait(UDPSocket* socketi, Buffers& bufsi, IPAddr& addri)
        : socket(socketi), bufs(bufsi), addr(addri) {}

    bool await_ready() { return false; }

    void await_suspend(coroutine_handle<> handlei) {
      handle = handlei;
      socket->AsyncSendtocb(move(bufs), addr,
                            [this](int len, int ec) { this->Resume(len, ec); });
    }

    int await_resume() { return retlen; }

    void Resume(int len, int ec) {
      if (ec != 0) {
        retlen = -ec;
      } else {
        retlen = len;
      }
      handle.resume();
    }

    UDPSocket* socket;
    IPAddr addr;
    Buffers bufs;
    int retlen;
    coroutine_handle<> handle;
  };

  SendAwait Sendto(IPAddr&& addr, Buffers&& bufs) {
    return SendAwait(this, bufs, addr);
  }

  struct ConnectedSendAwait {
    ConnectedSendAwait(UDPSocket* socketi, Buffers& bufsi)
        : socket(socketi), bufs(bufsi) {}

    bool await_ready() { return false; }

    void await_suspend(coroutine_handle<> handlei) {
      handle = handlei;
      socket->AsyncConnectedSendtocb(
          move(bufs), [this](int len, int ec) { this->Resume(len, ec); });
    }

    int await_resume() { return retlen; }

    void Resume(int len, int ec) {
      if (ec != 0) {
        retlen = -ec;
      } else {
        retlen = len;
      }
      handle.resume();
    }

    UDPSocket* socket;
    Buffers bufs;
    int retlen;
    coroutine_handle<> handle;
  };

  ConnectedSendAwait Sendto(Buffers&& bufs) {
    return ConnectedSendAwait(this, bufs);
  }
};

}  // namespace mocoder::venti
