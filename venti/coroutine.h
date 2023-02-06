/**
 * @file coroutine.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <coroutine>
#include <functional>
#include "venti/event.h"

#include "venti/context.h"

namespace mocoder::venti {

using namespace std;

template <typename T>
class Async {
 public:
  struct promise_type;

  using CoroHandle = coroutine_handle<promise_type>;
  CoroHandle handle_;

  Async(CoroHandle handle) : handle_(handle) {}
  Async(const Async&) = delete;
  Async(Async&& promise) : handle_(promise.handle_) {
    promise.handle_ = nullptr;
  }

  Async& operator=(const Async&) = delete;

  Async& operator=(Async&& promise) {
    handle_ = promise.handle_;
    promise.handle_ = nullptr;
    return *this;
  }
  
  T get() { return handle_.promise().value; }

  struct promise_type {
    T value;
    auto get_return_object() {
      return Async<T>{CoroHandle::from_promise(*this)};
    }

    auto initial_suspend() { return suspend_never{}; }

    auto return_value(T v) {
      value = v;
      return suspend_never{};
    }

    auto yield_value(T v) {
      value = v;
      return suspend_always{};
    }

    auto final_suspend() noexcept { return suspend_never{}; }

    void unhandled_exception() { exit(255); }
  };
};

template <>
class Async<void> {
 public:
  struct promise_type;

  using CoroHandle = coroutine_handle<promise_type>;
  CoroHandle handle_;

  Async(CoroHandle handle) : handle_(handle) {}
  Async(const Async&) = delete;
  Async(Async&& promise) : handle_(promise.handle_) {
    promise.handle_ = nullptr;
  }

  Async& operator=(const Async&) = delete;

  Async& operator=(Async&& promise) {
    handle_ = promise.handle_;
    promise.handle_ = nullptr;
    return *this;
  }

  struct promise_type {
    auto get_return_object() {
      return Async<void>{CoroHandle::from_promise(*this)};
    }

    auto initial_suspend() { return suspend_never{}; }

    auto return_void() {
      return suspend_never{};
    }

    auto yield_void() {
      return suspend_always{};
    }

    auto final_suspend() noexcept { return suspend_never{}; }

    void unhandled_exception() { exit(255); }
  };
};

void CoroSpawn(Context& ctx, function<Async<void>()> fn) {
  ctx.SubmitComplete(Event([fn](int, int) {
    fn();
  }));
}

}
