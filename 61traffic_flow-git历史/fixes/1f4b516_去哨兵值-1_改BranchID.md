# 1f4b516  （fix）增加BranchType，修改BranchID，去哨兵值-1

- 作者：fengxin
- 日期：2026-09-16
- 完整 hash：`1f4b51673fa9804910208b2fc12fdd968af363a4`
- 改动：+38 / -49 行，涉及 9 个文件

涉及文件：
- `src/common/config.h`
- `src/traffic_flow/detectors/accumulate_avg_stop_times_detector.cpp`
- `src/traffic_flow/detectors/accumulate_avg_stop_times_detector.h`
- `src/traffic_flow/detectors/accumulate_delay_detector.cpp`
- `src/traffic_flow/detectors/accumulate_delay_detector.h`
- `src/traffic_flow/detectors/accumulate_headway_detector.cpp`
- `src/traffic_flow/detectors/accumulate_headway_distance_detector.cpp`
- `src/traffic_flow/detectors/accumulate_headway_distance_detector.h`
- `src/traffic_flow/traffic_flow_factory.cpp`

```diff
diff --git a/src/common/config.h b/src/common/config.h
index bb3887c..055e38e 100644
--- a/src/common/config.h
+++ b/src/common/config.h
@@ -216,10 +216,11 @@ void from_json(const nlohmann::json& j, StopRulesConfig& rules);
 // 需求表给的是直行 6s、左转 9s、右转 12s（每 50m），掉头需求表未列，与右转同为 12s。
 // 一条车道可能同时挂多个转向（turn_dir_no 如 0011=直行+左转），取最大值——基准取小会把该慢的方向
 // 也当成快方向，延误只可能记少不会记多，是漏报方向上更保守的选择。
-// turn_dir_no 缺失或全 0 时按直行兜底（宁可有值也不整条车道出 -1）。
+// turn_dir_no 缺失或全 0 时按直行兜底（宁可用一个偏乐观的基准，也不让自由流秒数退化成 0，
+// 那样整段行程时间都会被算成延误）。
 // upstream_section_m：上游固定检测断面离停止线的距离。0 表示不截断，用车道轮廓远端那条边。
 // 轮廓常画到传感器实际覆盖之外（212 进口道轮廓 36~44m，轨迹最远只观到 30~36m），此时远端断面永远
-// 等不到车穿越，DelayTime 整条车道恒 -1；配一个落在覆盖范围内的米数即可出数，配得比轮廓还长则按轮廓。
+// 等不到车穿越，DelayTime 整条车道恒 0（无样本口径）；配一个落在覆盖范围内的米数即可出数，配得比轮廓还长则按轮廓。
 struct DelayRulesConfig {
     float straight_s = 6.0f;
     float left_s = 9.0f;
diff --git a/src/traffic_flow/detectors/accumulate_avg_stop_times_detector.cpp b/src/traffic_flow/detectors/accumulate_avg_stop_times_detector.cpp
index 53ab1a3..7b0388a 100644
--- a/src/traffic_flow/detectors/accumulate_avg_stop_times_detector.cpp
+++ b/src/traffic_flow/detectors/accumulate_avg_stop_times_detector.cpp
@@ -18,7 +18,7 @@ AccumulateAvgStopTimesDetector::AccumulateAvgStopTimesDetector(
 
     if (frame.along(section.middle_point()) <= 0.0) {
         std::cerr << "[AvgStopTimes] " << _detector_id << " unusable upstream section, lane contour degenerate, "
-                  << "AvgStopTimes will stay -1" << std::endl;
+                  << "AvgStopTimes will stay 0" << std::endl;
     }
 }
 
@@ -123,10 +123,10 @@ nlohmann::json AccumulateAvgStopTimesDetector::calculate_once() {
     entry["lane_no"] = _coil_info.lane_no;
     entry["road_section_tag"] = _coil_info.road_section_info.tag;
     entry["turn_dir_no"] = _coil_info.turn_dir_no;
-    // 窗口内没有完整通过本区间的有效车辆时写 -1，不能写 0：0 表示「一辆车都没停过」，和「没车可测」是两回事。
+    // 窗口内没有完整通过本区间的有效车辆时写 0（全模块哨兵统一为 0），与「一辆车都没停过」不可区分。
     entry["AvgStopTimes"] = _settled_vehicles > 0
                                 ? static_cast<double>(_settled_stops) / _settled_vehicles
-                                : -1.0;
+                                : 0.0;
 
     _settled_stops = 0;
     _settled_vehicles = 0;
diff --git a/src/traffic_flow/detectors/accumulate_avg_stop_times_detector.h b/src/traffic_flow/detectors/accumulate_avg_stop_times_detector.h
index b71111a..4432719 100644
--- a/src/traffic_flow/detectors/accumulate_avg_stop_times_detector.h
+++ b/src/traffic_flow/detectors/accumulate_avg_stop_times_detector.h
@@ -19,7 +19,7 @@ namespace fusion_perception::traffic_flow {
 // 「如发生变道，从有效车辆中删除」，不需要额外的变道检测代码；与延误检测器同一套配对口径。
 // 加热那一拍会把此前累计的停车次数清零：断面之前（上游路段、上一个路口的排队）的停车不算本区间的账。
 // 一次由行驶进入停车计 1 次，停着不动不重复计，重新起速后再停另计（见 stop_rules 的迟滞门限）。
-// 只对进口道线圈建实例、只挂 5min 通道（见 config/traffic_flow/traffic_flow_config.json）。
+// 只对进口道线圈建实例、只挂 5min 通道（见 config/traffic_flow_config.json）。
 class AccumulateAvgStopTimesDetector : public AccumulateDetector {
 public:
     AccumulateAvgStopTimesDetector(const std::string& detector_id, const CoilInfo& coil_info,
diff --git a/src/traffic_flow/detectors/accumulate_delay_detector.cpp b/src/traffic_flow/detectors/accumulate_delay_detector.cpp
index c79d595..3e3ad18 100644
--- a/src/traffic_flow/detectors/accumulate_delay_detector.cpp
+++ b/src/traffic_flow/detectors/accumulate_delay_detector.cpp
@@ -28,7 +28,7 @@ double free_flow_base_s(const std::string& turn_dir_no, const DelayRulesConfig&
     if (turn_dir_no.size() > 0 && turn_dir_no[0] == '1') {
         base = std::max(base, static_cast<double>(rules.uturn_s));
     }
-    // 没配转向的车道按直行兜底：整条车道恒 -1 比给一个偏乐观的基准更难排查。
+    // 没配转向的车道按直行兜底：基准退化成 0 会让整段行程时间都被算成延误，比一个偏乐观的基准更难排查。
     return base > 0.0 ? base : static_cast<double>(rules.straight_s);
 }
 
@@ -51,7 +51,7 @@ AccumulateDelayDetector::AccumulateDelayDetector(const std::string& detector_id,
     }
     if (_free_flow_s <= 0.0) {
         std::cerr << "[Delay] " << _detector_id << " unusable region, len_m=" << _region_len_m
-                  << " turn_dir_no=" << _coil_info.turn_dir_no << ", DelayTime will stay -1" << std::endl;
+                  << " turn_dir_no=" << _coil_info.turn_dir_no << ", DelayTime will stay 0" << std::endl;
     }
 }
 
@@ -113,8 +113,8 @@ nlohmann::json AccumulateDelayDetector::calculate_once() {
     entry["lane_no"] = _coil_info.lane_no;
     entry["road_section_tag"] = _coil_info.road_section_info.tag;
     entry["turn_dir_no"] = _coil_info.turn_dir_no;
-    // 无完整通过的车辆时写 -1，不能写 0：0 表示「一辆车都没延误」，和「没车可测」是两回事。
-    entry["DelayTime"] = _settled_vehicles > 0 ? _delay_sum_s / _settled_vehicles : -1.0;
+    // 无完整通过的车辆时写 0（全模块哨兵统一为 0），与「一辆车都没延误」不可区分。
+    entry["DelayTime"] = _settled_vehicles > 0 ? _delay_sum_s / _settled_vehicles : 0.0;
 
     _delay_sum_s = 0.0;
     _settled_vehicles = 0;
diff --git a/src/traffic_flow/detectors/accumulate_delay_detector.h b/src/traffic_flow/detectors/accumulate_delay_detector.h
index cf48340..3124992 100644
--- a/src/traffic_flow/detectors/accumulate_delay_detector.h
+++ b/src/traffic_flow/detectors/accumulate_delay_detector.h
@@ -16,12 +16,12 @@ namespace fusion_perception::traffic_flow {
 
 // 车均延误[s]：窗口内通过本进口车道停止线的车，在「上游固定检测断面 → 停止线」区间的实际行程时间
 // 与同区间自由流行程时间之差的平均值。上游断面默认取车道轮廓远端那条边，可配 `upstream_section_m`
-// 往里截：轮廓常被画到传感器实际覆盖之外（212 进口道 36~44m，轨迹只观到 30~36m），断面够不到就恒 -1。
+// 往里截：轮廓常被画到传感器实际覆盖之外（212 进口道 36~44m，轨迹只观到 30~36m），断面够不到就恒 0（无样本哨兵）。
 // 只有「先穿越上游断面、再穿越停止线」的车才算一次完整通过：起点就落在区间里的车没有可测的上游断面，
 // 不计入。变道车同样自动出局——加热用的是本车道的上游断面线段、结算用的是本车道的停止线线段，
 // 中途并到邻道就再也不会与本车道的停止线相交，两端配不上对，不需要额外的变道检测。
 // 实际行程快于自由流时差值为负，按 0 计而不是丢样本：那是「没延误」，不是「没测到」。
-// 时间颗粒度只挂 5min（见 config/traffic_flow/traffic_flow_config.json），且只对进口道线圈建实例。
+// 时间颗粒度只挂 5min（见 config/traffic_flow_config.json），且只对进口道线圈建实例。
 class AccumulateDelayDetector : public AccumulateDetector {
 public:
     AccumulateDelayDetector(const std::string& detector_id, const CoilInfo& coil_info, const DelayRulesConfig& rules);
diff --git a/src/traffic_flow/detectors/accumulate_headway_detector.cpp b/src/traffic_flow/detectors/accumulate_headway_detector.cpp
index 878801a..e614551 100644
--- a/src/traffic_flow/detectors/accumulate_headway_detector.cpp
+++ b/src/traffic_flow/detectors/accumulate_headway_detector.cpp
@@ -59,16 +59,16 @@ nlohmann::json AccumulateHeadwayDetector::calculate_once() {
     for (auto& [lane_tag, acc] : _lane_acc) {
         nlohmann::json entry = acc.meta;
         std::string detail;
-        // 无样本写 -1 而不是 0：窗口内不足两辆过线车就构不成时距，而 0 是「两车同一毫秒过线」的真实
-        // 极端值，两者不能混。与 HeadwayDistance/AvgStopTimes 同一约定，也才对得上协议 #23 的「≤1 个目标时写 -1」。
-        // 明细串不按此约定改写：需求表 #10 规定「周期内无车或仅一辆车通过则集合仅含一个元素 0」。
+        // 无样本写 0（全模块哨兵统一为 0）：代价是与「两车同一毫秒过线」的真实极端值不可区分，
+        // 下游要判断有没有样本得看 HeadTimeDiffDetail 的元素个数。
+        // 明细串仍按需求表 #10：「周期内无车或仅一辆车通过则集合仅含一个元素 0」。
         if (acc.count == 0) {
-            entry["Headway"] = -1.0;
-            entry["Gap"] = -1.0;
+            entry["Headway"] = 0.0;
+            entry["Gap"] = 0.0;
             detail = "0";
         } else if (acc.count == 1) {
-            entry["Headway"] = -1.0;
-            entry["Gap"] = -1.0;
+            entry["Headway"] = 0.0;
+            entry["Gap"] = 0.0;
             detail = acc.vehicle_types.front() + ";0";
         } else {
             entry["Headway"] = acc.headway_sum / double(acc.count - 1) / 1000.0;
diff --git a/src/traffic_flow/detectors/accumulate_headway_distance_detector.cpp b/src/traffic_flow/detectors/accumulate_headway_distance_detector.cpp
index 00db8f4..a3ecc53 100644
--- a/src/traffic_flow/detectors/accumulate_headway_distance_detector.cpp
+++ b/src/traffic_flow/detectors/accumulate_headway_distance_detector.cpp
@@ -88,8 +88,8 @@ nlohmann::json AccumulateHeadwayDistanceDetector::calculate_once() {
     entry["lane_no"] = _coil_info.lane_no;
     entry["road_section_tag"] = _coil_info.road_section_info.tag;
     entry["turn_dir_no"] = _coil_info.turn_dir_no;
-    // 无样本写 -1 而不是 0：0 表示车头贴车尾，是有物理含义的极端值。
-    entry["HeadwayDistance"] = _pair_count > 0 ? _dist_sum_m / _pair_count : -1.0;
+    // 无样本写 0（全模块哨兵统一为 0），与「车头贴车尾」的真实极端值不可区分。
+    entry["HeadwayDistance"] = _pair_count > 0 ? _dist_sum_m / _pair_count : 0.0;
 
     _dist_sum_m = 0.0;
     _pair_count = 0;
diff --git a/src/traffic_flow/detectors/accumulate_headway_distance_detector.h b/src/traffic_flow/detectors/accumulate_headway_distance_detector.h
index 7f04dce..99198b2 100644
--- a/src/traffic_flow/detectors/accumulate_headway_distance_detector.h
+++ b/src/traffic_flow/detectors/accumulate_headway_distance_detector.h
@@ -17,7 +17,7 @@ namespace fusion_perception::traffic_flow {
 // 每帧把中心点落在区间内的机动车沿车道行驶方向排序，相邻两两成对；窗口内全部有效车对合并取算术平均。
 // 两车等长时本量正好等于两车中心点距离（需求表允许的简化口径），不等长时差 (L_前−L_后)/2。
 // 与车头时距（AccumulateHeadwayDetector）口径不同：那个是断面级、只有过线瞬间才有样本，本项是区间内的瞬时跟驰状态。
-// 时间颗粒度只挂 1s / 5min（见 config/traffic_flow/traffic_flow_config.json）。
+// 时间颗粒度只挂 1s / 5min（见 config/traffic_flow_config.json）。
 class AccumulateHeadwayDistanceDetector : public AccumulateDetector {
 public:
     AccumulateHeadwayDistanceDetector(const std::string& detector_id, const CoilInfo& coil_info);
diff --git a/src/traffic_flow/traffic_flow_factory.cpp b/src/traffic_flow/traffic_flow_factory.cpp
index 4892893..57bc626 100644
--- a/src/traffic_flow/traffic_flow_factory.cpp
+++ b/src/traffic_flow/traffic_flow_factory.cpp
@@ -61,36 +61,28 @@ nlohmann::json merge_by_key(const nlohmann::json& items, const std::string& key_
     return merged;
 }
 
-// BranchID 是感知合成的断面标识，NodeID/UpstreamCrossID 必须回到 ROI 里配置的路口编号，按 tag 反查。
+// 记录标识里的 BranchID 上报断面的 8 方向码，即 ROI 的 f_dir_8_no 原值（1北 2东 3南 4西 5东北 6东南 7西南 8西北），
+// NodeID/UpstreamCrossID 回到 ROI 里配置的路口编号，都按断面 tag 反查。
+// 反查不到时方向填 0（DirectionCode::DEFAULT），与 NodeID/UpstreamCrossID 填空串一致：宁可显式无值，不猜方向。
+//
+// BranchType 真值只有 ROI 的 lane_sdtype_no，报文里推断不出来：出口道填 1，其余断面（进口道、人行横道、
+// 中间路段、非机动车道，以及 tag 反查不到的兜底）一律填 0，而不是省略字段。三个空间颗粒度都要带：同方向的
+// 进口道/出口道/人行横道共用一个方向码，只靠 BranchID 分不开断面，下游要靠 BranchID+BranchType(+FlowType/LaneID)
+// 才能唯一定位记录。
 void fill_section_ids(nlohmann::json& entry,
                       const std::unordered_map<std::string, RoadSectionInfo>& road_sections,
                       const std::string& section_tag) {
     auto find_it = road_sections.find(section_tag);
-    entry["NodeID"] = find_it != road_sections.end() ? find_it->second.node_id : "";
-    entry["UpstreamCrossID"] = find_it != road_sections.end() ? find_it->second.upstream_cross_id : "";
-    entry["BranchID"] = section_tag;
-}
-
-// 分支级的 BranchType（0 进口道 / 1 出口道）真值只有 ROI 的 lane_sdtype_no，报文里推断不出来。
-// 斑马线等非进/出口断面取不到值，返回 -1 由调用方原样上报（不能兜底成进口道）。
-int branch_type_of(const std::unordered_map<std::string, RoadSectionInfo>& road_sections,
-                   const std::string& section_tag) {
-    auto find_it = road_sections.find(section_tag);
-    if (find_it == road_sections.end()) {
-        return -1;
-    }
-    if (find_it->second.type == LaneType::ENTRY_SECTION) {
-        return 0;
-    }
-    if (find_it->second.type == LaneType::EXIT_SECTION) {
-        return 1;
-    }
-    return -1;
+    const bool found = find_it != road_sections.end();
+    entry["NodeID"] = found ? find_it->second.node_id : "";
+    entry["UpstreamCrossID"] = found ? find_it->second.upstream_cross_id : "";
+    entry["BranchID"] = static_cast<int>(found ? find_it->second.direction : DirectionCode::DEFAULT);
+    entry["BranchType"] = (found && find_it->second.type == LaneType::EXIT_SECTION) ? 1 : 0;
 }
 
 // 报文浮点字段统一保留 2 位小数。nlohmann 对 double 走「最短往返」序列化，没有按位格式化的开关，
 // 所以只能先把值舍到 0.01 再写：这同时消掉 float 累加带出的假精度尾数（1.7999999523162842 -> 1.8）。
-// 只碰 number_float，整型字段（计数、标志位、Cycle 等）原样出；-1 这类哨兵舍完还是 -1.0。
+// 只碰 number_float，整型字段（计数、标志位、Cycle 等）原样出。
 //
 // 必须走「格式化成 2 位小数字符串再读回」，不能写 std::round(v * 100.0) / 100.0：生产构建开了
 // -ffast-math（CMakeLists.txt:90），GCC 据此把「除以常量 100.0」换成「乘 0.01」，而 0.01 二进制不可
@@ -223,13 +215,9 @@ nlohmann::json TrafficFlowFactory::build_output(
     if (channel.spatial_output.count("BRANCH")) {
         for (const auto& branch : branch_results) {
             nlohmann::json entry;
-            const std::string branch_section_tag = branch.value("road_section_tag", "");
             entry["RegionID"] = global_config.get_region_id();
-            fill_section_ids(entry, road_sections, branch_section_tag);
+            fill_section_ids(entry, road_sections, branch.value("road_section_tag", ""));
             entry["SpaceType"] = 2;
-            // 分支级每条记录都带 BranchType：斑马线这类既非进口道也非出口道的断面（ROI 里 lane_sdtype_no=5）
-            // 写 -1，而不是省略字段——下游要靠它把人行横道分支和车行道分支分开。
-            entry["BranchType"] = branch_type_of(road_sections, branch_section_tag);
             if (branch.contains("PersonCrossCount")) entry["PersonCrossCount"] = branch["PersonCrossCount"];
             if (branch.contains("NonMotorCrossCount")) entry["NonMotorCrossCount"] = branch["NonMotorCrossCount"];
             if (branch.contains("OverflowOccur")) entry["OverflowOccur"] = branch["OverflowOccur"];
```
