# 7a5928c  agreement output fix

- 作者：fengwenxi
- 日期：2025-06-26
- 完整 hash：`7a5928ce83c467ae7517671d8647088c5c5fe3bd`
- 改动：+1 / -1 行，涉及 1 个文件

涉及文件：
- `src/traffic_flow/traffic_flow_factory.cpp`

```diff
diff --git a/src/traffic_flow/traffic_flow_factory.cpp b/src/traffic_flow/traffic_flow_factory.cpp
index e2f215d..77bb008 100644
--- a/src/traffic_flow/traffic_flow_factory.cpp
+++ b/src/traffic_flow/traffic_flow_factory.cpp
@@ -137,7 +137,7 @@ void TrafficFlowFactory::default_coil_result(nlohmann::json& coil_result, const
     coil_result["UpstreamCrossID"] = coil_info.road_section_info.upstream_cross_id;
     coil_result["MeasNo"] = coil_info.road_section_info.measure_no;
     coil_result["LaneID"] = coil_info.lane_id;
-    coil_result["LaneNO"] = coil_info.lane_no;
+    coil_result["LaneNo"] = coil_info.lane_no;
     coil_result["LaneAttribute"] = coil_info.turn_dir_no;
     coil_result["CoilNo"] = coil_info.coil_no;
     coil_result["Volume"] = 0;
```
