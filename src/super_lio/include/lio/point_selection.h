#ifndef LI2SUP_POINT_SELECTION_H
#define LI2SUP_POINT_SELECTION_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace LI2Sup {

struct FiniteStrideSelection {
  std::vector<std::size_t> finite_indices;
  std::vector<std::size_t> stride_indices;
  std::vector<std::size_t> accepted_indices;
};

template <typename PointContainer>
double maxPointOffset(const PointContainer& points) {
  if (points.empty()) return 0.0;
  double maximum = -std::numeric_limits<double>::infinity();
  for (const auto& point : points) {
    maximum = std::max(maximum, point.offset_time);
  }
  return maximum;
}

template <typename PointContainer, typename FinitePredicate,
          typename ValidPredicate>
FiniteStrideSelection selectFiniteThenStride(
    const PointContainer& points, std::size_t stride,
    FinitePredicate is_finite, ValidPredicate is_valid) {
  if (stride == 0) {
    throw std::invalid_argument("finite-to-stride selection requires stride > 0");
  }

  FiniteStrideSelection selection;
  selection.finite_indices.reserve(points.size());
  for (std::size_t index = 0; index < points.size(); ++index) {
    if (is_finite(points[index])) {
      selection.finite_indices.push_back(index);
    }
  }

  selection.stride_indices.reserve(
      (selection.finite_indices.size() + stride - 1) / stride);
  for (std::size_t compact_index = 0;
       compact_index < selection.finite_indices.size(); compact_index += stride) {
    selection.stride_indices.push_back(
        selection.finite_indices[compact_index]);
  }

  selection.accepted_indices.reserve(selection.stride_indices.size());
  for (const std::size_t index : selection.stride_indices) {
    if (is_valid(points[index])) {
      selection.accepted_indices.push_back(index);
    }
  }
  return selection;
}

inline bool validPointWithinRange(double x, double y, double z,
                                  double blind_squared,
                                  double maxrange_squared) {
  if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
    return false;
  }
  const double distance_squared = x * x + y * y + z * z;
  return distance_squared > blind_squared &&
         distance_squared < maxrange_squared;
}

}  // namespace LI2Sup

#endif
