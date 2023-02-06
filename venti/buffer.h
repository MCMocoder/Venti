/**
 * @file buffer.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <cstddef>
#include <initializer_list>
#include <vector>
#include <string>

namespace mocoder::venti {

using namespace std;

class Buffer {
 public:
  char* content_;
  size_t len_;
  Buffer(char* content, size_t len) : content_(content), len_(len) {}
  Buffer(std::string& content)
      : content_(content.data()), len_(content.size()) {}
};

class Buffers {
 public:
  vector<Buffer> bufs_;
  Buffers(initializer_list<Buffer> lst) : bufs_(lst) {}
  Buffers(Buffer buf) : bufs_(1, buf) {}
  Buffers(const Buffers& bufs) : bufs_(bufs.bufs_) {}
  Buffers() {}

  int Size() {
    int val = 0;
    for (auto i : bufs_) {
      val+=i.len_;
    }
    return val;
  }
};

}
