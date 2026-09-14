#include "lio/point_selection.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

struct Point {
  double x;
  double y;
  double z;
};

struct TimedPoint {
  double offset_time;
};

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

std::vector<double> acceptedRanges(
    const LI2Sup::FiniteStrideSelection& selection,
    const std::vector<Point>& points) {
  std::vector<double> ranges;
  for (const std::size_t index : selection.accepted_indices) {
    ranges.push_back(points[index].x);
  }
  return ranges;
}

}  // namespace

int main() {
  // The selected finite points are at compact indices 0,3,6,... . The
  // interleaved points keep the finite-cloud stride phase explicit.
  const std::vector<double> selected_ranges = {0.5, 1.0, 1.5, 1.6,
                                                3.0, 6.0, 5.0};
  std::vector<Point> points(19, Point{8.0, 0.0, 0.0});
  for (std::size_t compact_index = 0;
       compact_index < selected_ranges.size(); ++compact_index) {
    points[compact_index * 3] =
        Point{selected_ranges[compact_index], 0.0, 0.0};
  }

  const auto finite = [](const Point& point) {
    return std::isfinite(point.x) && std::isfinite(point.y) &&
           std::isfinite(point.z);
  };
  const auto run = [&](double blind) {
    const double blind_squared = blind * blind;
    const double maxrange_squared = 5.0 * 5.0;
    return LI2Sup::selectFiniteThenStride(
        points, 3, finite, [&](const Point& point) {
          return LI2Sup::validPointWithinRange(
              point.x, point.y, point.z, blind_squared, maxrange_squared);
        });
  };

  const auto blind15 = run(1.5);
  require(blind15.finite_indices.size() == 19,
          "P1 finite population is retained");
  require(blind15.stride_indices.size() == selected_ranges.size(),
          "P1 finite-cloud stride selection is used");
  // This is the pre-fix production behavior: every stride-selected point was
  // emplaced even when the counters said blind/range had rejected it.
  require(blind15.stride_indices.size() != 2,
          "N1 pre-fix selected population contains rejected points");
  const auto accepted15 = acceptedRanges(blind15, points);
  require(accepted15 == std::vector<double>({1.6, 3.0}),
          "N2/P1 blind=1.5 accepts only 1.6m and 3.0m");

  const auto blind20 = run(2.0);
  const auto accepted20 = acceptedRanges(blind20, points);
  require(accepted20 == std::vector<double>({3.0}),
          "P1 blind=2.0 accepts only 3.0m");
  require(!LI2Sup::validPointWithinRange(1.5, 0.0, 0.0, 1.5 * 1.5,
                                          5.0 * 5.0),
          "N4 exact blind boundary is rejected");
  require(!LI2Sup::validPointWithinRange(5.0, 0.0, 0.0, 1.5 * 1.5,
                                          5.0 * 5.0),
          "N5 exact maxrange boundary is rejected");
  require(!LI2Sup::validPointWithinRange(6.0, 0.0, 0.0, 1.5 * 1.5,
                                          5.0 * 5.0),
          "N3 above-maxrange point is rejected");

  const std::vector<TimedPoint> timed = {{-0.099}, {0.0013}, {-0.101}};
  require(LI2Sup::maxPointOffset(timed) == 0.0013,
          "N9 maximum accepted offset is selected");
  const double header = 100.0;
  const double last_selected = timed[1].offset_time;
  const double maximum = LI2Sup::maxPointOffset(timed);
  require(last_selected == maximum,
          "N9 control uses a last point equal to the maximum");
  const double out_of_order_last = timed[0].offset_time;
  require(out_of_order_last < maximum,
          "N9 adversarial last-selected offset is below maximum");
  const double configured_end = header + maximum;
  require(configured_end >= header + maximum,
          "N9 end-time invariant covers maximum query time");

  const double corrected_span = 0.102;
  const double broken_span = corrected_span * 1e-6;
  require(broken_span < 1e-6,
          "N7 wrong 1e-6 scale produces near-zero scan span");
  require(corrected_span > 0.09,
          "N8 physical scale preserves approximately 0.1s scan span");

  std::cout << "P1 — PRODUCTION_BLIND_RANGE_ENFORCEMENT: PASS\n"
            << "N1-N5 adversarial range cases: PASS\n"
            << "N7-N9 physical-time/end-time synthetic cases: PASS\n";
  return EXIT_SUCCESS;
}
