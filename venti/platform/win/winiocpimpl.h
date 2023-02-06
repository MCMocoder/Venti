/**
 * @file winiocpimpl.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-01-19
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <WinSock2.h>
#include <ioapiset.h>
#include <minwinbase.h>
#include <winsock.h>

#include <type_traits>

#include "venti/context.h"
#include "venti/event.h"
#include "winiocp.h"

namespace mocoder::venti {

void WinIocp::SubmitRead(WinSock& sock, Buffers& buf, Event&& ev) {
  vector<WSABUF> wsabufs(buf.bufs_.size());
  for (int i = 0; i < wsabufs.size(); ++i) {
    wsabufs[i].buf = buf.bufs_[i].content_;
    wsabufs[i].len = buf.bufs_[i].len_;
  }
  size_t len;
  DWORD flags = 0;
  SEOL* seol = new SEOL;
  memset(&seol->ol, 0, sizeof(OVERLAPPED));
  InitSEOL(seol, READ, sock, ev);
  int err = WSARecv(sock.sock_, wsabufs.data(), wsabufs.size(), (LPDWORD)&len,
                    &flags, (LPWSAOVERLAPPED)seol, NULL);
  if (err < 0) {
    err = WSAGetLastError();
    if (err != WSA_IO_PENDING) {
      ev.status_ = ERROR;
      ev.error_ = -err;
      ctx_->SubmitComplete(move(ev));
    }
  }
}

void WinIocp::SubmitWrite(WinSock& sock, Buffers& buf, Event&& ev) {
  vector<WSABUF> wsabufs(buf.bufs_.size());
  for (int i = 0; i < wsabufs.size(); ++i) {
    wsabufs[i].buf = buf.bufs_[i].content_;
    wsabufs[i].len = buf.bufs_[i].len_;
  }
  size_t len;
  SEOL* seol = new SEOL;
  memset(&seol->ol, 0, sizeof(OVERLAPPED));
  InitSEOL(seol, WRITE, sock, ev);
  int err = WSASend(sock.sock_, wsabufs.data(), wsabufs.size(), (LPDWORD)&len,
                    0, (LPWSAOVERLAPPED)seol, NULL);
  if (err < 0) {
    err = WSAGetLastError();
    if (err != WSA_IO_PENDING) {
      ev.status_ = ERROR;
      ev.error_ = -err;
      ctx_->SubmitComplete(move(ev));
    }
  }
}

void WinIocp::SubmitAccept(WinSock& sock, WinSock& res, Event&& ev) {
  SEOL* seol = new SEOL;
  memset(&seol->ol, 0, sizeof(OVERLAPPED));
  InitSEOL(seol, ACCEPT, sock, ev, sock.addr_.type_, &res.addr_);
  if (sock.addr_.type_ == IPAddr::IPV6) {
    size_t l;
    res.TCPSocket6();
    int err = AcceptEx(sock.sock_, res.sock_, seol->data, 0,
                       sizeof(sockaddr_in6) + 16, sizeof(sockaddr_in6) + 16,
                       (unsigned long*)&l, &seol->ol);
    if (err < 0) {
      err = WSAGetLastError();
      if (err != WSA_IO_PENDING) {
        ev.status_ = ERROR;
        ev.error_ = -err;
        ctx_->SubmitComplete(move(ev));
      }
    }
  } else {
    size_t l;
    res.TCPSocket4();
    int err =
        AcceptEx(sock.sock_, res.sock_, seol->data, 0, sizeof(sockaddr_in) + 16,
                 sizeof(sockaddr_in) + 16, (unsigned long*)&l, &seol->ol);
    if (err < 0) {
      err = WSAGetLastError();
      if (err != WSA_IO_PENDING) {
        ev.status_ = ERROR;
        ev.error_ = -err;
        ctx_->SubmitComplete(move(ev));
      }
    }
  }
}

void WinIocp::SubmitConnect(WinSock& sock, IPAddr& addr, Event&& ev) {
  SEOL* seol = new SEOL;
  memset(&seol->ol, 0, sizeof(OVERLAPPED));
  InitSEOL(seol, CONNECT, sock, ev, sock.addr_.type_);
  if (sock.addr_.type_ == IPAddr::IPV6) {
    sockaddr_in6* si = (sockaddr_in6*)seol->data;
    *si = WinUtils::ToSockaddr6(addr);
    int err = sock.ConnectEx(sock.sock_, (sockaddr*)si, sizeof(sockaddr_in6),
                             NULL, 0, 0, &seol->ol);
    if (err < 0) {
      err = WSAGetLastError();
      if (err != WSA_IO_PENDING) {
        ev.status_ = ERROR;
        ev.error_ = -err;
        ctx_->SubmitComplete(move(ev));
      }
    }
  } else {
    sockaddr_in* si = (sockaddr_in*)seol->data;
    *si = WinUtils::ToSockaddr4(addr);
    int err = sock.ConnectEx(sock.sock_, (sockaddr*)si, sizeof(sockaddr_in),
                             NULL, 0, 0, &seol->ol);
    if (err < 0) {
      err = WSAGetLastError();
      if (err != WSA_IO_PENDING) {
        ev.status_ = ERROR;
        ev.error_ = -err;
        ctx_->SubmitComplete(move(ev));
      }
    }
  }
}

void WinIocp::SubmitSendto(WinSock& sock, IPAddr* peer, Buffers& buf,
                           Event&& ev) {
  if (sock.addr_.type_ == IPAddr::Type::IPV6) {
    vector<WSABUF> wsabufs(buf.bufs_.size());
    WSAMSG wsamsg;
    memset(&wsamsg, 0, sizeof(wsamsg));
    for (int i = 0; i < wsabufs.size(); ++i) {
      wsabufs[i].buf = buf.bufs_[i].content_;
      wsabufs[i].len = buf.bufs_[i].len_;
    }
    sockaddr_in6 addr;
    if (peer != nullptr) {
      addr = WinUtils::ToSockaddr6(*peer);
      addr.sin6_port = peer->port_;
      wsamsg.name = (sockaddr*)&addr;
      wsamsg.namelen = sizeof(addr);
    } else {
      wsamsg.name = nullptr;
      wsamsg.namelen = 0;
    }
    wsamsg.lpBuffers = wsabufs.data();
    wsamsg.dwBufferCount = wsabufs.size();
    size_t len;
    SEOL* seol = new SEOL;
    memset(&seol->ol, 0, sizeof(OVERLAPPED));
    InitSEOL(seol, SENDTO, sock, ev);
    int err = WSASendMsg(sock.sock_, &wsamsg, 0, (LPDWORD)&len,
                         (LPWSAOVERLAPPED)seol, NULL);
    if (err < 0) {
      err = WSAGetLastError();
      if (err != WSA_IO_PENDING) {
        ev.status_ = ERROR;
        ev.error_ = -err;
        ctx_->SubmitComplete(move(ev));
      }
    }
  } else {
    vector<WSABUF> wsabufs(buf.bufs_.size());
    WSAMSG wsamsg;
    memset(&wsamsg, 0, sizeof(wsamsg));
    for (int i = 0; i < wsabufs.size(); ++i) {
      wsabufs[i].buf = buf.bufs_[i].content_;
      wsabufs[i].len = buf.bufs_[i].len_;
    }
    sockaddr_in addr;
    if (peer != nullptr) {
      addr = WinUtils::ToSockaddr4(*peer);
      addr.sin_port = peer->port_;
      wsamsg.name = (sockaddr*)&addr;
      wsamsg.namelen = sizeof(addr);
    } else {
      wsamsg.name = nullptr;
      wsamsg.namelen = 0;
    }
    wsamsg.lpBuffers = wsabufs.data();
    wsamsg.dwBufferCount = wsabufs.size();
    size_t len;
    SEOL* seol = new SEOL;
    memset(&seol->ol, 0, sizeof(OVERLAPPED));
    InitSEOL(seol, SENDTO, sock, ev);
    int err = WSASendMsg(sock.sock_, &wsamsg, 0, (LPDWORD)&len,
                         (LPWSAOVERLAPPED)seol, NULL);
    if (err < 0) {
      err = WSAGetLastError();
      if (err != WSA_IO_PENDING) {
        ev.status_ = ERROR;
        ev.error_ = -err;
        ctx_->SubmitComplete(move(ev));
      }
    }
  }
}

void WinIocp::SubmitRecvfrom(WinSock& sock, IPAddr* peer, Buffers& buf,
                             Event&& ev) {
  if (sock.addr_.type_ == IPAddr::Type::IPV6) {
    vector<WSABUF> wsabufs(buf.bufs_.size());
    WSAMSG wsamsg;
    memset(&wsamsg, 0, sizeof(wsamsg));
    for (int i = 0; i < wsabufs.size(); ++i) {
      wsabufs[i].buf = buf.bufs_[i].content_;
      wsabufs[i].len = buf.bufs_[i].len_;
    }
    SEOL* seol = new SEOL;
    memset(&seol->ol, 0, sizeof(OVERLAPPED));
    if (peer != nullptr) {
      InitSEOL(seol, RECVFROM, sock, ev, IPAddr::IPV6, peer);
      wsamsg.name = (sockaddr*)seol->data;
      wsamsg.namelen = sizeof(sockaddr_in6);
    } else {
      InitSEOL(seol, RECVFROM, sock, ev, IPAddr::IPV6);
      wsamsg.name = nullptr;
      wsamsg.namelen = 0;
    }
    wsamsg.lpBuffers = wsabufs.data();
    wsamsg.dwBufferCount = wsabufs.size();
    size_t len;
    int err = sock.WSARecvMsg(sock.sock_, &wsamsg, (unsigned long*)&len,
                              &seol->ol, NULL);
    if (err < 0) {
      err = WSAGetLastError();
      if (err != WSA_IO_PENDING) {
        ev.status_ = ERROR;
        ev.error_ = -err;
        ctx_->SubmitComplete(move(ev));
      }
    }
  } else {
    vector<WSABUF> wsabufs(buf.bufs_.size());
    WSAMSG wsamsg;
    memset(&wsamsg, 0, sizeof(wsamsg));
    for (int i = 0; i < wsabufs.size(); ++i) {
      wsabufs[i].buf = buf.bufs_[i].content_;
      wsabufs[i].len = buf.bufs_[i].len_;
    }
    SEOL* seol = new SEOL;
    memset(&seol->ol, 0, sizeof(OVERLAPPED));
    if (peer != nullptr) {
      InitSEOL(seol, RECVFROM, sock, ev, IPAddr::IPV4, peer);
      wsamsg.name = (sockaddr*)seol->data;
      wsamsg.namelen = sizeof(sockaddr_in);
    } else {
      InitSEOL(seol, RECVFROM, sock, ev, IPAddr::IPV4);
      wsamsg.name = nullptr;
      wsamsg.namelen = 0;
    }
    wsamsg.lpBuffers = wsabufs.data();
    wsamsg.dwBufferCount = wsabufs.size();
    size_t len;
    int err = sock.WSARecvMsg(sock.sock_, &wsamsg, (unsigned long*)&len,
                              &seol->ol, NULL);
    if (err < 0) {
      err = WSAGetLastError();
      if (err != WSA_IO_PENDING) {
        ev.status_ = ERROR;
        ev.error_ = -err;
        ctx_->SubmitComplete(move(ev));
      }
    }
  }
}

void WinIocp::Wait() {
  size_t len = 0;
  SEOL* seol = nullptr;
  unsigned long long ul;
  bool valid = GetQueuedCompletionStatus(iocp_, (unsigned long*)&len, &ul,
                                         (LPOVERLAPPED*)&seol, WSA_INFINITE);
  if (!valid) {
    seol->ev.status_ = ERROR;
    seol->ev.error_ = WSAGetLastError();
    ctx_->SubmitComplete(move(seol->ev));
    DelSEOL(seol);
    delete seol;
    return;
  }
  if (seol->type == RECVFROM) {
    if (seol->data != nullptr) {
      if (seol->ipa == IPAddr::IPV6) {
        *(IPAddr*)seol->extra =
            WinUtils::ToIPAddr6(&(((sockaddr_in6*)(seol->data))->sin6_addr));
        ((IPAddr*)seol->extra)->port_ =
            ((sockaddr_in6*)(seol->data))->sin6_port;
      } else {
        *(IPAddr*)seol->extra =
            WinUtils::ToIPAddr4(&(((sockaddr_in*)(seol->data))->sin_addr));
        ((IPAddr*)seol->extra)->port_ = ((sockaddr_in*)(seol->data))->sin_port;
      }
    }
  } else if (seol->type == ACCEPT) {
    if (seol->ipa == IPAddr::IPV6) {
      sockaddr_in6 *ploc, *prem;
      size_t lloc, lrem;
      GetAcceptExSockaddrs(seol->data, 0, sizeof(sockaddr_in6) + 16,
                           sizeof(sockaddr_in6) + 16, (sockaddr**)&ploc,
                           (LPINT)&lloc, (sockaddr**)&prem, (LPINT)&lrem);
      IPAddr* resaddr = (IPAddr*)seol->extra;
      *resaddr = WinUtils::ToIPAddr6(&prem->sin6_addr);
      resaddr->port_ = prem->sin6_port;
    } else {
      sockaddr_in *ploc, *prem;
      size_t lloc, lrem;
      GetAcceptExSockaddrs(seol->data, 0, sizeof(sockaddr_in) + 16,
                           sizeof(sockaddr_in) + 16, (sockaddr**)&ploc,
                           (LPINT)&lloc, (sockaddr**)&prem, (LPINT)&lrem);
      IPAddr* resaddr = (IPAddr*)seol->extra;
      *resaddr = WinUtils::ToIPAddr4(&prem->sin_addr);
      resaddr->port_ = prem->sin_port;
    }
  }
  seol->ev.status_ = COMPLETED;
  seol->ev.res_ = len;
  ctx_->SubmitComplete(move(seol->ev));
  DelSEOL(seol);
  delete seol;
}

}  // namespace mocoder::venti
