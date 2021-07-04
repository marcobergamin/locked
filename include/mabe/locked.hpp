// Copyright [2021] [Marco Bergamin - marco.bergamin@outlook.com]
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//         http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
//         limitations under the License.
#pragma once

#ifndef MABE_LOCKED_HPP
#define MABE_LOCKED_HPP

#include <functional>
#include <type_traits>
#include <utility>

namespace mabe {

namespace detail {
template <typename MAYBE_SHARABLE_MTX> class IsSharable {
  using yes = char[1]; // NOLINT suppress use std::array
  using no = char[2];  // NOLINT suppress use std::array

  char x[123];

  template <typename M>
  static yes &check_lock_shared(decltype(&M::lock_shared));

  template <typename M>
  static yes &check_unlock_shared(decltype(&M::unlock_shared));

  template <typename M> static no &check_lock_shared(...);

  template <typename M> static no &check_unlock_shared(...);

public:
  enum {
    value = (sizeof(check_lock_shared<MAYBE_SHARABLE_MTX>(nullptr)) ==
             sizeof(yes)) &&
            (sizeof(check_unlock_shared<MAYBE_SHARABLE_MTX>(nullptr)) ==
             sizeof(yes))
  };
};

template <typename T> using remove_cv_t = typename std::remove_cv<T>::type;
template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;
template <typename T>
using remove_reference_t = typename std::remove_reference<T>::type;
} // namespace detail

template <typename T, typename MTX> class Locked {
  mutable MTX mtx_;
  T obj_;

public:
  using self = Locked<T, MTX>;
  using obj_type = T;
  using mtx_type = MTX;

  template <typename UT, typename UMTX>
  Locked(UT &&obj, UMTX &&mtx)
      : mtx_{std::forward<UMTX>(mtx)}, obj_{std::forward<UT>(obj)} {}

  template <
      typename UT,
      detail::enable_if_t<
          !std::is_same<detail::remove_cv_t<detail::remove_reference_t<UT>>,
                        Locked>::value,
          bool> = true>
  explicit Locked(UT &&obj) : mtx_{}, obj_{std::forward<UT>(obj)} {}

  template <typename UT>
  Locked(std::initializer_list<UT> l) : mtx_{}, obj_{std::move(l)} {}

  Locked() = default;

  template <typename TT> class Locker {
    friend Locked;

    MTX &mtx_;
    TT &obj_;

    static_assert(std::is_same<T, typename std::decay<TT>::type>::value, "");

    Locker(TT &obj, MTX &mtx) : mtx_{mtx}, obj_{obj} {
#if MABE_LOCKED_HAS_CXX_STD_17
      if constexpr (std::is_const_v<TT> && detail::IsSharable<MTX>::value) {
        mtx_.lock_shared();
      } else {
        mtx_.lock();
      }
#else
      mtx_.lock();
#endif
    }

  public:
    Locker(const Locker &) = delete;
    Locker &operator=(const Locker &) = delete;
    Locker(Locker &&) noexcept = default;
    Locker &operator=(Locker &&) noexcept = default;

    ~Locker() {
#if MABE_LOCKED_HAS_CXX_STD_17
      if constexpr (std::is_const_v<TT> && detail::IsSharable<MTX>::value) {
        mtx_.unlock_shared();
      } else {
        mtx_.unlock();
      }
#else
      mtx_.unlock();
#endif
    }

    TT *operator->() { return &obj_; }

    TT *operator->() const { return &obj_; }
  };

  Locker<T> lock() { return Locker<T>(obj_, mtx_); }

  Locker<T> operator*() { return Locker<T>(obj_, mtx_); }

  Locker<const T> operator*() const { return Locker<const T>(obj_, mtx_); }

  void apply(std::function<void(T &)> fs) {
    Locker<T> l(obj_, mtx_);
    fs(obj_);
  }

  void apply(std::function<void(const T &)> fs) const {
    Locker<const T> l(obj_, mtx_);
    fs(obj_);
  }

  Locker<const T> operator->() const { return Locker<const T>(obj_, mtx_); }

  Locker<T> operator->() { return Locker<T>(obj_, mtx_); }

  template <typename TT> self &operator=(TT &&newObj) {
    Locker<const T> l(obj_, mtx_);
    obj_ = std::forward<TT>(newObj);
    return *this;
  }

  // NOLINTNEXTLINE: implicit conversion is desired
  operator T() const {
    Locker<const T> l(obj_, mtx_);
    return obj_;
  }

#if MABE_LOCKED_TESTING

  MTX &mtx() const { return mtx_; }

  const T &obj() const { return obj_; }

  T &obj() { return obj_; }

#endif
};

} // namespace mabe

#endif // MABE_LOCKED_HPP
