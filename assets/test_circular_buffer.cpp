
// build with `g++ -std=c++17 -O2 -DNDEBUG test_circular_buffer.cpp`

#include <algorithm>
#include <boost/circular_buffer.hpp>
#include <chrono>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

constexpr size_t buffer_size = 1024 * 1024 * 4;

void print_result(const std::string &title,
                  const std::vector<std::chrono::microseconds> &costs) {
  const auto [min, max] = std::minmax_element(costs.begin(), costs.end());
  auto avg = std::accumulate(costs.begin(), costs.end(),
                             std::chrono::microseconds::zero()) /
             costs.size();
  std::cout << title << " cost(min/max/avg): " << min->count() << "/"
            << max->count() << "/" << avg.count() << " ms\n";
}

template <class F>
std::vector<std::chrono::microseconds> run_test(F func, size_t times) {
  std::vector<std::chrono::microseconds> costs;

  std::string a;
  a.resize(buffer_size);

  for (int i = 0; i < 10; ++i) {
    auto start = std::chrono::steady_clock::now();
    func(a);
    auto end = std::chrono::steady_clock::now();

    costs.push_back(std::chrono::round<std::chrono::microseconds>(end - start));
  }
  return costs;
}

int main() {
  boost::circular_buffer<char> buffer(buffer_size);

  buffer.insert(buffer.begin(), buffer_size, 'a');
  std::string a("bcd");
  buffer.insert(buffer.end(), a.begin(), a.begin() + 3);
  std::cout << buffer.is_linearized() << "\n";
  std::cout << buffer.back() << "\n";
  return 0;

  std::cout << "capacity/size: " << buffer.capacity() << "/" << buffer.size()
            << "\n";
  std::cout << "is_linearized: " << buffer.is_linearized() << "\n";

  auto copy_by_array = [&buffer](std::string &a) {
    auto ar = buffer.array_one();
    std::copy(ar.first, ar.first + ar.second, a.data());
  };

  auto copy_by_iterator = [&buffer](std::string &a) {
    std::copy(buffer.begin(), buffer.end(), a.data());
  };

  std::vector<std::chrono::microseconds> array_costs;
  std::vector<std::chrono::microseconds> iter_costs;
  for (int i = 0; i < 10; ++i) {
    auto costs = run_test(copy_by_iterator, 10);
    iter_costs.insert(iter_costs.end(), costs.begin(), costs.end());
    costs = run_test(copy_by_array, 10);
    array_costs.insert(array_costs.end(), costs.begin(), costs.end());
  }

  print_result("copy with c array", array_costs);
  print_result("copy with iterator", iter_costs);
}
