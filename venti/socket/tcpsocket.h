/**
 * @file tcpsocket.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <cerrno>
#include <coroutine>
#include <functional>
#include <future>
#include <queue>
#include <utility>

#include "venti/buffer.h"
#include "venti/context.h"
#include "venti/ip/addr.h"
#include "venti/platform/plteventimpl.h"
#include "venti/socket/socket.h"

namespace mocoder::venti {

using namespace std;

class TCPSocket : public Socket {
 public:
  typedef function<void(size_t, int)> Callback;

  int err_ = 0;

  TCPSocket(Context& ctx) { ctx_ = &ctx; }

  TCPSocket(Context& ctx, IPAddr::Type type) {
    ctx_ = &ctx;
    if (type == IPAddr::Type::IPV6) {
      socket_.TCPSocket6();
    } else {
      socket_.TCPSocket4();
    }
    ctx_->handler_.RegSock(socket_);
  }

  void Setup() {
    ctx_->handler_.RegSock(socket_);
  };

  int SyncRead(Buffers&& bufs) { return socket_.Recv(bufs); }

  int SyncWrite(Buffers&& bufs) { return socket_.Send(bufs); }

  void SubmitRead(Buffers& bufs, Callback cb) {
    ctx_->handler_.SubmitRead(socket_, bufs, Event([cb](int len, int ec) {
                                if (ec != 0) {
                                  cb(-1, ec);
                                } else {
                                  cb(len, 0);
                                }
                              }));
  }

  void SubmitWrite(Buffers& bufs, Callback cb) {
    ctx_->handler_.SubmitWrite(socket_, bufs, Event([cb](int len, int ec) {
                                 if (ec != 0) {
                                   cb(-1, ec);
                                 } else {
                                   cb(len, 0);
                                 }
                               }));
  }

  void AsyncReadcb(Buffers&& bufs, Callback cb) { SubmitRead(bufs, cb); }

  void AsyncWritecb(Buffers&& bufs, Callback cb) { SubmitWrite(bufs, cb); }

  int SyncConnect(IPAddr&& addr) { return socket_.Connect(&addr); }

  void AsyncConnectcb(IPAddr&& addr, Callback cb) {
    ctx_->handler_.SubmitConnect(socket_, addr,
                                 Event([this, cb, addr](int len, int ec) {
                                   if (ec != 0) {
                                     cb(-1, ec);
                                   } else {
                                     cb(0, 0);
                                   }
                                 }));
  }

  int Shutdown(PltSock::Shut shut) { return socket_.Shutdown(shut); }

  int Close() { return ctx_->handler_.CloseSock(socket_); }

  IPAddr GetPeerAddr() { return socket_.GetPeer(); }

  int GetErr() { return err_; }

  struct ReadAwait {
    ReadAwait(TCPSocket* socketi, Buffers& bufsi)
        : socket(socketi), bufs(bufsi) {}

    bool await_ready() { return false; }

    void await_suspend(coroutine_handle<> handlei) {
      handle = handlei;
      socket->AsyncReadcb(move(bufs),
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

    TCPSocket* socket;
    Buffers bufs;
    int retlen;
    coroutine_handle<> handle;
  };

  ReadAwait Read(Buffers&& bufs) { return ReadAwait(this, bufs); }

  struct WriteAwait {
    WriteAwait(TCPSocket* socketi, Buffers& bufsi)
        : socket(socketi), bufs(bufsi) {}

    bool await_ready() { return false; }

    void await_suspend(coroutine_handle<> handlei) {
      handle = handlei;
      socket->AsyncWritecb(move(bufs),
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

    TCPSocket* socket;
    Buffers bufs;
    int retlen;
    coroutine_handle<> handle;
  };

  WriteAwait Write(Buffers&& bufs) { return WriteAwait(this, bufs); }

  struct ConnAwait {
    ConnAwait(TCPSocket* socketi, IPAddr& addri)
        : socket(socketi), addr(addri) {}

    bool await_ready() { return false; }

    void await_suspend(coroutine_handle<> handlei) {
      handle = handlei;
      socket->AsyncConnectcb(
          move(addr), [this](int len, int ec) { this->Resume(len, ec); });
    }

    int await_resume() { return reterr; }

    void Resume(int len, int ec) {
      reterr = ec;
      handle.resume();
    }

    TCPSocket* socket;
    IPAddr addr;
    int reterr;
    coroutine_handle<> handle;
  };

  ConnAwait Connect(IPAddr &&addr) { return ConnAwait(this, addr); }
};

}  // namespace mocoder::venti
