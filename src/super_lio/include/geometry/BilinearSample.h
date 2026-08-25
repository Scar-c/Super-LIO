#ifndef BILINEAR_SAMPLE_H_
#define BILINEAR_SAMPLE_H_

#include <cmath>
#include <cstdint>
#include <vector>

namespace LI2Sup {

// V-0C exact bilinear sample-with-gradient primitive (Owner prompt §5).
// value and (du, dv) are the exact value and exact derivatives of the SAME
// bilinear interpolant, using the SAME four neighbors, SAME alpha/beta and
// SAME validity decision. Used by both the photometric residual and the
// analytic image gradient — never a separately "sampled gradient image".

struct BilinearSample {
  double value = 0.0;
  double du = 0.0;
  double dv = 0.0;
  bool valid = false;
};

inline BilinearSample sampleBilinearWithGradient(const std::vector<uint8_t>& img,
                                                 int W, int H, double u,
                                                 double v) {
  BilinearSample out;
  const int i = static_cast<int>(std::floor(u));
  const int j = static_cast<int>(std::floor(v));
  // dense camera image: four bilinear neighbors must exist (no clamping)
  if (i < 0 || i >= W - 1 || j < 0 || j >= H - 1) return out;
  const double alpha = u - i;
  const double beta = v - j;
  const double I00 = img[static_cast<size_t>(j) * W + i];
  const double I10 = img[static_cast<size_t>(j) * W + (i + 1)];
  const double I01 = img[static_cast<size_t>(j + 1) * W + i];
  const double I11 = img[static_cast<size_t>(j + 1) * W + (i + 1)];
  const double b1 = 1.0 - beta, a1 = 1.0 - alpha;
  out.value = a1 * b1 * I00 + alpha * b1 * I10 + a1 * beta * I01 +
              alpha * beta * I11;
  // exact derivatives of the interpolant (not a gradient-image sample)
  out.du = b1 * (I10 - I00) + beta * (I11 - I01);
  out.dv = a1 * (I01 - I00) + alpha * (I11 - I10);
  out.valid = true;
  return out;
}

}  // namespace LI2Sup

#endif  // BILINEAR_SAMPLE_H_