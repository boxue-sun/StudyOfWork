# 90982e3  （fix）输出周期栅格化+spatial_output 升到通道级+Volume1~5 分型计数+流向分摊取整

- 作者：fengxin
- 日期：2026-09-07
- 完整 hash：`90982e3588c3f0a09dc389fbdc42385a6a66aa7b`
- 改动：+270 / -243 行，涉及 10 个文件

涉及文件：
- `src/common/config.cpp`
- `src/common/config.h`
- `src/traffic_flow/detectors/accumulate_volume_detector.cpp`
- `src/traffic_flow/detectors/accumulate_volume_detector.h`
- `src/traffic_flow/detectors/stop_line_crossing_helper.cpp`
- `src/traffic_flow/detectors/stop_line_crossing_helper.h`
- `src/traffic_flow/spatial_aggregator.cpp`
- `src/traffic_flow/traffic_flow_detector.h`
- `src/traffic_flow/traffic_flow_factory.cpp`
- `src/traffic_flow/traffic_flow_factory.h`

```diff
diff --git a/src/common/config.cpp b/src/common/config.cpp
index d9f9820..fb3e275 100644
--- a/src/common/config.cpp
+++ b/src/common/config.cpp
@@ -223,14 +223,6 @@ void from_json(const nlohmann::json& j, CrosswalkInfo& crosswalk_info) {
 
 void from_json(const nlohmann::json& j, DetectorItemConfig& item) {
     j.at("type").get_to(item.type);
-    if (j.contains("target")) {
-        j.at("target").get_to(item.target);
-    }
-    if (j.contains("spatial_output")) {
-        j.at("spatial_output").get_to(item.spatial_output);
-    } else if (j.contains("spatial")) {
-        j.at("spatial").get_to(item.spatial_output);
-    }
 }
 
 void from_json(const nlohmann::json& j, DetectorCycleConfig& cycle_config) {
@@ -244,6 +236,9 @@ void from_json(const nlohmann::json& j, DetectorCycleConfig& cycle_config) {
     if (j.contains("simulate_interval_ms")) {
         j.at("simulate_interval_ms").get_to(cycle_config.simulate_interval_ms);
     }
+    if (j.contains("spatial_output")) {
+        j.at("spatial_output").get_to(cycle_config.spatial_output);
+    }
     if (j.contains("detectors")) {
         j.at("detectors").get_to(cycle_config.detectors);
     }
@@ -254,16 +249,6 @@ GlobalConfig::GlobalConfig(const std::string& config_path) {
         std::cerr << "Error while construct global config." << std::endl;
         assert(false);
     }
-#ifdef DEBUG
-    std::cout << "<-----------------Global Config----------------->" << std::endl;
-    for (const auto& it : _detector_configs) {
-        std::cout << "Coil tag: " << it.first << std::endl;
-        for (const auto& one_detector : it.second) {
-            std::cout << "Detector type: " << one_detector.tag << std::endl;
-        }
-    }
-    std::cout << "<-----------------Global Config----------------->" << std::endl;
-#endif
 }
 
 bool GlobalConfig::init(const std::string& config_path) {
@@ -348,27 +333,6 @@ bool GlobalConfig::load_lane_info() {
     return true;
 }
 
-bool GlobalConfig::construct_detector_config() {
-    if (_working_mode != WorkingMode::ENSEMBLE) {
-        std::cerr << "Error, working mode only supports ENSEMBLE now." << std::endl;
-        return false;
-    }
-    for (const auto& one_road_section_info : _coil_infos) {
-        for (const auto& one_coil_info : one_road_section_info.second) {
-            DetectorConfig detector_config;
-            for (uint8_t i = 1; i <= static_cast<uint8_t>(DetectorType::OCCUPANCY); ++i) {
-                DetectorConfig one_detector_config;
-                one_detector_config.type = static_cast<DetectorType>(i);
-                one_detector_config.working_mode = _working_mode;
-                one_detector_config.coil_info = one_coil_info;
-                one_detector_config.tag = one_coil_info.tag + "_" + detector_type_to_string(one_detector_config.type);
-                _detector_configs[one_coil_info.tag].emplace_back(one_detector_config);
-            }
-        }
-    }
-    return true;
-}
-
 bool GlobalConfig::load_detector_cycle_config() {
     std::ifstream ifile;
     ifile.open(_config_path, std::ios::in);
@@ -438,10 +402,6 @@ bool GlobalConfig::load_global_config() {
             std::cerr << "Error, unable to load lane info." << std::endl;
             return false;
         }
-        if (!construct_detector_config()) {
-            std::cerr << "Error, unable to construct detector config." << std::endl;
-            return false;
-        }
         if (!load_detector_cycle_config()) {
             std::cerr << "Warning, unable to load detector cycle config." << std::endl;
         }
@@ -475,19 +435,6 @@ std::vector<CoilInfo> GlobalConfig::get_coil_infos(const std::string& road_secti
     return coil_infos;
 }
 
-std::vector<DetectorConfig> GlobalConfig::get_detector_config(const std::string& coil_tag) {
-    std::vector<DetectorConfig> detector_configs;
-    if (!_is_init) {
-        std::cerr << "Error, global config not inited!" << std::endl;
-        return detector_configs;
-    }
-    auto find_it = _detector_configs.find(coil_tag);
-    if (find_it != _detector_configs.end()) {
-        detector_configs = find_it->second;
-    }
-    return detector_configs;
-}
-
 std::vector<ExitLaneInfo> GlobalConfig::get_exit_lane_infos() {
     if (!_is_init) {
         std::cerr << "Error, global config not inited!" << std::endl;
diff --git a/src/common/config.h b/src/common/config.h
index cfe00f9..193a5c8 100644
--- a/src/common/config.h
+++ b/src/common/config.h
@@ -25,7 +25,6 @@
 #include "math/polygon2d.hpp"
 #include "common/perception_data.h"
 #include "common/track_info.h"
-#include "traffic_flow/detector_type.h"
 
 namespace fusion_perception::traffic_flow {
 constexpr float MS_TO_KMH = 3.6;
@@ -162,36 +161,26 @@ struct CrosswalkInfo {
 
 void from_json(const nlohmann::json& j, CrosswalkInfo& crosswalk_info);
 
+// 检测器项只声明指标类型；其 ROI 域由 type 隐含决定（volume/queue/speed→进口道线圈，crosswalk→斑马线）。
 struct DetectorItemConfig {
     std::string type;
-    std::string target;
-    std::vector<std::string> spatial_output;
 };
 
 void from_json(const nlohmann::json& j, DetectorItemConfig& item);
 
+// spatial_output 是通道级配置：决定本通道报文包含哪些空间颗粒度（FLOW/LANE/BRANCH）。
+// 每个字段落在哪个颗粒度由输出协议固定，不由配置决定。
 struct DetectorCycleConfig {
     std::string cycle_type;
     uint64_t cycle_duration_ms = 0;
     int reference_phase_id = -1;
     uint64_t simulate_interval_ms = 0;
+    std::vector<std::string> spatial_output;
     std::vector<DetectorItemConfig> detectors;
 };
 
 void from_json(const nlohmann::json& j, DetectorCycleConfig& cycle_config);
 
-struct DetectorConfig {
-    std::string tag;  // globally unique for each detector
-    CoilInfo coil_info;
-    DetectorType type = DetectorType::DEFAULT;
-    uint64_t detect_period;
-    WorkingMode working_mode = WorkingMode::DEFAULT;
-    // nlohmann::json custom_config_json;
-};
-
-// void to_json(nlohmann::json& j, const DetectorConfig& detector_config);
-// void from_json(const nlohmann::json& j, DetectorConfig& detector_config);
-
 class GlobalConfig final : public cybertron::base::Singleton<GlobalConfig> {
     friend class cybertron::base::Singleton<GlobalConfig>;
 
@@ -202,7 +191,6 @@ private:
 
     bool load_global_config();
     bool load_lane_info();
-    bool construct_detector_config();
     bool load_detector_cycle_config();
 
 public:
@@ -216,7 +204,6 @@ public:
     bool get_time_align_flag();
     std::unordered_map<std::string, RoadSectionInfo> get_road_section_infos();
     std::vector<CoilInfo> get_coil_infos(const std::string& road_section_tag);
-    std::vector<DetectorConfig> get_detector_config(const std::string& coil_tag);
     std::vector<ExitLaneInfo> get_exit_lane_infos();
     std::vector<CrosswalkInfo> get_crosswalk_infos();
     std::vector<DetectorCycleConfig> get_detector_cycle_configs();
@@ -232,7 +219,6 @@ private:
     WorkingMode _working_mode = WorkingMode::DEFAULT;
     std::unordered_map<std::string, RoadSectionInfo> _road_section_infos;
     std::unordered_map<std::string, std::vector<CoilInfo>> _coil_infos;
-    std::unordered_map<std::string, std::vector<DetectorConfig>> _detector_configs;
     std::vector<ExitLaneInfo> _exit_lane_infos;
     std::vector<CrosswalkInfo> _crosswalk_infos;
     std::vector<DetectorCycleConfig> _detector_cycle_configs;
diff --git a/src/traffic_flow/detectors/accumulate_volume_detector.cpp b/src/traffic_flow/detectors/accumulate_volume_detector.cpp
index 11be86f..0973c52 100644
--- a/src/traffic_flow/detectors/accumulate_volume_detector.cpp
+++ b/src/traffic_flow/detectors/accumulate_volume_detector.cpp
@@ -5,36 +5,40 @@ namespace fusion_perception::traffic_flow {
 AccumulateVolumeDetector::AccumulateVolumeDetector(const std::string& detector_id, const CoilInfo& coil_info)
     : AccumulateDetector(detector_id), _coil_info(coil_info), _tracker(coil_info.stop_line_segments) {
     std::string lane_tag = _coil_info.tag;
-    _lane_acc[lane_tag] = LaneAcc{
-        0, 0.0f,
-        {{"lane_tag", lane_tag},
-         {"road_section_tag", _coil_info.road_section_info.tag},
-         {"lane_no", _coil_info.lane_no},
-         {"lane_id", _coil_info.lane_id},
-         {"turn_dir_no", _coil_info.turn_dir_no}}
-    };
+    LaneAcc acc;
+    acc.meta = {{"lane_tag", lane_tag},
+                {"road_section_tag", _coil_info.road_section_info.tag},
+                {"lane_no", _coil_info.lane_no},
+                {"lane_id", _coil_info.lane_id},
+                {"turn_dir_no", _coil_info.turn_dir_no}};
+    _lane_acc[lane_tag] = acc;
 }
 
-float AccumulateVolumeDetector::get_pcu_coefficient(const TrafficObject& obj) const {
-    if (obj.inner_type == InnerType::BUS || obj.inner_type == InnerType::TRUCK) {
-        return 1.8f;
-    }
-    if (obj.inner_type == InnerType::CAR) {
-        if (obj.vehicle_subtype == VehicleSubtype::MINIVAN) {
+float AccumulateVolumeDetector::get_pcu_coefficient(const VolumeCategory category) const {
+    switch (category) {
+        case VolumeCategory::CAR_SMALL:
+            return 1.0f;
+        case VolumeCategory::VAN:
             return 1.25f;
-        }
-        return 1.0f;
+        case VolumeCategory::LARGE:
+            return 1.8f;
+        default:
+            return 0.0f;
     }
-    return 0.0f;
 }
 
 void AccumulateVolumeDetector::update(const PerceptionResult& frame) {
     auto crossed = _tracker.process_frame(frame);
     for (const auto& obj : crossed) {
+        const VolumeCategory category = classify_volume_category(obj);
+        if (category == VolumeCategory::NONE) {
+            continue;
+        }
+
         std::string lane_tag = _coil_info.tag;
         auto& acc = _lane_acc[lane_tag];
-        acc.volume++;
-        acc.pcu += get_pcu_coefficient(obj);
+        acc.counts[static_cast<int>(category) - 1]++;
+        acc.pcu += get_pcu_coefficient(category);
         acc.meta = {
             {"lane_tag", lane_tag},
             {"road_section_tag", _coil_info.road_section_info.tag},
@@ -49,12 +53,21 @@ nlohmann::json AccumulateVolumeDetector::calculate_once() {
     nlohmann::json result = nlohmann::json::array();
     for (auto& [lane_tag, acc] : _lane_acc) {
         nlohmann::json entry = acc.meta;
-        entry["Volume"] = acc.volume;
+
+        // 协议 Volume 为机动车总流量：Volume3~Volume5 恰为 counts 尾部连续的机动车三类
+        int motor_volume = 0;
+        for (size_t i = static_cast<int>(VolumeCategory::CAR_SMALL) - 1; i < acc.counts.size(); ++i) {
+            motor_volume += acc.counts[i];
+        }
+        entry["Volume"] = motor_volume;
         entry["PCU"] = acc.pcu;
+        for (size_t i = 0; i < kVolumeDetailKeys.size(); ++i) {
+            entry[kVolumeDetailKeys[i]] = acc.counts[i];
+        }
         result.push_back(entry);
     }
     for (auto& [lane_tag, acc] : _lane_acc) {
-        acc.volume = 0;
+        acc.counts.fill(0);
         acc.pcu = 0.0f;
     }
     _tracker.clear_crossing_state();
diff --git a/src/traffic_flow/detectors/accumulate_volume_detector.h b/src/traffic_flow/detectors/accumulate_volume_detector.h
index 2237ff2..e20a0c8 100644
--- a/src/traffic_flow/detectors/accumulate_volume_detector.h
+++ b/src/traffic_flow/detectors/accumulate_volume_detector.h
@@ -1,6 +1,7 @@
 #ifndef ACCUMULATE_VOLUME_DETECTOR_H
 #define ACCUMULATE_VOLUME_DETECTOR_H
 
+#include <array>
 #include <map>
 #include <string>
 #include <nlohmann/json.hpp>
@@ -19,13 +20,13 @@ public:
     nlohmann::json calculate_once() override;
 
 private:
-    float get_pcu_coefficient(const TrafficObject& obj) const;
+    float get_pcu_coefficient(VolumeCategory category) const;
 
     CoilInfo _coil_info;
     StopLineCrossingTracker _tracker;
 
     struct LaneAcc {
-        int volume = 0;
+        std::array<int, 5> counts{};  // 下标 0~4 对应 Volume1~Volume5
         float pcu = 0.0f;
         nlohmann::json meta;
     };
diff --git a/src/traffic_flow/detectors/stop_line_crossing_helper.cpp b/src/traffic_flow/detectors/stop_line_crossing_helper.cpp
index 808640a..d84e59a 100644
--- a/src/traffic_flow/detectors/stop_line_crossing_helper.cpp
+++ b/src/traffic_flow/detectors/stop_line_crossing_helper.cpp
@@ -3,6 +3,13 @@
 
 namespace fusion_perception::traffic_flow {
 
+namespace {
+constexpr double MIN_MOVE_DISTANCE = 0.5;       // in [m], 滤掉原地抖动
+constexpr double MIN_COS_TO_NORMAL = 0.5;       // cos(60°), 滤掉沿停止线平行擦过
+constexpr uint64_t POSITION_TTL = 10000;        // in [ms]
+constexpr double EPSILON = 1e-9;
+}  // namespace
+
 StopLineCrossingTracker::StopLineCrossingTracker(const std::vector<math::Segment2D>& stop_line_segments) {
     if (!stop_line_segments.empty()) {
         math::Vec2D p1 = stop_line_segments.front().start();
@@ -11,26 +18,61 @@ StopLineCrossingTracker::StopLineCrossingTracker(const std::vector<math::Segment
     }
 }
 
+bool StopLineCrossingTracker::passes_quality_gate(bool strict, const math::Vec2D& previous_pos,
+                                                 const math::Vec2D& current_pos) const {
+    if (!strict) {
+        return true;
+    }
+
+    if (previous_pos.geo_distance_to(current_pos) < MIN_MOVE_DISTANCE) {
+        return false;
+    }
+
+    math::Vec2D move_vec(current_pos.x() - previous_pos.x(), current_pos.y() - previous_pos.y());
+    double move_length = move_vec.length();
+    if (move_length < EPSILON) {
+        return false;
+    }
+
+    const math::Vec2D& line_dir = _stop_line.unit_direction();
+    math::Vec2D normal(-line_dir.y(), line_dir.x());
+    double cos_to_normal = std::fabs(move_vec.inner_product(normal)) / move_length;
+
+    return cos_to_normal >= MIN_COS_TO_NORMAL;
+}
+
 std::vector<TrafficObject> StopLineCrossingTracker::process_frame(const PerceptionResult& frame) {
     std::vector<TrafficObject> crossed;
 
+    for (auto it = _prev_positions.begin(); it != _prev_positions.end();) {
+        if (frame.timestamp > it->second.last_seen && frame.timestamp - it->second.last_seen > POSITION_TTL) {
+            it = _prev_positions.erase(it);
+        } else {
+            ++it;
+        }
+    }
+
     for (const auto& obj : frame.traffic_objects) {
-        if (!g_motor_vehicle_types.count(obj.inner_type)) {
+        VolumeCategory category = classify_volume_category(obj);
+        if (category == VolumeCategory::NONE) {
             continue;
         }
+        bool strict = (category == VolumeCategory::PEDESTRIAN || category == VolumeCategory::NON_MOTOR);
 
         math::Vec2D current_pos(obj.latitude, obj.longitude);
         auto prev_it = _prev_positions.find(obj.id);
 
         if (prev_it != _prev_positions.end()) {
-            math::Segment2D movement(prev_it->second, current_pos);
-            if (_stop_line.has_intersect(movement) && !_crossed_ids.count(obj.id)) {
+            math::Vec2D previous_pos = prev_it->second.position;
+            math::Segment2D movement(previous_pos, current_pos);
+            if (_stop_line.has_intersect(movement) && !_crossed_ids.count(obj.id) &&
+                passes_quality_gate(strict, previous_pos, current_pos)) {
                 _crossed_ids.insert(obj.id);
                 crossed.push_back(obj);
             }
         }
 
-        _prev_positions[obj.id] = current_pos;
+        _prev_positions[obj.id] = {current_pos, frame.timestamp};
     }
 
     return crossed;
diff --git a/src/traffic_flow/detectors/stop_line_crossing_helper.h b/src/traffic_flow/detectors/stop_line_crossing_helper.h
index 39d1bd6..2b76dc9 100644
--- a/src/traffic_flow/detectors/stop_line_crossing_helper.h
+++ b/src/traffic_flow/detectors/stop_line_crossing_helper.h
@@ -22,8 +22,15 @@ public:
     void clear_crossing_state();
 
 private:
+    struct PrevState {
+        math::Vec2D position;
+        uint64_t last_seen = 0;
+    };
+
+    bool passes_quality_gate(bool strict, const math::Vec2D& previous_pos, const math::Vec2D& current_pos) const;
+
     math::Segment2D _stop_line;
-    std::map<std::string, math::Vec2D> _prev_positions;
+    std::map<std::string, PrevState> _prev_positions;
     std::set<std::string> _crossed_ids;
 };
 
diff --git a/src/traffic_flow/spatial_aggregator.cpp b/src/traffic_flow/spatial_aggregator.cpp
index e1083c9..73a5190 100644
--- a/src/traffic_flow/spatial_aggregator.cpp
+++ b/src/traffic_flow/spatial_aggregator.cpp
@@ -3,7 +3,9 @@
  * @brief Implementation of SpatialAggregator.
  */
 #include "spatial_aggregator.h"
+#include <cmath>
 #include <unordered_map>
+#include "traffic_flow/traffic_flow_detector.h"
 
 namespace fusion_perception::traffic_flow {
 
@@ -67,7 +69,7 @@ nlohmann::json SpatialAggregator::aggregate_to_flow(
                 nlohmann::json entry;
                 entry["BranchID"] = branch_id;
                 entry["FlowType"] = static_cast<int>(split.flow_type);
-                entry["Volume"] = 0;
+                entry["Volume"] = 0.0f;
                 entry["PCU"] = 0.0f;
                 flow_accumulators[flow_key] = entry;
             }
@@ -77,6 +79,15 @@ nlohmann::json SpatialAggregator::aggregate_to_flow(
             if (lane_result.contains("Volume")) {
                 acc["Volume"] = acc["Volume"].get<float>() + lane_result["Volume"].get<float>() * split.ratio;
             }
+            for (const auto* field : kVolumeDetailKeys) {
+                if (!lane_result.contains(field)) {
+                    continue;
+                }
+                if (!acc.contains(field)) {
+                    acc[field] = 0.0f;
+                }
+                acc[field] = acc[field].get<float>() + lane_result[field].get<float>() * split.ratio;
+            }
             if (lane_result.contains("PCU")) {
                 acc["PCU"] = acc["PCU"].get<float>() + lane_result["PCU"].get<float>() * split.ratio;
             }
@@ -100,7 +111,15 @@ nlohmann::json SpatialAggregator::aggregate_to_flow(
 
     nlohmann::json result = nlohmann::json::array();
     for (const auto& [key, value] : flow_accumulators) {
-        result.push_back(value);
+        nlohmann::json entry = value;
+        // 流向级计数为按转向比例分摊后的近似值，取整不保证 Σflow == Σlane
+        entry["Volume"] = std::lround(value["Volume"].get<double>());
+        for (const auto* field : kVolumeDetailKeys) {
+            if (value.contains(field)) {
+                entry[field] = std::lround(value[field].get<double>());
+            }
+        }
+        result.push_back(entry);
     }
     return result;
 }
diff --git a/src/traffic_flow/traffic_flow_detector.h b/src/traffic_flow/traffic_flow_detector.h
index ae48ecc..c9efec4 100644
--- a/src/traffic_flow/traffic_flow_detector.h
+++ b/src/traffic_flow/traffic_flow_detector.h
@@ -1,7 +1,7 @@
 /**
  * @file traffic_flow_detector.h
  * @author FengWenxi (ThorfromAsgard@outlook.com)
- * @brief TrafficFlowDetector class.
+ * @brief Volume classification helpers and AccumulateDetector base class.
  * @version 0.1
  * @date 2024-10-11
  *
@@ -13,48 +13,48 @@
 #ifndef TRAFFIC_FLOW_DETECTOR_H
 #define TRAFFIC_FLOW_DETECTOR_H
 
+#include <array>
 #include <set>
-#include <map>
+#include <string>
 
 #include <nlohmann/json.hpp>
 #include "base/Noncopyable.h"
-#include "detector_type.h"
-#include "common/track_info.h"
 #include "common/config.h"
 #include "common/perception_data.h"
 
 namespace fusion_perception::traffic_flow {
-constexpr uint64_t DEFAULT_DETECT_PERIOD = 5000;  // in [ms]
 constexpr uint64_t OBJECT_CLEAR_TIME = 100000;    // in [ms]
-constexpr float STATIC_SPEED_THRESH = 5.0;        // in [km/h], if an object speed is below this threshold, it will be considered as static
-constexpr uint16_t STATIC_CHECK_NUM = 3;          // if an object is static for at least this number of times in one detect period, it will be considered as static  
 
-const static std::set<InnerType> g_all_types = {InnerType::PEDESTRIAN, InnerType::CAR,          InnerType::BUS,
-                                                InnerType::TRUCK,      InnerType::TRAFFIC_CONE, InnerType::BICYCLE,
-                                                InnerType::TRICYCLE,   InnerType::PLATE};
 const static std::set<InnerType> g_motor_vehicle_types = {InnerType::CAR, InnerType::BUS, InnerType::TRUCK};
-const static std::set<InnerType> g_non_motor_vehicle_types = {InnerType::PEDESTRIAN, InnerType::BICYCLE,
-                                                              InnerType::TRICYCLE};
-const static std::set<InnerType> g_pedestrian_types = {InnerType::PEDESTRIAN};
 
-class TrafficFlowDetector : public afl::Noncopyable {
-    using result_deliver_callback = std::function<bool(const std::string&)>;
-
-public:
-    TrafficFlowDetector(const traffic_flow::DetectorConfig& detector_config) : _detector_config(detector_config) {};
-    virtual ~TrafficFlowDetector() = default;
+// 取值即协议 VolumeN 的编号，Volume1(行人)~Volume5(大车)。
+enum class VolumeCategory : int {
+    NONE = 0,
+    PEDESTRIAN = 1,
+    NON_MOTOR = 2,
+    CAR_SMALL = 3,
+    VAN = 4,
+    LARGE = 5
+};
 
-    void register_deliver_callback(const result_deliver_callback& callback) { _deliver_callback = callback; };
-    DetectorConfig get_detector_config() { return _detector_config; };
-    virtual void update(const PerceptionResult& new_frame_result) = 0;
-    virtual void calculate_once(nlohmann::json& result_j) = 0;
+constexpr std::array<const char*, 5> kVolumeDetailKeys = {"Volume1", "Volume2", "Volume3", "Volume4", "Volume5"};
 
-protected:
-    DetectorConfig _detector_config;
-    result_deliver_callback _deliver_callback;
-    // key: object_id, value: track info
-    std::map<std::string, TrackInfo> _object_tracks;
-};
+inline VolumeCategory classify_volume_category(const TrafficObject& obj) {
+    switch (obj.inner_type) {
+        case InnerType::PEDESTRIAN:
+            return VolumeCategory::PEDESTRIAN;
+        case InnerType::BICYCLE:
+        case InnerType::TRICYCLE:
+            return VolumeCategory::NON_MOTOR;
+        case InnerType::CAR:
+            return obj.vehicle_subtype == VehicleSubtype::MINIVAN ? VolumeCategory::VAN : VolumeCategory::CAR_SMALL;
+        case InnerType::BUS:
+        case InnerType::TRUCK:
+            return VolumeCategory::LARGE;
+        default:
+            return VolumeCategory::NONE;
+    }
+}
 
 class AccumulateDetector : public afl::Noncopyable {
 public:
diff --git a/src/traffic_flow/traffic_flow_factory.cpp b/src/traffic_flow/traffic_flow_factory.cpp
index 97d77ef..e452990 100644
--- a/src/traffic_flow/traffic_flow_factory.cpp
+++ b/src/traffic_flow/traffic_flow_factory.cpp
@@ -18,73 +18,25 @@
 
 #include "common/config.h"
 #include "traffic_flow_detector.h"
-#include "detectors/volume_detector.h"
-#include "detectors/queue_detector.h"
-#include "detectors/headway_detector.h"
-#include "detectors/occupancy_detector.h"
 #include "detectors/accumulate_queue_detector.h"
 #include "detectors/accumulate_volume_detector.h"
+#include "detectors/accumulate_crosswalk_detector.h"
+#include "detectors/accumulate_speed_detector.h"
 #include "spatial_aggregator.h"
 #include "common/perception_data.h"
 
 namespace fusion_perception::traffic_flow {
 
-TrafficFlowDetector* TrafficFlowFactory::create_detector(const DetectorConfig& detector_config) {
-    TrafficFlowDetector* ptr = nullptr;
-    DetectorType detector_type = detector_config.type;
-    switch (detector_type) {
-        case DetectorType::VOLUME:
-            ptr = new VolumeDetector(detector_config);
-            break;
-        case DetectorType::QUEUE:
-            ptr = new QueueDetector(detector_config);
-            break;
-        case DetectorType::HEADWAY:
-            ptr = new HeadwayDetector(detector_config);
-            break;
-        case DetectorType::OCCUPANCY:
-            ptr = new OccupancyDetector(detector_config);
-            break;
-        default:
-            std::cerr << "Unknown detector type!" << std::endl;
-            assert(false);
-    }
-    return ptr;
-}
+namespace {
+const std::set<std::string> kValidSpatialOutputs = {"FLOW", "LANE", "BRANCH"};
+}  // namespace
 
 TrafficFlowFactory::TrafficFlowFactory()
     : _status(FactoryStatus::NEW),
       _input_buffer(BUFFER_CAPACITY),
       _publish_callback(),
       _signal_phase_manager(std::make_unique<SignalPhaseManager>()) {
-    auto& global_config = traffic_flow::GlobalConfig::get_instance();
-    auto road_section_info_map = global_config.get_road_section_infos();
-    for (const auto& one_road_section_info : road_section_info_map) {
-        std::vector<CoilInfo> coil_infos = global_config.get_coil_infos(one_road_section_info.first);
-        for (const auto& one_coil_info : coil_infos) {
-            std::vector<DetectorConfig> detector_configs = global_config.get_detector_config(one_coil_info.tag);
-            for (const auto& one_detector_config : detector_configs) {
-                std::unique_ptr<TrafficFlowDetector> one_detector_ptr(
-                    TrafficFlowFactory::create_detector(one_detector_config));
-                _coil_detectors[one_coil_info.tag].emplace_back(std::move(one_detector_ptr));
-            }
-        }
-    }
-#ifdef DEBUG
-    std::cout << "coil num: " << _coil_detectors.size() << std::endl;
-    for (const auto& it : _coil_detectors) {
-        std::cout << "Coil tag: " << it.first << std::endl;
-        for (const auto& one_detector : it.second) {
-            std::cout << "Detector type: " << one_detector->get_detector_config().tag << std::endl;
-            std::cout << "Detector stop line: " << std::endl;
-            for (const auto& one_segment : one_detector->get_detector_config().coil_info.stop_line_segments) {
-                std::cout << one_segment.to_string() << "\t";
-            }
-            std::cout << std::endl;
-        }
-    }
-#endif
-    _working_mode = global_config.get_working_mode();
+    _working_mode = traffic_flow::GlobalConfig::get_instance().get_working_mode();
 
     build_detectors_from_config();
 
@@ -114,26 +66,14 @@ bool TrafficFlowFactory::start_running() {
 void TrafficFlowFactory::register_publish_callback(const PublishCallback& callback) {
     std::cout << "Register _publish_callback for TrafficFlowFactory." << std::endl;
     _publish_callback = callback;
-    for (const auto& one_section_detector : _coil_detectors) {
-        for (const auto& one_detector : one_section_detector.second) {
-            one_detector->register_deliver_callback(
-                [this](const std::string& result) {
-                    if (!result.empty()) {
-                        _publish_callback(result);
-                        return true;
-                    }
-                    return false;
-                });
-        }
-    }
     _status = FactoryStatus::READY;
 }
 
 nlohmann::json TrafficFlowFactory::build_output(
     const std::vector<nlohmann::json>& flow_results,
     const std::vector<nlohmann::json>& lane_results,
+    const std::vector<nlohmann::json>& branch_results,
     uint64_t timestamp,
-    uint64_t start_time,
     const OutputChannel& channel) {
     auto& global_config = traffic_flow::GlobalConfig::get_instance();
 
@@ -150,6 +90,9 @@ nlohmann::json TrafficFlowFactory::build_output(
             entry["FlowType"] = flow.value("FlowType", 0);
             entry["Volume"] = flow.value("Volume", 0);
             entry["PCU"] = flow.value("PCU", 0.0f);
+            for (const auto* field : kVolumeDetailKeys) {
+                if (flow.contains(field)) entry[field] = flow[field];
+            }
             if (flow.contains("QueueLength")) entry["QueueLength"] = flow["QueueLength"];
             if (flow.contains("QueueCount")) entry["QueueCount"] = flow["QueueCount"];
             if (flow.contains("QueueTransCount")) entry["QueueTransCount"] = flow["QueueTransCount"];
@@ -169,10 +112,28 @@ nlohmann::json TrafficFlowFactory::build_output(
             entry["LaneID"] = lane.value("lane_id", "");
             entry["Volume"] = lane.value("Volume", 0);
             entry["PCU"] = lane.value("PCU", 0.0f);
+            for (const auto* field : kVolumeDetailKeys) {
+                if (lane.contains(field)) entry[field] = lane[field];
+            }
             if (lane.contains("QueueCount")) entry["QueueCount"] = lane["QueueCount"];
             if (lane.contains("QueueTransCount")) entry["QueueTransCount"] = lane["QueueTransCount"];
             if (lane.contains("ClassQueueNaturalNumber")) entry["ClassQueueNaturalNumber"] = lane["ClassQueueNaturalNumber"];
             if (lane.contains("ClassQueueEquivalentNumber")) entry["ClassQueueEquivalentNumber"] = lane["ClassQueueEquivalentNumber"];
+            if (lane.contains("SpaceMeanSpeed")) entry["SpaceMeanSpeed"] = lane["SpaceMeanSpeed"];
+            coil_list.push_back(entry);
+        }
+    }
+
+    if (channel.spatial_output.count("BRANCH")) {
+        for (const auto& branch : branch_results) {
+            nlohmann::json entry;
+            entry["RegionID"] = global_config.get_region_id();
+            entry["NodeID"] = branch.value("road_section_tag", "");
+            entry["UpstreamCrossID"] = branch.value("road_section_tag", "");
+            entry["SpaceType"] = 2;
+            entry["BranchID"] = branch.value("road_section_tag", "");
+            if (branch.contains("PersonCrossCount")) entry["PersonCrossCount"] = branch["PersonCrossCount"];
+            if (branch.contains("NonMotorCrossCount")) entry["NonMotorCrossCount"] = branch["NonMotorCrossCount"];
             coil_list.push_back(entry);
         }
     }
@@ -183,8 +144,8 @@ nlohmann::json TrafficFlowFactory::build_output(
     output["MecNo"] = global_config.get_mec_no();
     output["Timestamp"] = get_readable_timestamp(timestamp);
     output["Cycle"] = channel.get_cycle_value();
-    output["StartTime"] = get_readable_timestamp(start_time);
-    output["StopTime"] = get_readable_timestamp(timestamp);
+    output["StartTime"] = get_readable_timestamp(channel.window_start_ms);
+    output["StopTime"] = get_readable_timestamp(channel.window_stop_ms);
     output["CoilNum"] = coil_list.size();
     output["Coil_List"] = coil_list;
 
@@ -238,6 +199,19 @@ void TrafficFlowFactory::build_detectors_from_config() {
         channel.cycle_type = cycle_config.cycle_type;
         channel.interval_ms = cycle_config.cycle_duration_ms;
 
+        for (const auto& spatial : cycle_config.spatial_output) {
+            if (kValidSpatialOutputs.count(spatial) == 0) {
+                std::cerr << "[BuildDetectors] Channel [" << channel.cycle_type
+                          << "] has invalid spatial_output '" << spatial << "', ignored" << std::endl;
+                continue;
+            }
+            channel.spatial_output.insert(spatial);
+        }
+        if (channel.spatial_output.empty()) {
+            std::cerr << "[BuildDetectors] Channel [" << channel.cycle_type
+                      << "] has empty spatial_output, its reports will carry no Coil_List entry" << std::endl;
+        }
+
         if (cycle_config.cycle_type == "signal_cycle" && cycle_config.reference_phase_id > 0) {
             _signal_phase_manager->set_reference_phase_id(cycle_config.reference_phase_id);
             std::cout << "[BuildDetectors] Set reference_phase_id: "
@@ -250,35 +224,51 @@ void TrafficFlowFactory::build_detectors_from_config() {
                       << cycle_config.simulate_interval_ms << "ms" << std::endl;
         }
 
+        auto road_sections = global_config.get_road_section_infos();
         for (const auto& detector_item : cycle_config.detectors) {
-            if (cycle_config.cycle_type == "snapshot" &&
-                detector_item.target != "entry_lane" && !detector_item.target.empty()) {
+            if (detector_item.type == "crosswalk") {
+                for (const auto& crosswalk_info : global_config.get_crosswalk_infos()) {
+                    std::string detector_id = cycle_config.cycle_type + "_crosswalk_" + crosswalk_info.tag;
+                    channel.detectors.push_back(
+                        std::make_unique<AccumulateCrosswalkDetector>(detector_id, crosswalk_info));
+                    std::cout << "[BuildDetectors] Created AccumulateCrosswalkDetector: "
+                              << detector_id << std::endl;
+                }
                 continue;
             }
 
-            for (const auto& so : detector_item.spatial_output) {
-                channel.spatial_output.insert(so);
-            }
-
-            auto road_sections = global_config.get_road_section_infos();
+            bool handled = false;
             for (const auto& [section_tag, section_info] : road_sections) {
-                auto coil_infos = global_config.get_coil_infos(section_tag);
-                for (const auto& coil_info : coil_infos) {
+                for (const auto& coil_info : global_config.get_coil_infos(section_tag)) {
                     if (detector_item.type == "volume") {
                         std::string detector_id = cycle_config.cycle_type + "_volume_" + coil_info.tag;
                         channel.detectors.push_back(
                             std::make_unique<AccumulateVolumeDetector>(detector_id, coil_info));
                         std::cout << "[BuildDetectors] Created AccumulateVolumeDetector: "
                                   << detector_id << std::endl;
+                        handled = true;
                     } else if (detector_item.type == "queue") {
                         std::string detector_id = cycle_config.cycle_type + "_queue_" + coil_info.tag;
                         channel.detectors.push_back(
                             std::make_unique<AccumulateQueueDetector>(detector_id, coil_info));
                         std::cout << "[BuildDetectors] Created AccumulateQueueDetector: "
                                   << detector_id << std::endl;
+                        handled = true;
+                    } else if (detector_item.type == "speed") {
+                        std::string detector_id = cycle_config.cycle_type + "_speed_" + coil_info.tag;
+                        channel.detectors.push_back(
+                            std::make_unique<AccumulateSpeedDetector>(detector_id, coil_info));
+                        std::cout << "[BuildDetectors] Created AccumulateSpeedDetector: "
+                                  << detector_id << std::endl;
+                        handled = true;
                     }
                 }
             }
+
+            if (!handled) {
+                std::cerr << "[BuildDetectors] Channel [" << channel.cycle_type
+                          << "] has unsupported detector type '" << detector_item.type << "'" << std::endl;
+            }
         }
 
         std::cout << "[BuildDetectors] Channel [" << channel.cycle_type
@@ -297,12 +287,6 @@ void TrafficFlowFactory::detect() {
         uint64_t detect_start_time = get_timestamp();
 #endif
 
-        for (const auto& one_section_detectors : _coil_detectors) {
-            for (const auto& one_detector : one_section_detectors.second) {
-                one_detector->update(detected_result);
-            }
-        }
-
         for (auto& channel : _output_channels) {
             for (auto& detector : channel.detectors) {
                 detector->update(detected_result);
@@ -315,18 +299,25 @@ void TrafficFlowFactory::detect() {
             if (!channel.should_output(current_time, _signal_phase_manager.get()))
                 continue;
 
-            uint64_t start_time = channel.last_output_time;
-            channel.last_output_time = current_time;
-
-            nlohmann::json lane_results = nlohmann::json::array();
+            nlohmann::json all_results = nlohmann::json::array();
             for (auto& detector : channel.detectors) {
                 auto results = detector->calculate_once();
                 if (results.is_array()) {
                     for (const auto& item : results) {
-                        lane_results.push_back(item);
+                        all_results.push_back(item);
                     }
                 } else if (!results.empty()) {
-                    lane_results.push_back(results);
+                    all_results.push_back(results);
+                }
+            }
+
+            nlohmann::json lane_results = nlohmann::json::array();
+            std::vector<nlohmann::json> branch_vec;
+            for (const auto& item : all_results) {
+                if (item.value("_result_type", "") == "crosswalk") {
+                    branch_vec.push_back(item);
+                } else {
+                    lane_results.push_back(item);
                 }
             }
 
@@ -356,7 +347,7 @@ void TrafficFlowFactory::detect() {
                 flow_vec.push_back(item);
             }
 
-            nlohmann::json output = build_output(flow_vec, lane_vec, current_time, start_time, channel);
+            nlohmann::json output = build_output(flow_vec, lane_vec, branch_vec, current_time, channel);
             _publish_callback(output.dump());
 
             std::cout << "[" << channel.cycle_type << " Output] Cycle=" << channel.get_cycle_value()
diff --git a/src/traffic_flow/traffic_flow_factory.h b/src/traffic_flow/traffic_flow_factory.h
index ad1b9f2..a1e64ac 100644
--- a/src/traffic_flow/traffic_flow_factory.h
+++ b/src/traffic_flow/traffic_flow_factory.h
@@ -15,6 +15,7 @@
 
 #include <memory>
 #include <atomic>
+#include <iostream>
 #include <string>
 #include <vector>
 #include <thread>
@@ -46,9 +47,6 @@ constexpr size_t BUFFER_CAPACITY = 60;
 #endif
 constexpr float BUFFER_SIZE_WARNING_RATIO = 0.8;
 
-struct CoilInfo;
-struct DetectorConfig;
-
 enum class FactoryStatus : uint8_t { NEW = 0, READY = 1, RUNNING = 2, WAIT = 3, EXIT = 4 };
 
 static std::unordered_map<FactoryStatus, std::string> s_factory_status_to_string_map = {
@@ -71,29 +69,54 @@ struct OutputChannel {
     std::string cycle_type;
     std::vector<std::unique_ptr<AccumulateDetector>> detectors;
     std::set<std::string> spatial_output;
-    uint64_t last_output_time = 0;
     uint64_t interval_ms = 0;
+    uint64_t window_open_ms = 0;
+    uint64_t window_start_ms = 0;
+    uint64_t window_stop_ms = 0;
     bool initialized = false;
 
+    // 触发判定走 interval_ms 的绝对栅格：只有帧时间戳跨入新的栅格槽才上报，
+    // 帧到达的抖动不会累积进周期（5min 不会变成 5min+Δ 且只增不减）。
+    // 上报窗口取「上次上报时刻 → 本次帧时刻」，与聚合器「算完即清零」的语义一致，窗口不重叠不留缝。
+    // 断流跨过的槽不补发（检测器状态本就跨流保留），并入本次窗口，一次性上报并告警。
     bool should_output(uint64_t current_time, SignalPhaseManager* spm) {
-        if (cycle_type == "signal_cycle") {
-            if (!spm) return false;
-            if (!spm->check_and_consume_cycle_boundary()) return false;
-            if (!initialized) {
-                initialized = true;
-                last_output_time = current_time;
+        const bool is_signal = (cycle_type == "signal_cycle");
+        if (!is_signal && interval_ms == 0) {
+            return false;
+        }
+        if (is_signal && (!spm || !spm->check_and_consume_cycle_boundary())) {
+            return false;
+        }
+
+        if (!initialized) {
+            initialized = true;
+            window_open_ms = is_signal ? current_time : current_time / interval_ms * interval_ms;
+            window_stop_ms = current_time;
+            return false;
+        }
+
+        if (!is_signal) {
+            uint64_t grid_open_ms = current_time / interval_ms * interval_ms;
+            if (grid_open_ms <= window_open_ms) {
                 return false;
             }
-            return true;
+            uint64_t merged_windows = current_time / interval_ms - window_open_ms / interval_ms;
+            if (merged_windows > 1) {
+                std::cerr << "[OutputChannel] " << cycle_type << " stalled for " << (current_time - window_stop_ms)
+                          << "ms, merged " << merged_windows << " windows of " << interval_ms << "ms" << std::endl;
+            }
+            window_open_ms = grid_open_ms;
         }
-        if (interval_ms == 0) return false;
-        return (current_time - last_output_time) >= interval_ms;
+
+        window_start_ms = window_stop_ms;
+        window_stop_ms = current_time;
+        return true;
     }
 
     int get_cycle_value() const {
-        if (cycle_type == "snapshot") return 1;
-        if (cycle_type == "accumulate_5min") return 300;
-        return -1;
+        // signal_cycle 的窗口由相位边界决定，不是固定秒数；其余通道节奏就是 interval_ms。
+        if (cycle_type == "signal_cycle") return -1;
+        return static_cast<int>(interval_ms / 1000);
     }
 };
 
@@ -117,21 +140,19 @@ private:
     virtual ~TrafficFlowFactory();
 
     void detect();
-    static TrafficFlowDetector* create_detector(const DetectorConfig& detector_config);
     uint64_t get_timestamp();
     std::string get_readable_timestamp(const uint64_t& timestamp);
     void build_detectors_from_config();
     nlohmann::json build_output(
         const std::vector<nlohmann::json>& flow_results,
         const std::vector<nlohmann::json>& lane_results,
+        const std::vector<nlohmann::json>& branch_results,
         uint64_t timestamp,
-        uint64_t start_time,
         const OutputChannel& channel);
 
 private:
     std::atomic<FactoryStatus> _status;
 
-    std::unordered_map<std::string, std::vector<std::unique_ptr<TrafficFlowDetector>>> _coil_detectors;
     std::vector<OutputChannel> _output_channels;
 
     std::unique_ptr<SignalPhaseManager> _signal_phase_manager;
```
