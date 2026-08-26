#include "offline/OfflineReader.h"

#include <chrono>
#include <cstdio>

#include <livox_ros_driver/CustomMsg.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CompressedImage.h>
#include <csetjmp>
#include <jpeglib.h>

namespace LI2Sup {

namespace {

double nowMs() {
  return std::chrono::duration<double, std::milli>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

long rssKb() {
  FILE* f = fopen("/proc/self/statm", "r");
  if (!f) return -1;
  long total = 0, resident = 0;
  if (fscanf(f, "%ld %ld", &total, &resident) != 2) resident = -1;
  fclose(f);
  const long page_kb = sysconf(_SC_PAGESIZE) / 1024;
  return resident > 0 ? resident * page_kb : -1;
}

}  // namespace

bool OfflineReader::open(const OfflineOptions& opts) {
  opts_ = opts;
  return true;
}

bool OfflineReader::run(ROSWrapper& wrapper, SuperLIO& lio) {
  if (opts_.bag_path.empty() && opts_.bag_paths.empty()) {
    std::printf("[OfflineReader] ERROR: empty bag path\n");
    return false;
  }

  std::vector<std::string> bag_paths = opts_.bag_paths.empty()
                                           ? std::vector<std::string>{opts_.bag_path}
                                           : opts_.bag_paths;
  if (bag_paths.size() == 1 && bag_paths[0].empty()) {
    std::printf("[OfflineReader] ERROR: empty bag path\n");
    return false;
  }

  double start = opts_.start_offset;
  double end = -1.0;
  if (opts_.duration > 0.0 && opts_.start_offset >= 0.0) {
    end = opts_.start_offset + opts_.duration;
  }

  std::vector<std::string> topics = {opts_.lidar_topic, opts_.imu_topic};
  if (!opts_.camera_topic.empty()) topics.push_back(opts_.camera_topic);
  rosbag::TopicQuery query(topics);

  // ROS1 native multi-bag View: addQuery(Bag, query) merges all bags into
  // one MessageInstance stream ordered by getTime(). Bag time is used only
  // for read order/cropping; sensor physical time stays header-based.
  std::vector<rosbag::Bag> bags;
  rosbag::View view;
  bags.reserve(bag_paths.size());
  for (const auto& bp : bag_paths) {
    rosbag::Bag b;
    try {
      b.open(bp, rosbag::bagmode::Read);
    } catch (const std::exception& e) {
      std::printf("[OfflineReader] ERROR: cannot open bag %s: %s\n",
                  bp.c_str(), e.what());
      return false;
    }
    bags.push_back(std::move(b));
    try {
      std::printf("[OfflineReader] query bag %s -> lidar='%s' imu='%s' cam='%s'\n",
                  bp.c_str(), opts_.lidar_topic.c_str(), opts_.imu_topic.c_str(),
                  opts_.camera_topic.c_str());
      view.addQuery(bags.back(), rosbag::TopicQuery(topics));
    } catch (const std::exception& err) {
      std::printf("[OfflineReader] ERROR: view query failed for %s: %s\n",
                  bp.c_str(), err.what());
      return false;
    }
  }
  const double view_start = view.getBeginTime().toSec();
  const double view_end = view.getEndTime().toSec();
  double s = start >= 0.0 ? view_start + start : view_start;
  double e = end > 0.0 ? view_start + end : view_end;

  auto jpegDecodeGray = [](const std::vector<uint8_t>& jpg, int& w, int& h,
                            std::vector<uint8_t>& gray) -> bool {
    struct ErrMgr {
      jpeg_error_mgr pub;
      jmp_buf setjmp_buffer;
    };
    ErrMgr jerr;
    jpeg_decompress_struct cinfo;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = [](j_common_ptr cinfo) {
      ErrMgr* e = reinterpret_cast<ErrMgr*>(cinfo->err);
      longjmp(e->setjmp_buffer, 1);
    };
    if (setjmp(jerr.setjmp_buffer)) {
      jpeg_destroy_decompress(&cinfo);
      return false;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, jpg.data(), jpg.size());
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
      jpeg_destroy_decompress(&cinfo);
      return false;
    }
    cinfo.out_color_space = JCS_GRAYSCALE;
    if (!jpeg_start_decompress(&cinfo)) {
      jpeg_destroy_decompress(&cinfo);
      return false;
    }
    w = cinfo.output_width;
    h = cinfo.output_height;
    gray.resize(static_cast<size_t>(w) * h);
    while (cinfo.output_scanline < h) {
      JSAMPROW row = gray.data() + static_cast<size_t>(cinfo.output_scanline) * w;
      jpeg_read_scanlines(&cinfo, &row, 1);
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return true;
  };

  const std::string dt_imu = "sensor_msgs/Imu";
  const std::string dt_custom = "livox_ros_driver/CustomMsg";
  const std::string dt_pc2 = "sensor_msgs/PointCloud2";
  const std::string dt_image = "sensor_msgs/Image";

  double t0 = nowMs();
  double t_dispatch_ms = 0.0;
  double t_compute_ms = 0.0;
  bool first = true;
  int last_diag_sync_ = 0;
  const int kDiagEpochInterval = 500;
  for (const rosbag::MessageInstance& mi : view) {
    const double t_d0 = nowMs();
    const std::string& topic = mi.getTopic();
    const double rec_s = mi.getTime().toSec();
    if (rec_s < s || rec_s > e) continue;  // bag-time crop only
    t_dispatch_ms += nowMs() - t_d0;
    const std::string& dt = mi.getDataType();
    const ros::Time rec_time = mi.getTime();

    accounting_.bag_relevant_messages++;
    if (wrapper.syncCount() >= kDiagEpochInterval &&
        (wrapper.syncCount() % kDiagEpochInterval) == 0 &&
        wrapper.syncCount() != last_diag_sync_) {
      last_diag_sync_ = wrapper.syncCount();
      std::printf(
          "[diag] sensor_time=%.3f epochs=%d wall=%.3fs imu_consumed=%zu "
          "lidar_consumed=%zu imu_depth=%zu lidar_depth=%zu voxels=%zu "
          "rss=%ldKB\n",
          wrapper.lastSyncedLidarEndTime(), wrapper.syncCount(),
          (nowMs() - t0) / 1000.0, accounting_.imu_dispatched,
          accounting_.lidar_dispatched, wrapper.imuBufferSize(),
          wrapper.lidarBufferSize(), lio.mapVoxelCount(), rssKb());
    }
    if (first) {
      accounting_.first_bag_time = rec_time.toSec();
      first = false;
    }
    accounting_.last_bag_time = rec_time.toSec();


    if (!opts_.camera_topic.empty() && topic == opts_.camera_topic &&
        dt == dt_image) {
      auto msg = mi.instantiate<sensor_msgs::Image>();
      if (msg) {
        accounting_.images_read++;
        if (accounting_.images_dispatched == 0) {
          accounting_.first_image_time = msg->header.stamp.toSec();
        }
        accounting_.last_image_time = msg->header.stamp.toSec();
        wrapper.HandleImage(msg);
        accounting_.images_dispatched++;
        const double t_c0 = nowMs();
        const double t_c1 = nowMs();
        lio.process();
        t_compute_ms += nowMs() - t_c1;
        t_compute_ms += nowMs() - t_c1;
        accounting_.process_invocations++;
        continue;
      }
    }
    // Round 11Q: compressed camera adapter (M3DGR /camera/color/image_raw/compressed)
    if (!opts_.camera_topic.empty() && topic == opts_.camera_topic &&
        dt == "sensor_msgs/CompressedImage") {
      auto msg = mi.instantiate<sensor_msgs::CompressedImage>();
      if (msg) {
        int jw = 0, jh = 0;
        std::vector<uint8_t> gray;
        if (jpegDecodeGray(msg->data, jw, jh, gray) && jw > 0 && jh > 0) {
          sensor_msgs::Image im;
          im.header = msg->header;
          im.width = static_cast<uint32_t>(jw);
          im.height = static_cast<uint32_t>(jh);
          im.encoding = "mono8";
          im.step = static_cast<uint32_t>(jw);
          im.data = gray;
          accounting_.images_read++;
          if (accounting_.images_dispatched == 0) {
            accounting_.first_image_time = msg->header.stamp.toSec();
          }
          accounting_.last_image_time = msg->header.stamp.toSec();
          wrapper.HandleImage(
              boost::make_shared<sensor_msgs::Image>(im));
          accounting_.images_dispatched++;
        const double t_c2 = nowMs();
        lio.process();
        t_compute_ms += nowMs() - t_c2;
          accounting_.process_invocations++;
          continue;
        }
        accounting_.images_malformed++;
      }
    }

    if (topic == opts_.imu_topic && dt == dt_imu) {
      auto msg = mi.instantiate<sensor_msgs::Imu>();
      if (msg) {
        accounting_.imu_read++;
        if (accounting_.imu_dispatched == 0) {
          accounting_.first_sensor_time = msg->header.stamp.toSec();
        }
        accounting_.last_sensor_time = msg->header.stamp.toSec();
        wrapper.HandleImu(msg);
        accounting_.imu_dispatched++;
        const double t_c3 = nowMs();
        lio.process();
        t_compute_ms += nowMs() - t_c3;
        accounting_.process_invocations++;
        continue;
      }
    }

    if (topic == opts_.lidar_topic) {
      if (dt == dt_custom) {
        auto msg = mi.instantiate<livox_ros_driver::CustomMsg>();
        if (msg) {
          accounting_.lidar_read++;
          if (accounting_.lidar_dispatched == 0) {
            accounting_.first_sensor_time = msg->header.stamp.toSec();
          }
          accounting_.last_sensor_time = msg->header.stamp.toSec();
          wrapper.HandleLidarCustomMsg(msg);
          accounting_.lidar_dispatched++;
        const double t_c4 = nowMs();
        lio.process();
        t_compute_ms += nowMs() - t_c4;
          accounting_.process_invocations++;
          continue;
        }
      } else if (dt == dt_pc2) {
        auto msg = mi.instantiate<sensor_msgs::PointCloud2>();
        if (msg) {
          accounting_.lidar_read++;
          if (accounting_.lidar_dispatched == 0) {
            accounting_.first_sensor_time = msg->header.stamp.toSec();
          }
          accounting_.last_sensor_time = msg->header.stamp.toSec();
          wrapper.HandleLidarPointCloud2(msg);
          accounting_.lidar_dispatched++;
        const double t_c5 = nowMs();
        lio.process();
        t_compute_ms += nowMs() - t_c5;
          accounting_.process_invocations++;
          continue;
        }
      }
    }
  }
  double t1 = nowMs();
  accounting_.wall_processing_s = (t1 - t0) / 1000.0;
  std::printf("[OfflineReader] wall breakdown s: total=%.3f read_decompress=%.3f dispatch=%.3f compute=%.3f\n",
              (t1 - t0) / 1000.0,
              ((t1 - t0) - t_dispatch_ms - t_compute_ms) / 1000.0,
              t_dispatch_ms / 1000.0, t_compute_ms / 1000.0);

  accounting_.sync_count = wrapper.syncCount();
  if (accounting_.sync_count > 0) {
    accounting_.first_estimator_time = wrapper.firstSyncedLidarEndTime();
    accounting_.last_estimator_time = wrapper.lastSyncedLidarEndTime();
  }
  accounting_.sensor_duration_s =
      accounting_.last_sensor_time - accounting_.first_sensor_time;
  accounting_.images_skipped =
      accounting_.images_read - accounting_.images_dispatched;

  for (auto& b : bags) b.close();
  return true;
}

void OfflineReader::drain(ROSWrapper& wrapper, SuperLIO& lio) {
  while (true) {
    const int before = wrapper.syncCount();
    lio.process();
    accounting_.process_invocations++;
    if (wrapper.syncCount() == before) break;
  }
  accounting_.sync_count = wrapper.syncCount();
  if (accounting_.sync_count > 0) {
    accounting_.first_estimator_time = wrapper.firstSyncedLidarEndTime();
    accounting_.last_estimator_time = wrapper.lastSyncedLidarEndTime();
  }
  accounting_.heavy_process_count = static_cast<size_t>(accounting_.sync_count);
  accounting_.imu_remaining = wrapper.imuBufferSize();
  accounting_.lidar_remaining = wrapper.lidarBufferSize();
  accounting_.last_imu_time = wrapper.lastTimestampImu();
  if (accounting_.lidar_remaining > 0) {
    accounting_.front_lidar_end_time = wrapper.frontLidarEndTime();
    if (wrapper.lastTimestampImu() >= 0.0 &&
        wrapper.lastTimestampImu() < accounting_.front_lidar_end_time) {
      accounting_.unprocessed_reason =
          "front lidar lacks IMU coverage beyond its end_time (no more messages)";
    } else {
      accounting_.unprocessed_reason =
          "unexpected: lidar remaining but IMU covers it (sync loop stopped)";
    }
  } else if (accounting_.imu_remaining > 0) {
    accounting_.unprocessed_reason = "trailing IMU beyond last lidar end_time";
  } else {
    accounting_.unprocessed_reason = "all measurement groups processed";
  }
}

}  // namespace LI2Sup