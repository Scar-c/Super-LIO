// Prob-LIO G-P2.C2 evidence: TBB-style parallel counter race.
//
// Compiled by tools/prob_lio/run_race_evidence.sh with -fsanitize=thread:
//   - DEFAULT  : the NEW pattern (std::atomic relaxed increments) -> clean.
//   - -DBAD    : the OLD pattern (plain uint64_t shared RMW) -> TSAN reports
//                a data race (the race the P2 corrective removed).
// Both patterns are exercised inside a tbb::parallel_for-like section.

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#ifdef BAD
static std::uint64_t g_counter = 0;  // old: shared non-atomic RMW
#else
static std::atomic<std::uint64_t> g_counter{0};  // new: relaxed atomic
#endif

int main() {
  constexpr int kIter = 400000;
  tbb::parallel_for(tbb::blocked_range<int>(0, kIter),
                    [&](const tbb::blocked_range<int>& r) {
                      for (int i = r.begin(); i < r.end(); ++i) {
#ifdef BAD
                        g_counter++;  // data race
#else
                        g_counter.fetch_add(1, std::memory_order_relaxed);
#endif
                      }
                    });
#ifdef BAD
  std::printf("counter=%llu expected=%d\n", (unsigned long long)g_counter,
              kIter);
#else
  std::printf("counter=%llu expected=%d\n",
              (unsigned long long)g_counter.load(std::memory_order_relaxed),
              kIter);
#endif
#ifdef BAD
  std::printf("MODE=BAD(legacy)\n");
#else
  std::printf("MODE=GOOD(atomic)\n");
#endif
  return 0;
}
