// Round 11R: synthetic two-bag merge unit test — rosbag::View.addQuery must
// produce one chronologically ordered stream, each message exactly once.
#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <rosbag/query.h>

int main() {
  rosbag::Bag ba, bb;
  ba.open("/tmp/opencode/tb0/mb_a.bag", rosbag::bagmode::Read);
  bb.open("/tmp/opencode/tb0/mb_b.bag", rosbag::bagmode::Read);
  rosbag::View view;
  view.addQuery(ba, rosbag::TopicQuery("/topic_a"));
  view.addQuery(bb, rosbag::TopicQuery("/topic_b"));
  std::vector<double> times;
  for (const rosbag::MessageInstance& mi : view) {
    times.push_back(mi.getTime().toSec());
  }
  ba.close();
  bb.close();
  std::printf("merged n=%zu times:", times.size());
  for (double t : times) std::printf(" %.1f", t);
  std::printf("\n");
  // P1: 4 messages total (each once)
  assert(times.size() == 4);
  // P2: globally non-decreasing
  for (size_t i = 1; i < times.size(); ++i) assert(times[i] >= times[i - 1]);
  // P3: exact interleaved order 1,2,3,4
  assert(times[0] == 1.0 && times[1] == 2.0 && times[2] == 3.0 && times[3] == 4.0);
  std::printf("MULTI-BAG MERGE TDD: ALL PASS\n");
  return 0;
}