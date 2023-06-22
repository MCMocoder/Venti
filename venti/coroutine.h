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
#include <exception>
#include <functional>

#include "venti/context.h"
#include "venti/coroutine.h"
#include "venti/event.h"

namespace mocoder::venti {

using namespace std;

template <typename T>
struct Result {
  explicit Result() = default;

  explicit Result(T&& value) : value_(value) {}

  explicit Result(std::exception_ptr&& exception_ptr)
      : exception_ptr_(exception_ptr) {}

  T Get() {
    if (exception_ptr_) {
      std::rethrow_exception(exception_ptr_);
    }
    return value_;
  }

 private:
  T value_{};
  std::exception_ptr exception_ptr_;
};

template <>
struct Result<void> {
  explicit Result() = default;

  explicit Result(std::exception_ptr&& exception_ptr)
      : exception_ptr_(exception_ptr) {}

  void Get() {
    if (exception_ptr_) {
      std::rethrow_exception(exception_ptr_);
    }
  }

 private:
  std::exception_ptr exception_ptr_;
};

template <typename T>
class Future {
 public:
  struct promise_type;

  using CoroHandle = coroutine_handle<promise_type>;
  CoroHandle handle_;

  Future(CoroHandle handle) : handle_(handle) {}
  Future(const Future&) = delete;
  Future(Future&& promise) : handle_(promise.handle_) {
    promise.handle_ = nullptr;
  }

  Future& operator=(const Future&) = delete;

  Future& operator=(Future&& promise) {
    handle_ = promise.handle_;
    promise.handle_ = nullptr;
    return *this;
  }

  T get() { return handle_.promise().value; }

  bool await_ready() noexcept { return !handle_ || handle_.done(); }

  bool await_suspend(coroutine_handle<> handle) noexcept {
    return true;
    }

  bool await_suspend(CoroHandle handle) noexcept {
    handle.promise().inner_ = handle_;
    handle_.promise().outer_ = handle;
    return true;
  }

  auto await_resume() noexcept {
    return handle_.promise().result;
  }

  struct promise_type {
    Result<T> result;
    coroutine_handle<promise_type> inner_;
    coroutine_handle<promise_type> outer_;

    auto get_return_object() noexcept {
      return Future<T>{CoroHandle::from_promise(*this)};
    }

    auto initial_suspend() noexcept { return suspend_never{}; }

    auto return_value(T v) noexcept {
      result = Result<T>(std::move(v));
      return suspend_never{};
    }

    auto final_suspend() noexcept {
      struct FinalAwaitable {
        bool await_ready() noexcept { return false; }

        coroutine_handle<> await_suspend(
            coroutine_handle<promise_type> h) noexcept {
          auto& promise = h.promise();
          auto p = promise.outer_;
          if (p) {
            return p;
          }
          return noop_coroutine();
        }

        void await_resume() noexcept {}

      };
      return FinalAwaitable{};
      //return suspend_never{};
    }

    void unhandled_exception() { result = Result<T>(std::current_exception()); }
  };
};

template <>
class Future<void> {
 public:
  struct promise_type;

  using CoroHandle = coroutine_handle<promise_type>;
  CoroHandle handle_;

  Future(CoroHandle handle) : handle_(handle) {}
  Future(const Future&) = delete;
  Future(Future&& promise) : handle_(promise.handle_) {
    promise.handle_ = nullptr;
  }

  Future& operator=(const Future&) = delete;

  Future& operator=(Future&& promise) {
    handle_ = promise.handle_;
    promise.handle_ = nullptr;
    return *this;
  }

  bool await_ready() noexcept { return !handle_ || handle_.done(); }

  bool await_suspend(coroutine_handle<> handle) noexcept {
    return true;
    }

  bool await_suspend(CoroHandle handle) noexcept {
    handle.promise().inner_ = handle_;
    handle_.promise().outer_ = handle;
    return true;
  }

  auto await_resume() noexcept {
    return handle_.promise().result;
  }

  struct promise_type {
    Result<void> result;
    coroutine_handle<promise_type> inner_;
    coroutine_handle<promise_type> outer_;

    auto get_return_object() noexcept {
      return Future<void>{CoroHandle::from_promise(*this)};
    }

    auto initial_suspend() noexcept { return suspend_never{}; }

    auto return_void() { return suspend_never{}; }

    auto final_suspend() noexcept {
      struct FinalAwaitable {
        bool await_ready() noexcept { return false; }

        coroutine_handle<> await_suspend(
            coroutine_handle<promise_type> h) noexcept {
          auto& promise = h.promise();
          auto p = promise.outer_;
          if (p) {
            return p;
          }
          return noop_coroutine();
        }

        void await_resume() noexcept {}

      };
      // return FinalAwaitable{};
      return suspend_never{};
    }

    void unhandled_exception() { result = Result<void>(std::current_exception()); }
  };
};

template <typename T>
class Stream {

  

  struct promise_type {

  };
};

template <>
class Stream<void> {};

}  // namespace mocoder::venti
