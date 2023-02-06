/**
 * @file addr.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include "inet_pton.h"
#include "inet_ntop.h"
#include "htons.h"

namespace mocoder::venti {

using namespace std;

class IPAddr {
 public:
  enum Type { IPV4 = 1, IPV6 = 2 } type_;
  union {
    int8_t v4addr_[4];
    int8_t v6addr_[16];
  };
  short port_ = 0;
  IPAddr(){}
  IPAddr(Type type) : type_(type) {
    if (type == Type::IPV6) {
      memset(v6addr_, 0, 16);
    } else {
      memset(v4addr_, 0, 4);
    }
  }
  IPAddr(string addr, int port, Type type) {
    if (type == Type::IPV6) {
      venti::inet_pton(_AF_INET6, addr.c_str(), &v6addr_);
      type_ = Type::IPV6;
    } else {
      venti::inet_pton(_AF_INET, addr.c_str(), &v4addr_);
      type_ = Type::IPV4;
    }
    port_ = venti::htons(port);
  }
  IPAddr(const IPAddr& addr) {
    if (addr.type_ == Type::IPV6) {
      memcpy(v6addr_,addr.v6addr_,16);
      type_ = Type::IPV6;
    } else {
      memcpy(v4addr_, addr.v4addr_, 4);
      type_ = Type::IPV4;
    }
    port_ = addr.port_;
  }
  string ToString() {
    string res;
    if (type_ == Type::IPV6) {
      res.resize(47, 0);
      venti::inet_ntop(_AF_INET6, v6addr_, res.data(), 47);
    } else {
      res.resize(16, 0);
      venti::inet_ntop(_AF_INET, v4addr_, res.data(), 16);
    }
    if (port_ != 0) {
      res.append(":"+to_string(venti::ntohs(port_)));
    }
    return res;
  }
};

}  // namespace mocoder
