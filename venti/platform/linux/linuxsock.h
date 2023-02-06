/**
 * @file linuxsock.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-01-23
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/timerfd.h>

#include <chrono>
#include <cstring>
#include <ctime>
#include <memory>
#include <vector>

#include "sockhdr.h"
#include "venti/buffer.h"
#include "venti/event.h"
#include "venti/ip/addr.h"

namespace mocoder::venti {

class LinuxUtils {
 public:
  static IPAddr ToIPAddr6(sockaddr_in6 *addr) {
    IPAddr res(IPAddr::Type::IPV6);
    memcpy(res.v6addr_, &addr->sin6_addr, sizeof(in6_addr));
    res.port_ = addr->sin6_port;
    return res;
  }
  static IPAddr ToIPAddr4(sockaddr_in *addr) {
    IPAddr res(IPAddr::Type::IPV4);
    memcpy(res.v4addr_, &addr->sin_addr, sizeof(in_addr));
    res.port_ = addr->sin_port;
    return res;
  }
  static sockaddr_in ToSockaddr4(IPAddr &addr) {
    sockaddr_in res;
    memset(&res, 0, sizeof(res));
    memcpy(&res.sin_addr, addr.v4addr_, 4);
    res.sin_family = AF_INET;
    res.sin_port = addr.port_;
    return res;
  }
  static sockaddr_in6 ToSockaddr6(IPAddr &addr) {
    sockaddr_in6 res;
    memset(&res, 0, sizeof(res));
    memcpy(&res.sin6_addr, addr.v6addr_, 16);
    res.sin6_family = AF_INET6;
    res.sin6_port = addr.port_;
    return res;
  }
};

class LinuxSock {
 public:
  struct SockEventData {
    LinuxSock *sock;
    Buffers bufs;
    Event ev;
    bool rp = false, wp = false;  // rprocessing,wprocessing
    enum Type {
      READ,
      WRITE,
      ACCEPT,
      CONNECT,
      RECVFROM,
      SENDTO,
      TIMETICK,
      SIGNAL,
      NOOP
    };
    Type type;
    void *data;
    void Set(Type typei, LinuxSock *socki, Buffers &bufsi, Event evi) {
      sock = socki;
      type = typei;
      bufs = bufsi;
      ev = evi;
    }
    void Set(Type typei, LinuxSock *socki, Buffers &bufsi, Event evi,
             void *datai) {
      sock = socki;
      type = typei;
      bufs = bufsi;
      ev = evi;
      data = datai;
    }
    void Set(Type typei, LinuxSock *socki, Event evi) {
      sock = socki;
      type = typei;
      ev = evi;
    }
    void Set(Type typei, LinuxSock *socki, Event evi, void *datai) {
      sock = socki;
      type = typei;
      ev = evi;
      data = datai;
    }
    void Clear() { type = NOOP; }
  };

  shared_ptr<SockEventData> ev_;
  int sock_;
  IPAddr addr_;

  LinuxSock() : ev_(new SockEventData()) {}

  int GetSockErr() { return errno; }

  int Timer() {
    sock_ = timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK);
    return 0;
  }

  int RawSocket4() {
    sock_ = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
    int flag = 1;
    ioctl(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::IPV4;
    return 0;
  }

  int RawSocket6() {
    sock_ = socket(AF_INET, SOCK_RAW, IPPROTO_IPV6);
    int flag = 1;
    ioctl(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::IPV6;
    return 0;
  }

  int RawSocket46() {
    sock_ = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
    int flag = 1;
    ioctl(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::IPV4;
    return 0;
  }

  int TCPSocket4() {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    int flag = 1;
    ioctl(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::IPV4;
    return 0;
  }

  int TCPSocket6() {
    sock_ = socket(AF_INET6, SOCK_STREAM, 0);
    int flag = 1;
    ioctl(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::IPV6;
    return 0;
  }

  int UDPSocket4() {
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    int flag = 1;
    ioctl(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::IPV4;
    return 0;
  }

  int UDPSocket6() {
    sock_ = socket(AF_INET6, SOCK_DGRAM, 0);
    int flag = 1;
    ioctl(sock_, FIONBIO, &flag);
    if (sock_ < 0) {
      return sock_;
    }
    addr_.type_ = IPAddr::IPV6;
    return 0;
  }

  int Connect(IPAddr *addr) {
    if (addr_.type_ == IPAddr::IPV6) {
      sockaddr_in6 addr6 = LinuxUtils::ToSockaddr6(*addr);
      int err = connect(sock_, (sockaddr *)&addr6, sizeof(sockaddr_in6));
      if (err == -1) {
        return -GetSockErr();
      }
      addr_ = *addr;
      return 0;
    } else {
      sockaddr_in addr4 = LinuxUtils::ToSockaddr4(*addr);
      int err = connect(sock_, (sockaddr *)&addr4, sizeof(sockaddr_in));
      if (err == -1) {
        return -GetSockErr();
      }
      addr_ = *addr;
      return 0;
    }
  }

  int Bind(IPAddr &addr) {
    if (addr.type_ == IPAddr::Type::IPV6) {
      sockaddr_in6 addr6 = LinuxUtils::ToSockaddr6(addr);
      int err = ::bind(sock_, (sockaddr *)&addr6, sizeof(sockaddr_in6));
      if (err != 0) {
        return -GetSockErr();
      }
      addr_ = addr;
      return 0;
    } else {
      sockaddr_in addr4 = LinuxUtils::ToSockaddr4(addr);
      int err = ::bind(sock_, (sockaddr *)&addr4, sizeof(sockaddr_in));
      if (err != 0) {
        return -GetSockErr();
      }
      addr_ = addr;
      return 0;
    }
  }

  int Listen(int backlog) { return listen(sock_, backlog); }

  int Accept(LinuxSock *res) {
    if (addr_.type_ == IPAddr::IPV6) {
      sockaddr_in6 addr6;
      socklen_t len = sizeof(addr6);
      int newsock = accept(sock_, (sockaddr *)&addr6, &len);
      if (newsock < 0) {
        return -GetSockErr();
      }
      res->sock_ = newsock;
      res->addr_ = LinuxUtils::ToIPAddr6(&addr6);
    } else {
      sockaddr_in addr4;
      socklen_t len = sizeof(addr4);
      int newsock = accept(sock_, (sockaddr *)&addr4, &len);
      if (newsock < 0) {
        return -GetSockErr();
      }
      res->sock_ = newsock;
      res->addr_ = LinuxUtils::ToIPAddr4(&addr4);
    }
    return 0;
  }

  int Send(Buffers &buf) {
    vector<iovec> iov;
    for (auto &i : buf.bufs_) {
      iov.push_back({.iov_base = i.content_, .iov_len = i.len_});
    }
    msghdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.msg_iov = iov.data();
    hdr.msg_iovlen = iov.size();
    size_t len = sendmsg(sock_, &hdr, 0);
    if (len < 0) {
      return -GetSockErr();
    }
    return len;
  }

  int Recv(Buffers &buf) {
    vector<iovec> iov;
    for (auto &i : buf.bufs_) {
      iov.push_back({.iov_base = i.content_, .iov_len = i.len_});
    }
    msghdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.msg_iov = iov.data();
    hdr.msg_iovlen = iov.size();
    size_t len = recvmsg(sock_, &hdr, 0);
    if (len < 0) {
      return -GetSockErr();
    }
    return len;
  }

  int Sendto(Buffers &buf, IPAddr *peer) {
    if (addr_.type_ == IPAddr::IPV6) {
      vector<iovec> iov;
      for (auto &i : buf.bufs_) {
        iov.push_back({.iov_base = i.content_, .iov_len = i.len_});
      }
      msghdr hdr;
      memset(&hdr, 0, sizeof(hdr));
      sockaddr_in6 addr6;
      if (peer != nullptr) {
        addr6 = LinuxUtils::ToSockaddr6(*peer);
        hdr.msg_name = &addr6;
        hdr.msg_namelen = sizeof(sockaddr_in6);
      } else {
        hdr.msg_name = nullptr;
        hdr.msg_namelen = 0;
      }
      hdr.msg_iov = iov.data();
      hdr.msg_iovlen = iov.size();
      size_t len = sendmsg(sock_, &hdr, 0);
      if (len < 0) {
        return -GetSockErr();
      }
      return len;
    } else {
      vector<iovec> iov;
      for (auto &i : buf.bufs_) {
        iov.push_back({.iov_base = i.content_, .iov_len = i.len_});
      }
      msghdr hdr;
      memset(&hdr, 0, sizeof(hdr));
      sockaddr_in addr4;
      if (peer != nullptr) {
        addr4 = LinuxUtils::ToSockaddr4(*peer);
        hdr.msg_name = &addr4;
        hdr.msg_namelen = sizeof(sockaddr_in);
      } else {
        hdr.msg_name = nullptr;
        hdr.msg_namelen = 0;
      }
      hdr.msg_iov = iov.data();
      hdr.msg_iovlen = iov.size();
      size_t len = sendmsg(sock_, &hdr, 0);
      if (len < 0) {
        return -GetSockErr();
      }
      return len;
    }
  }

  int Recvfrom(Buffers &buf, IPAddr *peer) {
    if (addr_.type_ == IPAddr::IPV6) {
      vector<iovec> iov;
      for (auto &i : buf.bufs_) {
        iov.push_back({.iov_base = i.content_, .iov_len = i.len_});
      }
      msghdr hdr;
      memset(&hdr, 0, sizeof(hdr));
      sockaddr_in6 addr6;
      memset(&addr6, 0, sizeof(addr6));
      if (peer != nullptr) {
        hdr.msg_name = &addr6;
        hdr.msg_namelen = sizeof(sockaddr_in6);
      } else {
        hdr.msg_name = nullptr;
        hdr.msg_namelen = 0;
      }
      hdr.msg_iov = iov.data();
      hdr.msg_iovlen = iov.size();
      size_t len = recvmsg(sock_, &hdr, 0);
      if (len < 0) {
        return -GetSockErr();
      }
      if (peer != nullptr) {
        *peer = LinuxUtils::ToIPAddr6(&addr6);
      }
      return len;
    } else {
      vector<iovec> iov;
      for (auto &i : buf.bufs_) {
        iov.push_back({.iov_base = i.content_, .iov_len = i.len_});
      }
      msghdr hdr;
      memset(&hdr, 0, sizeof(hdr));
      sockaddr_in addr4;
      memset(&addr4, 0, sizeof(addr4));
      if (peer != nullptr) {
        hdr.msg_name = &addr4;
        hdr.msg_namelen = sizeof(sockaddr_in);
      } else {
        hdr.msg_name = nullptr;
        hdr.msg_namelen = 0;
      }
      hdr.msg_iov = iov.data();
      hdr.msg_iovlen = iov.size();
      size_t len = recvmsg(sock_, &hdr, 0);
      if (len < 0) {
        return -GetSockErr();
      }
      if(peer != nullptr) {
        *peer = LinuxUtils::ToIPAddr4(&addr4);
      }
      return len;
    }
  }

  void SetTime(chrono::nanoseconds ns) {
    itimerspec its = {
        .it_interval = {.tv_sec = 0, .tv_nsec = 0},
        .it_value = {
            .tv_sec = chrono::duration_cast<chrono::seconds>(ns).count(),
            .tv_nsec = ns.count() % 1000000000}};
    timerfd_settime(sock_, 0, &its, NULL);
  }

  enum Shut { R = SHUT_RD, W = SHUT_WR, RW = SHUT_RDWR };

  int Shutdown(int shut) {
    int err = shutdown(sock_, shut);
    if (err != 0) {
      return GetSockErr();
    }
    return 0;
  }

  int Close() {
    int err = close(sock_);
    if (err < 0) {
      return GetSockErr();
    }
    return 0;
  }

  IPAddr GetPeer() { return addr_; }

  IPAddr GetLocal() { return addr_; }

  bool operator==(const LinuxSock &another) const {
    return sock_ == another.sock_;
  }
};

}  // namespace mocoder::venti
