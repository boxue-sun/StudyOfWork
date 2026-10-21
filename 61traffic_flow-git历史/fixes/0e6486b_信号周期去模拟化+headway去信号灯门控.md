# 0e6486b  （fix）信号周期级去模拟化+headway检测去掉信号灯门控+time_align时间对齐

- 作者：fengxin
- 日期：2026-09-08
- 完整 hash：`0e6486b12ab4c31e333a969c5ab1bcd37ea58030`
- 改动：+188 / -208 行，涉及 9 个文件

涉及文件：
- `src/common/config.cpp`
- `src/common/config.h`
- `src/traffic_flow/detectors/accumulate_headway_detector.cpp`
- `src/traffic_flow/detectors/accumulate_headway_detector.h`
- `src/traffic_flow/signal_phase_manager.cpp`
- `src/traffic_flow/signal_phase_manager.h`
- `src/traffic_flow/traffic_flow_detector.h`
- `src/traffic_flow/traffic_flow_factory.cpp`
- `src/traffic_flow/traffic_flow_factory.h`

```diff
diff --git a/src/common/config.cpp b/src/common/config.cpp
index 98f951c..133d959 100644
--- a/src/common/config.cpp
+++ b/src/common/config.cpp
@@ -15,6 +15,7 @@
 #include <fstream>
 #include <iostream>
 #include <sstream>
+#include <stdexcept>
 #include <cassert>
 
 namespace fusion_perception::traffic_flow {
@@ -121,35 +122,6 @@ math::Polygon2D parse_polygon_from_json(const nlohmann::json& j) {
     return math::Polygon2D(points);
 }
 
-void parse_contour_segments(const nlohmann::json& contour_j,
-                            std::vector<math::Vec2D>& segment_1,
-                            std::vector<math::Vec2D>& segment_2) {
-    for (const auto& point_json : contour_j[0]) {
-        math::Vec2D p;
-        p.set_x(point_json[0]);
-        p.set_y(point_json[1]);
-        segment_1.emplace_back(p);
-    }
-    for (const auto& point_json : contour_j[1]) {
-        math::Vec2D p;
-        p.set_x(point_json[0]);
-        p.set_y(point_json[1]);
-        segment_2.emplace_back(p);
-    }
-}
-
-math::Polygon2D build_lane_contour(const std::vector<math::Vec2D>& segment_1,
-                                   const std::vector<math::Vec2D>& segment_2) {
-    std::vector<math::Vec2D> points;
-    for (const auto& p : segment_1) {
-        points.push_back(p);
-    }
-    for (auto it = segment_2.rbegin(); it != segment_2.rend(); ++it) {
-        points.push_back(*it);
-    }
-    return math::Polygon2D(points);
-}
-
 // 报文的 NodeID/UpstreamCrossID 由断面 tag 反查路口编号得到，进口道、出口道、斑马线都要登记。
 void register_road_section(std::unordered_map<std::string, RoadSectionInfo>& road_section_infos,
                            const RoadSectionInfo& road_section_info) {
@@ -158,8 +130,7 @@ void register_road_section(std::unordered_map<std::string, RoadSectionInfo>& roa
 }  // namespace
 
 void from_json(const nlohmann::json& j, ExitLaneInfo& exit_lane_info) {
-    nlohmann::json contour_j = j["contour"];
-    nlohmann::json lane_no_j = j["LaneNo"];
+    const nlohmann::json lane_no_j = j["LaneNo"];
 
     exit_lane_info.road_section_info.node_id = lane_no_j["inter_id"];
     exit_lane_info.road_section_info.upstream_cross_id = lane_no_j["upstream_cross_id"];
@@ -168,28 +139,7 @@ void from_json(const nlohmann::json& j, ExitLaneInfo& exit_lane_info) {
     exit_lane_info.road_section_info.measure_no = lane_no_j["f_dir_8_no"];
     exit_lane_info.road_section_info.tag = direction_code_to_string(exit_lane_info.road_section_info.direction) + "_" +
                                            lane_type_to_string(exit_lane_info.road_section_info.type);
-    exit_lane_info.lane_id = lane_no_j["LaneID"];
-    exit_lane_info.turn_dir_no = lane_no_j["turn_dir_no"];
-    exit_lane_info.lane_no = exit_lane_info.road_section_info.node_id +
-                             std::to_string(static_cast<uint8_t>(exit_lane_info.road_section_info.direction)) +
-                             std::to_string(static_cast<uint8_t>(exit_lane_info.road_section_info.direction)) +
-                             std::to_string(static_cast<uint8_t>(exit_lane_info.road_section_info.type)) +
-                             exit_lane_info.lane_id + exit_lane_info.turn_dir_no;
-    exit_lane_info.coil_no = std::stoi(exit_lane_info.lane_id);
-    exit_lane_info.tag = exit_lane_info.road_section_info.tag + "_" + exit_lane_info.lane_id;
-
-    parse_contour_segments(contour_j, exit_lane_info.contour_segment_1, exit_lane_info.contour_segment_2);
-    // 出口道 ROI 可以只画溢流区域不画车道轮廓，此时不构造停止线与轮廓，避免空 vector 下标越界。
-    if (!exit_lane_info.contour_segment_1.empty() && !exit_lane_info.contour_segment_2.empty()) {
-        exit_lane_info.stop_line_points.push_back(exit_lane_info.contour_segment_1[0]);
-        exit_lane_info.stop_line_points.push_back(exit_lane_info.contour_segment_2[0]);
-        for (size_t i = 0; i + 1 < exit_lane_info.stop_line_points.size(); ++i) {
-            exit_lane_info.stop_line_segments.emplace_back(exit_lane_info.stop_line_points[i],
-                                                           exit_lane_info.stop_line_points[i + 1]);
-        }
-        exit_lane_info.lane_contour = build_lane_contour(exit_lane_info.contour_segment_1,
-                                                         exit_lane_info.contour_segment_2);
-    }
+    exit_lane_info.tag = exit_lane_info.road_section_info.tag + "_" + lane_no_j["LaneID"].get<std::string>();
 
     if (j.contains("overflow_zones")) {
         const auto& zones = j["overflow_zones"];
@@ -236,9 +186,6 @@ void from_json(const nlohmann::json& j, DetectorCycleConfig& cycle_config) {
     if (j.contains("reference_phase_id")) {
         j.at("reference_phase_id").get_to(cycle_config.reference_phase_id);
     }
-    if (j.contains("simulate_interval_ms")) {
-        j.at("simulate_interval_ms").get_to(cycle_config.simulate_interval_ms);
-    }
     if (j.contains("spatial_output")) {
         j.at("spatial_output").get_to(cycle_config.spatial_output);
     }
@@ -247,6 +194,25 @@ void from_json(const nlohmann::json& j, DetectorCycleConfig& cycle_config) {
     }
 }
 
+void from_json(const nlohmann::json& j, QueueRulesConfig& rules) {
+    // 缺项一律保留默认值（默认值即原先写死的常量），只有显式写错类型才报错。
+    auto read = [&j](const char* key, auto& target) {
+        if (!j.contains(key)) {
+            return;
+        }
+        try {
+            j.at(key).get_to(target);
+        } catch (std::exception&) {
+            throw std::runtime_error(std::string("queue_rules.'") + key + "' has unexpected type");
+        }
+    };
+    read("entry_speed_kmh", rules.entry_speed_kmh);
+    read("entry_hold_frames", rules.entry_hold_frames);
+    read("entry_gap_body_ratio", rules.entry_gap_body_ratio);
+    read("overflow_speed_kmh", rules.overflow_speed_kmh);
+    read("overflow_hold_ms", rules.overflow_hold_ms);
+}
+
 GlobalConfig::GlobalConfig(const std::string& config_path) {
     if (!init(config_path)) {
         std::cerr << "Error while construct global config." << std::endl;
@@ -285,11 +251,14 @@ bool GlobalConfig::get_ensemble_mode() {
     return _ensemble_mode;
 }
 
-bool GlobalConfig::get_time_align_flag()
-{
+bool GlobalConfig::get_time_align_flag() {
     return _time_align_flag;
 }
 
+QueueRulesConfig GlobalConfig::get_queue_rules() {
+    return _queue_rules;
+}
+
 bool GlobalConfig::load_lane_info() {
     std::ifstream ifile;
     ifile.open(_lane_info_path, std::ios::in);
@@ -399,6 +368,9 @@ bool GlobalConfig::load_global_config() {
         else{
             _time_align_flag = false;
         }
+        if (config_json.contains("queue_rules")) {
+            config_json.at("queue_rules").get_to(_queue_rules);
+        }
         
         if (!load_lane_info()) {
             std::cerr << "Error, unable to load lane info." << std::endl;
diff --git a/src/common/config.h b/src/common/config.h
index af5870d..7f4a604 100644
--- a/src/common/config.h
+++ b/src/common/config.h
@@ -129,18 +129,11 @@ struct CoilInfo {
 void to_json(nlohmann::json& j, const CoilInfo& coil_info);
 void from_json(const nlohmann::json& j, CoilInfo& coil_info);
 
+// 出口道只被溢流检测器读取：断面标识 + 三级溢流区域。
+// 出口道不需要车道轮廓/停止线（只有进口道线圈有读者），因此配置里的 contour 已删除。
 struct ExitLaneInfo {
     std::string tag;
-    std::string lane_id;
-    std::string lane_no;
-    int32_t coil_no;
-    std::string turn_dir_no;
     RoadSectionInfo road_section_info;
-    std::vector<math::Vec2D> contour_segment_1;
-    std::vector<math::Vec2D> contour_segment_2;
-    std::vector<math::Vec2D> stop_line_points;
-    std::vector<math::Segment2D> stop_line_segments;
-    math::Polygon2D lane_contour;
     math::Polygon2D overflow_zone_near;
     math::Polygon2D overflow_zone_far;
     math::Polygon2D overflow_zone_upstream;
@@ -167,17 +160,30 @@ void from_json(const nlohmann::json& j, DetectorItemConfig& item);
 
 // spatial_output 是通道级配置：决定本通道报文包含哪些空间颗粒度（FLOW/LANE/BRANCH）。
 // 每个字段落在哪个颗粒度由输出协议固定，不由配置决定。
+// cycle_type 只有 signal_cycle 有语义：其触发靠 reference_phase_id 相位的真实「非绿转绿」边界。
 struct DetectorCycleConfig {
     std::string cycle_type;
     uint64_t cycle_duration_ms = 0;
     int reference_phase_id = -1;
-    uint64_t simulate_interval_ms = 0;
     std::vector<std::string> spatial_output;
     std::vector<DetectorItemConfig> detectors;
 };
 
 void from_json(const nlohmann::json& j, DetectorCycleConfig& cycle_config);
 
+// 排队判定的门限，需求表要求可调（原先写死在 detector 的常量里）。
+// 进口道排队与出口道溢流的「排队」定义不同，因此两套门限各自独立。
+struct QueueRulesConfig {
+    float entry_speed_kmh = 10.0f;
+    uint16_t entry_hold_frames = 10;
+    // 与前车（车队头车则相对停止线）的间距小于 N 个车身才算连在队里；<0 表示关掉连续判定。
+    float entry_gap_body_ratio = 1.0f;
+    float overflow_speed_kmh = 10.0f;
+    uint64_t overflow_hold_ms = 5000;
+};
+
+void from_json(const nlohmann::json& j, QueueRulesConfig& rules);
+
 class GlobalConfig final : public cybertron::base::Singleton<GlobalConfig> {
     friend class cybertron::base::Singleton<GlobalConfig>;
 
@@ -199,6 +205,7 @@ public:
     std::string get_mec_no();
     bool get_ensemble_mode();
     bool get_time_align_flag();
+    QueueRulesConfig get_queue_rules();
     std::unordered_map<std::string, RoadSectionInfo> get_road_section_infos();
     std::vector<CoilInfo> get_coil_infos(const std::string& road_section_tag);
     std::vector<ExitLaneInfo> get_exit_lane_infos();
@@ -213,6 +220,7 @@ private:
     std::string _region_id;
     bool _ensemble_mode = false;
     bool _time_align_flag = false;
+    QueueRulesConfig _queue_rules;
     WorkingMode _working_mode = WorkingMode::DEFAULT;
     std::unordered_map<std::string, RoadSectionInfo> _road_section_infos;
     std::unordered_map<std::string, std::vector<CoilInfo>> _coil_infos;
diff --git a/src/traffic_flow/detectors/accumulate_headway_detector.cpp b/src/traffic_flow/detectors/accumulate_headway_detector.cpp
index 77b62b6..8de737c 100644
--- a/src/traffic_flow/detectors/accumulate_headway_detector.cpp
+++ b/src/traffic_flow/detectors/accumulate_headway_detector.cpp
@@ -30,11 +30,6 @@ AccumulateHeadwayDetector::AccumulateHeadwayDetector(
 void AccumulateHeadwayDetector::update(const PerceptionResult& frame) {
     auto crossed = _tracker.process_frame(frame);
     for (const auto& obj : crossed) {
-        if (_signal_phase_manager && _reference_phase_id > 0 &&!_signal_phase_manager->is_simulation_running()) {
-            if (!_signal_phase_manager->is_green(_reference_phase_id)) {
-                continue;
-            }
-        }
         auto& acc = _lane_acc[_coil_info.tag];
         uint64_t current_time = frame.timestamp;
         uint64_t tail_time = current_time +
diff --git a/src/traffic_flow/detectors/accumulate_headway_detector.h b/src/traffic_flow/detectors/accumulate_headway_detector.h
index 5c245f1..21926c1 100644
--- a/src/traffic_flow/detectors/accumulate_headway_detector.h
+++ b/src/traffic_flow/detectors/accumulate_headway_detector.h
@@ -8,7 +8,6 @@
 #include "traffic_flow/traffic_flow_detector.h"
 #include "common/config.h"
 #include "traffic_flow/detectors/stop_line_crossing_helper.h"
-#include "traffic_flow/signal_phase_manager.h"
 namespace fusion_perception::traffic_flow {
 
 class AccumulateHeadwayDetector : public AccumulateDetector {
@@ -19,16 +18,9 @@ public:
     void update(const PerceptionResult& frame) override;
     nlohmann::json calculate_once() override;
 
-    void set_signal_phase_manager(SignalPhaseManager* spm, int reference_phase_id) {
-        _signal_phase_manager = spm;
-        _reference_phase_id = reference_phase_id;
-    }
-
 private:
     CoilInfo _coil_info;
     StopLineCrossingTracker _tracker;
-    SignalPhaseManager* _signal_phase_manager = nullptr;
-    int _reference_phase_id = -1;
 
     struct LaneHeadwayAcc {
         uint64_t last_arrival = 0;
diff --git a/src/traffic_flow/signal_phase_manager.cpp b/src/traffic_flow/signal_phase_manager.cpp
index cbc0a84..a53fa98 100644
--- a/src/traffic_flow/signal_phase_manager.cpp
+++ b/src/traffic_flow/signal_phase_manager.cpp
@@ -7,35 +7,6 @@
 
 namespace fusion_perception::traffic_flow {
 
-SignalPhaseManager::SignalPhaseManager() = default;
-
-SignalPhaseManager::~SignalPhaseManager() {
-    _sim_running = false;
-    if (_sim_thread && _sim_thread->joinable()) {
-        _sim_thread->join();
-    }
-}
-
-void SignalPhaseManager::start_simulation(uint64_t interval_ms) {
-    if (_sim_running) {
-        std::cout << "[SignalSim] Already running, ignoring start_simulation(" << interval_ms << ")" << std::endl;
-        return;
-    }
-    _sim_interval_ms = interval_ms;
-    _sim_running = true;
-    _sim_thread = std::make_unique<std::thread>([this, interval_ms]() {
-        std::cout << "[SignalSim] Simulation started, interval=" << interval_ms << "ms" << std::endl;
-        while (_sim_running) {
-            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
-            if (!_sim_running) break;
-            std::lock_guard<std::mutex> lock(_mutex);
-            _cycle_boundary = true;
-            std::cout << "[SignalSim] Simulated cycle boundary TRIGGERED" << std::endl;
-        }
-        std::cout << "[SignalSim] Simulation stopped" << std::endl;
-    });
-}
-
 void SignalPhaseManager::on_signal_data(const nlohmann::json& signal_json) {
     std::lock_guard<std::mutex> lock(_mutex);
 
@@ -51,25 +22,33 @@ void SignalPhaseManager::on_signal_data(const nlohmann::json& signal_json) {
             if (!phase.contains("phaseId") || !phase.contains("phaseStates")) {
                 continue;
             }
-            int phase_id = phase["phaseId"];
+            const int phase_id = phase["phaseId"];
             for (const auto& state : phase["phaseStates"]) {
-                if (state.contains("light")) {
-                    int light = state["light"];
-                    _phase_states[phase_id] = light;
+                if (!state.contains("light")) {
+                    continue;
+                }
+                const int light = state["light"];
+                // 全相位都留档：目前只有周期切分用参考相位，绿信比/WasteTime 等后续指标要查任意相位。
+                _phase_states[phase_id] = light;
 
-                    if (phase_id == _reference_phase_id) {
-                        bool now_green = (light == LIGHT_PERMISSIVE_GREEN || light == LIGHT_PROTECTED_GREEN);
-                        bool was_green = (_prev_reference_light == LIGHT_PERMISSIVE_GREEN ||
-                                          _prev_reference_light == LIGHT_PROTECTED_GREEN);
-                        std::cout << "[SignalPhase] Reference phase " << _reference_phase_id
-                                  << ": light " << _prev_reference_light << " -> " << light << std::endl;
-                        if (now_green && !was_green && _prev_reference_light != 0) {
-                            _cycle_boundary = true;
-                            std::cout << "[SignalPhase] Cycle boundary TRIGGERED (non-green -> green)" << std::endl;
-                        }
-                        _prev_reference_light = light;
-                    }
+                if (phase_id != _reference_phase_id) {
+                    continue;
                 }
+                const bool now_green = (light == LIGHT_PERMISSIVE_GREEN || light == LIGHT_PROTECTED_GREEN);
+                const bool was_green = (_prev_reference_light == LIGHT_PERMISSIVE_GREEN ||
+                                        _prev_reference_light == LIGHT_PROTECTED_GREEN);
+#ifdef DEBUG
+                std::cout << "[SignalPhase] Reference phase " << _reference_phase_id
+                          << ": light " << _prev_reference_light << " -> " << light << std::endl;
+#endif
+                // 首个观测只记状态：无法判断它是不是刚转绿的，贸然触发会上报一个不完整的周期。
+                if (_reference_observed && now_green && !was_green) {
+                    ++_cycle_boundary_count;
+                    std::cout << "[SignalPhase] Cycle boundary TRIGGERED, count="
+                              << _cycle_boundary_count << std::endl;
+                }
+                _prev_reference_light = light;
+                _reference_observed = true;
             }
         }
     }
@@ -84,28 +63,16 @@ bool SignalPhaseManager::is_green(int phase_id) const {
     return it->second == LIGHT_PERMISSIVE_GREEN || it->second == LIGHT_PROTECTED_GREEN;
 }
 
-bool SignalPhaseManager::is_green_any(const std::vector<int>& phase_ids) const {
-    for (int phase_id : phase_ids) {
-        if (is_green(phase_id)) {
-            return true;
-        }
-    }
-    return false;
-}
-
-bool SignalPhaseManager::check_and_consume_cycle_boundary() {
+uint64_t SignalPhaseManager::cycle_boundary_count() const {
     std::lock_guard<std::mutex> lock(_mutex);
-    if (_cycle_boundary) {
-        _cycle_boundary = false;
-        return true;
-    }
-    return false;
+    return _cycle_boundary_count;
 }
 
 void SignalPhaseManager::set_reference_phase_id(int phase_id) {
     std::lock_guard<std::mutex> lock(_mutex);
     _reference_phase_id = phase_id;
     _prev_reference_light = 0;
+    _reference_observed = false;
 }
 
 }  // namespace fusion_perception::traffic_flow
diff --git a/src/traffic_flow/signal_phase_manager.h b/src/traffic_flow/signal_phase_manager.h
index bf31fee..1d56b8f 100644
--- a/src/traffic_flow/signal_phase_manager.h
+++ b/src/traffic_flow/signal_phase_manager.h
@@ -1,19 +1,14 @@
 /**
  * @file signal_phase_manager.h
- * @brief Thread-safe signal phase state manager.
- *        Receives signal data from DDS callback thread, provides query interface for detect thread.
+ * @brief Signal phase state manager: keeps every phase's latest light and counts cycle boundaries.
+ *        Receives signal data from DDS callback thread, query interface for detect thread.
  */
 #ifndef SIGNAL_PHASE_MANAGER_H
 #define SIGNAL_PHASE_MANAGER_H
 
+#include <cstdint>
 #include <mutex>
-#include <atomic>
-#include <memory>
-#include <thread>
-#include <chrono>
 #include <unordered_map>
-#include <vector>
-#include <cstdint>
 #include <nlohmann/json.hpp>
 
 namespace fusion_perception::traffic_flow {
@@ -25,32 +20,27 @@ constexpr int LIGHT_YELLOW = 7;
 
 class SignalPhaseManager {
 public:
-    SignalPhaseManager();
-    ~SignalPhaseManager();
+    SignalPhaseManager() = default;
 
     void on_signal_data(const nlohmann::json& signal_json);
 
+    // 暂无调用方，但刻意保留：所有相位的灯色都缓存在此，绿信比/WasteTime 等后续指标要查任意相位。
     bool is_green(int phase_id) const;
-    bool is_green_any(const std::vector<int>& phase_ids) const;
-
-    bool check_and_consume_cycle_boundary();
 
     void set_reference_phase_id(int phase_id);
 
-    void start_simulation(uint64_t interval_ms);
-    
-    bool is_simulation_running() const { return _sim_running.load(); }
+    // 参考相位「非绿 → 绿」的次数，即已到达的信号周期边界数。各通道记住自己看到过的次数，
+    // 因此一条信号报文可以同时触发多个 signal_cycle 通道。
+    uint64_t cycle_boundary_count() const;
 
 private:
     mutable std::mutex _mutex;
     std::unordered_map<int, int> _phase_states;
-    bool _cycle_boundary = false;
+    uint64_t _cycle_boundary_count = 0;
     int _reference_phase_id = -1;
     int _prev_reference_light = 0;
-
-    std::atomic<bool> _sim_running{false};
-    std::unique_ptr<std::thread> _sim_thread;
-    uint64_t _sim_interval_ms = 0;
+    // light 取值 0（灭灯）是合法状态，不能用它当「未观测到」的哨兵。
+    bool _reference_observed = false;
 };
 
 }  // namespace fusion_perception::traffic_flow
diff --git a/src/traffic_flow/traffic_flow_detector.h b/src/traffic_flow/traffic_flow_detector.h
index c1ef503..c9efec4 100644
--- a/src/traffic_flow/traffic_flow_detector.h
+++ b/src/traffic_flow/traffic_flow_detector.h
@@ -25,9 +25,6 @@
 namespace fusion_perception::traffic_flow {
 constexpr uint64_t OBJECT_CLEAR_TIME = 100000;    // in [ms]
 
-// 需求表统一的「排队」速度门限，进口道排队指标与出口道溢流指标共用。
-constexpr float QUEUE_SPEED_THRESHOLD = 10.0f;    // in [km/h]
-
 const static std::set<InnerType> g_motor_vehicle_types = {InnerType::CAR, InnerType::BUS, InnerType::TRUCK};
 
 // 取值即协议 VolumeN 的编号，Volume1(行人)~Volume5(大车)。
diff --git a/src/traffic_flow/traffic_flow_factory.cpp b/src/traffic_flow/traffic_flow_factory.cpp
index c7eb6cf..d6cbb02 100644
--- a/src/traffic_flow/traffic_flow_factory.cpp
+++ b/src/traffic_flow/traffic_flow_factory.cpp
@@ -195,7 +195,14 @@ bool TrafficFlowFactory::update_new_frame(const std::string& new_frame_data) {
         }
         PerceptionResult detected_result;
         try {
-            detected_result = nlohmann::json::parse(new_frame_data);
+            nlohmann::json frame_json = nlohmann::json::parse(new_frame_data);
+            // 信号灯报文被误发到感知 topic 时只会报「缺 dev_no」，看着像感知数据的问题，这里点名提示。
+            if (frame_json.contains("intersections")) {
+                std::cerr << "Signal data received on the perception topic, "
+                             "it must go to the SIGNAL_JSON_SUB (id 2) topic" << std::endl;
+                return false;
+            }
+            detected_result = frame_json;
         } catch (std::exception& e) {
             std::cerr << "Unable to parse JSON from message: " << e.what() << std::endl;
             return false;
@@ -214,22 +221,33 @@ bool TrafficFlowFactory::update_new_frame(const std::string& new_frame_data) {
 }
 
 void TrafficFlowFactory::on_signal_data(const nlohmann::json& signal_json) {
-    if (_signal_phase_manager) {
-        std::cout << "[SignalForward] Forwarding signal data to SignalPhaseManager" << std::endl;
-        _signal_phase_manager->on_signal_data(signal_json);
-    } else {
+    if (!_signal_phase_manager) {
         std::cerr << "[SignalForward] SignalPhaseManager is null!" << std::endl;
+        return;
     }
+    // 信号灯 topic 上收到非信号报文（多半是推送配置指错了数据文件），静默丢掉会查不到不出数的原因。
+    if (!signal_json.contains("intersections")) {
+        std::cerr << "[SignalForward] Dropped: message without 'intersections' on the signal topic" << std::endl;
+        return;
+    }
+    std::cout << "[SignalForward] Forwarding signal data to SignalPhaseManager" << std::endl;
+    _signal_phase_manager->on_signal_data(signal_json);
 }
 
 void TrafficFlowFactory::build_detectors_from_config() {
     auto& global_config = traffic_flow::GlobalConfig::get_instance();
     auto cycle_configs = global_config.get_detector_cycle_configs();
+    const QueueRulesConfig queue_rules = global_config.get_queue_rules();
+    std::cout << "[BuildDetectors] queue rules: entry(speed<" << queue_rules.entry_speed_kmh
+              << "km/h, " << queue_rules.entry_hold_frames << "frames, gap<" << queue_rules.entry_gap_body_ratio
+              << "body) overflow(speed<" << queue_rules.overflow_speed_kmh << "km/h, "
+              << queue_rules.overflow_hold_ms << "ms)" << std::endl;
 
     for (const auto& cycle_config : cycle_configs) {
         OutputChannel channel;
         channel.cycle_type = cycle_config.cycle_type;
         channel.interval_ms = cycle_config.cycle_duration_ms;
+        channel.align_to_grid = global_config.get_time_align_flag();
 
         for (const auto& spatial : cycle_config.spatial_output) {
             if (kValidSpatialOutputs.count(spatial) == 0) {
@@ -244,16 +262,15 @@ void TrafficFlowFactory::build_detectors_from_config() {
                       << "] has empty spatial_output, its reports will carry no Coil_List entry" << std::endl;
         }
 
-        if (cycle_config.cycle_type == "signal_cycle" && cycle_config.reference_phase_id > 0) {
-            _signal_phase_manager->set_reference_phase_id(cycle_config.reference_phase_id);
-            std::cout << "[BuildDetectors] Set reference_phase_id: "
-                      << cycle_config.reference_phase_id << std::endl;
-        }
-
-        if (cycle_config.cycle_type == "signal_cycle" && cycle_config.simulate_interval_ms > 0) {
-            _signal_phase_manager->start_simulation(cycle_config.simulate_interval_ms);
-            std::cout << "[BuildDetectors] Signal cycle simulation enabled, interval="
-                      << cycle_config.simulate_interval_ms << "ms" << std::endl;
+        if (cycle_config.cycle_type == "signal_cycle") {
+            if (cycle_config.reference_phase_id >= 0) {
+                _signal_phase_manager->set_reference_phase_id(cycle_config.reference_phase_id);
+                std::cout << "[BuildDetectors] Set reference_phase_id: "
+                          << cycle_config.reference_phase_id << std::endl;
+            } else {
+                std::cerr << "[BuildDetectors] Channel [signal_cycle] has no reference_phase_id, "
+                             "it will never output without a signal phase to watch" << std::endl;
+            }
         }
 
         auto road_sections = global_config.get_road_section_infos();
@@ -273,7 +290,7 @@ void TrafficFlowFactory::build_detectors_from_config() {
                 for (const auto& exit_lane_info : global_config.get_exit_lane_infos()) {
                     std::string detector_id = cycle_config.cycle_type + "_overflow_" + exit_lane_info.tag;
                     channel.detectors.push_back(
-                        std::make_unique<AccumulateOverflowDetector>(detector_id, exit_lane_info));
+                        std::make_unique<AccumulateOverflowDetector>(detector_id, exit_lane_info, queue_rules));
                     std::cout << "[BuildDetectors] Created AccumulateOverflowDetector: "
                               << detector_id << std::endl;
                 }
@@ -293,7 +310,7 @@ void TrafficFlowFactory::build_detectors_from_config() {
                     } else if (detector_item.type == "queue") {
                         std::string detector_id = cycle_config.cycle_type + "_queue_" + coil_info.tag;
                         channel.detectors.push_back(
-                            std::make_unique<AccumulateQueueDetector>(detector_id, coil_info));
+                            std::make_unique<AccumulateQueueDetector>(detector_id, coil_info, queue_rules));
                         std::cout << "[BuildDetectors] Created AccumulateQueueDetector: "
                                   << detector_id << std::endl;
                         handled = true;
@@ -306,12 +323,8 @@ void TrafficFlowFactory::build_detectors_from_config() {
                         handled = true;
                     } else if (detector_item.type == "headway") {
                         std::string detector_id = cycle_config.cycle_type + "_headway_" + coil_info.tag;
-                        auto headway_detector = std::make_unique<AccumulateHeadwayDetector>(detector_id, coil_info);
-                        if (cycle_config.cycle_type == "signal_cycle") {
-                            headway_detector->set_signal_phase_manager(
-                                _signal_phase_manager.get(), cycle_config.reference_phase_id);
-                        }
-                        channel.detectors.push_back(std::move(headway_detector));
+                        channel.detectors.push_back(
+                            std::make_unique<AccumulateHeadwayDetector>(detector_id, coil_info));
                         std::cout << "[BuildDetectors] Created AccumulateHeadwayDetector: "
                                   << detector_id << std::endl;
                         handled = true;
@@ -350,7 +363,7 @@ void TrafficFlowFactory::detect() {
         uint64_t current_time = detected_result.timestamp;
 
         for (auto& channel : _output_channels) {
-            if (!channel.should_output(current_time, _signal_phase_manager.get()))
+            if (!channel.check_trigger(current_time, _signal_phase_manager.get()))
                 continue;
 
             nlohmann::json all_results = nlohmann::json::array();
diff --git a/src/traffic_flow/traffic_flow_factory.h b/src/traffic_flow/traffic_flow_factory.h
index 02a1f22..7cfc833 100644
--- a/src/traffic_flow/traffic_flow_factory.h
+++ b/src/traffic_flow/traffic_flow_factory.h
@@ -75,48 +75,96 @@ struct OutputChannel {
     uint64_t window_open_ms = 0;
     uint64_t window_start_ms = 0;
     uint64_t window_stop_ms = 0;
+    uint64_t seen_cycle_boundary = 0;
     bool initialized = false;
-
-    // 触发判定走 interval_ms 的绝对栅格：只有帧时间戳跨入新的栅格槽才上报，
-    // 帧到达的抖动不会累积进周期（5min 不会变成 5min+Δ 且只增不减）。
-    // 上报窗口取「上次上报时刻 → 本次帧时刻」，与聚合器「算完即清零」的语义一致，窗口不重叠不留缝。
-    // 断流跨过的槽不补发（检测器状态本就跨流保留），并入本次窗口，一次性上报并告警。
-    bool should_output(uint64_t current_time, SignalPhaseManager* spm) {
+    bool align_to_grid = true;
+    bool head_partial = false;
+
+    // 栅格通道的触发判定走 interval_ms 的绝对栅格（align_to_grid=true，即配置项 time_align）：
+    // 只有帧时间戳跨入新的栅格槽才上报，帧到达的抖动不会累积进周期（5min 不会变成 5min+Δ 且只增不减）。
+    // 启动时切进来的头一段不是完整窗口，到下一个栅格边界时清零丢弃、不上报，
+    // 否则第一条 5min 报文只有几分钟的数据却仍标 Cycle=300。
+    // align_to_grid=false 则退回相对窗口：以首帧为起点，距上次上报满 interval_ms 就出。
+    // 上报窗口取「上次上报时刻 → 本次上报时刻」，与聚合器「算完即清零」的语义一致，窗口不重叠不留缝。
+    // 栅格模式下窗口标签即栅格时刻（16:40:00.000 → 16:45:00.000），不带帧到达的零头。
+    // 断流跨过的槽不补发（检测器状态本就跨流保留），并入本次窗口一次性上报并告警。
+    bool check_trigger(uint64_t current_time, SignalPhaseManager* spm) {
         const bool is_signal = (cycle_type == "signal_cycle");
         if (!is_signal && interval_ms == 0) {
             return false;
         }
-        if (is_signal && (!spm || !spm->check_and_consume_cycle_boundary())) {
-            return false;
+        if (is_signal) {
+            // 边界由真实信号报文推进：参考相位非绿转绿。没有信号数据时本通道就是不出数，不做定时模拟。
+            const uint64_t boundary_count = spm ? spm->cycle_boundary_count() : 0;
+            if (boundary_count == seen_cycle_boundary) {
+                return false;
+            }
+            seen_cycle_boundary = boundary_count;
         }
 
         if (!initialized) {
             initialized = true;
-            window_open_ms = is_signal ? current_time : current_time / interval_ms * interval_ms;
             window_stop_ms = current_time;
+            if (is_signal) {
+                window_open_ms = current_time;
+                head_partial = false;
+            } else if (align_to_grid) {
+                const uint64_t grid = current_time / interval_ms * interval_ms;
+                window_open_ms = grid;
+                // 首帧正好踩在栅格边界上时，首个窗口本来就是完整的，不算残窗。
+                head_partial = (current_time != grid);
+            } else {
+                window_open_ms = current_time;
+            }
+            // 信号周期级开窗时边界之前累积的数据属于上一个周期（或模块启动前的时段），丢掉才能和窗口标签对齐。
+            if (is_signal) {
+                drain_accumulators();
+            }
             return false;
         }
 
-        if (!is_signal) {
-            uint64_t grid_open_ms = current_time / interval_ms * interval_ms;
+        if (is_signal) {
+            window_start_ms = window_stop_ms;
+            window_stop_ms = current_time;
+        } else if (align_to_grid) {
+            const uint64_t grid_open_ms = current_time / interval_ms * interval_ms;
             if (grid_open_ms <= window_open_ms) {
                 return false;
             }
-            uint64_t merged_windows = current_time / interval_ms - window_open_ms / interval_ms;
+            const uint64_t merged_windows = current_time / interval_ms - window_open_ms / interval_ms;
             if (merged_windows > 1) {
-                std::cerr << "[OutputChannel] " << cycle_type << " stalled for " << (current_time - window_stop_ms)
-                          << "ms, merged " << merged_windows << " windows of " << interval_ms << "ms" << std::endl;
+                std::cerr << "[OutputChannel] " << cycle_type << " merged " << merged_windows << " windows of "
+                          << interval_ms << "ms, no frame in between" << std::endl;
             }
+            // 标签取栅格时刻：帧落在槽内的哪一拍是到达抖动，不该写进 StartTime/StopTime。
+            window_start_ms = window_open_ms;
+            window_stop_ms = grid_open_ms;
             window_open_ms = grid_open_ms;
+        } else {
+            if (current_time - window_stop_ms < interval_ms) {
+                return false;
+            }
+            window_start_ms = window_stop_ms;
+            window_stop_ms = current_time;
         }
 
-        window_start_ms = window_stop_ms;
-        window_stop_ms = current_time;
+        if (head_partial) {
+            head_partial = false;
+            drain_accumulators();
+            return false;
+        }
         return true;
     }
 
+    // 丢弃当前累积状态：窗口标签之外的数据不该被算进下一条报文。
+    void drain_accumulators() {
+        for (auto& detector : detectors) {
+            detector->calculate_once();
+        }
+    }
+
     int get_cycle_value() const {
-        // signal_cycle 的窗口由相位边界决定，不是固定秒数；其余通道节奏就是 interval_ms。
+        // signal_cycle 的窗口长度看 StartTime/StopTime；Cycle 固定 -1，作为下游识别周期级报文的标记。
         if (cycle_type == "signal_cycle") return -1;
         return static_cast<int>(interval_ms / 1000);
     }
@@ -135,8 +183,6 @@ public:
     bool start_running();
     bool stop_running();
 
-    SignalPhaseManager& get_signal_phase_manager() { return *_signal_phase_manager; }
-
 private:
     TrafficFlowFactory();
     virtual ~TrafficFlowFactory();
```
