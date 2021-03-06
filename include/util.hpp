#ifndef WILLIAM_KILLIAN_UTIL_HPP_
#define WILLIAM_KILLIAN_UTIL_HPP_

#include <algorithm>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <type_traits>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace util {

template <typename T>
struct safe_queue {

  template <typename U>
  void enqueue (U && u) {
    std::lock_guard lock{m};
    q.push(std::forward<U>(u));
    c.notify_one();
  }

  T dequeue () {
    std::unique_lock lock{m};
    c.wait(lock, [&] { return !q.empty(); });
    T val = q.front();
    q.pop();
    return val;
  }

  bool empty() const {
    return q.empty();
  }

private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
};

template <unsigned int I, typename TupleLike>
struct View {
  std::tuple_element_t<I, TupleLike> val;
  View(TupleLike v) : val(std::get<I>(v)) {}
  bool operator<(const View &o) const { return val < o.val; }
};

std::string
knot_hash(std::string const & str);

std::array<bool, 128>
hash2array(std::string const & hash);

template <typename T, typename HashFn = std::hash<T>>
struct disjoint_set {
  struct set {
    T id;
    int size;
  };

  T find(T t) {
    return find_impl(t).id;
  }

  void join(T t1, T t2) {
    auto & p1 = find_impl(t1);
    auto & p2 = find_impl(t2);
    if (&p1 == &p2)
      return;
    --distinct;
    p1.id = p2.id;
    p2.size += p1.size;
  }

  int size_of(T t) {
    return find_impl(t).size;
  }

  int size() const {
    return total_size;
  }

  int sets() const {
    return distinct;
  }

private:

  set& find_impl (T t) {
    set& s = table[t];
    if (s.size == 0) {
      ++distinct;
      ++total_size;
      return s = {t, 1};
    }
    if (s.id == t)
      return s;
    set& s1 = find_impl(s.id);
    s.id = s1.id;
    return s1;
  }

  std::unordered_map<T,set, HashFn> table;
  int distinct {0};
  int total_size {0};
};

template <int N = -1, typename Fn, typename... Args>
void
parallel_do(Fn&& f, Args&&... args)
{
  std::vector<std::thread> threads;
  const int groupSize((N == -1) ? std::thread::hardware_concurrency() : N);
  for (int idx{0}; idx < groupSize; ++idx)
    threads.emplace_back(f, idx, groupSize, std::forward<Args>(args)...);
  for (auto& t : threads)
    t.join();
}

int
htoi(char);

char
itoh(int);

int
fast_itoa(int, char*);

class combination {
  bool              cont{true};
  std::vector<bool> v;

  public:
  combination(size_t n, size_t r)
    : v(r, true)
  {
    v.resize(n, false);
  }

  bool
  next(std::vector<size_t>& res)
  {
    if (!cont)
      return false;
    for (size_t i{0}, j{0}; i < v.size(); ++i)
      if (v[i])
        res[j++] = i;
    cont = std::prev_permutation(std::begin(v), std::end(v));
    return true;
  }
};

std::vector<std::string>
split(const std::string& text, const std::string& delims);

constexpr uint64_t
hash(const char* str, uint64_t ret = 0x0ull)
{
  return (*str) ? hash(str + 1, (ret << 8) | *(str + 1)) : ret;
}

constexpr uint64_t
hash(const std::string& str)
{
  return hash(str.c_str());
}
}

constexpr uint64_t operator"" _hash(const char* p, size_t)
{
  return util::hash(p);
}

#endif
