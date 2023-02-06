/**
 * @file wineventimpl.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-26
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include "venti/context.h"
#include "venti/event.h"
#include "venti/platform/win/winevent.h"

namespace mocoder::venti {

void WinSockSelect::SubmitRead(WinSock& sock, Buffers& bufs, Event&& ev) {
  SubmitReadEvent(sock, SockEventData(SockEventData::READ, sock, bufs, ev));
  if (IsReadable(sock)) {
    auto& i = socks_.find(sock.GetSocket())->second;
    int len = TryRead(i);
    if (len < 0) {
      ev.error_ = -len;
      ev.status_ = ERR;
      i.ractivated = false;
      ctx_->SubmitComplete(move(ev));
    } else if (len != 0) {
      ev.res_ = len;
      i.ractivated = false;
      ctx_->SubmitComplete(move(ev));
    }
  }
}

int WinSockSelect::TryRead(SockEvent& se) {
  int len = se.rdata.sock->Recv(se.rdata.bufs);
  if (len < 0) {
    int err = se.rdata.sock->GetSockErr();
    if (err == WSAEWOULDBLOCK) {
      se.rdata.processing = false;
      return 0;
    } else {
      return -err;
    }
  }
  return len;
}

void WinSockSelect::SubmitWrite(WinSock& sock, Buffers& bufs, Event&& ev) {
  SubmitWriteEvent(sock, SockEventData(SockEventData::WRITE, sock, bufs, ev));
  if (IsWriteable(sock)) {
    auto& i = socks_.find(sock.GetSocket())->second;
    int len = TryWrite(i);
    if (len < 0) {
      ev.error_ = -len;
      ev.status_ = ERR;
      i.wactivated = false;
      ctx_->SubmitComplete(move(ev));
    } else if (len != 0) {
      ev.res_ = len;
      i.wactivated = false;
      ctx_->SubmitComplete(move(ev));
    }
  }
}

int WinSockSelect::TryWrite(SockEvent& se) {
  int len = se.wdata.sock->Send(se.wdata.bufs);
  if (len < 0) {
    int err = se.wdata.sock->GetSockErr();
    if (err == WSAEWOULDBLOCK) {
      se.wdata.processing = false;
      return 0;
    } else {
      return -err;
    }
  }
  return len;
}

void WinSockSelect::SubmitConnect(WinSock& sock, IPAddr& addr, Event&& ev) {
  int err = sock.Connect(&addr);
  if (err == 0) {
    ev.status_ = COMPLETED;
    ctx_->SubmitComplete(move(ev));
  } else if (err == WSAEWOULDBLOCK) {
    SubmitWriteEvent(
        sock, SockEventData(SockEventData::CONNECT, sock, ev, (void*)&addr));
  } else {
    ev.status_ = ERR;
    ev.error_ = err;
    ctx_->SubmitComplete(move(ev));
  }
}

int WinSockSelect::TryConnect(SockEvent& se) {
  return se.wdata.sock->Connect((IPAddr*)se.wdata.data);
}

void WinSockSelect::SubmitAccept(WinSock& sock, WinSock& res, Event&& ev) {
  SubmitReadEvent(sock,
                  SockEventData(SockEventData::ACCEPT, sock, ev, (void*)&res));
  if (IsReadable(sock)) {
    auto& i = socks_.find(sock.GetSocket())->second;
    int len = TryAccept(i);
    if (len > 0) {
      ev.error_ = -len;
      ev.status_ = ERR;
      i.wactivated = false;
      ctx_->SubmitComplete(move(ev));
    } else if (len != 0) {
      ev.res_ = len;
      i.wactivated = false;
      ctx_->SubmitComplete(move(ev));
    }
  }
}

int WinSockSelect::TryAccept(SockEvent& se) {
  int err = se.rdata.sock->Accept((WinSock*)se.rdata.data);
  if (err < 0) {
    int ec = se.rdata.sock->GetSockErr();
    if (ec == WSAEWOULDBLOCK) {
      se.rdata.processing = false;
      return -1;
    } else {
      return ec;
    }
  }
  return 0;
}

void WinSockSelect::SubmitRecvfrom(WinSock& sock, IPAddr* addr, Buffers& bufs,
                                   Event&& ev) {
  SubmitReadEvent(sock, SockEventData(SockEventData::RECVFROM, sock, bufs, ev,
                                      (void*)addr));
  if (IsReadable(sock)) {
    auto& i = socks_.find(sock.GetSocket())->second;
    int len = TryRecvfrom(i);
    if (len < 0) {
      ev.error_ = -len;
      ev.status_ = ERR;
      i.ractivated = false;
      ctx_->SubmitComplete(move(ev));
    } else if (len != 0) {
      ev.res_ = len;
      i.ractivated = false;
      ctx_->SubmitComplete(move(ev));
    }
  }
}

int WinSockSelect::TryRecvfrom(SockEvent& se) {
  int len = se.rdata.sock->Recvfrom(se.rdata.bufs, (IPAddr*)se.rdata.data);
  if (len < 0) {
    int err = se.rdata.sock->GetSockErr();
    if (err == WSAEWOULDBLOCK) {
      se.rdata.processing = false;
      return 0;
    } else {
      return -err;
    }
  }
  return len;
}

void WinSockSelect::SubmitSendto(WinSock& sock, IPAddr* addr, Buffers& bufs,
                                 Event&& ev) {
  SubmitWriteEvent(
      sock, SockEventData(SockEventData::SENDTO, sock, bufs, ev, (void*)addr));
  if (IsWriteable(sock)) {
    auto& i = socks_.find(sock.GetSocket())->second;
    int len = TrySendto(i);
    if (len < 0) {
      ev.error_ = -len;
      ev.status_ = ERR;
      i.wactivated = false;
      ctx_->SubmitComplete(move(ev));
    } else if (len != 0) {
      ev.res_ = len;
      i.wactivated = false;
      ctx_->SubmitComplete(move(ev));
    }
  }
}

int WinSockSelect::TrySendto(SockEvent& se) {
  int len = se.wdata.sock->Sendto(se.wdata.bufs, (IPAddr*)se.wdata.data);
  if (len < 0) {
    int err = se.wdata.sock->GetSockErr();
    if (err == WSAEWOULDBLOCK) {
      se.wdata.processing = false;
      return 0;
    } else {
      return -err;
    }
  }
  return len;
}

int WinSockSelect::Wait() {
  int maxfds = InitSet();
  int rc = select(maxfds + 1, &rs_, &ws_, nullptr, NULL);
  if (rc < 0) {
    int t = WSAGetLastError();
    return -1;
  }
  for (auto& i : socks_) {
    if (FD_ISSET(i.first, &rs_)) {
      if (!i.second.ractivated) {
        continue;
      }
      switch (i.second.rdata.type) {
        case SockEventData::READ: {
          i.second.rdata.processing = true;
          int len = TryRead(i.second);
          if (len < 0) {
            i.second.rdata.event.error_ = -len;
            i.second.rdata.event.status_ = ERR;
          } else {
            i.second.rdata.event.res_ = len;
            i.second.rdata.event.status_ = COMPLETED;
          }
          i.second.ractivated = false;
          ctx_->SubmitComplete(move(i.second.rdata.event));
        } break;
        case SockEventData::RECVFROM: {
          i.second.rdata.processing = true;
          int len = TryRecvfrom(i.second);
          if (len < 0) {
            i.second.rdata.event.error_ = -len;
            i.second.rdata.event.status_ = ERR;
          } else {
            i.second.rdata.event.res_ = len;
            i.second.rdata.event.status_ = COMPLETED;
          }
          i.second.ractivated = false;
          ctx_->SubmitComplete(move(i.second.rdata.event));
        } break;
        case SockEventData::ACCEPT: {
          i.second.rdata.processing = true;
          int res = TryAccept(i.second);
          if (res > 0) {
            i.second.rdata.event.error_ = res;
            i.second.rdata.event.status_ = ERR;
          } else {
            i.second.rdata.event.status_ = COMPLETED;
          }
          i.second.ractivated = false;
          ctx_->SubmitComplete(move(i.second.rdata.event));
        } break;
        case SockEventData::CONNECT:
        case SockEventData::WRITE:
        case SockEventData::SENDTO:
          break;
      }
    }
    if (FD_ISSET(i.first, &ws_)) {
      switch (i.second.wdata.type) {
        case SockEventData::WRITE: {
          if (!i.second.wactivated) {
            continue;
          }
          i.second.wdata.processing = true;
          int len = TryWrite(i.second);
          if (len < 0) {
            i.second.wdata.event.error_ = -len;
            i.second.wdata.event.status_ = ERR;
          } else {
            i.second.wdata.event.res_ = len;
            i.second.wdata.event.status_ = COMPLETED;
          }
          i.second.wactivated = false;
          ctx_->SubmitComplete(move(i.second.wdata.event));
        } break;
        case SockEventData::SENDTO: {
          if (!i.second.wactivated) {
            continue;
          }
          i.second.wdata.processing = true;
          int len = TrySendto(i.second);
          if (len < 0) {
            i.second.wdata.event.error_ = -len;
            i.second.wdata.event.status_ = ERR;
          } else {
            i.second.wdata.event.res_ = len;
            i.second.wdata.event.status_ = COMPLETED;
          }
          i.second.wactivated = false;
          ctx_->SubmitComplete(move(i.second.wdata.event));
        } break;
        case SockEventData::CONNECT: {
          int res = TryConnect(i.second);
          if (res == WSAEISCONN) {
            i.second.wdata.event.status_ = COMPLETED;
            ctx_->SubmitComplete(move(i.second.wdata.event));
          } else {
            i.second.wdata.event.status_ = ERR;
            i.second.wdata.event.error_ = res;
            ctx_->SubmitComplete(move(i.second.wdata.event));
          }
          i.second.wactivated = false;
        } break;
        case SockEventData::READ:
        case SockEventData::RECVFROM:
        case SockEventData::ACCEPT:
          break;
      }
    }
  }
  return 0;
}

}  // namespace mocoder::venti
