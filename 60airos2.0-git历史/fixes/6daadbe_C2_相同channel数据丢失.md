# fix(运维):【BUG25534】【顺义车网】【智路OS】mec自检使用相同的channel存在数据丢失

- 作者：zhangenwei
- 日期：2026-04-03
- 完整 hash：`6daadbe7c780cab00ab3ecd8fadd36c2fb554e65`
- 改动：+791 / -513 行

```diff
@@@C	zhangenwei	6daadbe7c780cab00ab3ecd8fadd36c2fb554e65	2026-04-03	fix(运维):【BUG25534】【顺义车网】【智路OS】mec自检使用相同的channel存在数据丢失
diff --git a/middleware/protocol/om/configer/configer_om_mec.h b/middleware/protocol/om/configer/configer_om_mec.h
index 007c516..c5d63bb 100644
--- a/middleware/protocol/om/configer/configer_om_mec.h
+++ b/middleware/protocol/om/configer/configer_om_mec.h
@@ -459,14 +459,14 @@ struct TimeRule  : public afl::base::SerializableData
     uint64_t timeoutSecond;                 // 超时触发阈值 (秒)
     uint64_t recoverySecond;                // 恢复所需持续时间 (秒)
     bool     enable;                        //是否检测
-
+    uint64_t maxTimeoutTolerance;          //
 private:
     virtual void serialize(json &j) override
     {
         JsonSerialize(timeoutSecond,                      "timeoutSecond",                      j, false);
         JsonSerialize(recoverySecond,                         "recoverySecond",                         j, false);
         JsonSerialize(enable,                         "enable",                         j, false);
-
+        JsonSerialize(maxTimeoutTolerance,                         "maxTimeoutTolerance",                         j, false);
     }
 
     virtual void deserialize(const json &j) override
@@ -474,6 +474,7 @@ private:
         JsonDeserialize(timeoutSecond,                    "timeoutSecond",                      j, noUse_isEmptyFlag);
         JsonDeserialize(recoverySecond,                       "recoverySecond",                         j, noUse_isEmptyFlag);
         JsonDeserialize(enable,                         "enable",                         j, noUse_isEmptyFlag);
+        JsonDeserialize(maxTimeoutTolerance,                         "maxTimeoutTolerance",                         j, noUse_isEmptyFlag);
     }
 
 public:
@@ -483,6 +484,7 @@ public:
         ss << std::left << std::setw(40) << "timeoutSecond: "           << timeoutSecond           << std::endl;
         ss << std::left << std::setw(40) << "recoverySecond: "              << recoverySecond              << std::endl;
         ss << std::left << std::setw(40) << "enable: "              << (enable?"true":"false")              << std::endl;
+        ss << std::left << std::setw(40) << "maxTimeoutTolerance: "              << maxTimeoutTolerance   << std::endl;
         return ss.str();
     }
 };
@@ -606,88 +608,106 @@ struct MecSelfCheckTime : public afl::base::SerializableData
 		spat_src_data_tr.timeoutSecond = 5;
         spat_src_data_tr.recoverySecond = 5;
         spat_src_data_tr.enable = true;
+        spat_src_data_tr.maxTimeoutTolerance = 3;
 
         sensor_objs_data_tr.timeoutSecond = 10;
         sensor_objs_data_tr.recoverySecond = 10;
         sensor_objs_data_tr.enable = true;
+        sensor_objs_data_tr.maxTimeoutTolerance = 3;
+
 
         ccindex_tc_data_tr.timeoutSecond = 15;
         ccindex_tc_data_tr.recoverySecond = 15;
         ccindex_tc_data_tr.enable = true;
-
+        ccindex_tc_data_tr.maxTimeoutTolerance = 3;
 
         ccindex_tm_data_tr.timeoutSecond = 3;
         ccindex_tm_data_tr.recoverySecond = 3;
         ccindex_tm_data_tr.enable = true;
+        ccindex_tm_data_tr.maxTimeoutTolerance = 3;
         //实时轨迹数据
         ccindex_tm_data_trajectories_tr.timeoutSecond = 3;
         ccindex_tm_data_trajectories_tr.recoverySecond = 3;
         ccindex_tm_data_trajectories_tr.enable = true;
+        sensor_objs_data_tr.maxTimeoutTolerance = 3;
         //实时过车数据
         ccindex_tm_data_vehiclePass_tr.timeoutSecond = 3;
         ccindex_tm_data_vehiclePass_tr.recoverySecond = 3;
         ccindex_tm_data_vehiclePass_tr.enable = true;
+        ccindex_tm_data_vehiclePass_tr.maxTimeoutTolerance = 3;
 
         //实时排队数据
         ccindex_tm_data_queueUp_tr.timeoutSecond = 3;
         ccindex_tm_data_queueUp_tr.recoverySecond = 3;
         ccindex_tm_data_queueUp_tr.enable = true;
+        ccindex_tm_data_queueUp_tr.maxTimeoutTolerance = 3;
         //实时区域状态数据
         ccindex_tm_data_areaState_tr.timeoutSecond = 3;
         ccindex_tm_data_areaState_tr.recoverySecond = 3;
         ccindex_tm_data_areaState_tr.enable = true;
+        ccindex_tm_data_areaState_tr.maxTimeoutTolerance = 3;
         //实时溢出数据
         ccindex_tm_data_overflow_tr.timeoutSecond = 3;
         ccindex_tm_data_overflow_tr.recoverySecond = 3;
         ccindex_tm_data_overflow_tr.enable = true;
+        ccindex_tm_data_overflow_tr.maxTimeoutTolerance = 3;
         //实时出口通道数据
         ccindex_tm_data_outlane_tr.timeoutSecond = 3;
         ccindex_tm_data_outlane_tr.recoverySecond = 3;
         ccindex_tm_data_outlane_tr.enable = true;
+        ccindex_tm_data_outlane_tr.maxTimeoutTolerance = 3;
 
         //定时统计数据
-        ccindex_tm_data_statistics_tr.timeoutSecond = 3;
-        ccindex_tm_data_statistics_tr.recoverySecond = 3;
+        ccindex_tm_data_statistics_tr.timeoutSecond = 320;
+        ccindex_tm_data_statistics_tr.recoverySecond = 320;
         ccindex_tm_data_statistics_tr.enable = true;
+        ccindex_tm_data_statistics_tr.maxTimeoutTolerance = 3;
         //定时评价数据
-        ccindex_tm_data_evaluations_tr.timeoutSecond = 3;
-        ccindex_tm_data_evaluations_tr.recoverySecond = 3;
+        ccindex_tm_data_evaluations_tr.timeoutSecond = 65;
+        ccindex_tm_data_evaluations_tr.recoverySecond = 65;
         ccindex_tm_data_evaluations_tr.enable = true;
+        ccindex_tm_data_evaluations_tr.maxTimeoutTolerance = 3;
         //定时行人及非机动车数据
-        ccindex_tm_data_nonmotor_tr.timeoutSecond = 40;
-        ccindex_tm_data_nonmotor_tr.recoverySecond = 40;
+        ccindex_tm_data_nonmotor_tr.timeoutSecond = 45;
+        ccindex_tm_data_nonmotor_tr.recoverySecond = 45;
         ccindex_tm_data_nonmotor_tr.enable = true;
+        ccindex_tm_data_nonmotor_tr.maxTimeoutTolerance = 3;
         //实时设备状态
-        ccindex_tm_data_deviceStatus_tr.timeoutSecond = 350;
-        ccindex_tm_data_deviceStatus_tr.recoverySecond = 350;
+        ccindex_tm_data_deviceStatus_tr.timeoutSecond = 360;
+        ccindex_tm_data_deviceStatus_tr.recoverySecond = 360;
         ccindex_tm_data_deviceStatus_tr.enable = true;
+        ccindex_tm_data_deviceStatus_tr.maxTimeoutTolerance = 3;
 
 
-        ccindex_st_data_tr.timeoutSecond = 900;
-        ccindex_st_data_tr.recoverySecond = 900;
+        ccindex_st_data_tr.timeoutSecond = 920;
+        ccindex_st_data_tr.recoverySecond = 920;
         ccindex_st_data_tr.enable = true;
+        ccindex_st_data_tr.maxTimeoutTolerance = 3;
 
-
-        cloud_point_data_tr.timeoutSecond = 3600;
-        cloud_point_data_tr.recoverySecond = 3600;
+        cloud_point_data_tr.timeoutSecond = 3610;
+        cloud_point_data_tr.recoverySecond = 3610;
         cloud_point_data_tr.enable = true;
+        cloud_point_data_tr.maxTimeoutTolerance = 3;
 
         sensor_cloud_link_tr.timeoutSecond = 5;
         sensor_cloud_link_tr.recoverySecond = 5;
         sensor_cloud_link_tr.enable = true;
+        sensor_cloud_link_tr.maxTimeoutTolerance = 3;
 
-        ccindex_st_link_tr.timeoutSecond = 5;
-        ccindex_st_link_tr.recoverySecond = 5;
+        ccindex_st_link_tr.timeoutSecond = 310;
+        ccindex_st_link_tr.recoverySecond = 310;
         ccindex_st_link_tr.enable = true;
+        ccindex_st_link_tr.maxTimeoutTolerance = 3;
 
         ccindex_tm_link_tr.timeoutSecond = 5;
         ccindex_tm_link_tr.recoverySecond = 5;
         ccindex_tm_link_tr.enable = true;
+        ccindex_tm_link_tr.maxTimeoutTolerance = 3;
 
-        cloud_point_link_tr.timeoutSecond = 3600;
-        cloud_point_link_tr.recoverySecond = 3600;
+        cloud_point_link_tr.timeoutSecond = 3610;
+        cloud_point_link_tr.recoverySecond = 3610;
         cloud_point_link_tr.enable = true;
-
+        cloud_point_link_tr.maxTimeoutTolerance = 3;
 
     }
 
diff --git a/middleware/protocol/om/data_model/mec/data_monitor_mec.h b/middleware/protocol/om/data_model/mec/data_monitor_mec.h
index 7017125..2a9f6ad 100644
--- a/middleware/protocol/om/data_model/mec/data_monitor_mec.h
+++ b/middleware/protocol/om/data_model/mec/data_monitor_mec.h
@@ -37,6 +37,7 @@ struct MonitorState
     bool        isLinkCheck;                // true: 链路状态检查, false: 数据超时检查
     bool        enable;                     // 是否检测
     int         selfCheckFaultType;         // 故障类型ID
+    int maxTimeoutTolerance;     // 容忍的最大连续超时次数（例如设为3，即连续3秒超时才告警）
     std::string topic; //消息
     // ==========================================
     // 2. 运行时状态 (Runtime State) - 随时间变化
@@ -48,8 +49,13 @@ struct MonitorState
     uint64_t lastAlarmPublishTime = 0;  //最后推送时间
     bool isAlarmActive = false;     //告警是否激活
     bool lastLinkStatus = true;   //最后链路状态
-
+    // 【新增】是否为首次检测标志，默认为 true
+    bool isFirstCheck = true;
     mutable std::mutex stateMutex;
+
+    // 【新增】防抖相关字段
+    int continuousTimeoutCount = 0;  // 连续检测到超时的次数
+
     // ==========================================
     // [修复] 必须添加默认构造函数
     // ==========================================
@@ -68,7 +74,7 @@ struct MonitorState
                  uint64_t rm,
                  bool ilc,
                  bool en,
-                 int sft)
+                 int sft, uint64_t mtt)
         : tag(t),
           name(std::move(n)),
           alarmType(at),
@@ -76,7 +82,8 @@ struct MonitorState
           recoveryMs(rm),
           isLinkCheck(ilc),
           enable(en),
-          selfCheckFaultType(sft)
+          selfCheckFaultType(sft),
+          maxTimeoutTolerance(mtt)
     {
     }
     // 删除拷贝构造和赋值（强制使用指针，防止误拷贝）
@@ -107,6 +114,50 @@ struct MonitorState
 };
 // 【关键】定义智能指针类型
 using MonitorStatePtr = std::shared_ptr<MonitorState>;
+
+// ==========================================
+// [新增] 1. 定义枚举索引 (放在类外或类内 public)
+// ==========================================
+enum CcindexTmTopicIndex {
+    TM_TOPIC_TRAJECTORIES = 0,
+    TM_TOPIC_VEHICLE_PASS,
+    TM_TOPIC_QUEUE_UP,
+    TM_TOPIC_AREA_STATE,
+    TM_TOPIC_OVERFLOW,
+    TM_TOPIC_OUTLANE,
+    TM_TOPIC_STATISTICS,
+    TM_TOPIC_EVALUATIONS,
+    TM_TOPIC_NONMOTOR,
+    TM_TOPIC_DEVICE_STATUS,
+    TM_TOPIC_COUNT // 总数
+};
+
+enum CcindexStTopicIndex {
+    ST_TOPIC_QUERY_ACK = 0,
+    ST_TOPIC_UPDATE,
+    ST_TOPIC_COUNT // 总数
+};
+
+// ==========================================
+// [新增] 2. 定义设备上下文 (替代内层 Map)
+// ==========================================
+struct CcindexTmDeviceContext {
+    // 使用 vector 替代 map，索引为 CcindexTmTopicIndex
+    std::vector<MonitorStatePtr> states;
+    CcindexTmDeviceContext() {
+        states.resize(TM_TOPIC_COUNT, nullptr);
+    }
+};
+using CcindexTmDeviceContextPtr = std::shared_ptr<CcindexTmDeviceContext>;
+
+struct CcindexStDeviceContext {
+    std::vector<MonitorStatePtr> states;
+    CcindexStDeviceContext() {
+        states.resize(ST_TOPIC_COUNT, nullptr);
+    }
+};
+using CcindexStDeviceContextPtr = std::shared_ptr<CcindexStDeviceContext>;
+
 /**
  * @brief 故障类型定义
  * 对应协议字段: faultType
diff --git a/middleware/protocol/om/om_component.cc b/middleware/protocol/om/om_component.cc
index 6a44e46..9c40b8e 100644
--- a/middleware/protocol/om/om_component.cc
+++ b/middleware/protocol/om/om_component.cc
@@ -483,7 +483,9 @@ void OM_COMPONENT::processMecCheckSpatSrcData(const std::shared_ptr<const airos:
         {
             OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]has_md_spat_src_data" ;
         }
-        handleMonitorDataUpdate(std::ref(m_MonitorStateSpatSrcData), mecDeviceData->md_spat_src_data().tag(), mecDeviceData->md_spat_src_data().timestamp());
+        if (m_MonitorStateSpatSrcData) {
+            handleMonitorDataUpdate(*m_MonitorStateSpatSrcData, mecDeviceData->md_spat_src_data().tag(), mecDeviceData->md_spat_src_data().timestamp());
+        }
     }
 
 }
@@ -518,7 +520,9 @@ void OM_COMPONENT::processMecCheckRsapData(const std::shared_ptr<const airos::mo
         {
             OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]has_md_sensor_objs" ;
         }
-        handleMonitorDataUpdate(std::ref(m_MonitorStateRsapData), mecDeviceData->md_sensor_objs().tag(), mecDeviceData->md_sensor_objs().timestamp());
+        if (m_MonitorStateRsapData) {
+            handleMonitorDataUpdate(*m_MonitorStateRsapData, mecDeviceData->md_sensor_objs().tag(), mecDeviceData->md_sensor_objs().timestamp());
+        }
     }
 }
 
@@ -553,7 +557,10 @@ void OM_COMPONENT::processMecCheckRsapLink(const std::shared_ptr<const airos::mo
         {
             OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_rsap" ;
         }
-        handleMonitorLinkUpdate(std::ref(m_MonitorStateRsapLink), mecDeviceData->ml_rsap().tag(), mecDeviceData->ml_rsap().con_flag(), mecDeviceData->ml_rsap().timestamp());
+        if (m_MonitorStateRsapLink)
+            {
+            handleMonitorLinkUpdate(*m_MonitorStateRsapLink, mecDeviceData->ml_rsap().tag(), mecDeviceData->ml_rsap().con_flag(), mecDeviceData->ml_rsap().timestamp());
+        }
     }
 }
 
@@ -588,7 +595,9 @@ void OM_COMPONENT::processMecCheckCcindexTcData(const std::shared_ptr<const airo
         {
             OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]has_md_ccindex_tc" ;
         }
-        handleMonitorDataUpdate(std::ref(m_MonitorStateCcindexTcData), mecDeviceData->md_ccindex_tc().tag(), mecDeviceData->md_ccindex_tc().timestamp());
+        if (m_MonitorStateCcindexTcData) {
+            handleMonitorDataUpdate(*m_MonitorStateCcindexTcData, mecDeviceData->md_ccindex_tc().tag(), mecDeviceData->md_ccindex_tc().timestamp());
+        }
     }
 }
 
@@ -625,7 +634,6 @@ void OM_COMPONENT::processMecCheckCcindexTmData(const std::shared_ptr<const airo
         handleMonitorDataUpdateCcindexTm(mecDeviceData->md_ccindex_tm().device_id(),
             mecDeviceData->md_ccindex_tm().topic(), mecDeviceData->md_ccindex_tm().tag(), mecDeviceData->md_ccindex_tm().timestamp());
     }
-
 }
 
 //MEC自检 - CcindexTmLink
@@ -659,7 +667,9 @@ void OM_COMPONENT::processMecCheckCcindexTmLink(const std::shared_ptr<const airo
         {
             OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_ccindex_tm" ;
         }
-        handleMonitorLinkUpdate(std::ref(m_MonitorStateCcindexTmLink), mecDeviceData->ml_ccindex_tm().tag(), mecDeviceData->ml_ccindex_tm().con_flag(), mecDeviceData->ml_ccindex_tm().timestamp());
+        if (m_MonitorStateCcindexTmLink){
+            handleMonitorLinkUpdate(*m_MonitorStateCcindexTmLink, mecDeviceData->ml_ccindex_tm().tag(), mecDeviceData->ml_ccindex_tm().con_flag(), mecDeviceData->ml_ccindex_tm().timestamp());
+        }
     }
 }
 
@@ -732,9 +742,10 @@ void OM_COMPONENT::processMecCheckCcindexStLink(const std::shared_ptr<const airo
         {
             OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_ccindex_st" ;
         }
-        handleMonitorLinkUpdate(std::ref(m_MonitorStateCcindexStLink), mecDeviceData->ml_ccindex_st().tag(), mecDeviceData->ml_ccindex_st().con_flag(), mecDeviceData->ml_ccindex_st().timestamp());
+        if (m_MonitorStateCcindexStLink){
+            handleMonitorLinkUpdate(*m_MonitorStateCcindexStLink, mecDeviceData->ml_ccindex_st().tag(), mecDeviceData->ml_ccindex_st().con_flag(), mecDeviceData->ml_ccindex_st().timestamp());
+        }
     }
-
 }
 
 //MEC自检 - CcindexRcpData
@@ -805,10 +816,11 @@ void OM_COMPONENT::processMecCheckCcindexRcpLink(const std::shared_ptr<const air
         {
             OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_radar_cp" ;
         }
-        handleMonitorLinkUpdate(std::ref(m_MonitorStateCcindexRcpLink),
-            mecDeviceData->ml_radar_cp().tag(), mecDeviceData->ml_radar_cp().con_flag(), mecDeviceData->ml_radar_cp().timestamp());
+        if (m_MonitorStateCcindexRcpLink){
+            handleMonitorLinkUpdate(*m_MonitorStateCcindexRcpLink,
+                mecDeviceData->ml_radar_cp().tag(), mecDeviceData->ml_radar_cp().con_flag(), mecDeviceData->ml_radar_cp().timestamp());
+        }
     }
-
 }
 
 //mec自检：处理数据更新逻辑
@@ -833,6 +845,7 @@ void OM_COMPONENT::handleMonitorDataUpdate(MonitorState &state, int tag, uint64_
 
     // 1. 喂狗：更新最后收到时间
     state.lastReceivedTime = now;
+    state.isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
     if(getConfiger().enableDebugPrint)
     {
         OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][lastReceivedTime]" << state.lastReceivedTime;
@@ -862,6 +875,7 @@ void OM_COMPONENT::handleMonitorDataUpdate(MonitorState &state, int tag, uint64_
                 state.isAlarmActive = false;
                 state.recoveryStartTime = 0;
                 state.lastAlarmPublishTime = 0;  // 重置告警上报时间
+                state.continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
                 if(getConfiger().enableDebugPrint)
                 {
                     OM_CHECK_DATA_WARN_PRINT << "[om_check_data] RECOVERED: " << state.name;
@@ -891,51 +905,57 @@ void OM_COMPONENT::handleMonitorDataUpdateRcpData(const std::string & deviceId,
         return;
     }
 
-    auto& state = m_MonitorStateCcindexRcpData[deviceId];
-    // 【新增】加锁
-    std::lock_guard<std::mutex> lock(state.stateMutex);
-    if (! state.enable)
+    auto statePtr = it->second;
+    if (statePtr)
     {
-        return;
-    }
+        // 【新增】加锁
+        std::lock_guard<std::mutex> lock(statePtr->stateMutex);
+        if (! statePtr->enable)
+        {
+            return;
+        }
 
-    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
+        uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
 
-    // 1. 喂狗：更新最后收到时间
-    state.lastReceivedTime = now;
-    if(getConfiger().enableDebugPrint)
-    {
-        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm]"<< "[lastReceivedTime]" << state.lastReceivedTime;
-    }
-    // 2. 检查恢复逻辑
-    if (state.isAlarmActive) {
-        if (state.recoveryStartTime == 0)
+        // 1. 喂狗：更新最后收到时间
+        statePtr->lastReceivedTime = now;
+        statePtr->isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
+        if(getConfiger().enableDebugPrint)
         {
-            // 收到告警后的第一帧数据，开始恢复计时
-            state.recoveryStartTime = now;
-            if(getConfiger().enableDebugPrint)
-            {
-                OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm][recovery-start] tag:" << tag ;
-            }
+            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm]"<< "[lastReceivedTime]" << statePtr->lastReceivedTime;
         }
-        else
-        {
-            // 检查数据是否持续了足够长的时间
-            if (now - state.recoveryStartTime >=  state.recoveryMs)
+        // 2. 检查恢复逻辑
+        if (statePtr->isAlarmActive) {
+            if (statePtr->recoveryStartTime == 0)
             {
-                // 满足恢复条件 -> 消除告警
-                std::string errorDetail = state.name + " [deviceId]" + deviceId ;
-                alarmDisappeared(errorDetail, state.alarmType, getConfiger().rscuEsn, errorDetail, state.alarmOccurredTime);
-                state.isAlarmActive = false;
-                state.recoveryStartTime = 0;
-                state.lastAlarmPublishTime = 0;  // 重置告警上报时间
+                // 收到告警后的第一帧数据，开始恢复计时
+                statePtr->recoveryStartTime = now;
                 if(getConfiger().enableDebugPrint)
                 {
-                     OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm] RECOVERED: " << state.name << " [deviceId: " << deviceId << "]";
-                 }
+                    OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm][recovery-start] tag:" << tag ;
+                }
+            }
+            else
+            {
+                // 检查数据是否持续了足够长的时间
+                if (now - statePtr->recoveryStartTime >=  statePtr->recoveryMs)
+                {
+                    // 满足恢复条件 -> 消除告警
+                    std::string errorDetail = statePtr->name + " [deviceId]" + deviceId ;
+                    alarmDisappeared(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, errorDetail, statePtr->alarmOccurredTime);
+                    statePtr->isAlarmActive = false;
+                    statePtr->recoveryStartTime = 0;
+                    statePtr->lastAlarmPublishTime = 0;  // 重置告警上报时间
+                    statePtr->continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
+                    if(getConfiger().enableDebugPrint)
+                    {
+                        OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm] RECOVERED: " << statePtr->name << " [deviceId: " << deviceId << "]";
+                    }
+                }
             }
         }
     }
+
 }
 
 //////////////////////////
@@ -953,57 +973,53 @@ void OM_COMPONENT::handleMonitorDataUpdateCcindexTm(const std::string & deviceId
         OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm][id]" << pair.first ;
     }
     MonitorStatePtr targetState = nullptr;
-    // --- 第一步：容器层操作（极短时间） ---
+   // --- 第一步：查找设备并获取上下文 ---
     {
         std::lock_guard<std::mutex> mapLock(m_CcindexTmMapMutex);
-
-        // 使用 find 查找
         auto it = m_MonitorStatesCcindexTmData.find(deviceId);
-        // 判断是否找到
-        if (it == m_MonitorStatesCcindexTmData.end())
+        if (it != m_MonitorStatesCcindexTmData.end())
         {
-            OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_tm]m_MonitorRulesCcindexTm.find(deviceid) == m_MonitorRulesCcindexTm.end()";
-            return;
+            auto deviceCtx = it->second;
+            // 查找配置以进行 topic 匹配 (如果配置Map也是线程安全的或只读的)
+            // 注意：m_MecSelfCheckTmDeviceIdTopicMap 应该在初始化后只读，所以这里直接访问是安全的
+            auto itConfig = m_MecSelfCheckTmDeviceIdTopicMap.find(deviceId);
+            if (itConfig != m_MecSelfCheckTmDeviceIdTopicMap.end())
+            {
+                // 将字符串转为索引
+                int index = getCcindexTmTopicIndex(topic, itConfig->second);
+                if (index >= 0 && index < TM_TOPIC_COUNT)
+                {
+                    targetState = deviceCtx->states[index];
+                }
+            }
         }
-        // 查找第二层 topic
-        auto itTopic = it->second.find(topic);
-        if (itTopic != it->second.end()) {
-            targetState = itTopic->second; // 拿到指针，引用计数+1
+        else
+        {
+             // 没找到设备，打印一次即可，防止刷屏
+             // OM_CHECK_DATA_ERROR_PRINT << ...
         }
     }
-    // auto& devices = m_MonitorStatesCcindexTmData[deviceId];
-    //
-    // // 检查该topic是否在监控状态表中
-    // if (devices.find(topic) == devices.end())
-    // {
-    //     OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_tm]topic not found in m_MonitorStatesCcindexTm: " << topic;
-    //     return;
-    // }
-    //
-    // auto& state = m_MonitorStatesCcindexTmData[deviceId][topic];
 
+    // --- 第二步：更新状态 (持有对象锁) ---
     if (targetState)
     {
-        // 【新增】找到具体的 state 对象后，立即加锁
         std::lock_guard<std::mutex> lock(targetState->stateMutex);
-        if (! targetState->enable)
-        {
-            return;
-        }
+        if (! targetState->enable) return;
 
         uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
 
-        // 1. 喂狗：更新最后收到时间
+        // 1. 喂狗
         targetState->lastReceivedTime = now;
+        targetState->isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
         if(getConfiger().enableDebugPrint)
         {
             OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm][topic]" << topic << "[lastReceivedTime]" << targetState->lastReceivedTime;
         }
+
         // 2. 检查恢复逻辑
         if (targetState->isAlarmActive) {
             if (targetState->recoveryStartTime == 0)
             {
-                // 收到告警后的第一帧数据，开始恢复计时
                 targetState->recoveryStartTime = now;
                 if(getConfiger().enableDebugPrint)
                 {
@@ -1012,17 +1028,15 @@ void OM_COMPONENT::handleMonitorDataUpdateCcindexTm(const std::string & deviceId
             }
             else
             {
-                // 检查数据是否持续了足够长的时间
-                if (now - targetState->recoveryStartTime >=  targetState->recoveryMs)
+                if (now - targetState->recoveryStartTime >= targetState->recoveryMs)
                 {
-                    // 满足恢复条件 -> 消除告警
                     std::string errorDetail = targetState->name + " [topic: " + topic + "]";
                     alarmDisappeared(errorDetail, targetState->alarmType, getConfiger().rscuEsn, topic, targetState->alarmOccurredTime);
                     targetState->isAlarmActive = false;
                     targetState->recoveryStartTime = 0;
-                    targetState->lastAlarmPublishTime = 0;  // 重置告警上报时间
-                    if(getConfiger().enableDebugPrint)
-                    {
+                    targetState->lastAlarmPublishTime = 0;
+                    targetState->continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
+                    if(getConfiger().enableDebugPrint) {
                         OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm] RECOVERED: " << targetState->name << " [topic: " << topic << "]";
                     }
                 }
@@ -1039,86 +1053,73 @@ void OM_COMPONENT::handleMonitorDataUpdateCcindexSt(const std::string & deviceId
         OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st][device_id]" << deviceId  << "[tag]" << tag << "[topic]" << topic;
     }
 
-    // 使用 find 查找
-    auto it = m_MonitorStatesCcindexStData.find(deviceId);
-    for (auto& pair:m_MonitorStatesCcindexStData)
-    {
-        OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_st][id]" << pair.first;
-    }
-    for (auto& pair: m_MonitorStatesCcindexStData[deviceId])
-    {
-        OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_st][topic]" << pair.first;
-    }
-    // 判断是否找到
-    if (it == m_MonitorStatesCcindexStData.end())
-    {
-        OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_st]m_MonitorRulesCcindexTm.find(deviceid) == m_MonitorRulesCcindexTm.end()";
-        return;
-    }
-
-    auto& devices = m_MonitorStatesCcindexStData[deviceId];
-
-    // 检查该topic是否在监控状态表中
-    if (devices.find(topic) == devices.end())
-    {
-        OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_st]topic not found in m_MonitorStatesCcindexTm: " << topic;
-        return;
-    }
-
-    auto& state = m_MonitorStatesCcindexStData[deviceId][topic];
-    // 【新增】加锁
-    std::lock_guard<std::mutex> lock(state.stateMutex);
-    if (! state.enable)
+    MonitorStatePtr targetState = nullptr;
+    // --- 第一步：查找设备并获取上下文 ---
     {
-        return;
+        std::lock_guard<std::mutex> mapLock(m_CcindexStMapMutex);
+        auto it = m_MonitorStatesCcindexStData.find(deviceId);
+        // 查找配置以进行 topic 匹配 (如果配置Map也是线程安全的或只读的)
+        // 注意：m_MecSelfCheckStDeviceIdTopicMap 应该在初始化后只读，所以这里直接访问是安全的
+        if (it != m_MonitorStatesCcindexStData.end())
+        {
+            auto deviceCtx = it->second;
+            auto itConfig = m_MecSelfCheckStDeviceIdTopicMap.find(deviceId);
+            if (itConfig != m_MecSelfCheckStDeviceIdTopicMap.end())
+            {
+                // 将字符串转为索引
+                int index = getCcindexStTopicIndex(topic, itConfig->second);
+                if (index >= 0 && index < ST_TOPIC_COUNT)
+                {
+                    targetState = deviceCtx->states[index];
+                }
+            }
+        }
     }
-    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
 
-    // 1. 喂狗：更新最后收到时间
-    state.lastReceivedTime = now;
-    if(getConfiger().enableDebugPrint)
+    // --- 第二步：更新状态 (持有对象锁) ---
+    if (targetState)
     {
-        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st][topic]" << topic << "[lastReceivedTime]" << state.lastReceivedTime;
-    }
+        std::lock_guard<std::mutex> lock(targetState->stateMutex);
+        if (! targetState->enable) return;
 
-    // 2. 检查恢复逻辑
-    if (state.isAlarmActive) {
-        if (state.recoveryStartTime == 0)
+        uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
+        // 1. 喂狗
+        targetState->lastReceivedTime = now;
+        targetState->isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
+        if(getConfiger().enableDebugPrint)
         {
-            // 收到告警后的第一帧数据，开始恢复计时
-            state.recoveryStartTime = now;
-            if (getConfiger().enableDebugPrint)
-            {
-                OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st][recovery-start] tag:" << tag << "[topic]" << topic;
-            }
+            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st][topic]" << topic << "[lastReceivedTime]" << targetState->lastReceivedTime;
         }
-        else
+        // 2. 检查恢复逻辑
+        if (targetState->isAlarmActive)
         {
-            if (getConfiger().enableDebugPrint)
+            if (targetState->recoveryStartTime == 0)
             {
-                OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st][recovery-start] state.recoveryStartTime != 0  [now - state.recoveryStartTime ] "
-                    << (now - state.recoveryStartTime ) << state.recoveryMs;
+                targetState->recoveryStartTime = now;
+                if(getConfiger().enableDebugPrint)
+                {
+                    OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_st][recovery-start] tag:" << tag ;
+                }
             }
-            // 检查数据是否持续了足够长的时间
-            if (now - state.recoveryStartTime >= state.recoveryMs)
+            else
             {
-                // 满足恢复条件 -> 消除告警
-                std::string errorDetail = state.name + " [topic: " + topic + "]";
-                alarmDisappeared(errorDetail, state.alarmType, getConfiger().rscuEsn, topic, state.alarmOccurredTime);
-                state.isAlarmActive = false;
-                state.recoveryStartTime = 0;
-                state.lastAlarmPublishTime = 0;  // 重置告警上报时间
-                if(getConfiger().enableDebugPrint)
+                if (now - targetState->recoveryStartTime >= targetState->recoveryMs)
                 {
-                    OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_st] RECOVERED: " << state.name << " [topic: " << topic << "]";
+                    std::string errorDetail = targetState->name ;
+                    alarmDisappeared(errorDetail, targetState->alarmType, getConfiger().rscuEsn, topic, targetState->alarmOccurredTime);
+
+                    targetState->isAlarmActive = false;
+                    targetState->recoveryStartTime = 0;
+                    targetState->lastAlarmPublishTime = 0;
+                    targetState->continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
+                    if(getConfiger().enableDebugPrint)
+                    {
+                        OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_st] RECOVERED: " << targetState->name << " [deviceId: " << deviceId << "]";
+                    }
                 }
             }
         }
     }
-    else
-    {
-        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st] !state.isAlarmActive";
-    }
 }
 //////////////////////////////////////////////////////////////
 ////////////////////////////////////////////////////////////////
@@ -1144,6 +1145,7 @@ void OM_COMPONENT::handleMonitorLinkUpdate(MonitorState &state, int tag, bool st
     uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
     // 1. 喂狗：只要收到消息，说明模块在线，更新心跳时间
     state.lastReceivedTime = now;
+    state.isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
     if(getConfiger().enableDebugPrint)
     {
         OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link][lastReceivedTime]" << state.lastReceivedTime;
@@ -1205,6 +1207,7 @@ void OM_COMPONENT::handleMonitorLinkUpdate(MonitorState &state, int tag, bool st
             state.isAlarmActive = false;
             state.recoveryStartTime = 0;
             state.lastAlarmPublishTime = 0;  // 重置告警上报时间
+            state.continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
             if(getConfiger().enableDebugPrint)
             {
                 OM_CHECK_LINK_SUCCESS_PRINT << "[om_check_link] LINK RECOVERED: " << state.name;
@@ -1221,8 +1224,7 @@ void OM_COMPONENT::initMonitorRules()
     // 1. 信号机原始数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.enable)
     {
-        m_MonitorStateSpatSrcData =os::v2x::protocol::om::mec::MonitorState
-        {
+        m_MonitorStateSpatSrcData = std::make_shared<MonitorState>(
             MONITOR_TAG_DATA_SPAT_SRC,
             "[data ] spat src ",
             MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS,
@@ -1230,52 +1232,55 @@ void OM_COMPONENT::initMonitorRules()
             getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.recoverySecond * m_Second2MSUnit,
             false,
             getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.enable,
-            OmFaultType::OFT_SPAT_SRC_DATA
-        };
+            OmFaultType::OFT_SPAT_SRC_DATA,
+            getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.maxTimeoutTolerance
+        );
     }
     //感知数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.enable)
     {
-        m_MonitorStateRsapData =os::v2x::protocol::om::mec::MonitorState
-        {
+        m_MonitorStateRsapData =std::make_shared<MonitorState>(
             MONITOR_TAG_DATA_SENSOR_OBJ,
             "[data ] sensor obj ", MEC_ALARM_TYPE_MONITOR_SENSOR_OBJ_DATA_LOSS,
             getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.enable,
-            OmFaultType::OFT_SENSOR_OBJS_DATA
-        };
+            OmFaultType::OFT_SENSOR_OBJS_DATA,
+            getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.maxTimeoutTolerance
+        );
     }
     //v2x信控数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.enable)
     {
-        m_MonitorStateCcindexTcData =os::v2x::protocol::om::mec::MonitorState
-        {
+        m_MonitorStateCcindexTcData =std::make_shared<MonitorState>(
             MONITOR_TAG_DATA_CCINDEX_TC,
             "[data ] cc tc ", MEC_ALARM_TYPE_MONITOR_V2X_CC_DATA_LOSS,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.enable,
-            OmFaultType::OFT_CCINDEX_TC_DATA
-        };
+            OmFaultType::OFT_CCINDEX_TC_DATA,
+            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.maxTimeoutTolerance
+        );
     }
 
     //雷达动态数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable)
     {
         std::lock_guard<std::mutex> mapLock(m_CcindexTmMapMutex); // 初始化也要加容器锁
-        for (auto pair:m_MecSelfCheckTmDeviceIdTopicMap)
+        for (auto pair : m_MecSelfCheckTmDeviceIdTopicMap)
         {
-
             std::string id = pair.first;
-            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm]id: " << id  ;
+            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm]id: " << id;
             TopicCCIndexTm topicCCIndexTm = pair.second;
+
+            // [修改] 创建设备上下文
+            auto deviceCtx = std::make_shared<CcindexTmDeviceContext>();
+            m_MonitorStatesCcindexTmData[id] = deviceCtx;
+
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.enable)
             {
-                //实时轨迹数据
-
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Trajectories] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
-                (
+                // [修改] 使用数组索引赋值
+                deviceCtx->states[TM_TOPIC_TRAJECTORIES] = std::make_shared<MonitorState>(
                     MONITOR_TAG_DATA_CCINDEX_TM_TRAJECTORIES,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1283,28 +1288,28 @@ void OM_COMPONENT::initMonitorRules()
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.recoverySecond * m_Second2MSUnit,
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.maxTimeoutTolerance
                 );
+                // 保存 topic 字符串以便日志打印
+                deviceCtx->states[TM_TOPIC_TRAJECTORIES]->topic = topicCCIndexTm.Topic_Trajectories;
             }
             //实时过车数
-            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable)
-            {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_VehiclePass] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
-                (
-                    MONITOR_TAG_DATA_CCINDEX_TM_VEHICLEPASS,
-                    "[data ]cc dynamic ",
-                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable) {
+                deviceCtx->states[TM_TOPIC_VEHICLE_PASS] = std::make_shared<MonitorState>(
+                    MONITOR_TAG_DATA_CCINDEX_TM_VEHICLEPASS, "[data ]cc dynamic ", MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.timeoutSecond * m_Second2MSUnit,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.recoverySecond * m_Second2MSUnit,
-                    false,
-                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
+                    false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.maxTimeoutTolerance
                 );
+                deviceCtx->states[TM_TOPIC_VEHICLE_PASS]->topic = topicCCIndexTm.Topic_VehiclePass;
             }
             //实时排队数
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_QueueUp] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                deviceCtx->states[TM_TOPIC_QUEUE_UP] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                 (
                     MONITOR_TAG_DATA_CCINDEX_TM_QUEUEUP,
                     "[data ]cc dynamic ",
@@ -1313,13 +1318,15 @@ void OM_COMPONENT::initMonitorRules()
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.recoverySecond * m_Second2MSUnit,
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.maxTimeoutTolerance
                 );
+                deviceCtx->states[TM_TOPIC_QUEUE_UP]->topic = topicCCIndexTm.Topic_QueueUp;
             }
             //实时区域状态数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_AreaState] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                deviceCtx->states[TM_TOPIC_AREA_STATE]  = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                 (
                     MONITOR_TAG_DATA_CCINDEX_TM_AREASTATE,
                     "[data ]cc dynamic ",
@@ -1328,14 +1335,16 @@ void OM_COMPONENT::initMonitorRules()
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.recoverySecond * m_Second2MSUnit,
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.maxTimeoutTolerance
                 );
+                deviceCtx->states[TM_TOPIC_AREA_STATE]->topic = topicCCIndexTm.Topic_AreaState;
             }
 
             //实时溢出数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Overflow] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+               deviceCtx->states[TM_TOPIC_OVERFLOW] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                 (
                     MONITOR_TAG_DATA_CCINDEX_TM_OVERFLOW,
                     "[data ]cc dynamic ",
@@ -1344,13 +1353,15 @@ void OM_COMPONENT::initMonitorRules()
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.recoverySecond * m_Second2MSUnit,
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.maxTimeoutTolerance
                 );
+                deviceCtx->states[TM_TOPIC_OVERFLOW]->topic = topicCCIndexTm.Topic_Overflow;
             }
             //实时出口通道数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Outlane] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                deviceCtx->states[TM_TOPIC_OUTLANE]  = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                 (
                     MONITOR_TAG_DATA_CCINDEX_TM_OUTLANE,
                     "[data ]cc dynamic ",
@@ -1359,14 +1370,16 @@ void OM_COMPONENT::initMonitorRules()
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.recoverySecond * m_Second2MSUnit,
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.maxTimeoutTolerance
                 );
+                deviceCtx->states[TM_TOPIC_OUTLANE]->topic = topicCCIndexTm.Topic_Outlane;
             }
 
             //    定时统计数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Statistics] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+               deviceCtx->states[TM_TOPIC_STATISTICS]  = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                 (
                     MONITOR_TAG_DATA_CCINDEX_TM_STATISTICS,
                     "[data ]cc dynamic ",
@@ -1375,14 +1388,16 @@ void OM_COMPONENT::initMonitorRules()
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.recoverySecond * m_Second2MSUnit,
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.maxTimeoutTolerance
                 );
+                deviceCtx->states[TM_TOPIC_STATISTICS]->topic = topicCCIndexTm.Topic_Statistics;
             }
 
             // 定时评价数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Evaluations] =std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                deviceCtx->states[TM_TOPIC_EVALUATIONS]  =std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                 (
                     MONITOR_TAG_DATA_CCINDEX_TM_EVALUATIONS,
                     "[data ]cc dynamic ",
@@ -1391,13 +1406,15 @@ void OM_COMPONENT::initMonitorRules()
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.recoverySecond * m_Second2MSUnit,
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.maxTimeoutTolerance
                 );
+                deviceCtx->states[TM_TOPIC_EVALUATIONS]->topic = topicCCIndexTm.Topic_Evaluations;
             }
             //定时行人及非机动车数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Nonmotor] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+               deviceCtx->states[TM_TOPIC_NONMOTOR]  = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                 (
                     MONITOR_TAG_DATA_CCINDEX_TM_NONMOTOR,
                     "[data ]cc dynamic ",
@@ -1406,23 +1423,22 @@ void OM_COMPONENT::initMonitorRules()
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.recoverySecond * m_Second2MSUnit,
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.maxTimeoutTolerance
                 );
+                deviceCtx->states[TM_TOPIC_NONMOTOR]->topic = topicCCIndexTm.Topic_Nonmotor;
             }
             //设备状态数据
-            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable)
-            {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_DeviceStatus] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
-                (
-                    MONITOR_TAG_DATA_CCINDEX_TM_DEVICESTATUS,
-                    "[data ]cc dynamic ",
-                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
-                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.timeoutSecond * m_Second2MSUnit,
-                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.recoverySecond * m_Second2MSUnit,
-                    false,
-                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable,
-                    OmFaultType::OFT_CCINDEX_TM_DATA
-                );
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable) {
+                deviceCtx->states[TM_TOPIC_DEVICE_STATUS] = std::make_shared<MonitorState>(
+                   MONITOR_TAG_DATA_CCINDEX_TM_DEVICESTATUS, "[data ]cc dynamic ", MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                   getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.timeoutSecond * m_Second2MSUnit,
+                   getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.recoverySecond * m_Second2MSUnit,
+                   false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable,
+                   OmFaultType::OFT_CCINDEX_TM_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.maxTimeoutTolerance
+               );
+                deviceCtx->states[TM_TOPIC_DEVICE_STATUS]->topic = topicCCIndexTm.Topic_DeviceStatus;
             }
             // //实时脉冲数据
             // m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Pulse] = {
@@ -1437,35 +1453,41 @@ void OM_COMPONENT::initMonitorRules()
 
         }
     }
-    //静态数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable)
     {
-
+        std::lock_guard<std::mutex> mapLock(m_CcindexStMapMutex);
         for (auto pair:m_MecSelfCheckStDeviceIdTopicMap)
         {
             std::string id = pair.first;
-
             TopicCCIndexSt topicCCIndexSt = pair.second;
-            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st]id: " << id << "[topicCCIndexSt]" << topicCCIndexSt.to_string() ;
-            m_MonitorStatesCcindexStData[id][topicCCIndexSt.Topic_Static_Query_Ack] =os::v2x::protocol::om::mec::MonitorState
-            {
-                MONITOR_TAG_DATA_CCINDEX_ST_QUERY_ACK,
-                "[data ] cc static ",
-                MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS,
-                getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.timeoutSecond * m_Second2MSUnit,
-                getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.recoverySecond * m_Second2MSUnit,
-                false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable,
-                OmFaultType::OFT_CCINDEX_ST_DATA
-            };
-            m_MonitorStatesCcindexStData[id][topicCCIndexSt.Topic_Static_Update] =os::v2x::protocol::om::mec::MonitorState
-            {
+            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st]id: " << id;
+
+            // [修改] 创建静态数据上下文
+            auto deviceCtx = std::make_shared<CcindexStDeviceContext>();
+            m_MonitorStatesCcindexStData[id] = deviceCtx;
+
+            // deviceCtx->states[ST_TOPIC_QUERY_ACK] = std::make_shared<MonitorState>(
+            //     MONITOR_TAG_DATA_CCINDEX_ST_QUERY_ACK,
+            //     "[data ] cc static ",
+            //     MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS,
+            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.timeoutSecond * m_Second2MSUnit,
+            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.recoverySecond * m_Second2MSUnit,
+            //     false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable,
+            //     OmFaultType::OFT_CCINDEX_ST_DATA,
+            //         getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.maxTimeoutTolerance
+            // );
+            // deviceCtx->states[ST_TOPIC_QUERY_ACK]->topic = topicCCIndexSt.Topic_Static_Query_Ack;
+
+            deviceCtx->states[ST_TOPIC_UPDATE] = std::make_shared<MonitorState>(
                 MONITOR_TAG_DATA_CCINDEX_ST_UPDATE,
                 "[data ] cc static ", MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS,
                 getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.timeoutSecond * m_Second2MSUnit,
                 getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.recoverySecond * m_Second2MSUnit,
                 false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable,
-                OmFaultType::OFT_CCINDEX_ST_DATA
-            };
+                OmFaultType::OFT_CCINDEX_ST_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.maxTimeoutTolerance
+            );
+            deviceCtx->states[ST_TOPIC_UPDATE]->topic = topicCCIndexSt.Topic_Static_Update;
         }
     }
     //雷达点云数据
@@ -1479,15 +1501,15 @@ void OM_COMPONENT::initMonitorRules()
             //点云数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable)
             {
-                m_MonitorStateCcindexRcpData[id] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStateCcindexRcpData[id] = std::make_shared<MonitorState>(
                     MONITOR_TAG_DATA_RADAR_DATA,
                     "[data ] radar raw ", MEC_ALARM_TYPE_MONITOR_RADAR_RAW_DATA_LOSS,
                     getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.timeoutSecond * m_Second2MSUnit,
                     getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.recoverySecond * m_Second2MSUnit,
                     false, getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable,
-                    OmFaultType::OFT_RADAR_RAW_DATA
-                };
+                    OmFaultType::OFT_RADAR_RAW_DATA,
+                    getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.maxTimeoutTolerance
+                );
             }
         }
     }
@@ -1496,55 +1518,55 @@ void OM_COMPONENT::initMonitorRules()
     //感知数据上云：链路
     if(getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.enable)
     {
-        m_MonitorStateRsapLink =os::v2x::protocol::om::mec::MonitorState
-        {
+        m_MonitorStateRsapLink =std::make_shared<MonitorState>(
             MONITOR_TAG_LINK_RSAP,
             "[link ] sensor cloud ", MEC_ALARM_TYPE_MONITOR_CLOUD_LINK_ERROR,
             getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.enable,
-            OmFaultType::OFT_SENSOR_CLOUD_LINK_STATUS
-        };
+            OmFaultType::OFT_SENSOR_CLOUD_LINK_STATUS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.maxTimeoutTolerance
+        );
     }
     //雷达信控指标：静态链路
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.enable)
     {
-       m_MonitorStateCcindexStLink =os::v2x::protocol::om::mec::MonitorState
-        {
+       m_MonitorStateCcindexStLink =std::make_shared<MonitorState>(
            MONITOR_TAG_LINK_CCINDEX_ST,
             "[link ] cc static ", MEC_ALARM_TYPE_MONITOR_CC_STAT_LINK_ERROR,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.enable,
-            OmFaultType::OFT_CCINDEX_ST_LINK_STATUS
-        };
+            OmFaultType::OFT_CCINDEX_ST_LINK_STATUS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.maxTimeoutTolerance
+        );
     }
         //动态数据：链路
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.enable)
     {
-        m_MonitorStateCcindexTmLink =os::v2x::protocol::om::mec::MonitorState
-        {
+        m_MonitorStateCcindexTmLink =std::make_shared<MonitorState>(
             MONITOR_TAG_LINK_CCINDEX_TM,
             "[link ] cc dynamic ", MEC_ALARM_TYPE_MONITOR_CC_DYN_LINK_ERROR,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.enable,
-            OmFaultType::OFT_CCINDEX_TM_LINK_STATUS
-        };
+            OmFaultType::OFT_CCINDEX_TM_LINK_STATUS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.maxTimeoutTolerance
+        );
     }
 
     //点云链路
     if(getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.enable)
     {
-        m_MonitorStateCcindexRcpLink =os::v2x::protocol::om::mec::MonitorState
-        {
+        m_MonitorStateCcindexRcpLink =std::make_shared<MonitorState>(
             MONITOR_TAG_LINK_DAR,
             "[link ] point cloud", MEC_ALARM_TYPE_MONITOR_POINT_CLOUD_LINK_ERROR,
             getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.enable,
-            OmFaultType::OFT_CLOUD_POINT_LINK_STATUS
-            };
+            OmFaultType::OFT_CLOUD_POINT_LINK_STATUS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.maxTimeoutTolerance
+            );
     }
 
 
@@ -1596,20 +1618,20 @@ void OM_COMPONENT::initMonitorRules()
 // MEC自检结果： 监控是否超时 - SpatSrcData
 void OM_COMPONENT::checkMonitorSpatSrcDataTimeouts()
 {
-    checkMonitorMecDataTimeoutsImpl(m_MonitorStateSpatSrcData, MONITOR_TAG_DATA_SPAT_SRC);
+    checkMonitorMecDataTimeoutsImpl(*m_MonitorStateSpatSrcData, MONITOR_TAG_DATA_SPAT_SRC);
 }
 
 // MEC自检结果： 监控是否超时 - RsapData
 void OM_COMPONENT::checkMonitorRsapDataTimeouts()
 {
-    checkMonitorMecDataTimeoutsImpl(m_MonitorStateRsapData, MONITOR_TAG_DATA_SENSOR_OBJ);
+    checkMonitorMecDataTimeoutsImpl(*m_MonitorStateRsapData, MONITOR_TAG_DATA_SENSOR_OBJ);
 }
 
 
 // MEC自检结果： 监控是否超时 - CcindexTcData
 void OM_COMPONENT::checkMonitorCcindexTcDataTimeouts()
 {
-    checkMonitorMecDataTimeoutsImpl(m_MonitorStateCcindexTcData, MONITOR_TAG_DATA_CCINDEX_TC);
+    checkMonitorMecDataTimeoutsImpl(*m_MonitorStateCcindexTcData, MONITOR_TAG_DATA_CCINDEX_TC);
 }
 
 // MEC自检结果： 监控是否超时 - CcindexRcp
@@ -1634,15 +1656,26 @@ void OM_COMPONENT::checkMonitorMecDataTimeoutsImpl(MonitorState& state, int tag)
     std::lock_guard<std::mutex> lock(state.stateMutex);
     uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
     /////////////////////////////////////////////////////////////////////////////////////////
-
+    // 【新增】如果是启动后的第一次有效检测，跳过并重置起始时间
+    if (state.isFirstCheck)
+    {
+        state.isFirstCheck = false;
+        state.lastReceivedTime = now;
+        return;
+    }
     uint64_t timeDiff = now - state.lastReceivedTime;
     if(getConfiger().enableDebugPrint)
     {
         OM_CHECK_DEBUG_PRINT << "[om-check]" << state.to_string();
     }
-
+    // 【新增】0. 数据正常未超时：重置连续超时计数器
+    if ((timeDiff + 1000) < state.timeoutMs)
+    {
+        state.continuousTimeoutCount = 0;
+        // 注意：这里不要直接 return，因为可能还需要处理告警恢复的逻辑（如果你的恢复逻辑写在函数后面的话）
+    }
     // --- 1. 检查超时 (故障触发) ---
-    if ((timeDiff+1000) >= state.timeoutMs)
+   else  if ((timeDiff+1000) >= state.timeoutMs)
     {
         if(getConfiger().enableDebugPrint)
         {
@@ -1650,28 +1683,42 @@ void OM_COMPONENT::checkMonitorMecDataTimeoutsImpl(MonitorState& state, int tag)
         }
         if (!state.isAlarmActive)
         {
-
-            std::string errorDetail = state.name + " loss";
-            state.isAlarmActive = true;
-            state.alarmOccurredTime = now;
-            state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
-            state.recoveryStartTime = 0;
-            if (tag == MONITOR_TAG_DATA_SPAT_SRC)
+            // 【新增】累加连续超时次数
+            state.continuousTimeoutCount++;
+            // 【新增】判断是否达到容忍上限（防抖）
+            if (state.continuousTimeoutCount >= state.maxTimeoutTolerance)
             {
-                //没有信号机原始数据
-                m_SpatDataStatusFromTLDeviceFlag.store(false, std::memory_order_relaxed);
-            }
-            //产生告警
-            alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, state.name,
-                "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
+                std::string errorDetail = state.name + " loss";
+                state.isAlarmActive = true;
+                state.alarmOccurredTime = now;
+                state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
+                state.recoveryStartTime = 0;
+                if (tag == MONITOR_TAG_DATA_SPAT_SRC)
+                {
+                    //没有信号机原始数据
+                    m_SpatDataStatusFromTLDeviceFlag.store(false, std::memory_order_relaxed);
+                }
+                //产生告警
+                alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, state.name,
+                    "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
 
-            if(getConfiger().enableDebugPrint)
+                if(getConfiger().enableDebugPrint)
+                {
+                    OM_CHECK_DEBUG_PRINT <<" [---->][om-check] !state.isAlarmActive [errorDetail[errorDetail]"<< errorDetail
+                    << " [timeDiff]" << timeDiff
+                    << "[timeout]" << state.timeoutMs
+                    << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
+                     << "[alarmOccurredTime]" << state.alarmOccurredTime;
+                }
+            }
+            else
             {
-                OM_CHECK_DEBUG_PRINT <<" [---->][om-check] !state.isAlarmActive [errorDetail[errorDetail]"<< errorDetail
-                << " [timeDiff]" << timeDiff
-                << "[timeout]" << state.timeoutMs
-                << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
-                 << "[alarmOccurredTime]" << state.alarmOccurredTime;
+                // 【新增】未达到阈值，仅打印 Debug 日志，不触发告警
+                if(getConfiger().enableDebugPrint)
+                {
+                    OM_CHECK_DEBUG_PRINT <<" [---->][om-check] Timeout detected but within tolerance. Count: "
+                                         << state.continuousTimeoutCount << "/" << state.maxTimeoutTolerance;
+                }
             }
         }
         // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
@@ -1749,42 +1796,68 @@ void OM_COMPONENT::checkMonitorRcpDataTimeoutsImpl()
     for (auto& statePair : m_MonitorStateCcindexRcpData)
     {
         std::string deviceId = statePair.first;
-        auto& state = statePair.second;
+        auto statePtr = statePair.second;
         // 【新增】加锁
-        std::lock_guard<std::mutex> lock(state.stateMutex);
-        if (!state.enable)
+        std::lock_guard<std::mutex> lock(statePtr->stateMutex);
+        if (!statePtr->enable)
         {
             continue;
         }
-
-        uint64_t timeDiff = now - state.lastReceivedTime;
+        // 【新增】如果是启动后的第一次有效检测，跳过并重置起始时间
+        if (statePtr->isFirstCheck)
+        {
+            statePtr->isFirstCheck = false;
+            statePtr->lastReceivedTime = now;
+            continue;
+        }
+        uint64_t timeDiff = now - statePtr->lastReceivedTime;
         if(getConfiger().enableDebugPrint)
         {
-            OM_CHECK_DEBUG_PRINT << "[om-check][rcp]" << state.to_string();
+            OM_CHECK_DEBUG_PRINT << "[om-check][rcp]" << statePtr->to_string();
+        }
+        // 【新增】0. 数据正常未超时：重置连续超时计数器
+        if ((timeDiff + 1000) < statePtr->timeoutMs)
+        {
+            statePtr->continuousTimeoutCount = 0;
         }
         // --- 1. 检查超时 (故障触发) ---
-        if ((timeDiff + 1000) >= state.timeoutMs)
+        else if ((timeDiff + 1000) >= statePtr->timeoutMs)
         {
             if (getConfiger().enableDebugPrint)
             {
                 OM_CHECK_WARN_PRINT << "[om-check][rcp] timeDiff >= rule.timeoutMs" ;
             }
-            if (!state.isAlarmActive)
+            // 【新增】累加连续超时次数
+            statePtr->continuousTimeoutCount++;
+            // 【新增】判断是否达到容忍上限（防抖）
+            if (!statePtr->isAlarmActive)
             {
-                std::string errorDetail = state.name + "[deviceid]" + deviceId;
-                state.isAlarmActive = true;
-                state.alarmOccurredTime = now;
-                state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
-                state.recoveryStartTime = 0;
-                alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, errorDetail,
-                    "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
-                if(getConfiger().enableDebugPrint)
+                if (statePtr->continuousTimeoutCount >= statePtr->maxTimeoutTolerance)
                 {
-                    OM_CHECK_DEBUG_PRINT <<" [---->][om-check][rcp] !state.isAlarmActive [errorDetail [errorDetail]"<< errorDetail
-                    << " [timeDiff]" << timeDiff
-                    << "[timeout]" << state.timeoutMs
-                    << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
-                     << "[alarmOccurredTime]" << state.alarmOccurredTime;
+                    std::string errorDetail = statePtr->name + "[deviceid]" + deviceId;
+                    statePtr->isAlarmActive = true;
+                    statePtr->alarmOccurredTime = now;
+                    statePtr->lastAlarmPublishTime = now;  // 记录首次告警上报时间
+                    statePtr->recoveryStartTime = 0;
+                    alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, errorDetail,
+                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);
+                    if(getConfiger().enableDebugPrint)
+                    {
+                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][rcp] !statePtr->isAlarmActive [errorDetail [errorDetail]"<< errorDetail
+                        << " [timeDiff]" << timeDiff
+                        << "[timeout]" << statePtr->timeoutMs
+                        << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
+                         << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
+                    }
+                }
+                else
+                {
+                    // 【新增】未达到阈值，仅打印 Debug 日志，不触发告警
+                    if(getConfiger().enableDebugPrint)
+                    {
+                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][rcp] Timeout detected but within tolerance. Count: "
+                                             << statePtr->continuousTimeoutCount << "/" << statePtr->maxTimeoutTolerance;
+                    }
                 }
             }
             // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
@@ -1792,47 +1865,47 @@ void OM_COMPONENT::checkMonitorRcpDataTimeoutsImpl()
             {
                 if(getConfiger().enableDebugPrint)
                 {
-                    OM_CHECK_DEBUG_PRINT << "[====>][om-check][rcp]  state.isAlarmActive " ;
+                    OM_CHECK_DEBUG_PRINT << "[====>][om-check][rcp]  statePtr->isAlarmActive " ;
                 }
                 // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
                 // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
-                if (!state.isLinkCheck && state.recoveryStartTime != 0)
+                if (!statePtr->isLinkCheck && statePtr->recoveryStartTime != 0)
                 {
                     if(getConfiger().enableDebugPrint)
                     {
-                        OM_CHECK_WARN_PRINT << "[====>][om-check]  recovery reset for " << state.name
+                        OM_CHECK_WARN_PRINT << "[====>][om-check]  recovery reset for " << statePtr->name
                                            << " (data stopped again for " << timeDiff << "ms)";
                     }
-                    state.recoveryStartTime = 0;
+                    statePtr->recoveryStartTime = 0;
                 }
 
                 // 检查是否需要重复上报告警
-                if (state.lastAlarmPublishTime > 0)
+                if (statePtr->lastAlarmPublishTime > 0)
                 {
-                    uint64_t alarmPublishTimeDiff = (now - state.lastAlarmPublishTime);
+                    uint64_t alarmPublishTimeDiff = (now - statePtr->lastAlarmPublishTime);
                     if (getConfiger().enableDebugPrint)
                     {
                         OM_CHECK_WARN_PRINT << "[====>][om-check][rcp] alarm-repeat "
-                                           << "[name]" << state.name
+                                           << "[name]" << statePtr->name
                                            << "[timeDiff(ms)]" << alarmPublishTimeDiff
-                                           << "[interval(ms)]" << state.timeoutMs
-                                            << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
-                                            << "[alarmOccurredTime]" << state.alarmOccurredTime;
+                                           << "[interval(ms)]" << statePtr->timeoutMs
+                                            << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
+                                            << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                     }
 
                     // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
-                    if (alarmPublishTimeDiff >= state.timeoutMs)
+                    if (alarmPublishTimeDiff >= statePtr->timeoutMs)
                     {
-                        std::string errorDetail = state.name + "[deviceid]" +  deviceId;
-                        state.lastAlarmPublishTime = now;  // 更新上次上报时间
-                        alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, errorDetail,
-                            "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
+                        std::string errorDetail = statePtr->name + "[deviceid]" +  deviceId;
+                        statePtr->lastAlarmPublishTime = now;  // 更新上次上报时间
+                        alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, errorDetail,
+                            "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);
 
                         if (getConfiger().enableDebugPrint)
                         {
                             OM_CHECK_WARN_PRINT << "[====>][om-check][rcp] alarm repeated [errorDetail]" << errorDetail
                             << "[timeout]" << timeDiff
-                            << "[interval]" << state.timeoutMs;
+                            << "[interval]" << statePtr->timeoutMs;
                         }
                     }
                 }
@@ -1849,52 +1922,65 @@ void OM_COMPONENT::checkMonitorCcindexTmDataTimeouts()
     {
         return;
     }
-    // 临时容器，用于存放待检查的指针
+    // 临时容器，用于存放待检查的指针 (快照)
     std::vector<MonitorStatePtr> statesToCheck;
 
-    // --- 第一步：获取快照 (只读 Map 结构) ---
+    // --- 第一步：获取快照 ---
     {
         std::lock_guard<std::mutex> mapLock(m_CcindexTmMapMutex);
-        // 遍历 Map，收集所有指针
+        // 遍历所有设备
         for (auto& devicePair : m_MonitorStatesCcindexTmData) {
-            for (auto& topicPair : devicePair.second) {
-                if (topicPair.second) {
-                    statesToCheck.push_back(topicPair.second);
+            auto& deviceCtx = devicePair.second;
+            if (deviceCtx) {
+                // 遍历设备下的所有 Topic (数组遍历，极快)
+                for (auto& statePtr : deviceCtx->states) {
+                    if (statePtr) {
+                        statesToCheck.push_back(statePtr);
+                    }
                 }
             }
         }
     }
 
-    // 解锁 Map。此时其他线程可以随意插入/删除 Map 元素，不会影响 statesToCheck 里的指针。
-
     // --- 第二步：遍历快照进行检查 ---
     uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
-    /////////////////////////////////////////////////////////////////////////////////////////
-    //检查多雷达动态数据的超时
+
     for (auto& statePtr : statesToCheck)
     {
-            // 【新增】细粒度锁：每次循环只锁当前这一个 topic 的状态
-            // 使用 unique_lock 配合 defer_lock 可以做更多控制，但这里 lock_guard 足够
-            std::lock_guard<std::mutex> lock(statePtr->stateMutex);
-            if (!statePtr->enable)
-            {
-                continue;
-            }
+        std::lock_guard<std::mutex> lock(statePtr->stateMutex);
+        if (!statePtr->enable) continue;
+        // 【新增】如果是启动后的第一次有效检测，跳过并重置起始时间
+        if (statePtr->isFirstCheck)
+        {
+            statePtr->isFirstCheck = false;
+            statePtr->lastReceivedTime = now;
+            continue;
+        }
+        uint64_t timeDiff = now - statePtr->lastReceivedTime;
+        if (getConfiger().enableDebugPrint)
+        {
+            OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm]"<< "[timeDiff]" << timeDiff << "[now]" << now  << "[topic]" << statePtr->topic
+                                << "[state]" << statePtr->to_string();
+        }
 
-            uint64_t timeDiff = now - statePtr->lastReceivedTime;
+        // 【新增】0. 数据正常未超时：重置连续超时计数器
+        if ((timeDiff + 1000) < statePtr->timeoutMs)
+        {
+            statePtr->continuousTimeoutCount = 0;
+        }
+        // --- 1. 检查超时 (故障触发) ---
+        else if ((timeDiff + 1000) >= statePtr->timeoutMs)
+        {
             if (getConfiger().enableDebugPrint)
             {
-                OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm]"<< "[timeDiff]" << timeDiff << "[now]" << now  << "[topic]" << statePtr->topic
-                                    << "[state]" << statePtr->to_string();
+                OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm] timeDiff >= rule.timeoutMs for topic: " << statePtr->topic;
             }
-            // --- 1. 检查超时 (故障触发) ---
-            if ((timeDiff + 1000) >= statePtr->timeoutMs)
+            if (!statePtr->isAlarmActive)
             {
-                if (getConfiger().enableDebugPrint)
-                {
-                    OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm] timeDiff >= rule.timeoutMs for topic: " << statePtr->topic;
-                }
-                if (!statePtr->isAlarmActive)
+                // 【新增】累加连续超时次数
+                statePtr->continuousTimeoutCount++;
+                // 【新增】判断是否达到容忍上限（防抖）
+                if (statePtr->continuousTimeoutCount >= statePtr->maxTimeoutTolerance)
                 {
                     if (getConfiger().enableDebugPrint)
                     {
@@ -1917,57 +2003,67 @@ void OM_COMPONENT::checkMonitorCcindexTmDataTimeouts()
                         << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                     }
                 }
-                // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
                 else
+                {
+                    // 【新增】未达到阈值，仅打印 Debug 日志，不触发告警
+                    if(getConfiger().enableDebugPrint)
+                    {
+                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-tm] Timeout detected but within tolerance. Count: "
+                                             << statePtr->continuousTimeoutCount << "/" << statePtr->maxTimeoutTolerance;
+                    }
+                }
+            }
+            // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
+            else
+            {
+                if(getConfiger().enableDebugPrint)
+                {
+                    OM_CHECK_DEBUG_PRINT << "[====>][om-check][ccindex-tm] statePtr->isAlarmActive " ;
+                }
+                // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
+                // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
+                if (!statePtr->isLinkCheck && statePtr->recoveryStartTime != 0)
                 {
                     if(getConfiger().enableDebugPrint)
                     {
-                        OM_CHECK_DEBUG_PRINT << "[====>][om-check][ccindex-tm] statePtr->isAlarmActive " ;
+                        OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] recovery reset for " << statePtr->name
+                                        << " (data stopped again for " << timeDiff << "ms)";
                     }
-                    // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
-                    // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
-                    if (!statePtr->isLinkCheck && statePtr->recoveryStartTime != 0)
+                    statePtr->recoveryStartTime = 0;
+                }
+
+                // 检查是否需要重复上报告警
+                if (statePtr->lastAlarmPublishTime > 0)
+                {
+                    uint64_t alarmPublishTimeDiff = (now - statePtr->lastAlarmPublishTime);
+                    if (getConfiger().enableDebugPrint)
                     {
-                        if(getConfiger().enableDebugPrint)
-                        {
-                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] recovery reset for " << statePtr->name
-                                            << " (data stopped again for " << timeDiff << "ms)";
-                        }
-                        statePtr->recoveryStartTime = 0;
+                        OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] alarm-repeat "
+                                        << "[name]" << statePtr->name
+                                        << "[timeDiff(ms)]" << alarmPublishTimeDiff
+                                        << "[interval(ms)]" << statePtr->timeoutMs
+                                            << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
+                                            << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                     }
 
-                    // 检查是否需要重复上报告警
-                    if (statePtr->lastAlarmPublishTime > 0)
+                    // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
+                    if (alarmPublishTimeDiff >= statePtr->timeoutMs)
                     {
-                        uint64_t alarmPublishTimeDiff = (now - statePtr->lastAlarmPublishTime);
-                        if (getConfiger().enableDebugPrint)
-                        {
-                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] alarm-repeat "
-                                            << "[name]" << statePtr->name
-                                            << "[timeDiff(ms)]" << alarmPublishTimeDiff
-                                            << "[interval(ms)]" << statePtr->timeoutMs
-                                                << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
-                                                << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
-                        }
+                        std::string errorDetail = statePtr->name + " [topic: " + statePtr->topic + "] [no data]";
+                        statePtr->lastAlarmPublishTime = now;  // 更新上次上报时间
+                        alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
+                            "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);
 
-                        // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
-                        if (alarmPublishTimeDiff >= statePtr->timeoutMs)
+                        if (getConfiger().enableDebugPrint)
                         {
-                            std::string errorDetail = statePtr->name + " [topic: " + statePtr->topic + "] [no data]";
-                            statePtr->lastAlarmPublishTime = now;  // 更新上次上报时间
-                            alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
-                                "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);
-
-                            if (getConfiger().enableDebugPrint)
-                            {
-                                OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] alarm repeated [errorDetail]" << errorDetail
-                                << "[timeout]" << timeDiff
-                                << "[interval]" << statePtr->timeoutMs;
-                            }
+                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] alarm repeated [errorDetail]" << errorDetail
+                            << "[timeout]" << timeDiff
+                            << "[interval]" << statePtr->timeoutMs;
                         }
                     }
                 }
             }
+        }
     }
 }
 
@@ -1978,111 +2074,139 @@ void OM_COMPONENT::checkMonitorCcindexStDataTimeouts()
     {
         return;
     }
-    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
-  /////////////////////////////////////////////////////////////////////////////////////////
-    //检查多雷达静态数据的超时
-    for (auto& statesPair : m_MonitorStatesCcindexStData)
-    {
-        std::string deviceId = statesPair.first;
-        auto& statesMap = statesPair.second;
-
+    std::vector<MonitorStatePtr> statesToCheck;
 
-        // 遍历每个雷达的每个topic
-        for (auto& statePair : statesMap)
-        {
-            const std::string& topic = statePair.first;
-            auto& state = statePair.second;
-            // 【新增】加锁
-            std::lock_guard<std::mutex> lock(state.stateMutex);
-            if (!state.enable)
-            {
-                continue;
+    {
+        std::lock_guard<std::mutex> mapLock(m_CcindexStMapMutex);
+        for (auto& devicePair : m_MonitorStatesCcindexStData) {
+            auto& deviceCtx = devicePair.second;
+            if (deviceCtx) {
+                for (auto& statePtr : deviceCtx->states) {
+                    if (statePtr) {
+                        statesToCheck.push_back(statePtr);
+                    }
+                }
             }
+        }
+    }
 
-            uint64_t timeDiff = now - state.lastReceivedTime;
+    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
+   /////////////////////////////////////////////////////////////////////////////////////////
+    // 遍历每个雷达的每个topic
+    for (auto& statePtr : statesToCheck)
+    {
+        std::lock_guard<std::mutex> lock(statePtr->stateMutex);
+        if (!statePtr->enable) continue;
+        // 【新增】如果是启动后的第一次有效检测，跳过并重置起始时间
+        if (statePtr->isFirstCheck)
+        {
+            statePtr->isFirstCheck = false;
+            statePtr->lastReceivedTime = now;
+            continue;
+        }
+        uint64_t timeDiff = now - statePtr->lastReceivedTime;
+        if (getConfiger().enableDebugPrint)
+        {
+            OM_CHECK_ERROR_PRINT << "[om-check][ccindex-st]" << "[timeDiff]" << timeDiff  << "[now]" << now  << "[topic]" << statePtr->topic;
+            OM_CHECK_WARN_PRINT << "[om-check][ccindex-st]"<< "[timeDiff]" << timeDiff << "[now]" << now  << "[topic]" << statePtr->topic
+                                << "[state]" << statePtr->to_string();
+        }
+        // 【新增】0. 数据正常未超时：重置连续超时计数器
+        if ((timeDiff + 1000) < statePtr->timeoutMs)
+        {
+            statePtr->continuousTimeoutCount = 0;
+        }
+        // --- 1. 检查超时 (故障触发) ---
+        else if ((timeDiff + 1000) >= statePtr->timeoutMs)
+        {
             if (getConfiger().enableDebugPrint)
             {
-                OM_CHECK_ERROR_PRINT << "[om-check][ccindex-st][deviceId]" << deviceId << "[timeDiff]" << timeDiff  << "[now]" << now  << "[topic]" << topic;
-                OM_CHECK_WARN_PRINT << "[om-check][ccindex-st]"<< "[timeDiff]" << timeDiff << "[now]" << now  << "[topic]" << topic
-                                    << "[state]" << state.to_string();
+                OM_CHECK_WARN_PRINT << "[om-check][ccindex-st] timeDiff >= rule.timeoutMs for topic: " << statePtr->topic;
             }
-            // --- 1. 检查超时 (故障触发) ---
-            if ((timeDiff + 1000) >= state.timeoutMs)
+            if (!statePtr->isAlarmActive)
             {
-                if (getConfiger().enableDebugPrint)
-                {
-                    OM_CHECK_WARN_PRINT << "[om-check][ccindex-st] timeDiff >= rule.timeoutMs for topic: " << topic;
-                }
-                if (!state.isAlarmActive)
+                // 【新增】累加连续超时次数
+                statePtr->continuousTimeoutCount++;
+                // 【新增】判断是否达到容忍上限（防抖）
+                if (statePtr->continuousTimeoutCount >= statePtr->maxTimeoutTolerance)
                 {
                     if (getConfiger().enableDebugPrint)
                     {
-                        OM_CHECK_WARN_PRINT << "[---->][om-check][ccindex-st] !state.isAlarmActive for topic: " << topic;
+                        OM_CHECK_WARN_PRINT << "[---->][om-check][ccindex-st] !statePtr->isAlarmActive for topic: " << statePtr->topic;
                     }
-                    std::string errorDetail = state.name + "[data-loss]" + " [topic] " + topic;
-                    state.isAlarmActive = true;
-                    state.alarmOccurredTime = now;
-                    state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
-                    state.recoveryStartTime = 0;
-                    alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, topic,
-                    "mec-check", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
+                    std::string errorDetail = statePtr->name + "[data-loss]" + " [topic] " + statePtr->topic;
+                    statePtr->isAlarmActive = true;
+                    statePtr->alarmOccurredTime = now;
+                    statePtr->lastAlarmPublishTime = now;  // 记录首次告警上报时间
+                    statePtr->recoveryStartTime = 0;
+                    alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
+                    "mec-check", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);
 
                     if(getConfiger().enableDebugPrint)
                     {
-                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-st] !state.isAlarmActive  [errorDetail]"<< errorDetail
+                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-st] !statePtr->isAlarmActive  [errorDetail]"<< errorDetail
                         << " [timeDiff]" << timeDiff
-                        << "[timeout]" << state.timeoutMs
-                        << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
-                        << "[alarmOccurredTime]" << state.alarmOccurredTime;
+                        << "[timeout]" << statePtr->timeoutMs
+                        << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
+                        << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                     }
                 }
-                // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
                 else
                 {
-                   if(getConfiger().enableDebugPrint)
+                    // 【新增】未达到阈值，仅打印 Debug 日志，不触发告警
+                    if(getConfiger().enableDebugPrint)
                     {
-                        OM_CHECK_DEBUG_PRINT << "[====>][om-check][ccindex-st] state.isAlarmActive " ;
+                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-st] Timeout detected but within tolerance. Count: "
+                                             << statePtr->continuousTimeoutCount << "/" << statePtr->maxTimeoutTolerance;
                     }
-                    // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
-                    // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
-                    if (!state.isLinkCheck && state.recoveryStartTime != 0)
+                }
+            }
+            // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
+            else
+            {
+               if(getConfiger().enableDebugPrint)
+                {
+                    OM_CHECK_DEBUG_PRINT << "[====>][om-check][ccindex-st] statePtr->isAlarmActive " ;
+                }
+                // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
+                // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
+                if (!statePtr->isLinkCheck && statePtr->recoveryStartTime != 0)
+                {
+                    if(getConfiger().enableDebugPrint)
                     {
-                        if(getConfiger().enableDebugPrint)
-                        {
-                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] recovery reset for " << state.name
-                                            << " (data stopped again for " << timeDiff << "ms)";
-                        }
-                        state.recoveryStartTime = 0;
+                        OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] recovery reset for " << statePtr->name
+                                        << " (data stopped again for " << timeDiff << "ms)";
                     }
+                    statePtr->recoveryStartTime = 0;
+                }
 
-                    // 检查是否需要重复上报告警
-                    if (state.lastAlarmPublishTime > 0)
+                // 检查是否需要重复上报告警
+                if (statePtr->lastAlarmPublishTime > 0)
+                {
+                    uint64_t alarmPublishTimeDiff = (now - statePtr->lastAlarmPublishTime);
+                    if (getConfiger().enableDebugPrint)
                     {
-                        uint64_t alarmPublishTimeDiff = (now - state.lastAlarmPublishTime);
-                        if (getConfiger().enableDebugPrint)
-                        {
-                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-st] alarm-repeat "
-                                            << "[name]" << state.name
-                                            << "[timeDiff(ms)]" << alarmPublishTimeDiff
-                                            << "[interval(ms)]" << state.timeoutMs
-                                            << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
-                                            << "[alarmOccurredTime]" << state.alarmOccurredTime;
-                        }
+                        OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-st] alarm-repeat "
+                                        << "[name]" << statePtr->name
+                                        << "[timeDiff(ms)]" << alarmPublishTimeDiff
+                                        << "[interval(ms)]" << statePtr->timeoutMs
+                                        << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
+                                        << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
+                    }
 
-                        // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
-                        if (alarmPublishTimeDiff >= state.timeoutMs)
-                        {
-                            std::string errorDetail = state.name +  "[data-loss]" + " [topic] " + topic;
-                            state.lastAlarmPublishTime = now;  // 更新上次上报时间
-                            alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, topic,
-                            "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
+                    // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
+                    if (alarmPublishTimeDiff >= statePtr->timeoutMs)
+                    {
+                        std::string errorDetail = statePtr->name +  "[data-loss]" + " [topic] " + statePtr->topic;
+                        statePtr->lastAlarmPublishTime = now;  // 更新上次上报时间
+                        alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
+                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);
 
-                            if (getConfiger().enableDebugPrint)
-                            {
-                                OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-st] alarm repeated [errorDetail]" << errorDetail
-                                << "[timeout]" << timeDiff
-                                << "[interval]" << state.timeoutMs;
-                            }
+                        if (getConfiger().enableDebugPrint)
+                        {
+                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-st] alarm repeated [errorDetail]" << errorDetail
+                            << "[timeout]" << timeDiff
+                            << "[interval]" << statePtr->timeoutMs;
                         }
                     }
                 }
@@ -2135,6 +2259,42 @@ int OM_COMPONENT::getSelfCheckFaultType(MecAlarmTypeErrorCodeEnum alarmType)
         return 0; // 0 表示非自检类告警，不上报
     }
 }
+
+/**
+ * 通用字符串提取拼接函数
+ * @param path 原始字符串
+ * @param indices 需要提取的索引列表
+ * @param delimiter 分隔符，默认为 '/'
+ * @return 拼接后的新字符串
+ */
+std::string OM_COMPONENT::extractAndJoin(const std::string& path, const std::vector<int>& indices, char delimiter)
+{
+    std::vector<std::string> parts;
+    std::stringstream ss(path);
+    std::string item;
+
+    // 1. 按分隔符拆分字符串并存入 vector
+    while (std::getline(ss, item, delimiter)) {
+        parts.push_back(item);
+    }
+
+    std::string result = "";
+    bool isFirst = true;
+
+    // 2. 提取指定索引的元素并重新拼接
+    for (int idx : indices) {
+        // 边界检查，防止索引越界
+        if (idx >= 0 && idx < parts.size()) {
+            if (!isFirst) {
+                result += delimiter; // 非首个元素前添加分隔符
+            }
+            result += parts[idx];
+            isFirst = false;
+        }
+    }
+
+    return result;
+}
 //MEC自检结果：发送自检消息
 void OM_COMPONENT::sendMecSelfCheckMsg(MecAlarmTypeErrorCodeEnum alarmType, int faultType, int faultStatus, uint64_t startTime, uint64_t stopTime, const std::string& desc)
 {
@@ -2164,12 +2324,47 @@ void OM_COMPONENT::sendMecSelfCheckMsg(MecAlarmTypeErrorCodeEnum alarmType, int
     {
         OM_CHECK_DEBUG_PRINT << "[monitor] Push msg: " << data.to_string();
     }
-
+    std::string topicUpload = "";
+#ifdef OM_SELF_CHECK
+    topicUpload = m_MqttClientConfig.configerMec.configerTopic.Topic_MecSelfCheckResult_Up + std::to_string(faultType);
+    if (faultType == OmFaultType::OFT_CCINDEX_TM_DATA)
+    {
+        std::string result = extractAndJoin(desc, targetIndices);
+       topicUpload +=  "/" + result;
+    }
+#else
+    topicUpload = m_MqttClientConfig.configerMec.configerTopic.Topic_MecSelfCheckResult_Up;
+#endif
     if(m_MqttConnectedCloud && m_RegisterFlag)
     {
-        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_MecSelfCheckResult_Up, data, desc);
+        mqttPushMsg2BrokerCloud(topicUpload, data, desc);
     }
 }
+
+// 将动态数据的 topic 字符串转换为数组索引
+int OM_COMPONENT::getCcindexTmTopicIndex(const std::string& topic, const TopicCCIndexTm& config)
+{
+    // 这里进行字符串比较，虽然有开销，但比 map 查找快，且只在 handle 时调用
+    if (topic == config.Topic_Trajectories) return TM_TOPIC_TRAJECTORIES;
+    if (topic == config.Topic_VehiclePass) return TM_TOPIC_VEHICLE_PASS;
+    if (topic == config.Topic_QueueUp) return TM_TOPIC_QUEUE_UP;
+    if (topic == config.Topic_AreaState) return TM_TOPIC_AREA_STATE;
+    if (topic == config.Topic_Overflow) return TM_TOPIC_OVERFLOW;
+    if (topic == config.Topic_Outlane) return TM_TOPIC_OUTLANE;
+    if (topic == config.Topic_Statistics) return TM_TOPIC_STATISTICS;
+    if (topic == config.Topic_Evaluations) return TM_TOPIC_EVALUATIONS;
+    if (topic == config.Topic_Nonmotor) return TM_TOPIC_NONMOTOR;
+    if (topic == config.Topic_DeviceStatus) return TM_TOPIC_DEVICE_STATUS;
+    return -1;
+}
+
+// 将静态数据的 topic 字符串转换为数组索引
+int OM_COMPONENT::getCcindexStTopicIndex(const std::string& topic, const TopicCCIndexSt& config)
+{
+    if (topic == config.Topic_Static_Query_Ack) return ST_TOPIC_QUERY_ACK;
+    if (topic == config.Topic_Static_Update) return ST_TOPIC_UPDATE;
+    return -1;
+}
 //////////////////////////////////////////////////////////////////////////////////////////////
 /////////////////////////////////////////////////////////////////////////////////////////////////
 ///
@@ -7894,9 +8089,10 @@ bool OM_COMPONENT::getMqttConfigerMsg()
             // {
             //     m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Query_Ack.pop_back();
             // }
-
-            topicCCIndexSt.Topic_Static_Query_Ack = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Profix +
-                                                          m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Query_Ack + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
+            //20260403:去掉对查询的监控，只监控update
+            // topicCCIndexSt.Topic_Static_Query_Ack = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Profix +
+            //                                               m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Query_Ack + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
+            //
             // if (!m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Update_Ack.empty()
             //         && m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Update_Ack.back() == '/')
             // {
@@ -11821,4 +12017,4 @@ std::string OM_COMPONENT::getTLFaultDesc(TrafficlightDetectStatus tlds)
     }
     return tldsStr;
 }
-NAMESPACE_ENDED_OM
+NAMESPACE_ENDED_OM
\ No newline at end of file
diff --git a/middleware/protocol/om/om_component.h b/middleware/protocol/om/om_component.h
index 4a106e3..8ff1ad6 100644
--- a/middleware/protocol/om/om_component.h
+++ b/middleware/protocol/om/om_component.h
@@ -888,52 +888,51 @@ private:
     void  processMecCheckCcindexRcpLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
     //mec自检 ：信号机原始数据
     std::unique_ptr<std::thread>    m_TaskProcessMecCheckSpatSrcData;
-    MonitorState                    m_MonitorStateSpatSrcData;
+    MonitorStatePtr                     m_MonitorStateSpatSrcData; // [修改] 改为指针
 
     //mec自检： 感知数据
     std::unique_ptr<std::thread>    m_TaskProcessMecCheckRsapData;
-    MonitorState                    m_MonitorStateRsapData;
-
+    MonitorStatePtr                     m_MonitorStateRsapData;    // [修改] 改为指针
 
     //mec自检： V2X信控数据
     std::unique_ptr<std::thread>    m_TaskProcessMecCheckCcindexTcData;
-    MonitorState                    m_MonitorStateCcindexTcData;
+    MonitorStatePtr                 m_MonitorStateCcindexTcData; // [修改] 改为指针
 
-    //mec自检： 雷达动态数据
-    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexTmData;
-    // 容器锁：保护 m_MonitorStatesCcindexTmData 的结构
+    // --- 雷达动态数据 (优化后) ---
+    std::unique_ptr<std::thread> m_TaskProcessMecCheckCcindexTmData;
     std::mutex m_CcindexTmMapMutex;
-    // std::map<std::string, std::map<std::string, MonitorState>>     m_MonitorStatesCcindexTmData;  // <id, <topic, state>>
-    // 数据容器：存指针
-    std::map<std::string, std::map<std::string, MonitorStatePtr>> m_MonitorStatesCcindexTmData;
-    std::map<std::string, TopicCCIndexTm>  m_MecSelfCheckTmDeviceIdTopicMap;
+    // Map<DeviceId, Context>
+    std::map<std::string, CcindexTmDeviceContextPtr> m_MonitorStatesCcindexTmData;
+    std::map<std::string, TopicCCIndexTm> m_MecSelfCheckTmDeviceIdTopicMap;
 
-    //mec自检： 雷达静态数据
-    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexStData;
+    // --- 雷达静态数据 (优化后) ---
+    std::unique_ptr<std::thread> m_TaskProcessMecCheckCcindexStData;
     std::mutex m_CcindexStMapMutex;
-    std::map<std::string, std::map<std::string, MonitorState>>     m_MonitorStatesCcindexStData;  // <id, <topic, state>>
-    std::map<std::string, TopicCCIndexSt>  m_MecSelfCheckStDeviceIdTopicMap;
+    // Map<DeviceId, Context>
+    std::map<std::string, CcindexStDeviceContextPtr> m_MonitorStatesCcindexStData;
+    std::map<std::string, TopicCCIndexSt> m_MecSelfCheckStDeviceIdTopicMap;
 
 
     //mec自检： 点云数据
     std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexCcindexRcpData;
-    // MonitorState                            m_MonitorStateCcindexRcpData;
-    std::map<std::string, MonitorState>      m_MonitorStateCcindexRcpData;  //<id， state>
+    std::map<std::string, MonitorStatePtr>  m_MonitorStateCcindexRcpData;
     /////////////////////////////////////////////////////////////////////////
     //mec自检： 云控链路
     std::unique_ptr<std::thread>    m_TaskProcessMecCheckRsapLink;
-    MonitorState                    m_MonitorStateRsapLink;
+    MonitorStatePtr                 m_MonitorStateRsapLink;      // [修改] 改为指针
+
 
     //mec自检： 雷达动态链路
     std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexTmLink;
-    MonitorState                            m_MonitorStateCcindexTmLink;
+    MonitorStatePtr                         m_MonitorStateCcindexTmLink; // [修改] 改为指针
+
     //mec自检： 雷达静态链路
     std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexStLink;
-    MonitorState                            m_MonitorStateCcindexStLink;
+    MonitorStatePtr                         m_MonitorStateCcindexStLink; // [修改] 改为指针
 
     //mec自检： 点云链路
     std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexCcindexRcpLink;
-    MonitorState     m_MonitorStateCcindexRcpLink;
+    MonitorStatePtr                         m_MonitorStateCcindexRcpLink; // [修改] 改为指针
 
 
     void handleMonitorDataUpdateCcindexTm(const std::string & deviceId, const std::string& topic, int tag, uint64_t timestamp);
@@ -977,6 +976,14 @@ private:
 
     // 发送自检结果消息
     void sendMecSelfCheckMsg(MecAlarmTypeErrorCodeEnum alarmType,  int faultType, int faultStatus, uint64_t startTime, uint64_t stopTime, const std::string& desc);
+    // ==========================================
+    // [新增] 4. 辅助函数声明
+    // ==========================================
+    int getCcindexTmTopicIndex(const std::string& topic, const TopicCCIndexTm& config);
+    int getCcindexStTopicIndex(const std::string& topic, const TopicCCIndexSt& config);
+    std::vector<int> targetIndices = {1, 3, 6};
+    std::string extractAndJoin(const std::string& path, const std::vector<int>& indices, char delimiter = '/');
+
     ///////////////////////////////////////////////////////////////////////////////////////////////////////
     std::string getTLFaultDesc(TrafficlightDetectStatus tlds);
     //信号灯状态检测
diff --git a/middleware/protocol/radar_point_cloud/DianYunRecorder.py b/middleware/protocol/radar_point_cloud/DianYunRecorder.py
index d310206..050909a 100644
--- a/middleware/protocol/radar_point_cloud/DianYunRecorder.py
+++ b/middleware/protocol/radar_point_cloud/DianYunRecorder.py
@@ -267,6 +267,9 @@ class TCPReceiver:
 
                 self.buffer += data
                 self.process_data()
+            except socket.timeout:
+                # 【修复】只是 15 秒内没收到数据，不需要断开重连，继续等待即可
+                continue
             except(socket.error, socket.timeout) as e:
                 if self.running:
                     self.logger.error(f'TCP 接收 {self.host} 数据错误/超时: {e}')
@@ -361,8 +364,8 @@ class FTPUploader:
         """建立 FTP 连接"""
         while True:
             try:
+                self.logger.info(f"Connected to FTP server: {self.host} [host]{self.host} [port]{self.port}")
                 self.ftp.connect(self.host, self.port, timeout=10)
-                self.logger.info(f"Connected to FTP server: {self.host}")
                 self.connected = True
                 break
             except Exception as e:
@@ -486,36 +489,37 @@ def check_ftp_connection_periodically(host, port, username, password, interval,
     """
     global GLOBAL_FTP_LINK_STATUS
     logger.info(f"启动 FTP 连接状态检测线程，检测周期: {interval}秒")
+    ftp = None
     while not stop_event.is_set():
-        ftp = FTP()
         try:
-            # 尝试连接
-            ftp.connect(host, port, timeout=5)
-            # 尝试登录
-            ftp.login(user=username, passwd=password)
-
-            # 如果能走到这里，说明连接和登录都成功
+            # 如果没有连接，则建立连接
+            if ftp is None:
+                ftp = FTP()
+                ftp.connect(host, port, timeout=5)
+                ftp.login(user=username, passwd=password)
+
+            # 发送 NOOP 命令测试连接是否存活
+            ftp.voidcmd("NOOP")
             GLOBAL_FTP_LINK_STATUS = True
-            # logger.debug("FTP connection check: Success")
-
-            # 主动断开，释放资源
-            try:
-                ftp.quit()
-            except Exception:
-                pass
 
         except Exception as e:
-            # 连接或登录失败
+            # 连接断开或 NOOP 失败
             GLOBAL_FTP_LINK_STATUS = False
             logger.warning(f"FTP connection check failed: {e}")
-            try:
-                ftp.close()
-            except:
-                pass
+            if ftp:
+                try:
+                    ftp.close()
+                except:
+                    pass
+                ftp = None  # 置空，下次循环将重新连接
 
-        # 等待下一个周期
         stop_event.wait(interval)
 
+    if ftp:
+        try:
+            ftp.quit()
+        except:
+            pass
     logger.info("FTP 连接状态检测线程已停止")
```
