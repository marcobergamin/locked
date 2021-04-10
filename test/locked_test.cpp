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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mabe/locked.hpp>

struct MockMutex {
    MOCK_METHOD(void, lock, ());
    MOCK_METHOD(void, unlock, ());
};

TEST(LockedTest, Creation) {
    mabe::Locked<int, MockMutex> locked;
    EXPECT_CALL(locked.Mtx(), lock()).Times(0);
    EXPECT_CALL(locked.Mtx(), unlock()).Times(0);
}

TEST(LockedTest, SetGetValue) {
    mabe::Locked<int, MockMutex> locked;

    testing::InSequence s;
    EXPECT_CALL(locked.Mtx(), lock()).Times(1);
    EXPECT_CALL(locked.Mtx(), unlock()).Times(1);
    locked = 10;
    EXPECT_CALL(locked.Mtx(), lock()).Times(1);
    EXPECT_CALL(locked.Mtx(), unlock()).Times(1);
    EXPECT_EQ(locked, 10);
}

struct Foo {
    MOCK_METHOD(void, foo, ());
    MOCK_METHOD(void, cfoo, (), (const));
};

TEST(LockedTest, MultipleOperations) {
    mabe::Locked<Foo, MockMutex> locked;

    testing::InSequence s;
    EXPECT_CALL(locked.Mtx(), lock()).Times(1);
    EXPECT_CALL(locked.Obj(), foo()).Times(3);
    EXPECT_CALL(locked.Mtx(), unlock()).Times(1);

    locked.Apply([](Foo &foo) {
        foo.foo();
        foo.foo();
        foo.foo();
    });
}

TEST(LockedTest, MultipleConstOperations) {
    const mabe::Locked<Foo, MockMutex> locked;

    testing::InSequence s;
    EXPECT_CALL(locked.Mtx(), lock()).Times(1);
    EXPECT_CALL(locked.Obj(), cfoo()).Times(3);
    EXPECT_CALL(locked.Mtx(), unlock()).Times(1);

    locked.Apply([](const Foo &foo) {
        foo.cfoo();
        foo.cfoo();
        foo.cfoo();
    });
}

TEST(LockedTest, FunctionCall) {
    mabe::Locked<Foo, MockMutex> locked;

    testing::InSequence s;
    EXPECT_CALL(locked.Mtx(), lock()).Times(1);
    EXPECT_CALL(locked.Obj(), foo()).Times(1);
    EXPECT_CALL(locked.Mtx(), unlock()).Times(1);

    locked->foo();
}

TEST(LockedTest, OperatorStar) {
    mabe::Locked<Foo, MockMutex> locked;

    testing::InSequence s;
    EXPECT_CALL(locked.Mtx(), lock()).Times(1);
    EXPECT_CALL(locked.Obj(), foo()).Times(1);
    EXPECT_CALL(locked.Mtx(), unlock()).Times(1);

    (*locked)->foo();
}

TEST(LockedTest, OperatorStarConst) {
    const mabe::Locked<Foo, MockMutex> locked;

    testing::InSequence s;
    EXPECT_CALL(locked.Mtx(), lock()).Times(1);
    EXPECT_CALL(locked.Obj(), cfoo()).Times(1);
    EXPECT_CALL(locked.Mtx(), unlock()).Times(1);

    (*locked)->cfoo();
}

TEST(LockedTest, SharedPtrMultipleOperations) {
    const auto lockedPtr = std::make_shared<mabe::Locked<Foo, MockMutex>>();

    testing::InSequence s;
    EXPECT_CALL(lockedPtr->Mtx(), lock()).Times(1);
    EXPECT_CALL(lockedPtr->Obj(), cfoo()).Times(2);
    EXPECT_CALL(lockedPtr->Mtx(), unlock()).Times(1);

    lockedPtr->Apply([](const Foo& foo) {
        foo.cfoo();
        foo.cfoo();
    });
}

TEST(LockedTest, SharedPtrFunctionCall) {
    const auto lockedPtr = std::make_shared<mabe::Locked<Foo, MockMutex>>();

    testing::InSequence s;
    EXPECT_CALL(lockedPtr->Mtx(), lock()).Times(1);
    EXPECT_CALL(lockedPtr->Obj(), cfoo()).Times(1);
    EXPECT_CALL(lockedPtr->Mtx(), unlock()).Times(1);

    (*lockedPtr)->cfoo();
}

#include <mutex>
TEST(LockedTest, StdMutexFunctionCall) {
    const auto lockedPtr = std::make_shared<mabe::Locked<Foo, std::mutex>>();

    testing::InSequence s;
    EXPECT_CALL(lockedPtr->Obj(), cfoo()).Times(1);

    (*lockedPtr)->cfoo();
}
