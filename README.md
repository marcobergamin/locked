[![CMake](https://github.com/marcobergamin/locked/actions/workflows/cmake.yml/badge.svg)](https://github.com/marcobergamin/locked/actions/workflows/cmake.yml)

# Locked

Helper class to make an object thread-safe without breaking the Open-Close principle.\
Requires a C++11 compiler, if the compiler supports also C++17 standards supports also std::shared_mutex.

## Basic example
Given this structures:
```cpp
struct MyLocker {
    void lock();
    void unlock();
};

struct MyResource {
    void foo();
    void bar();
};
```

### Single operation
```cpp
mabe::Locked<MyResource, MyLocker> lockedResource;

lockedResource->foo();
```

The resulting sequence of operations is
```cpp
lockedResource.mtx_.lock();
lockedResource.obj_.foo();
lockedResource.mtx_.unlock();
```

### Sequence of operations
```cpp
mabe::Locked<MyResource, MyLocker> lockedResource;

lockedResource.Apply([](MyResource &res) {
    res.foo();
    res.bar();
});
```

The resulting sequence of operations is
```cpp
lockedResource.mtx_.lock();
lockedResource.obj_.foo();
lockedResource.obj_.bar();
lockedResource.mtx_.unlock();
```

## Example with shared mutex
When `std::shared_mutex` is available (since C++17), `Locked` provides support for multiple concurrent read operations.
```cpp
struct Foo {
    MOCK_METHOD(void, foo, ());
    MOCK_METHOD(void, cfoo, (), (const));
};

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
    EXPECT_CALL(lockedFoo.Mtx(), lock()).Times(1);
    EXPECT_CALL(lockedFoo.Obj(), foo()).Times(1);
    EXPECT_CALL(lockedFoo.Mtx(), unlock()).Times(1);
    lockedFoo->foo();

    // Expects calls to lock_shared/unlock_shared on const objects
    EXPECT_CALL(lockedFoo.Mtx(), lock_shared()).Times(1);
    EXPECT_CALL(lockedFoo.Obj(), cfoo()).Times(1);
    EXPECT_CALL(lockedFoo.Mtx(), unlock_shared()).Times(1);
    std::as_const(lockedFoo)->cfoo();
}
```
