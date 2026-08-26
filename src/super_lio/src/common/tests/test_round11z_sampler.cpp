// Round11Z Z-T1..T4/T12: camera temporal sampler semantics (pure helper).
#include <cstdint>
#include <cstdio>

#include "common/CadencePolicy.h"

using LI2Sup::temporalStrideAccept;

int main() {
  bool ok = true;
  auto expect = [&](const char* name, bool cond) {
    std::printf("%s: %s\n", name, cond ? "PASS" : "FAIL");
    if (!cond) ok = false;
  };

  // Z-T1: default stride=1 accepts all
  {
    int64_t counter = 0;
    int accepted = 0;
    for (int i = 0; i < 10; ++i) {
      if (temporalStrideAccept(counter, 1)) ++accepted;
    }
    expect("Z-T1 stride=1 accepts all", counter == 10 && accepted == 10);
  }

  // Z-T2: stride=3 accepts 3,6,9,... (increment-before-modulo)
  {
    int64_t counter = 0;
    int accepted[10] = {0};
    int n = 0;
    for (int i = 1; i <= 9; ++i) {
      if (temporalStrideAccept(counter, 3)) accepted[n++] = i;
    }
    expect("Z-T2 stride=3 accepts 3,6,9", n == 3 && accepted[0] == 3 &&
               accepted[1] == 6 && accepted[2] == 9);
  }

  // Z-T3: zero/negative stride fails closed (never accepts)
  {
    int64_t counter = 0;
    int accepted = 0;
    for (int i = 0; i < 10; ++i) {
      if (temporalStrideAccept(counter, 0)) ++accepted;
    }
    expect("Z-T3 stride=0 fails closed", accepted == 0);
    int64_t c2 = 0;
    int accepted2 = 0;
    for (int i = 0; i < 10; ++i) {
      if (temporalStrideAccept(c2, -2)) ++accepted2;
    }
    expect("Z-T3 stride=-2 fails closed", accepted2 == 0);
  }

  // Z-T4: raw = decimated + accepted
  {
    int64_t counter = 0;
    int accepted = 0;
    int decimated = 0;
    for (int i = 0; i < 9736; ++i) {
      if (temporalStrideAccept(counter, 3)) ++accepted;
      else ++decimated;
    }
    expect("Z-T4 raw=decimated+accepted",
           counter == 9736 && accepted + decimated == 9736);
    // canonical Day10 expectation: floor(9736/3) = 3245
    expect("Z-T4 stride=3 accepted == floor(9736/3)", accepted == 3245);
  }

  // Z-T12: caller-owned counter; two instances do not share state
  {
    int64_t a = 0, b = 0;
    temporalStrideAccept(a, 3);  // a=1 no
    temporalStrideAccept(b, 3);  // b=1 no
    temporalStrideAccept(a, 3);  // a=2 no
    temporalStrideAccept(a, 3);  // a=3 yes
    temporalStrideAccept(b, 3);  // b=2 no
    int64_t ra = a, rb = b;
    expect("Z-T12 independent counters",
           ra == 3 && rb == 2);
  }

  std::printf("Z-T1..T4/T12: %s\n", ok ? "ALL PASS" : "FAIL");
  return ok ? 0 : 1;
}
