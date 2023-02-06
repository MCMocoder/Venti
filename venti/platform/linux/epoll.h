/**
 * @file epoll.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-01-24
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <sys/epoll.h>

#include <map>

#include "venti/ctxdef.h"
#include "venti/event.h"
#include "venti/platform/linux/linuxsock.h"

namespace mocoder::venti {

using namespace std;

class Epoll {
 public:
  int epoll_;
  Context* ctx_;

  Epoll(Context* ctx) : ctx_(ctx) { epoll_ = epoll_create(1); }

  void RegSock(LinuxSock& sock) {
    epoll_event ev = {.events = 0, .data = {.u64 = 0}};
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.ptr = sock.ev_.get();
    epoll_ctl(epoll_, EPOLL_CTL_ADD, sock.sock_, &ev);
  }

  void SubmitRead(LinuxSock& sock, Buffers& buf, Event&& ev);

  void SubmitWrite(LinuxSock& sock, Buffers& buf, Event&& ev);

  void SubmitConnect(LinuxSock& sock, IPAddr& addr, Event&& ev);

  void SubmitAccept(LinuxSock& sock, LinuxSock& res, Event&& ev);

  void SubmitRecvfrom(LinuxSock& sock, IPAddr* peer, Buffers& buf, Event&& ev);

  void SubmitSendto(LinuxSock& sock, IPAddr* peer, Buffers& buf, Event&& ev);

  void SubmitTimer(LinuxSock& sock, Event&& ev);

  void SubmitSignal(LinuxSock& sock,Event&& ev);

  int TryRead(LinuxSock& sock);

  int TryWrite(LinuxSock& sock);

  int TryRecvfrom(LinuxSock& sock);

  int TrySendto(LinuxSock& sock);

  int TryAccept(LinuxSock& sock);

  void Wait();

  int CloseSock(LinuxSock& sock) { return sock.Close(); }
};

}  // namespace mocoder::venti
