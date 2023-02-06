/**
 * @file winiocp.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-01-16
 *
 * @copyright Copyright (c) 2023
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
#include <winsock.h>

#include <cerrno>
#include <cstdlib>
#include <list>
#include <map>
#include <set>

#include "venti/ctxdef.h"
#include "venti/event.h"
#include "venti/platform/pltsock.h"
#include "venti/platform/win/winsock.h"

namespace mocoder::venti {

class WinIocp {
 public:
  enum Type { READ, WRITE, RECVFROM, SENDTO, ACCEPT, CONNECT };
  struct SEOL {
    OVERLAPPED ol;
    Type type;
    WinSock* sock;
    Event ev;
    void* data;
    void* extra;
    IPAddr::Type ipa;
  };

  void InitSEOL(SEOL* s, Type typei, WinSock& sk, Event& evi) {
    s->type = typei;
    s->sock = &sk;
    s->ev = evi;
    s->data = nullptr;
  }

  void InitSEOL(SEOL* s, Type typei, WinSock& sk, Event& evi,
                IPAddr::Type ipai) {
    InitSEOL(s, typei, sk, evi);
    s->ipa = ipai;
    s->data = nullptr;
    if (typei == CONNECT) {
      if (s->ipa == IPAddr::IPV6) {
        s->data = new sockaddr_in6;
      } else {
        s->data = new sockaddr_in;
      }
    }
  }

  void InitSEOL(SEOL* s, Type typei, WinSock& sk, Event& evi, IPAddr::Type ipai,
                void* extrai) {
    s->type = typei;
    s->sock = &sk;
    s->ev = evi;
    s->ipa = ipai;
    s->extra = extrai;
    s->data = nullptr;
    if (typei == RECVFROM) {
      if (s->ipa == IPAddr::IPV6) {
        s->data = new sockaddr_in6;
      } else {
        s->data = new sockaddr_in;
      }
    }
    if (typei == ACCEPT) {
      if (s->ipa == IPAddr::IPV6) {
        s->data = new char[(sizeof(sockaddr_in6) + 16) * 2];
      } else {
        s->data = new char[(sizeof(sockaddr_in) + 16) * 2];
      }
    }
  }

  void DelSEOL(SEOL* s) {
    if (s->type == RECVFROM) {
      if (s->ipa == IPAddr::IPV6) {
        delete ((sockaddr_in6*)s->data);
      } else {
        delete ((sockaddr_in*)s->data);
      }
    } else if (s->type == ACCEPT) {
      delete[]((char*)s->data);
    } else if (s->type == CONNECT) {
      if (s->ipa == IPAddr::IPV6) {
        delete ((sockaddr_in6*)s->data);
      } else {
        delete ((sockaddr_in*)s->data);
      }
    }
  }

  HANDLE iocp_;

  Context* ctx_;

  WinIocp(Context* ctx) : ctx_(ctx) {
    iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
  }

  void RegSock(WinSock& sock) {
    CreateIoCompletionPort((HANDLE)sock.sock_, iocp_, 0, 0);
  }

  void SubmitRead(WinSock& sock, Buffers& buf, Event&& ev);

  void SubmitWrite(WinSock& sock, Buffers& buf, Event&& ev);

  void SubmitConnect(WinSock& sock, IPAddr& addr, Event&& ev);

  void SubmitAccept(WinSock& sock, WinSock& res, Event&& ev);

  void SubmitRecvfrom(WinSock& sock, IPAddr* peer, Buffers& buf, Event&& ev);

  void SubmitSendto(WinSock& sock, IPAddr* peer, Buffers& buf, Event&& ev);

  void Wait();

  int CloseSock(WinSock& sock) {
    sock.DisconnectEx(sock.sock_, NULL, 0, 0);
    return sock.Close();
  }
};

}  // namespace mocoder::venti
