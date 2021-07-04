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

#include <algorithm>
#include <atomic>
#include <iostream>
#include <mabe/locked.hpp>
#include <mutex>
#include <thread>
#include <vector>

using IntVec = std::vector<int>;
using LockedIntVec = mabe::Locked<IntVec, std::mutex>;

void PrintIntVec(const IntVec &v, std::ostream &os) {
  for (const auto &val : v) {
    os << val << " ";
  }
  os << '\n';
}

using namespace std::chrono_literals;

int main() {
  std::atomic_bool exit(false);

  LockedIntVec intVec{1, 10, 3, 4, 2};

  PrintIntVec(intVec, std::cout);

  // Periodically sort the vector
  std::thread t1([&]() {
    while (!exit) {
      intVec.apply([&](IntVec &v) {
        if (v.empty())
          return;
        std::sort(v.begin(), v.end());
        PrintIntVec(v, std::cout);
      });
      std::this_thread::sleep_for(1ms);
    }
  });

  // Push a number using the apply function, exit when size > 500
  std::thread t2([&]() {
    while (!exit) {
      intVec.apply([&](IntVec &v) {
        v.push_back(v.size() + 42 % 10);
        if (v.size() > 500) {
          exit = true;
        }
        std::this_thread::sleep_for(10ms);
      });
    }
  });

  // Push a number using the arrow operator
  std::thread t3([&]() {
    auto x = 1;
    while (!exit) {
      intVec->push_back(++x);
      std::this_thread::sleep_for(1ms);
    }
  });

  t1.join();
  t2.join();
  t3.join();

  return EXIT_SUCCESS;
}
