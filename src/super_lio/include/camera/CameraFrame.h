#ifndef CAMERA_FRAME_H_
#define CAMERA_FRAME_H_

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace LI2Sup {

struct CameraFrame {
  double timestamp = 0.0;
  int64_t timestamp_ns = 0;  // exact ROS stamp ns (P0R2-A audit)
  int width = 0;
  int height = 0;
  std::string encoding;
  uint64_t sequence_id = 0;
  // Bounded ownership: buffer owns (or shares) the decoded image payload.
  std::shared_ptr<const std::vector<uint8_t>> data;
};

// Bounded camera frame buffer (TB-1: input-layer only, zero estimator
// influence). Capacity and drop accounting are explicit.
class CameraBuffer {
 public:
  explicit CameraBuffer(std::size_t capacity = 10) : capacity_(capacity) {}

  void setCapacity(std::size_t c) { capacity_ = c > 0 ? c : 1; }
  std::size_t capacity() const { return capacity_; }

  bool push(const CameraFrame& frame) {
    if (frames_.empty()) {
      first_timestamp_ = frame.timestamp;
    }
    last_timestamp_ = frame.timestamp;
    while (frames_.size() >= capacity_) {
      frames_.pop_front();
      ++drop_count_;
    }
    frames_.push_back(frame);
    if (frames_.size() > peak_size_) peak_size_ = frames_.size();
    return true;
  }

  void clear() {
    frames_.clear();
    first_timestamp_ = 0.0;
    last_timestamp_ = 0.0;
    peak_size_ = 0;
  }

  std::size_t size() const { return frames_.size(); }
  std::size_t dropped() const { return drop_count_; }
  std::size_t peakSize() const { return peak_size_; }
  bool empty() const { return frames_.empty(); }
  double firstTimestamp() const { return first_timestamp_; }
  double lastTimestamp() const { return last_timestamp_; }
  const CameraFrame& oldest() const { return frames_.front(); }
  void popOldest() {
    if (!frames_.empty()) frames_.pop_front();
  }
  const CameraFrame& newest() const { return frames_.back(); }

 private:
  std::size_t capacity_;
  std::deque<CameraFrame> frames_;
  std::size_t drop_count_ = 0;
  std::size_t peak_size_ = 0;
  double first_timestamp_ = 0.0;
  double last_timestamp_ = 0.0;
};

}  // namespace LI2Sup

#endif  // CAMERA_FRAME_H_