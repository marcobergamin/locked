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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mabe/locked.hpp>

struct MockMutex {
  MOCK_METHOD(void, lock, ());
  MOCK_METHOD(void, unlock, ());
};

TEST(LockedTest, Creation) {
  mabe::Locked<int, MockMutex> locked;
  EXPECT_CALL(locked.mtx(), lock()).Times(0);
  EXPECT_CALL(locked.mtx(), unlock()).Times(0);
}

TEST(LockedTest, SetGetValue) {
  mabe::Locked<int, MockMutex> locked;

  constexpr auto testValue = 10;

  testing::InSequence s;
  EXPECT_CALL(locked.mtx(), lock()).Times(1);
  EXPECT_CALL(locked.mtx(), unlock()).Times(1);
  locked = testValue;
  EXPECT_CALL(locked.mtx(), lock()).Times(1);
  EXPECT_CALL(locked.mtx(), unlock()).Times(1);
  EXPECT_EQ(locked, testValue);
}

struct Foo {
  MOCK_METHOD(void, foo, ());
  MOCK_METHOD(void, cfoo, (), (const));
};

TEST(LockedTest, MultipleOperations) {
  mabe::Locked<Foo, MockMutex> locked;

  testing::InSequence s;
  EXPECT_CALL(locked.mtx(), lock()).Times(1);
  EXPECT_CALL(locked.obj(), foo()).Times(3);
  EXPECT_CALL(locked.mtx(), unlock()).Times(1);

  locked.apply([](Foo &foo) {
    foo.foo();
    foo.foo();
    foo.foo();
  });
}

TEST(LockedTest, MultipleConstOperations) {
  const mabe::Locked<Foo, MockMutex> locked;

  testing::InSequence s;
  EXPECT_CALL(locked.mtx(), lock()).Times(1);
  EXPECT_CALL(locked.obj(), cfoo()).Times(3);
  EXPECT_CALL(locked.mtx(), unlock()).Times(1);

  locked.apply([](const Foo &foo) {
    foo.cfoo();
    foo.cfoo();
    foo.cfoo();
  });
}

TEST(LockedTest, FunctionCall) {
  mabe::Locked<Foo, MockMutex> locked;

  testing::InSequence s;
  EXPECT_CALL(locked.mtx(), lock()).Times(1);
  EXPECT_CALL(locked.obj(), foo()).Times(1);
  EXPECT_CALL(locked.mtx(), unlock()).Times(1);

  locked->foo();
}

TEST(LockedTest, OperatorStar) {
  mabe::Locked<Foo, MockMutex> locked;

  testing::InSequence s;
  EXPECT_CALL(locked.mtx(), lock()).Times(1);
  EXPECT_CALL(locked.obj(), foo()).Times(1);
  EXPECT_CALL(locked.mtx(), unlock()).Times(1);

  (*locked)->foo();
}

TEST(LockedTest, OperatorStarConst) {
  const mabe::Locked<Foo, MockMutex> locked;

  testing::InSequence s;
  EXPECT_CALL(locked.mtx(), lock()).Times(1);
  EXPECT_CALL(locked.obj(), cfoo()).Times(1);
  EXPECT_CALL(locked.mtx(), unlock()).Times(1);

  (*locked)->cfoo();
}

TEST(LockedTest, SharedPtrMultipleOperations) {
  const auto lockedPtr = std::make_shared<mabe::Locked<Foo, MockMutex>>();

  testing::InSequence s;
  EXPECT_CALL(lockedPtr->mtx(), lock()).Times(1);
  EXPECT_CALL(lockedPtr->obj(), cfoo()).Times(2);
  EXPECT_CALL(lockedPtr->mtx(), unlock()).Times(1);

  lockedPtr->apply([](const Foo &foo) {
    foo.cfoo();
    foo.cfoo();
  });
}

TEST(LockedTest, SharedPtrFunctionCall) {
  const auto lockedPtr = std::make_shared<mabe::Locked<Foo, MockMutex>>();

  testing::InSequence s;
  EXPECT_CALL(lockedPtr->mtx(), lock()).Times(1);
  EXPECT_CALL(lockedPtr->obj(), cfoo()).Times(1);
  EXPECT_CALL(lockedPtr->mtx(), unlock()).Times(1);

  (*lockedPtr)->cfoo();
}

#include <mutex>

TEST(LockedTest, StdMutexFunctionCall) {
  const auto lockedPtr = std::make_shared<mabe::Locked<Foo, std::mutex>>();

  testing::InSequence s;
  EXPECT_CALL(lockedPtr->obj(), cfoo()).Times(1);

  (*lockedPtr)->cfoo();
}

struct MockSharedMutex {
  MOCK_METHOD(void, lock, ());
  MOCK_METHOD(void, unlock, ());
  MOCK_METHOD(void, lock_shared, ());
  MOCK_METHOD(void, unlock_shared, ());
};

TEST(LockedTest, SharedMutexFunctionCall) {
  mabe::Locked<Foo, MockSharedMutex> lockedFoo;

  testing::InSequence s;

  // Expects calls to lock/unlock on non-const objects
  EXPECT_CALL(lockedFoo.mtx(), lock()).Times(1);
  EXPECT_CALL(lockedFoo.obj(), foo()).Times(1);
  EXPECT_CALL(lockedFoo.mtx(), unlock()).Times(1);
  lockedFoo->foo();

  // Expects calls to lock_shared/unlock_shared on const objects
  EXPECT_CALL(lockedFoo.mtx(), lock_shared()).Times(1);
  EXPECT_CALL(lockedFoo.obj(), cfoo()).Times(1);
  EXPECT_CALL(lockedFoo.mtx(), unlock_shared()).Times(1);
  std::as_const(lockedFoo)->cfoo();

  EXPECT_TRUE(false); // TEST FAILURE
}
