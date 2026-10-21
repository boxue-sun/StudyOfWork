# 63c9b08  change default destruct func

- 作者：fengwenxi
- 日期：2025-02-18
- 完整 hash：`63c9b0817da844faa90f8d371eb164570cab6301`
- 改动：+1 / -1 行，涉及 1 个文件

涉及文件：
- `src/traffic_flow/detectors/occupancy_detector.cpp`

```diff
diff --git a/src/traffic_flow/detectors/occupancy_detector.cpp b/src/traffic_flow/detectors/occupancy_detector.cpp
index 0bf7f09..b49fb4e 100644
--- a/src/traffic_flow/detectors/occupancy_detector.cpp
+++ b/src/traffic_flow/detectors/occupancy_detector.cpp
@@ -21,7 +21,7 @@ namespace fusion_perception::traffic_flow {
 OccupancyDetector::OccupancyDetector(const DetectorConfig& detector_config)
     : TrafficFlowDetector(detector_config), _current_period_start(0) {}
 
-OccupancyDetector::~OccupancyDetector() {}
+// OccupancyDetector::~OccupancyDetector() {}
 
 void OccupancyDetector::update(const PerceptionResult& new_frame_result) {
 //     for (const auto& one_object : new_frame_result.traffic_objects) {
```
