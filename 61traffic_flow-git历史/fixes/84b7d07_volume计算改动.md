# 84b7d07  change volume calculation

- 作者：fengwenxi
- 日期：2025-02-18
- 完整 hash：`84b7d07f25389e12f48f026af1779ee7746595b6`
- 改动：+5 / -5 行，涉及 2 个文件

涉及文件：
- `src/traffic_flow/detectors/volume_detector.cpp`
- `src/traffic_flow/traffic_flow_detector.h`

```diff
diff --git a/src/traffic_flow/detectors/volume_detector.cpp b/src/traffic_flow/detectors/volume_detector.cpp
index b1be0b4..91a95b2 100644
--- a/src/traffic_flow/detectors/volume_detector.cpp
+++ b/src/traffic_flow/detectors/volume_detector.cpp
@@ -140,7 +140,7 @@ void VolumeDetector::update(const PerceptionResult& new_frame_result) {
 
 // TODO: Update current_period_start
 void VolumeDetector::calculate_once(nlohmann::json& result_j) {
-    _volume = _volume1 + _volume2 + _volume3 + _volume4 + _volume5;
+    _volume = _volume3 + _volume4 + _volume5;
     result_j["Volume"] = _volume;
     result_j["Volume1"] = _volume1;
     result_j["Volume2"] = _volume2;
diff --git a/src/traffic_flow/traffic_flow_detector.h b/src/traffic_flow/traffic_flow_detector.h
index bd1091f..527a891 100644
--- a/src/traffic_flow/traffic_flow_detector.h
+++ b/src/traffic_flow/traffic_flow_detector.h
@@ -25,14 +25,14 @@
 
 namespace fusion_perception::traffic_flow {
 constexpr uint64_t DEFAULT_DETECT_PERIOD = 5000;  // in ms
-constexpr uint64_t OBJECT_CLEAR_TIME = 100000;     // in ms
+constexpr uint64_t OBJECT_CLEAR_TIME = 100000;    // in ms
 
 const static std::set<InnerType> g_all_types = {InnerType::PEDESTRIAN, InnerType::CAR,          InnerType::BUS,
                                                 InnerType::TRUCK,      InnerType::TRAFFIC_CONE, InnerType::BICYCLE,
                                                 InnerType::TRICYCLE,   InnerType::PLATE};
-const static std::set<InnerType> g_motor_vehicle_types = {InnerType::CAR, InnerType::BUS, InnerType::TRUCK,
-                                                          InnerType::BICYCLE, InnerType::TRICYCLE};
-const static std::set<InnerType> g_non_motor_vehicle_types = {InnerType::PEDESTRIAN};
+const static std::set<InnerType> g_motor_vehicle_types = {InnerType::CAR, InnerType::BUS, InnerType::TRUCK};
+const static std::set<InnerType> g_non_motor_vehicle_types = {InnerType::PEDESTRIAN, InnerType::BICYCLE,
+                                                              InnerType::TRICYCLE};
 const static std::set<InnerType> g_pedestrian_types = {InnerType::PEDESTRIAN};
 
 class TrafficFlowDetector : public afl::Noncopyable {
```
