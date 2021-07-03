[![CMake](https://github.com/marcobergamin/locked/actions/workflows/cmake.yml/badge.svg)](https://github.com/marcobergamin/locked/actions/workflows/cmake.yml)

# Locked

Helper class to make a class thread-safe without breaking the Open-Close principle.\
Requires a C++11 compiler. If the compiler supports the C++17 standard, it allows multiple thread-safe read operations
with specific optimizations for `std::shared_mutex`.

## Use the library
Locked is a header only library. The header can be simply copied in your project, or the whole project can be added as 
cmake subdirectory. For example: 
```cmake
# Add locked as CMake subdirectory
add_subdirectory(locked)

# Define a target 'foo'
add_executable(${foo_name} ${foo_srcs})

# Link mabe::locked to your target
target_link_libraries(${foo_name} mabe::locked)
```

## Basic example

Given this structures:

```cpp
struct MyMutex {
    void lock();
    void unlock();
};

struct MyClass {
    void foo();
    void bar();
};
```

### Single operation

```cpp
mabe::Locked<MyClass, MyMutex> lockedInstance;

lockedInstance->foo();
```

The resulting sequence of operations is

```cpp
lockedInstance.mtx_.lock();
lockedInstance.obj_.foo();
lockedInstance.mtx_.unlock();
```

### Sequence of operations

```cpp
mabe::Locked<MyClass, MyMutex> lockedInstance;

lockedInstance.apply([](MyClass &c) {
    c.foo();
    c.bar();
});
```

The resulting sequence of operations is

```cpp
lockedInstance.mtx_.lock();
lockedInstance.obj_.foo();
lockedInstance.obj_.bar();
lockedInstance.mtx_.unlock();
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
    EXPECT_CALL(lockedFoo.mtx(), lock()).Times(1);
    EXPECT_CALL(lockedFoo.obj(), foo()).Times(1);
    EXPECT_CALL(lockedFoo.mtx(), unlock()).Times(1);
    lockedFoo->foo();

    // Expects calls to lock_shared/unlock_shared on const objects
    EXPECT_CALL(lockedFoo.mtx(), lock_shared()).Times(1);
    EXPECT_CALL(lockedFoo.obj(), cfoo()).Times(1);
    EXPECT_CALL(lockedFoo.mtx(), unlock_shared()).Times(1);
    std::as_const(lockedFoo)->cfoo();
}
```
