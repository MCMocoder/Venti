/**
 * @file winsock.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <MSWSock.h>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <inaddr.h>
#include <minwinbase.h>
#include <winerror.h>

#include <cstring>
#include <memory>
#include <vector>

#include "venti/buffer.h"
#include "venti/ip/addr.h"

namespace mocoder::venti {

class WinUtils {
 public:
  static IPAddr ToIPAddr4(in_addr* addr) {
    IPAddr res(IPAddr::Type::IPV4);
    memcpy(res.v4addr_, addr, sizeof(in_addr));
    return res;
  }
  static IPAddr ToIPAddr6(in_addr6* addr) {
    IPAddr res(IPAddr::Type::IPV6);
    memcpy(res.v4addr_, addr, sizeof(in_addr6));
    return res;
  }
  static sockaddr_in ToSockaddr4(IPAddr& addr) {
    sockaddr_in res;
    memset(&res, 0, sizeof(res));
    memcpy(&res.sin_addr, addr.v4addr_, 4);
    res.sin_family = AF_INET;
    res.sin_port = addr.port_;
    return res;
  }
  static sockaddr_in6 ToSockaddr6(IPAddr& addr) {
    sockaddr_in6 res;
    memset(&res, 0, sizeof(res));
    memcpy(&res.sin6_addr, addr.v6addr_, 16);
    res.sin6_family = AF_INET6;
    res.sin6_port = addr.port_;
    return res;
  }
};

class WinSock {
 public:
  typedef SOCKET SockFd;

  SockFd sock_;
  IPAddr addr_;

  LPFN_WSARECVMSG WSARecvMsg;
  LPFN_CONNECTEX ConnectEx;
  LPFN_DISCONNECTEX DisconnectEx;

  bool inited_ = false;

  WinSock() {
    Init();
    sock_ = 0;
  }

  bool Valid() { return sock_ != 0; }

  int Init() {
    WORD versionreq = MAKEWORD(2, 2);
    WSADATA data;
    return WSAStartup(versionreq, &data);
  }

  int InitFunc() {
    if (!inited_) {
      GUID guid = WSAID_WSARECVMSG;
      DWORD outsize = 0;

      WSAIoctl(sock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
               &WSARecvMsg, sizeof(WSARecvMsg), &outsize, NULL, NULL);

      guid = WSAID_CONNECTEX;
      WSAIoctl(sock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
               &ConnectEx, sizeof(ConnectEx), &outsize, NULL, NULL);

      guid = WSAID_DISCONNECTEX;
      WSAIoctl(sock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
               &DisconnectEx, sizeof(DisconnectEx), &outsize, NULL, NULL);

      inited_ = true;
      return 0;
    }
    return 0;
  }

  int GetSockErr() {
    int err = WSAGetLastError();
    if (err == 0) {
      return 0;
    } else {
      return err;
    }
  }

  int RawSocket4() {
    sock_ = WSASocketA(AF_INET, SOCK_RAW, IPPROTO_IPV4, NULL, 0,
                       WSA_FLAG_OVERLAPPED);
    u_long flag = 1;
    ioctlsocket(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::Type::IPV4;
    return InitFunc();
  }

  int RawSocket6() {
    sock_ = WSASocketA(AF_INET6, SOCK_RAW, IPPROTO_IPV6, NULL, 0,
                       WSA_FLAG_OVERLAPPED);
    u_long flag = 1;
    ioctlsocket(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::Type::IPV6;
    return InitFunc();
  }

  int TCPSocket4() {
    sock_ = WSASocketA(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    u_long flag = 1;
    ioctlsocket(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::Type::IPV4;
    return InitFunc();
  }

  int TCPSocket6() {
    sock_ = WSASocketA(AF_INET6, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    u_long flag = 1;
    ioctlsocket(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::Type::IPV6;
    return InitFunc();
  }

  int UDPSocket4() {
    sock_ = WSASocketA(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    u_long flag = 1;
    ioctlsocket(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::Type::IPV4;
    return InitFunc();
  }

  int UDPSocket6() {
    sock_ = WSASocketA(AF_INET6, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    u_long flag = 1;
    ioctlsocket(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::Type::IPV6;
    return InitFunc();
  }

  int Connect(IPAddr* addr) {
    if (addr_.type_ == IPAddr::Type::IPV6) {
      sockaddr_in6 addr6 = WinUtils::ToSockaddr6(*addr);
      int err = connect(sock_, (sockaddr*)&addr6, sizeof(sockaddr_in6));
      if (err != 0) {
        return GetSockErr();
      }
      addr_ = *addr;
      return 0;
    } else {
      sockaddr_in addr4 = WinUtils::ToSockaddr4(*addr);
      int err = connect(sock_, (sockaddr*)&addr4, sizeof(sockaddr_in));
      if (err != 0) {
        return GetSockErr();
      }
      addr_ = *addr;
      return 0;
    }
  }

  int Bind(IPAddr& addr) {
    if (addr.type_ == IPAddr::Type::IPV6) {
      sockaddr_in6 addr6 = WinUtils::ToSockaddr6(addr);
      int err = ::bind(sock_, (sockaddr*)&addr6, sizeof(sockaddr_in6));
      if (err != 0) {
        return err;
      }
      addr_ = addr;
      return 0;
    } else {
      sockaddr_in addr4 = WinUtils::ToSockaddr4(addr);
      int err = ::bind(sock_, (sockaddr*)&addr4, sizeof(sockaddr_in));
      if (err != 0) {
        return err;
      }
      addr_ = addr;
      return 0;
    }
  }

  int Listen(int backlog) { return listen(sock_, backlog); }

  int Accept(WinSock* res) {
    if (addr_.type_ == IPAddr::Type::IPV6) {
      sockaddr_in6 addr6;
      int len = sizeof(addr6);
      SockFd newsock = accept(sock_, (sockaddr*)&addr6, &len);
      if (newsock < 0) {
        return -1;
      }
      res->sock_ = newsock;
      res->addr_.type_ = IPAddr::Type::IPV6;
      memcpy(&res->addr_.v6addr_, &addr6.sin6_addr, sizeof(addr6.sin6_addr));
      res->addr_.port_ = addr6.sin6_port;
      return 0;
    } else {
      sockaddr_in addr4;
      int len = sizeof(addr4);
      SockFd newsock = accept(sock_, (sockaddr*)&addr4, &len);
      if (newsock < 0) {
        return -1;
      }
      res->sock_ = newsock;
      res->addr_.type_ = IPAddr::Type::IPV4;
      memcpy(&res->addr_.v6addr_, &addr4.sin_addr, sizeof(addr4.sin_addr));
      res->addr_.port_ = addr4.sin_port;
      return 0;
    }
  }

  int Send(Buffers& buf) {
    vector<WSABUF> wsabufs(buf.bufs_.size());
    for (int i = 0; i < wsabufs.size(); ++i) {
      wsabufs[i].buf = buf.bufs_[i].content_;
      wsabufs[i].len = buf.bufs_[i].len_;
    }
    size_t len;
    int ec = WSASend(sock_, wsabufs.data(), wsabufs.size(), (LPDWORD)&len, 0,
                     NULL, NULL);
    if (ec != 0) {
      return -1;
    }
    return len;
  }

  int Recv(Buffers& buf) {
    vector<WSABUF> wsabufs(buf.bufs_.size());
    for (int i = 0; i < wsabufs.size(); ++i) {
      wsabufs[i].buf = buf.bufs_[i].content_;
      wsabufs[i].len = buf.bufs_[i].len_;
    }
    size_t len;
    DWORD flags = 0;
    int ec = WSARecv(sock_, wsabufs.data(), wsabufs.size(), (LPDWORD)&len,
                     &flags, NULL, NULL);
    if (ec != 0) {
      return -1;
    }
    return len;
  }

  int Sendto(Buffers& buf, IPAddr* peer) {
    if (addr_.type_ == IPAddr::Type::IPV6) {
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
      int ec = WSASendMsg(sock_, &wsamsg, 0, (LPDWORD)&len, NULL, NULL);
      if (ec != 0) {
        return -1;
      }
      return len;
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
      int ec = WSASendMsg(sock_, &wsamsg, 0, (LPDWORD)&len, NULL, NULL);
      if (ec != 0) {
        return -1;
      }
      return len;
    }
  }

  int Recvfrom(Buffers& buf, IPAddr* peer) {
    if (addr_.type_ == IPAddr::Type::IPV6) {
      vector<WSABUF> wsabufs(buf.bufs_.size());
      WSAMSG wsamsg;
      memset(&wsamsg, 0, sizeof(wsamsg));
      for (int i = 0; i < wsabufs.size(); ++i) {
        wsabufs[i].buf = buf.bufs_[i].content_;
        wsabufs[i].len = buf.bufs_[i].len_;
      }
      sockaddr_in6 addr;
      if (peer != nullptr) {
        wsamsg.name = (sockaddr*)&addr;
        wsamsg.namelen = sizeof(addr);
      } else {
        wsamsg.name = nullptr;
        wsamsg.namelen = 0;
      }
      wsamsg.lpBuffers = wsabufs.data();
      wsamsg.dwBufferCount = wsabufs.size();
      size_t len;
      int ec = WSARecvMsg(sock_, &wsamsg, (unsigned long*)&len, NULL, NULL);
      if (ec != 0) {
        return -1;
      }
      if (peer != nullptr) {
        *peer = WinUtils::ToIPAddr6(&addr.sin6_addr);
        peer->port_ = addr.sin6_port;
      }
      return len;
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
        wsamsg.name = (sockaddr*)&addr;
        wsamsg.namelen = sizeof(addr);
      } else {
        wsamsg.name = nullptr;
        wsamsg.namelen = 0;
      }
      wsamsg.lpBuffers = wsabufs.data();
      wsamsg.dwBufferCount = wsabufs.size();
      size_t len;
      int ec = WSARecvMsg(sock_, &wsamsg, (unsigned long*)&len, NULL, NULL);
      if (ec != 0) {
        return -1;
      }
      if (peer != nullptr) {
        *peer = WinUtils::ToIPAddr4(&addr.sin_addr);
        peer->port_ = addr.sin_port;
      }
      return len;
    }
  }

  enum Shut { R = SD_RECEIVE, W = SD_SEND, RW = SD_BOTH };

  int Shutdown(int shut) {
    int err = shutdown(sock_, shut);
    if (err != 0) {
      return GetSockErr();
    }
    return 0;
  }

  int Close() {
    int err = closesocket(sock_);
    if (err != 0) {
      return GetSockErr();
    }
    return 0;
  }

  IPAddr GetPeer() { return addr_; }

  IPAddr GetLocal() { return addr_; }

  SockFd GetSocket() { return sock_; }

  bool operator==(const WinSock& another) const {
    return sock_ == another.sock_;
  }
};

}  // namespace mocoder::venti
