/**
 * @file acceptor.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <functional>

#include "venti/platform/pltsock.h"
#include "venti/socket/tcpsocket.h"

namespace mocoder::venti {

using namespace std;

class Acceptor : public Socket {
 public:
  typedef function<void(PltSock, int)> Callback;

  struct SubmitWaiting {
    PltSock* sock;
    Callback cb;
    SubmitWaiting() {}
    SubmitWaiting(PltSock& socki, Callback cbi) : sock(&socki), cb(cbi) {}
  };

  SubmitWaiting waiting_;

  Context* ctx_;

  IPAddr local_;

  Acceptor(Context& ctx) { ctx_ = &ctx; }

  int Bind(IPAddr&& local) {
    local_ = local;
    if (local.type_ == IPAddr::Type::IPV6) {
      socket_.TCPSocket6();
    } else {
      socket_.TCPSocket4();
    }
    ctx_->handler_.RegSock(socket_);
    return socket_.Bind(local);
  }

  int Listen(int backlog) { return socket_.Listen(backlog); }

  int SyncAccept(TCPSocket& sock) {
    int err = socket_.Accept(&(sock.socket_));
    if (err == 0) {
      sock.Setup();
      return 0;
    } else {
      return err;
    }
  }

  void AcceptCallback(int ec) {
    if (ec != 0) {
      waiting_.cb(PltSock(), ec);
    } else {
      waiting_.cb(*waiting_.sock, ec);
    }
  }

  void AsyncAcceptcb(TCPSocket& skt, Callback cb) {
    waiting_ = SubmitWaiting(skt.socket_, cb);
    ctx_->handler_.SubmitAccept(
        socket_, *waiting_.sock,
        Event([this](int, int ec) { this->AcceptCallback(ec); }));
  }

  int Close() { return ctx_->handler_.CloseSock(socket_); }

  struct AcceptAwait {
    AcceptAwait(Acceptor* socketi, TCPSocket& skt)
        : socket(socketi), oskt(skt) {}

    bool await_ready() { return false; }

    void await_suspend(coroutine_handle<> handlei) {
      handle = handlei;
      socket->AsyncAcceptcb(
          oskt, [this](PltSock skt, int ec) { this->Resume(skt, ec); });
    }

    int await_resume() { return errc; }

    void Resume(PltSock& skt, int ec) {
      errc = ec;
      if (ec == 0) {
        oskt.Setup();
      }
      handle.resume();
    }

    Acceptor* socket;
    int errc = 0;
    TCPSocket& oskt;
    coroutine_handle<> handle;
  };

  AcceptAwait Accept(TCPSocket& skt) { return AcceptAwait(this, skt); }
};

}  // namespace mocoder::venti
