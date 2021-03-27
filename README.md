[![CMake](https://github.com/marcobergamin/locked/actions/workflows/cmake.yml/badge.svg)](https://github.com/marcobergamin/locked/actions/workflows/cmake.yml)

# Locker

Simple helper class to lock a resource protecting the access.

## Example
Given this structures:
```cpp
struct MyLocker {
    void lock();
    void unlock();
};

struct MyResource {
    void init();
    void loadSettings();
};
```

### Single operation
```cpp
mabe::Locked<MyResource, MyLocker> lockedResource;

lockedResource->init();
```

The resulting sequence of operations is
```cpp
lockedResource.mtx_.lock();
lockedResource.obj_.init();
lockedResource.mtx_.unlock();
```

### Sequence of operations
```cpp
mabe::Locked<MyResource, MyLocker> lockedResource;

lockedResource.Apply([](MyResource &res) {
    res.init();
    res.loadSettings();
});
```

The resulting sequence of operations is
```cpp
lockedResource.mtx_.lock();
lockedResource.obj_.init();
lockedResource.obj_.loadSettings();
lockedResource.mtx_.unlock();
```

