# 7f71330  deqing debug,using id instead of inner_id

- 作者：fengwenxi
- 日期：2025-02-18
- 完整 hash：`7f71330353b1df4c97ef00a5596e494d60c98f78`
- 改动：+49 / -14 行，涉及 5 个文件

涉及文件：
- `src/common/config.cpp`
- `src/common/track_info.h`
- `src/traffic_flow/detectors/volume_detector.cpp`
- `src/traffic_flow/traffic_flow_detector.h`
- `src/traffic_flow/traffic_flow_factory.cpp`

```diff
diff --git a/src/common/config.cpp b/src/common/config.cpp
index a2c52d1..63c0dbd 100644
--- a/src/common/config.cpp
+++ b/src/common/config.cpp
@@ -114,6 +114,16 @@ GlobalConfig::GlobalConfig(const std::string& config_path) {
         std::cerr << "Error while construct global config." << std::endl;
         assert(false);
     }
+#ifdef DEBUG
+    std::cout << "<-----------------Global Config----------------->" << std::endl;
+    for (const auto& it : _detector_configs) {
+        std::cout << "Coil tag: " << it.first << std::endl;
+        for (const auto& one_detector : it.second) {
+            std::cout << "Detector type: " << one_detector.tag << std::endl;
+        }
+    }
+    std::cout << "<-----------------Global Config----------------->" << std::endl;
+#endif
 }
 
 bool GlobalConfig::init(const std::string& config_path) {
@@ -131,19 +141,19 @@ WorkingMode GlobalConfig::get_working_mode() {
     return _working_mode;
 }
 
-std::string GlobalConfig::get_region_id(){
+std::string GlobalConfig::get_region_id() {
     return _region_id;
 }
 
-std::string GlobalConfig::get_dev_no(){
+std::string GlobalConfig::get_dev_no() {
     return _dev_no;
 }
 
-std::string GlobalConfig::get_mec_no(){
+std::string GlobalConfig::get_mec_no() {
     return _mec_no;
 }
 
-uint64_t GlobalConfig::get_ensemble_cycle(){
+uint64_t GlobalConfig::get_ensemble_cycle() {
     return _ensemble_cycle;
 }
 
@@ -191,7 +201,7 @@ bool GlobalConfig::construct_detector_config() {
                 one_detector_config.working_mode = _working_mode;
                 one_detector_config.coil_info = one_coil_info;
                 one_detector_config.tag = one_coil_info.tag + detector_type_to_string(one_detector_config.type);
-                _detector_configs[one_coil_info.road_section_info.tag].emplace_back(one_detector_config);
+                _detector_configs[one_coil_info.tag].emplace_back(one_detector_config);
             }
         }
     }
diff --git a/src/common/track_info.h b/src/common/track_info.h
index 1cc2fff..c169cd5 100644
--- a/src/common/track_info.h
+++ b/src/common/track_info.h
@@ -34,7 +34,7 @@ struct TrackPosition {
 };
 
 struct TrackInfo {
-    uint64_t id = 0;
+    std::string id;
     bool is_intersected = false;
     float object_length = 0.0;
     InnerType type = InnerType::DEFAULT;
diff --git a/src/traffic_flow/detectors/volume_detector.cpp b/src/traffic_flow/detectors/volume_detector.cpp
index 91a95b2..d559a21 100644
--- a/src/traffic_flow/detectors/volume_detector.cpp
+++ b/src/traffic_flow/detectors/volume_detector.cpp
@@ -70,7 +70,7 @@ void VolumeDetector::update_volume() {
                 one_object_track.second.is_intersected = true;
 #ifdef DEBUG
                 std::cout << "[DEBUG] VolumeDetector " << detector_type_to_string(_detector_config.type)
-                          << " tag: " << _detector_config.tag << " calculates once, innerid: " << one_object_track.first
+                          << " tag: " << _detector_config.tag << " calculates once, id: " << one_object_track.first
                           << " from " << _current_period_start << " in " << _detector_config.detect_period << " ms."
                           << std::endl;
 #endif
@@ -104,11 +104,11 @@ void VolumeDetector::update_volume() {
 
 void VolumeDetector::update(const PerceptionResult& new_frame_result) {
     for (const auto& one_object : new_frame_result.traffic_objects) {
-        auto& object_track_info = _object_tracks[one_object.inner_id];
+        auto& object_track_info = _object_tracks[one_object.id];
         object_track_info.track.emplace_back(one_object.latitude, one_object.longitude, one_object.speed,
                                              new_frame_result.timestamp);
-        if (object_track_info.id != one_object.inner_id) {
-            object_track_info.id = one_object.inner_id;
+        if (object_track_info.id != one_object.id) {
+            object_track_info.id = one_object.id;
         }
         if (object_track_info.type != one_object.inner_type) {
             object_track_info.type = one_object.inner_type;
diff --git a/src/traffic_flow/traffic_flow_detector.h b/src/traffic_flow/traffic_flow_detector.h
index 527a891..fbd44c0 100644
--- a/src/traffic_flow/traffic_flow_detector.h
+++ b/src/traffic_flow/traffic_flow_detector.h
@@ -50,8 +50,8 @@ public:
 protected:
     DetectorConfig _detector_config;
     result_deliver_callback _deliver_callback;
-    // key: inner_id, value: track info
-    std::map<uint64_t, TrackInfo> _object_tracks;
+    // key: object_id, value: track info
+    std::map<std::string, TrackInfo> _object_tracks;
 };
 
 }  // namespace fusion_perception::traffic_flow
diff --git a/src/traffic_flow/traffic_flow_factory.cpp b/src/traffic_flow/traffic_flow_factory.cpp
index d7a30f5..276f658 100644
--- a/src/traffic_flow/traffic_flow_factory.cpp
+++ b/src/traffic_flow/traffic_flow_factory.cpp
@@ -49,7 +49,7 @@ TrafficFlowFactory::TrafficFlowFactory()
         std::vector<CoilInfo> coil_infos = global_config.get_coil_infos(one_road_section_info.first);
         for (const auto& one_coil_info : coil_infos) {
             std::vector<DetectorConfig> detector_configs =
-                global_config.get_detector_config(one_road_section_info.first);
+                global_config.get_detector_config(one_coil_info.tag);
             for (const auto& one_detector_config : detector_configs) {
                 std::unique_ptr<TrafficFlowDetector> one_detector_ptr(
                     TrafficFlowFactory::create_detector(one_detector_config));
@@ -57,6 +57,20 @@ TrafficFlowFactory::TrafficFlowFactory()
             }
         }
     }
+#ifdef DEBUG
+    std::cout << "coil num: " << _coil_detectors.size() << std::endl;
+    for (const auto& it : _coil_detectors) {
+        std::cout << "Coil tag: " << it.first << std::endl;
+        for (const auto& one_detector : it.second) {
+            std::cout << "Detector type: " << one_detector->get_detector_config().tag << std::endl;
+            std::cout << "Detector stop line: " << std::endl;
+            for (const auto& one_segment : one_detector->get_detector_config().coil_info.stop_line_segments) {
+                std::cout << one_segment.to_string() << "\t";
+            }
+            std::cout << std::endl;
+        }
+    }
+#endif
     _ensemble_detect_period = global_config.get_ensemble_cycle();
     _working_mode = global_config.get_working_mode();
     _working_thread = std::make_unique<std::thread>(&TrafficFlowFactory::detect, this);
@@ -200,12 +214,19 @@ void TrafficFlowFactory::detect() {
                 default_ensemble_result(ensemble_result_j, get_readable_timestamp(detected_result.timestamp));
                 auto& global_config = traffic_flow::GlobalConfig::get_instance();
                 auto road_sections = global_config.get_road_section_infos();
-                for (const auto& one_road_section : road_sections) {                
+                for (const auto& one_road_section : road_sections) {
                     std::vector<CoilInfo> coils = global_config.get_coil_infos(one_road_section.first);
                     for (const auto& one_coil : coils) {
+#ifdef DEBUG
+                        std::cout << "[DEBUG] detect coil: " << one_coil.tag << std::endl;
+#endif
                         nlohmann::json detected_result_j;
                         default_coil_result(detected_result_j, one_coil);
                         for (const auto& one_detector : _coil_detectors[one_coil.tag]) {
+#ifdef DEBUG
+                            std::cout << "[DEBUG] detect detector: " << one_detector->get_detector_config().tag
+                                      << std::endl;
+#endif
                             one_detector->calculate_once(detected_result_j);
                         }
                         ensemble_result_j["Coil_List"].emplace_back(detected_result_j);
@@ -214,6 +235,10 @@ void TrafficFlowFactory::detect() {
                 ensemble_result_j["CoilNum"] = ensemble_result_j["Coil_List"].size();
                 _current_period_start = detected_result.timestamp;
                 _publish_callback(ensemble_result_j.dump());
+
+#ifdef DEBUG
+                std::cout << "[DEBUG] detect result: " << ensemble_result_j.dump() << std::endl;
+#endif
             }
         } else {
             std::cerr << "Unsupported working mode: " << working_mode_to_string(_working_mode) << std::endl;
```
