# fix(运维):【20260323-zhangenwei-1】mec自检中针对动态数据和静态数据存在竞争，细化锁的颗粒度

- 作者：zhangenwei
- 日期：2026-03-23
- 完整 hash：`4fea6d5a607c929a5a0c47c4db4e6a0cb847071f`
- 改动：+162 / -126 行

```diff
@@@C	zhangenwei	4fea6d5a607c929a5a0c47c4db4e6a0cb847071f	2026-03-23	fix(运维):【20260323-zhangenwei-1】mec自检中针对动态数据和静态数据存在竞争，细化锁的颗粒度
diff --git a/middleware/protocol/om/data_model/mec/data_monitor_mec.h b/middleware/protocol/om/data_model/mec/data_monitor_mec.h
index 8a5519a..7017125 100644
--- a/middleware/protocol/om/data_model/mec/data_monitor_mec.h
+++ b/middleware/protocol/om/data_model/mec/data_monitor_mec.h
@@ -37,6 +37,7 @@ struct MonitorState
     bool        isLinkCheck;                // true: 链路状态检查, false: 数据超时检查
     bool        enable;                     // 是否检测
     int         selfCheckFaultType;         // 故障类型ID
+    std::string topic; //消息
     // ==========================================
     // 2. 运行时状态 (Runtime State) - 随时间变化
     // ==========================================
@@ -78,7 +79,9 @@ struct MonitorState
           selfCheckFaultType(sft)
     {
     }
-
+    // 删除拷贝构造和赋值（强制使用指针，防止误拷贝）
+    MonitorState(const MonitorState&) = delete;
+    MonitorState& operator=(const MonitorState&) = delete;
 
     // to_string 保持不变
     std::string to_string() const
@@ -102,6 +105,8 @@ struct MonitorState
         return oss.str();
     }
 };
+// 【关键】定义智能指针类型
+using MonitorStatePtr = std::shared_ptr<MonitorState>;
 /**
  * @brief 故障类型定义
  * 对应协议字段: faultType
diff --git a/middleware/protocol/om/om_component.cc b/middleware/protocol/om/om_component.cc
index 6cbca28..6a44e46 100644
--- a/middleware/protocol/om/om_component.cc
+++ b/middleware/protocol/om/om_component.cc
@@ -952,66 +952,80 @@ void OM_COMPONENT::handleMonitorDataUpdateCcindexTm(const std::string & deviceId
 
         OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm][id]" << pair.first ;
     }
-    // 使用 find 查找
-    auto it = m_MonitorStatesCcindexTmData.find(deviceId);
-    // 判断是否找到
-    if (it == m_MonitorStatesCcindexTmData.end())
+    MonitorStatePtr targetState = nullptr;
+    // --- 第一步：容器层操作（极短时间） ---
     {
-        OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_tm]m_MonitorRulesCcindexTm.find(deviceid) == m_MonitorRulesCcindexTm.end()";
-        return;
-    }
-
-    auto& devices = m_MonitorStatesCcindexTmData[deviceId];
+        std::lock_guard<std::mutex> mapLock(m_CcindexTmMapMutex);
 
-    // 检查该topic是否在监控状态表中
-    if (devices.find(topic) == devices.end())
-    {
-        OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_tm]topic not found in m_MonitorStatesCcindexTm: " << topic;
-        return;
+        // 使用 find 查找
+        auto it = m_MonitorStatesCcindexTmData.find(deviceId);
+        // 判断是否找到
+        if (it == m_MonitorStatesCcindexTmData.end())
+        {
+            OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_tm]m_MonitorRulesCcindexTm.find(deviceid) == m_MonitorRulesCcindexTm.end()";
+            return;
+        }
+        // 查找第二层 topic
+        auto itTopic = it->second.find(topic);
+        if (itTopic != it->second.end()) {
+            targetState = itTopic->second; // 拿到指针，引用计数+1
+        }
     }
+    // auto& devices = m_MonitorStatesCcindexTmData[deviceId];
+    //
+    // // 检查该topic是否在监控状态表中
+    // if (devices.find(topic) == devices.end())
+    // {
+    //     OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_tm]topic not found in m_MonitorStatesCcindexTm: " << topic;
+    //     return;
+    // }
+    //
+    // auto& state = m_MonitorStatesCcindexTmData[deviceId][topic];
 
-    auto& state = m_MonitorStatesCcindexTmData[deviceId][topic];
-    // 【新增】找到具体的 state 对象后，立即加锁
-    std::lock_guard<std::mutex> lock(state.stateMutex);
-    if (! state.enable)
+    if (targetState)
     {
-        return;
-    }
+        // 【新增】找到具体的 state 对象后，立即加锁
+        std::lock_guard<std::mutex> lock(targetState->stateMutex);
+        if (! targetState->enable)
+        {
+            return;
+        }
 
-    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
+        uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
 
-    // 1. 喂狗：更新最后收到时间
-    state.lastReceivedTime = now;
-    if(getConfiger().enableDebugPrint)
-    {
-        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm][topic]" << topic << "[lastReceivedTime]" << state.lastReceivedTime;
-    }
-    // 2. 检查恢复逻辑
-    if (state.isAlarmActive) {
-        if (state.recoveryStartTime == 0)
+        // 1. 喂狗：更新最后收到时间
+        targetState->lastReceivedTime = now;
+        if(getConfiger().enableDebugPrint)
         {
-            // 收到告警后的第一帧数据，开始恢复计时
-            state.recoveryStartTime = now;
-            if(getConfiger().enableDebugPrint)
-            {
-                OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm][recovery-start] tag:" << tag << "[topic]" << topic;
-            }
+            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm][topic]" << topic << "[lastReceivedTime]" << targetState->lastReceivedTime;
         }
-        else
-        {
-            // 检查数据是否持续了足够长的时间
-            if (now - state.recoveryStartTime >=  state.recoveryMs)
+        // 2. 检查恢复逻辑
+        if (targetState->isAlarmActive) {
+            if (targetState->recoveryStartTime == 0)
             {
-                // 满足恢复条件 -> 消除告警
-                std::string errorDetail = state.name + " [topic: " + topic + "]";
-                alarmDisappeared(errorDetail, state.alarmType, getConfiger().rscuEsn, topic, state.alarmOccurredTime);
-                state.isAlarmActive = false;
-                state.recoveryStartTime = 0;
-                state.lastAlarmPublishTime = 0;  // 重置告警上报时间
+                // 收到告警后的第一帧数据，开始恢复计时
+                targetState->recoveryStartTime = now;
                 if(getConfiger().enableDebugPrint)
                 {
-                     OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm] RECOVERED: " << state.name << " [topic: " << topic << "]";
-                 }
+                    OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm][recovery-start] tag:" << tag << "[topic]" << topic;
+                }
+            }
+            else
+            {
+                // 检查数据是否持续了足够长的时间
+                if (now - targetState->recoveryStartTime >=  targetState->recoveryMs)
+                {
+                    // 满足恢复条件 -> 消除告警
+                    std::string errorDetail = targetState->name + " [topic: " + topic + "]";
+                    alarmDisappeared(errorDetail, targetState->alarmType, getConfiger().rscuEsn, topic, targetState->alarmOccurredTime);
+                    targetState->isAlarmActive = false;
+                    targetState->recoveryStartTime = 0;
+                    targetState->lastAlarmPublishTime = 0;  // 重置告警上报时间
+                    if(getConfiger().enableDebugPrint)
+                    {
+                        OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm] RECOVERED: " << targetState->name << " [topic: " << topic << "]";
+                    }
+                }
             }
         }
     }
@@ -1249,6 +1263,7 @@ void OM_COMPONENT::initMonitorRules()
     //雷达动态数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable)
     {
+        std::lock_guard<std::mutex> mapLock(m_CcindexTmMapMutex); // 初始化也要加容器锁
         for (auto pair:m_MecSelfCheckTmDeviceIdTopicMap)
         {
 
@@ -1258,8 +1273,9 @@ void OM_COMPONENT::initMonitorRules()
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.enable)
             {
                 //实时轨迹数据
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Trajectories] =os::v2x::protocol::om::mec::MonitorState
-                {
+
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Trajectories] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_TRAJECTORIES,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1268,13 +1284,13 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
             //实时过车数
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_VehiclePass] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_VehiclePass] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_VEHICLEPASS,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1283,13 +1299,13 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
             //实时排队数
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_QueueUp] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_QueueUp] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_QUEUEUP,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1298,13 +1314,13 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
             //实时区域状态数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_AreaState] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_AreaState] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_AREASTATE,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1313,14 +1329,14 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
 
             //实时溢出数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Overflow] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Overflow] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_OVERFLOW,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1329,13 +1345,13 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
             //实时出口通道数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Outlane] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Outlane] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_OUTLANE,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1344,14 +1360,14 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
 
             //    定时统计数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Statistics] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Statistics] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_STATISTICS,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1360,14 +1376,14 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
 
             // 定时评价数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Evaluations] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Evaluations] =std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_EVALUATIONS,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1376,13 +1392,13 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
             //定时行人及非机动车数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Nonmotor] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Nonmotor] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_NONMOTOR,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1391,13 +1407,13 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
             //设备状态数据
             if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable)
             {
-                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_DeviceStatus] =os::v2x::protocol::om::mec::MonitorState
-                {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_DeviceStatus] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
+                (
                     MONITOR_TAG_DATA_CCINDEX_TM_DEVICESTATUS,
                     "[data ]cc dynamic ",
                     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
@@ -1406,7 +1422,7 @@ void OM_COMPONENT::initMonitorRules()
                     false,
                     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable,
                     OmFaultType::OFT_CCINDEX_TM_DATA
-                };
+                );
             }
             // //实时脉冲数据
             // m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Pulse] = {
@@ -1833,61 +1849,72 @@ void OM_COMPONENT::checkMonitorCcindexTmDataTimeouts()
     {
         return;
     }
+    // 临时容器，用于存放待检查的指针
+    std::vector<MonitorStatePtr> statesToCheck;
+
+    // --- 第一步：获取快照 (只读 Map 结构) ---
+    {
+        std::lock_guard<std::mutex> mapLock(m_CcindexTmMapMutex);
+        // 遍历 Map，收集所有指针
+        for (auto& devicePair : m_MonitorStatesCcindexTmData) {
+            for (auto& topicPair : devicePair.second) {
+                if (topicPair.second) {
+                    statesToCheck.push_back(topicPair.second);
+                }
+            }
+        }
+    }
+
+    // 解锁 Map。此时其他线程可以随意插入/删除 Map 元素，不会影响 statesToCheck 里的指针。
+
+    // --- 第二步：遍历快照进行检查 ---
     uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
     /////////////////////////////////////////////////////////////////////////////////////////
     //检查多雷达动态数据的超时
-    for (auto& statesPair : m_MonitorStatesCcindexTmData)
+    for (auto& statePtr : statesToCheck)
     {
-        std::string deviceId = statesPair.first;
-        auto& statesMap = statesPair.second;
-
-        // 遍历每个雷达的每个topic
-        for (auto& statePair : statesMap)
-        {
-            const std::string& topic = statePair.first;
-            auto& state = statePair.second;
             // 【新增】细粒度锁：每次循环只锁当前这一个 topic 的状态
             // 使用 unique_lock 配合 defer_lock 可以做更多控制，但这里 lock_guard 足够
-            std::lock_guard<std::mutex> lock(state.stateMutex);
-            if (!state.enable)
+            std::lock_guard<std::mutex> lock(statePtr->stateMutex);
+            if (!statePtr->enable)
             {
                 continue;
             }
 
-            uint64_t timeDiff = now - state.lastReceivedTime;
+            uint64_t timeDiff = now - statePtr->lastReceivedTime;
             if (getConfiger().enableDebugPrint)
             {
-                OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm]"<< "[timeDiff]" << timeDiff << "[now]" << now  << "[topic]" << topic
-                                    << "[state]" << state.to_string();
+                OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm]"<< "[timeDiff]" << timeDiff << "[now]" << now  << "[topic]" << statePtr->topic
+                                    << "[state]" << statePtr->to_string();
             }
             // --- 1. 检查超时 (故障触发) ---
-            if ((timeDiff + 1000) >= state.timeoutMs)
+            if ((timeDiff + 1000) >= statePtr->timeoutMs)
             {
                 if (getConfiger().enableDebugPrint)
                 {
-                    OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm] timeDiff >= rule.timeoutMs for topic: " << topic;
+                    OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm] timeDiff >= rule.timeoutMs for topic: " << statePtr->topic;
                 }
-                if (!state.isAlarmActive)
+                if (!statePtr->isAlarmActive)
                 {
                     if (getConfiger().enableDebugPrint)
                     {
-                        OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm] !state.isAlarmActive for topic: " << topic;
+                        OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm] !statePtr->isAlarmActive for topic: " << statePtr->topic;
                     }
-                    std::string errorDetail = state.name + "[data-loss]" + " [topic] " + topic;
-                    state.isAlarmActive = true;
-                    state.alarmOccurredTime = now;
-                    state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
-                    state.recoveryStartTime = 0;
-                    alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, topic,
-                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
+                    std::string errorDetail = statePtr->name + "[data-loss]" + " [topic] " + statePtr->topic;
+                    statePtr->isAlarmActive = true;
+                    statePtr->alarmOccurredTime = now;
+                    statePtr->lastAlarmPublishTime = now;  // 记录首次告警上报时间
+                    statePtr->recoveryStartTime = 0;
+                    alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
+                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);
 
                     if(getConfiger().enableDebugPrint)
                     {
-                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-tm] !state.isAlarmActive [errorDetail [errorDetail]"<< errorDetail
+                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-tm] !statePtr->isAlarmActive [errorDetail [errorDetail]"<< errorDetail
                         << " [timeDiff]" << timeDiff
-                        << "[timeout]" << state.timeoutMs
-                        << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
-                        << "[alarmOccurredTime]" << state.alarmOccurredTime;
+                        << "[timeout]" << statePtr->timeoutMs
+                        << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
+                        << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                     }
                 }
                 // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
@@ -1895,53 +1922,52 @@ void OM_COMPONENT::checkMonitorCcindexTmDataTimeouts()
                 {
                     if(getConfiger().enableDebugPrint)
                     {
-                        OM_CHECK_DEBUG_PRINT << "[====>][om-check][ccindex-tm] state.isAlarmActive " ;
+                        OM_CHECK_DEBUG_PRINT << "[====>][om-check][ccindex-tm] statePtr->isAlarmActive " ;
                     }
                     // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
                     // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
-                    if (!state.isLinkCheck && state.recoveryStartTime != 0)
+                    if (!statePtr->isLinkCheck && statePtr->recoveryStartTime != 0)
                     {
                         if(getConfiger().enableDebugPrint)
                         {
-                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] recovery reset for " << state.name
+                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] recovery reset for " << statePtr->name
                                             << " (data stopped again for " << timeDiff << "ms)";
                         }
-                        state.recoveryStartTime = 0;
+                        statePtr->recoveryStartTime = 0;
                     }
 
                     // 检查是否需要重复上报告警
-                    if (state.lastAlarmPublishTime > 0)
+                    if (statePtr->lastAlarmPublishTime > 0)
                     {
-                        uint64_t alarmPublishTimeDiff = (now - state.lastAlarmPublishTime);
+                        uint64_t alarmPublishTimeDiff = (now - statePtr->lastAlarmPublishTime);
                         if (getConfiger().enableDebugPrint)
                         {
                             OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] alarm-repeat "
-                                            << "[name]" << state.name
+                                            << "[name]" << statePtr->name
                                             << "[timeDiff(ms)]" << alarmPublishTimeDiff
-                                            << "[interval(ms)]" << state.timeoutMs
-                                                << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
-                                                << "[alarmOccurredTime]" << state.alarmOccurredTime;
+                                            << "[interval(ms)]" << statePtr->timeoutMs
+                                                << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
+                                                << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                         }
 
                         // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
-                        if (alarmPublishTimeDiff >= state.timeoutMs)
+                        if (alarmPublishTimeDiff >= statePtr->timeoutMs)
                         {
-                            std::string errorDetail = state.name + " [topic: " + topic + "] [no data]";
-                            state.lastAlarmPublishTime = now;  // 更新上次上报时间
-                            alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, topic,
-                                "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
+                            std::string errorDetail = statePtr->name + " [topic: " + statePtr->topic + "] [no data]";
+                            statePtr->lastAlarmPublishTime = now;  // 更新上次上报时间
+                            alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
+                                "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);
 
                             if (getConfiger().enableDebugPrint)
                             {
                                 OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] alarm repeated [errorDetail]" << errorDetail
                                 << "[timeout]" << timeDiff
-                                << "[interval]" << state.timeoutMs;
+                                << "[interval]" << statePtr->timeoutMs;
                             }
                         }
                     }
                 }
             }
-        }
     }
 }
 
diff --git a/middleware/protocol/om/om_component.h b/middleware/protocol/om/om_component.h
index 3ec89a1..4a106e3 100644
--- a/middleware/protocol/om/om_component.h
+++ b/middleware/protocol/om/om_component.h
@@ -901,11 +901,16 @@ private:
 
     //mec自检： 雷达动态数据
     std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexTmData;
-    std::map<std::string, std::map<std::string, MonitorState>>     m_MonitorStatesCcindexTmData;  // <id, <topic, state>>
+    // 容器锁：保护 m_MonitorStatesCcindexTmData 的结构
+    std::mutex m_CcindexTmMapMutex;
+    // std::map<std::string, std::map<std::string, MonitorState>>     m_MonitorStatesCcindexTmData;  // <id, <topic, state>>
+    // 数据容器：存指针
+    std::map<std::string, std::map<std::string, MonitorStatePtr>> m_MonitorStatesCcindexTmData;
     std::map<std::string, TopicCCIndexTm>  m_MecSelfCheckTmDeviceIdTopicMap;
 
     //mec自检： 雷达静态数据
     std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexStData;
+    std::mutex m_CcindexStMapMutex;
     std::map<std::string, std::map<std::string, MonitorState>>     m_MonitorStatesCcindexStData;  // <id, <topic, state>>
     std::map<std::string, TopicCCIndexSt>  m_MecSelfCheckStDeviceIdTopicMap;
```
