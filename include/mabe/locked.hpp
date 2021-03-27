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

#ifndef LOCKER_HPP
#define LOCKER_HPP

#include <utility>
#include <type_traits>
#include <functional>

#ifndef MABE_LOCKER_NS
#define MABE_LOCKER_NS mabe
#endif

namespace MABE_LOCKER_NS {

    template<typename T, typename MTX>
    class Locked {
        mutable MTX mtx_;
        T obj_;
    public:
        using Self = Locked<T, MTX>;

        template<typename UT, typename UMTX>
        Locked(UT &&obj, UMTX &&mtx) :
                mtx_{std::forward<UMTX>(mtx)},
                obj_{std::forward<UT>(obj)} {}

        template<typename UT,
                 typename = typename std::enable_if<!std::is_same<
                    typename std::remove_cv<typename std::remove_reference<UT>::type>::type, Locked>::value>>
        explicit Locked(UT &&obj) : Locked(std::forward<UT>(obj), MTX{}) {}

        Locked() = default;

        template<typename TT>
        class Locker {
            friend Locked;

            MTX &mtx_;
            TT &obj_;

            static_assert(std::is_same<T, typename std::decay<TT>::type>::value, "");

            Locker(TT &obj, MTX &mtx) : mtx_{mtx}, obj_{obj} {
                mtx_.lock();
            }

        public:
            ~Locker() {
                mtx_.unlock();
            }

            TT *operator->() {
                return &obj_;
            }

            TT *operator->() const {
                return &obj_;
            }
        };

        Locker<T> lock() {
            return Locker<T>(obj_, mtx_);
        }

        Locker<T> operator*() {
            return Locker<T>(obj_, mtx_);
        }

        Locker<const T> operator*() const {
            return Locker<const T>(obj_, mtx_);
        }

        void Apply(std::function<void(T &)> fs) {
            Locker<T> l(obj_, mtx_);
            fs(obj_);
        }

        void Apply(std::function<void(const T &)> fs) const {
            Locker<const T> l(obj_, mtx_);
            fs(obj_);
        }

        Locker<T> operator->() {
            return Locker<T>(obj_, mtx_);
        }

        template <typename TT>
        Self &operator=(TT &&newObj) {
            Locker<const T> l(obj_, mtx_);
            obj_ = std::forward<TT>(newObj);
            return *this;
        }

        operator T() const {
            Locker<const T> l(obj_, mtx_);
            return obj_;
        }

#if MABE_LOCKER_TESTING
        MTX& Mtx() { return mtx_; }

        T& Obj() { return obj_; }
#endif
    };

}// namespace

#endif //LOCKER_HPP
