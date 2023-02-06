/**
 * @file winevent.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <MSWSock.h>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <handleapi.h>
#include <ioapiset.h>
#include <winerror.h>

#include <cerrno>
#include <cstdlib>
#include <list>
#include <map>

#include "venti/ctxdef.h"
#include "venti/event.h"
#include "venti/platform/pltsock.h"
#include "venti/platform/win/winsock.h"

namespace mocoder::venti {

using namespace std;

class WinSockSelect {
 public:
  struct SockEventData {
    WinSock* sock = nullptr;
    Buffers bufs;
    Event event;
    void* data;
    bool processing = false;
    enum Type { READ, WRITE, RECVFROM, SENDTO, ACCEPT, CONNECT };
    Type type;
    SockEventData() {}
    SockEventData(Type typei, WinSock& socki, Buffers bufsi, Event& eventi)
        : type(typei), sock(&socki), bufs(bufsi), event(eventi) {}
    SockEventData(Type typei, WinSock& socki, Buffers bufsi, Event& eventi,
                  void* datai)
        : type(typei), sock(&socki), bufs(bufsi), event(eventi), data(datai) {}
    SockEventData(Type typei, WinSock& socki, Event& eventi, void* datai)
        : type(typei), sock(&socki), event(eventi), data(datai) {}
    SockEventData(Type typei, WinSock& socki, Event& eventi)
        : type(typei), sock(&socki), event(eventi) {}
    SockEventData(const SockEventData& se)
        : type(se.type),
          sock(se.sock),
          bufs(se.bufs),
          event(se.event),
          processing(se.processing),
          data(se.data) {}
    void Set(const SockEventData& se) {
      type = se.type;
      sock = se.sock;
      bufs = se.bufs;
      event = se.event;
      data = se.data;
    }
  };

  struct SockEvent {
    SockEventData rdata, wdata;
    bool ractivated = false, wactivated = false;
    SockEvent() {}
    SockEvent(SockEventData& datai) { AddEvent(datai); }
    SockEvent(const SockEvent& se) {
      rdata = se.rdata;
      wdata = se.wdata;
    }
    void AddEvent(SockEventData& datai) {
      switch (datai.type) {
        case SockEventData::READ:
        case SockEventData::RECVFROM:
        case SockEventData::ACCEPT: {
          rdata.Set(datai);
          ractivated = true;
        } break;
        case SockEventData::WRITE:
        case SockEventData::SENDTO:
        case SockEventData::CONNECT: {
          wdata.Set(datai);
          wactivated = true;
        } break;
      }
    }
  };

  Context* ctx_;

  WinSockSelect(Context* ctx) : ctx_(ctx) {
    FD_ZERO(&rs_);
    FD_ZERO(&ws_);
  }

  map<SOCKET, SockEvent> socks_;

  fd_set rs_, ws_;

  void RegSock(WinSock& sock) { socks_[sock.GetSocket()] = SockEvent(); }

  void SubmitReadEvent(WinSock& sock, SockEventData&& ev) {
    if (socks_.find(sock.GetSocket()) == socks_.end()) {
      socks_[sock.GetSocket()] = SockEvent();
    }
    socks_[sock.GetSocket()].AddEvent(ev);
  }

  void SubmitWriteEvent(WinSock& sock, SockEventData&& ev) {
    if (socks_.find(sock.GetSocket()) == socks_.end()) {
      socks_[sock.GetSocket()] = SockEvent();
    }
    socks_[sock.GetSocket()].AddEvent(ev);
  }

  bool IsReadable(WinSock& sock) {
    if (socks_.find(sock.GetSocket()) == socks_.end()) {
      return false;
    }
    return socks_[sock.GetSocket()].rdata.processing;
  }

  bool IsWriteable(WinSock& sock) {
    if (socks_.find(sock.GetSocket()) == socks_.end()) {
      return false;
    }
    return socks_[sock.GetSocket()].wdata.processing;
  }

  void SubmitRead(WinSock& sock, Buffers& bufs, Event&& ev);

  void SubmitWrite(WinSock& sock, Buffers& bufs, Event&& ev);

  void SubmitConnect(WinSock& sock, IPAddr& addr, Event&& ev);

  void SubmitAccept(WinSock& sock, WinSock& res, Event&& ev);

  void SubmitRecvfrom(WinSock& sock, IPAddr* addr, Buffers& bufs, Event&& ev);

  void SubmitSendto(WinSock& sock, IPAddr* addr, Buffers& bufs, Event&& ev);

  int InitSet() {
    FD_ZERO(&rs_);
    FD_ZERO(&ws_);
    int maxfds = 0;
    for (auto& i : socks_) {
      SOCKET t = i.first;
      if (i.second.ractivated) {
        FD_SET(t, &rs_);
      }
      if (i.second.wactivated) {
        FD_SET(t, &ws_);
      }
      if (maxfds < t) {
        maxfds = t;
      }
    }
    return maxfds;
  }

  int TryRead(SockEvent& se);

  int TryWrite(SockEvent& se);

  int TryRecvfrom(SockEvent& se);

  int TrySendto(SockEvent& se);

  int TryAccept(SockEvent& se);

  int TryConnect(SockEvent& se);

  int Wait();

  int CloseSock(WinSock& sock) {
    if (socks_.find(sock.GetSocket()) != socks_.end()) {
      socks_.erase(sock.GetSocket());
    }
    return sock.Close();
  }
};

}  // namespace mocoder::venti
