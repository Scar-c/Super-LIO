#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>

#include "geometry/BilinearSample.h"

using namespace LI2Sup;

static std::vector<uint8_t> makeImage(int W, int H,
                                      const std::function<double(double, double)>& f) {
  std::vector<uint8_t> img(W * H);
  for (int j = 0; j < H; ++j) {
    for (int i = 0; i < W; ++i) {
      double v = f(i, j);
      img[static_cast<size_t>(j) * W + i] =
          static_cast<uint8_t>(std::min(255.0, std::max(0.0, v)));
    }
  }
  return img;
}

int main() {
  const int W = 64, H = 48;

  // 9.1 constant image
  {
    auto img = makeImage(W, H, [](double, double) { return 128.0; });
    BilinearSample s = sampleBilinearWithGradient(img, W, H, 10.3, 12.7);
    assert(s.valid);
    assert(std::abs(s.value - 128.0) < 1e-9);
    assert(s.du == 0.0 && s.dv == 0.0);
  }

  // 9.2 affine image: I = a x + b y + c -> Iu = a, Iv = b
  // (integer coefficients so uint8 quantization is lossless)
  {
    const double a = 2.0, b = 1.0, c = 30.0;
    auto img = makeImage(
        W, H, [&](double x, double y) { return a * x + b * y + c; });
    for (double u = 5.5; u < W - 2; u += 7.3) {
      for (double v = 5.5; v < H - 2; v += 5.1) {
        BilinearSample s = sampleBilinearWithGradient(img, W, H, u, v);
        assert(s.valid);
        // affine bilinear interpolant is exact -> value and derivatives exact
        assert(std::abs(s.du - a) < 1e-6);
        assert(std::abs(s.dv - b) < 1e-6);
        assert(std::abs(s.value - (a * u + b * v + c)) < 1e-6);
      }
    }
  }

  // 9.3 single cell hand oracle
  {
    std::vector<uint8_t> img(W * H, 0);
    const double I00 = 10, I10 = 30, I01 = 50, I11 = 90;
    img[5 * W + 5] = static_cast<uint8_t>(I00);
    img[5 * W + 6] = static_cast<uint8_t>(I10);
    img[6 * W + 5] = static_cast<uint8_t>(I01);
    img[6 * W + 6] = static_cast<uint8_t>(I11);
    const double alphas[3] = {0.25, 0.5, 0.75};
    const double betas[3] = {0.3, 0.5, 0.8};
    for (double al : alphas) {
      for (double be : betas) {
        BilinearSample s =
            sampleBilinearWithGradient(img, W, H, 5.0 + al, 5.0 + be);
        assert(s.valid);
        const double a1 = 1 - al, b1 = 1 - be;
        const double expect =
            a1 * b1 * I00 + al * b1 * I10 + a1 * be * I01 + al * be * I11;
        const double expdu = b1 * (I10 - I00) + be * (I11 - I01);
        const double expdv = a1 * (I01 - I00) + al * (I11 - I10);
        assert(std::abs(s.value - expect) < 1e-9);
        assert(std::abs(s.du - expdu) < 1e-9);
        assert(std::abs(s.dv - expdv) < 1e-9);
      }
    }
  }

  // 9.4 central difference inside one cell (same bilinear value function)
  {
    const double a = 1.0, b = 1.0, c = 10.0;
    auto img = makeImage(
        W, H, [&](double x, double y) { return a * x + b * y + c; });
    const double u = 20.3, v = 15.4;  // far from integer boundaries
    const double eps = 1e-3;
    const double up = u + eps, um = u - eps;
    const double vp = v + eps, vm = v - eps;
    assert(std::floor(up) == std::floor(um));
    assert(std::floor(vp) == std::floor(vm));
    const double Iu =
        (sampleBilinearWithGradient(img, W, H, up, v).value -
         sampleBilinearWithGradient(img, W, H, um, v).value) /
        (2 * eps);
    const double Iv =
        (sampleBilinearWithGradient(img, W, H, u, vp).value -
         sampleBilinearWithGradient(img, W, H, u, vm).value) /
        (2 * eps);
    BilinearSample s = sampleBilinearWithGradient(img, W, H, u, v);
    assert(std::abs(s.du - Iu) < 1e-6);
    assert(std::abs(s.dv - Iv) < 1e-6);
    assert(std::abs(s.du - a) < 1e-6);
    assert(std::abs(s.dv - b) < 1e-6);
  }

  // 9.5 border validity: no out-of-bounds four-pixel footprint, no clamping
  {
    std::vector<uint8_t> img(W * H, 100);
    for (double u = -1.0; u < W + 1.0; u += 0.3) {
      for (double v = -1.0; v < H + 1.0; v += 0.3) {
        const bool inb = (u >= 0.0 && u < W - 1.0 && v >= 0.0 && v < H - 1.0);
        BilinearSample s = sampleBilinearWithGradient(img, W, H, u, v);
        assert(s.valid == inb);
      }
    }
  }

  std::printf("all bilinear primitive tests passed\n");
  return 0;
}