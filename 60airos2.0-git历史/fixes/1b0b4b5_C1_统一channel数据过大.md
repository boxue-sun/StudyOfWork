# fix(运维):【20260316-zhangenwei-1】由于统一channel中数据过大，导致监控关键数据丢失，修改成多个channel

- 作者：zhangenwei
- 日期：2026-03-16
- 完整 hash：`1b0b4b54541d2d00af65bcda9a3a467d7883d21e`
- 改动：+1952 / -593 行

```diff
@@@C	zhangenwei	1b0b4b54541d2d00af65bcda9a3a467d7883d21e	2026-03-16	fix(运维):【20260316-zhangenwei-1】由于统一channel中数据过大，导致监控关键数据丢失，修改成多个channel
diff --git a/app/modules/v2x_application/traffic_light_adapter/traffic_light_adapter.h b/app/modules/v2x_application/traffic_light_adapter/traffic_light_adapter.h
index 0a55648..b8571e8 100644
--- a/app/modules/v2x_application/traffic_light_adapter/traffic_light_adapter.h
+++ b/app/modules/v2x_application/traffic_light_adapter/traffic_light_adapter.h
@@ -102,12 +102,12 @@ private:
     std::map<int, MecPhaseInfo> mec_trafficlights_map;
     uint64_t mec_recv_time;
     std::shared_ptr<apollo::cyber::Writer<airos::usecase::EventOutputResult>> writer_;
-    std::string channel_name = "/om/spat/check";
+    std::string channel_name = "/v2x/mec/om/spat/detect/data_";
     std::shared_ptr <airos::usecase::EventOutputResult> output_data_trafficlight_detect_ = nullptr;
     uint64_t normal_time = 0;
     //信号机数据状态
     std::shared_ptr<apollo::cyber::Writer<airos::monitor_mec::MonitorSpat>> writer_trafficlght_data_status_;
-    std::string channel_name_trafficlight_data_status = "/v2x/monitor/spat";
+    std::string channel_name_trafficlight_data_status = "/v2x/mec/om/spat/status_";
     std::shared_ptr <airos::monitor_mec::MonitorSpat> output_data_status_trafficlight_ = nullptr;
 };
 
diff --git a/middleware/device_service/framework/mec/mec_component.cc b/middleware/device_service/framework/mec/mec_component.cc
index 255d292..1c8762b 100644
--- a/middleware/device_service/framework/mec/mec_component.cc
+++ b/middleware/device_service/framework/mec/mec_component.cc
@@ -107,10 +107,10 @@ void AIROS_COMPONENT_CLASS_NAME(MECComponent)::CallBack(const MecDataType& data)
       if (mec_sensor_data_monitor.sensor_data_in_flag())
       {
         auto* md_sensor_objs =  mec_monitor_->mutable_md_sensor_objs();
-        md_sensor_objs->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_SENSOR_OBJ_DATA);
+        md_sensor_objs->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_SENSOR_OBJ);
         md_sensor_objs->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
-        MEC_SERVICE_LOG_WARN << "[mec-monitor-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_->ShortDebugString();
-        Send("/v2x/monitor/mec", mec_monitor_);
+        MEC_SERVICE_LOG_WARN << "[mec-monitor-out] Send topic: /v2x/mec/om/check/rsap/data_,[data]" << mec_monitor_->ShortDebugString();
+        Send("/v2x/mec/om/check/rsap/data_", mec_monitor_);
       }
     }
 }
diff --git a/middleware/device_service/framework/traffic_light/traffic_light_component.cc b/middleware/device_service/framework/traffic_light/traffic_light_component.cc
index 6711359..670d02d 100644
--- a/middleware/device_service/framework/traffic_light/traffic_light_component.cc
+++ b/middleware/device_service/framework/traffic_light/traffic_light_component.cc
@@ -80,13 +80,13 @@ bool AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::Proc(
 void AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::CallBack(
     const TrafficLightDataType& data) {
     Send("/airos/device/traffic_light/data", data);
-
+    //信号机原始数据
     {
       auto* md_spat_src_data =  mec_monitor_->mutable_md_spat_src_data();
-      md_spat_src_data->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_SPAT_SRC_DATA);
+      md_spat_src_data->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_SPAT_SRC);
       md_spat_src_data->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
-      TRAFFIC_LIFGT_SERVICE_LOG_INFO << "[monitor-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_->ShortDebugString();
-      Send("/v2x/monitor/mec", mec_monitor_);
+      TRAFFIC_LIFGT_SERVICE_LOG_INFO << "[monitor-out] Send topic: /v2x/mec/om/check/spat_src/data_,[data]" << mec_monitor_->ShortDebugString();
+      Send("/v2x/mec/om/check/spat_src/data_", mec_monitor_);
     }
 }
 
diff --git a/middleware/protocol/ccindex/ccindex_component.cc b/middleware/protocol/ccindex/ccindex_component.cc
index 8109245..447aefa 100644
--- a/middleware/protocol/ccindex/ccindex_component.cc
+++ b/middleware/protocol/ccindex/ccindex_component.cc
@@ -2220,6 +2220,77 @@ bool CCINDEX_COMPONENT::mqttPublishMsg(const std::string &topic, const std::stri
 
     return true;
 }
+//根据topic获取
+bool CCINDEX_COMPONENT::getMecCheckCcindexTmTag(std::string& topic)
+{
+
+ // 1. 提取第二部分的内容 (例如 "vehiclePass", "trajectories" 等)
+    std::string type_str;
+    size_t first_slash = topic.find('/');
+    size_t second_slash = topic.find('/', first_slash + 1);
+
+    // 确保找到了两个 '/'，保证格式正确
+    if (first_slash != std::string::npos && second_slash != std::string::npos) {
+        // substr(起始位置, 长度)
+        type_str = topic.substr(first_slash + 1, second_slash - first_slash - 1);
+    } else {
+        // 格式不对，直接返回或记录错误
+        return false;
+    }
+    // ================= 新增：提取最后一个 '/' 之后的内容 =================
+    std::string device_id_str;
+    size_t last_slash = topic.rfind('/');
+    if (last_slash != std::string::npos) {
+        // 提取最后一个 '/' 之后的所有字符 (例如 "{device_id}")
+        device_id_str = topic.substr(last_slash + 1);
+    }
+    // =====================================================================
+    // 2. 定义默认 Tag (可选)
+    airos::monitor_mec::MonitorMecTag target_tag;
+    bool is_match = true;
+
+    // 3. 字符串匹配并赋值对应的 Enum
+    if (type_str == "trajectories") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_TRAJECTORIES; // 3028
+    }
+    else if (type_str == "vehiclePass") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_VEHICLEPASS; // 3029
+    }
+    else if (type_str == "queueUp") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_QUEUEUP; // 3030
+    }
+    else if (type_str == "areaState") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_AREASTATE; // 3031
+    }
+    else if (type_str == "overflow") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_OVERFLOW; // 3032
+    }
+    else if (type_str == "outlane") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_OUTLANE; // 3033
+    }
+    else if (type_str == "statistics") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_STATISTICS; // 3034
+    }
+    else if (type_str == "evaluations") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_EVALUATIONS; // 3035
+    }
+    else if (type_str == "nonmotor") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_NONMOTOR; // 3036
+    }
+    else if (type_str == "deviceStatus") {
+        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_DEVICESTATUS; // 3037
+    }
+    else {
+        // 未知的类型
+        is_match = false;
+    }
+
+    // 4. 如果匹配成功，调用函数
+    if (is_match) {
+        pushMonitorMecDataStatusTm(target_tag, topic, device_id_str);
+    }
+    return true;
+}
 
 int CCINDEX_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
 {
@@ -2248,7 +2319,7 @@ int CCINDEX_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int
             //     CCINDEX_MQTT_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << thiz->mec_monitor_->ShortDebugString();
             //     thiz->Send("/v2x/monitor/mec", thiz->mec_monitor_);
             // }
-            thiz->pushMonitorMecDataStatusTm(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_TM, topic);
+            thiz->getMecCheckCcindexTmTag(topic);
             if(thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_TrafficMetrics)
             {
                 afl::base::json trafficMetricsJson;
@@ -2280,6 +2351,7 @@ int CCINDEX_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int
             //     CCINDEX_MQTT_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << thiz->mec_monitor_->ShortDebugString();
             //     thiz->Send("/v2x/monitor/mec", thiz->mec_monitor_);
             // }
+            thiz->pushMonitorMecDataStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TC);
             if(thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Tc)
             {
                 std::string msg((char*)message->payload, message->payloadlen);
@@ -2296,7 +2368,24 @@ int CCINDEX_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int
             //     CCINDEX_MQTT_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << thiz->mec_monitor_->ShortDebugString();
             //     thiz->Send("/v2x/monitor/mec", thiz->mec_monitor_);
             // }
-            thiz->pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_ST, topic);
+            std::string device_id_str;
+            size_t last_slash = topic.rfind('/');
+            if (last_slash != std::string::npos) {
+                // 提取最后一个 '/' 之后的所有字符 (例如 "{device_id}")
+                device_id_str = topic.substr(last_slash + 1);
+            }
+            if (topic.find("static") != std::string::npos &&
+                topic.find("query") != std::string::npos &&
+                topic.find("ack") != std::string::npos)
+            {
+               thiz->pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_ST_QUERY_ACK, topic, device_id_str);
+            }
+
+            if (topic.find("static") != std::string::npos &&
+                topic.find("update") != std::string::npos )
+            {
+                thiz->pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_ST_UPDATE, topic, device_id_str);
+            }
             if(thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Static)
             {
                 afl::base::json staticJson;
@@ -3394,7 +3483,7 @@ bool CCINDEX_COMPONENT::mqttPublishMsgCloud(const std::string &topic, const std:
     std::string topicProfix = topic.substr(0, topic.find("/"));
     if(topicProfix == "upload" )
     {
-        pushMonitorMecDataStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_TC);
+        pushMonitorMecDataStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TC);
     }
     if (msg.size() <= 0 || topic.empty())
     {
@@ -7451,7 +7540,7 @@ void CCINDEX_COMPONENT::pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecT
     //
     switch (tag)
     {
-    case airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_ST:
+    case airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_CCINDEX_ST:
         {
             auto* ml_ccindex_st =  mec_monitor_->mutable_ml_ccindex_st();
             ml_ccindex_st->set_tag(tag);
@@ -7459,12 +7548,12 @@ void CCINDEX_COMPONENT::pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecT
             ml_ccindex_st->set_con_flag(con_flag);
             if(getConfiger().enableDebugPrint)
             {
-                RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_->ShortDebugString();
+                CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/st/link_ ,[data]" << mec_monitor_->ShortDebugString();
             }
-            Send("/v2x/monitor/mec", mec_monitor_);
+            Send("/v2x/mec/om/check/ccindex/st/link_", mec_monitor_);
         }
         break;
-    case airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM:
+    case airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_CCINDEX_TM:
         {
             auto* ml_ccindex_tm =  mec_monitor_->mutable_ml_ccindex_tm();
             ml_ccindex_tm->set_tag(tag);
@@ -7472,13 +7561,13 @@ void CCINDEX_COMPONENT::pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecT
             ml_ccindex_tm->set_con_flag(con_flag);
             if(getConfiger().enableDebugPrint)
             {
-                RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_->ShortDebugString();
+                CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/tm/link_,[data]" << mec_monitor_->ShortDebugString();
             }
-            Send("/v2x/monitor/mec", mec_monitor_);
+            Send("/v2x/mec/om/check/ccindex/tm/link_", mec_monitor_);
         }
         break;
     default:
-        RSAP_DEBUG_PRINT << "[monitor-mec-out]no support this tag!";
+        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out]no support this tag!";
         break;
     }
 
@@ -7506,113 +7595,128 @@ void CCINDEX_COMPONENT::pushMonitorMecDataStatusTm(airos::monitor_mec::MonitorMe
     {
         md_ccindex_tm->set_topic(topic);
     }
+    md_ccindex_tm->set_device_id(device_id);
     if(getConfiger().enableDebugPrint)
     {
-        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_->ShortDebugString();
+        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/tm/data_,[data]" << mec_monitor_->ShortDebugString();
     }
-    Send("/v2x/monitor/mec", mec_monitor_);
+    Send("/v2x/mec/om/check/ccindex/tm/data_", mec_monitor_);
 }
-void CCINDEX_COMPONENT::pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag tag, std::string topic)
+void CCINDEX_COMPONENT::pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag tag, std::string topic, std::string device_id)
 {
     //airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_TC
-    auto* md_ccindex_tm =  mec_monitor_->mutable_md_ccindex_st();
-    md_ccindex_tm->set_tag(tag);
-    md_ccindex_tm->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
+    auto* md_ccindex_st =  mec_monitor_->mutable_md_ccindex_st();
+    md_ccindex_st->set_tag(tag);
+    md_ccindex_st->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
     if (!topic.empty())
     {
-        md_ccindex_tm->set_topic(topic);
+        md_ccindex_st->set_topic(topic);
     }
+    md_ccindex_st->set_device_id(device_id);
     if(getConfiger().enableDebugPrint)
     {
-        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_->ShortDebugString();
+        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/st/data_,[data]" << mec_monitor_->ShortDebugString();
     }
-    Send("/v2x/monitor/mec", mec_monitor_);
+    Send("/v2x/mec/om/check/ccindex/st/data_", mec_monitor_);
 }
 
 void CCINDEX_COMPONENT::monitorMecLinkStatusCcindexSt()
 {
     //
     auto* ml_ccindex_st =  mec_monitor_link_status_ccindex_st_->mutable_ml_ccindex_st();
-    ml_ccindex_st->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_ST);
+    ml_ccindex_st->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_CCINDEX_ST);
     ml_ccindex_st->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
 
     size_t uniqueCategoryCount = m_VendorStats.size();
 
     if (uniqueCategoryCount > 0)
     {
-	
-	
-
-
-        RSAP_DEBUG_PRINT << "[monitor-mec-out] uniqueCategoryCount： " << uniqueCategoryCount ;
-        bool final_con_flag = false;       
-        bool is_radar_found = false;      
+        // =======================================================
+        // 1. 逻辑判断部分：直接查找特定 Key，而不是依赖循环顺序
+        // =======================================================
+        if (getConfiger().enableDebugPrint)
+        {
+            CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] uniqueCategoryCount： " << uniqueCategoryCount ;
+        }
 
+        bool final_con_flag = false;       // 最终要设置的状态
+        bool is_radar_found = false;       // 是否找到了关心的雷达类型
 
+        // --- 检查 tsmtc ---
         auto it_tsmtc = m_VendorStats.find("tsmtc");
         if (it_tsmtc != m_VendorStats.end() && it_tsmtc->second > 0)
         {
             is_radar_found = true;
-            RSAP_DEBUG_PRINT << "[monitor-mec-out] tsmtc radar found" ;
-
+            if (getConfiger().enableDebugPrint)
+            {
+                CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] tsmtc radar found" ;
+            }
+            // 如果存在 tsmtc，状态取决于 m_MecLinkStatucCcindexTmTsmtc
             final_con_flag = m_MecLinkStatucCcindexTmTsmtc;
         }
 
-
+        // --- 检查 desaysv ---
         auto it_desaysv = m_VendorStats.find("desaysv");
         if (it_desaysv != m_VendorStats.end() && it_desaysv->second > 0)
         {
-
+            // 如果之前已经找到了 tsmtc (即两种雷达共存)
             if (is_radar_found)
             {
-                RSAP_DEBUG_PRINT << "[monitor-mec-out] desaysv radar found" ;
-
-
+                if (getConfiger().enableDebugPrint)
+                {
+                    CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] desaysv radar found" ;
+                }
+                // 逻辑策略：假设所有存在的雷达都必须连接正常，Flag 才为 true (逻辑与)
+                // 如果您的业务是“只要有一个正常就行”，请将 && 改为 ||
                 final_con_flag = final_con_flag && m_MecLinkStatucCcindexTmDesaysv;
             }
             else
             {
-
+                // 只有 desaysv
                 is_radar_found = true;
                 final_con_flag = m_MecLinkStatucCcindexTmDesaysv;
             }
         }
 
-
-
-
+        // =======================================================
+        // 2. 执行赋值：只在最后执行一次
+        // =======================================================
         if (is_radar_found)
         {
             ml_ccindex_st->set_con_flag(final_con_flag);
         }
         else
         {
-
-
+            // 如果 map 里有数据，但不是 tsmtc 也不是 desaysv，或者数量为0
+            // 根据需求设置默认值，通常设为 false
             ml_ccindex_st->set_con_flag(false);
         }
 
-
-
-
+        // =======================================================
+        // 3. 日志打印部分：保留遍历用于输出信息
+        // =======================================================
         for (const auto& item : m_VendorStats)
         {
-
-
-            RSAP_DEBUG_PRINT << "Vendor: " << item.first << ", Count: " << item.second;
+            // 注意：RSAP_DEBUG_PRINT 流式输出通常不需要 c_str()，除非是 printf 风格
+            // 且 item.second 就是 count
+            if (getConfiger().enableDebugPrint)
+            {
+                CCINDEX_INTER_DEBUG_PRINT << "Vendor: " << item.first << ", Count: " << item.second;
+            }
         }
     }
+
     if(getConfiger().enableDebugPrint)
     {
-        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_link_status_ccindex_st_->ShortDebugString();
+        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/st/link_,[data]" << mec_monitor_link_status_ccindex_st_->ShortDebugString();
     }
-    Send("/v2x/monitor/mec", mec_monitor_link_status_ccindex_st_);
+    Send("/v2x/mec/om/check/ccindex/st/link_", mec_monitor_link_status_ccindex_st_);
 }
 
 void CCINDEX_COMPONENT::monitorMecLinkStatusCcindexTm()
 {
     auto* ml_ccindex_tm =  mec_monitor_link_status_ccindex_tm_->mutable_ml_ccindex_tm();
-    ml_ccindex_tm->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM);
+    ml_ccindex_tm->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_CCINDEX_TM);
     ml_ccindex_tm->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
     if (m_MqttConnectedCloud )
     {
@@ -7624,8 +7728,8 @@ void CCINDEX_COMPONENT::monitorMecLinkStatusCcindexTm()
     }
     if(getConfiger().enableDebugPrint)
     {
-        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_link_status_ccindex_tm_->ShortDebugString();
+        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/tm/link_,[data]" << mec_monitor_link_status_ccindex_tm_->ShortDebugString();
     }
-    Send("/v2x/monitor/mec", mec_monitor_link_status_ccindex_tm_);
+    Send("/v2x/mec/om/check/ccindex/tm/link_", mec_monitor_link_status_ccindex_tm_);
 }
 NAMESPACE_ENDED_RADAR_CCINDEX
diff --git a/middleware/protocol/ccindex/ccindex_component.h b/middleware/protocol/ccindex/ccindex_component.h
index 3dbba8a..00c6531 100644
--- a/middleware/protocol/ccindex/ccindex_component.h
+++ b/middleware/protocol/ccindex/ccindex_component.h
@@ -286,14 +286,14 @@ struct TrafficMetricsStatistics
         return ss.str();
     }
 };
-
+//动态数据状态
 struct CcindexTmStatus
 {
     std::string deviceEsn = "";
     std::string deviceSn = "";
-
+    //推送标识
     std::atomic<bool> tm_pub_flag{false};
-
+    //订阅标识
     std::atomic<bool> tm_sub_flag{false};
 
     std::string to_string()
@@ -306,12 +306,12 @@ struct CcindexTmStatus
         return ss.str();
     }
 };
-
+//信控状态
 struct CcindexTcStatus
 {
-
+    //发布标识
     std::atomic<bool> tc_pub_flag{false};
-
+    //订阅标识
     std::atomic<bool> tc_sub_flag{false};
 
     std::string to_string()
@@ -323,7 +323,7 @@ struct CcindexTcStatus
         return ss.str();
     }
 };
-
+//静态数据
 struct CcindexStaticStatus
 {
     std::atomic<bool> st_post_flag{false};
@@ -1128,8 +1128,9 @@ private:
     std::shared_ptr <airos::monitor_mec::MonitorMec> mec_monitor_ = nullptr;
     void pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag tag, bool con_flag);
     void pushMonitorMecDataStatus(airos::monitor_mec::MonitorMecTag tag);
-    void pushMonitorMecDataStatusTm(airos::monitor_mec::MonitorMecTag tag, std::string topic);
-    void pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag tag, std::string topic);
+    void pushMonitorMecDataStatusTm(airos::monitor_mec::MonitorMecTag tag, std::string topic, std::string device_id);
+    bool getMecCheckCcindexTmTag(std::string& topic);
+    void pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag tag, std::string topic, std::string device_id);
 
     void monitorMecLinkStatusCcindexTm();
     int m_TimerMecLinkStatusCcindexTm = -1;
diff --git a/middleware/protocol/om/configer/configer_om_mec.h b/middleware/protocol/om/configer/configer_om_mec.h
index 9d173fc..c5c5ce9 100644
--- a/middleware/protocol/om/configer/configer_om_mec.h
+++ b/middleware/protocol/om/configer/configer_om_mec.h
@@ -491,6 +491,17 @@ struct MecSelfCheckTime : public afl::base::SerializableData
     TimeRule  ccindex_tc_data_tr;
     //雷达动态数据
     TimeRule  ccindex_tm_data_tr;
+    TimeRule  ccindex_tm_data_trajectories_tr;  //实时轨迹数据
+    TimeRule  ccindex_tm_data_vehiclePass_tr;   //实时过车数据
+    TimeRule  ccindex_tm_data_queueUp_tr;       //实时排队数据
+    TimeRule  ccindex_tm_data_areaState_tr;     //实时区域状态数据
+    TimeRule  ccindex_tm_data_overflow_tr;      //实时溢出数据
+    TimeRule  ccindex_tm_data_outlane_tr;       //实时出口通道数据
+    TimeRule  ccindex_tm_data_statistics_tr;    //定时统计数据
+    TimeRule  ccindex_tm_data_evaluations_tr;   //定时评价数据
+    TimeRule  ccindex_tm_data_nonmotor_tr;      //定时行人及非机动车数据
+    TimeRule  ccindex_tm_data_deviceStatus_tr;  //实时设备状态
+
     TimeRule  ccindex_st_data_tr;
     TimeRule  cloud_point_data_tr;
     TimeRule  sensor_cloud_link_tr;
@@ -499,7 +510,7 @@ struct MecSelfCheckTime : public afl::base::SerializableData
     TimeRule  cloud_point_link_tr;
     MecSelfCheckTime()
     {
-        spat_src_data_tr.timeoutSecond = 5;
+		spat_src_data_tr.timeoutSecond = 5;
         spat_src_data_tr.recoverySecond = 5;
         spat_src_data_tr.enable = true;
 
@@ -511,21 +522,61 @@ struct MecSelfCheckTime : public afl::base::SerializableData
         ccindex_tc_data_tr.recoverySecond = 15;
         ccindex_tc_data_tr.enable = true;
 
+
         ccindex_tm_data_tr.timeoutSecond = 3;
         ccindex_tm_data_tr.recoverySecond = 3;
         ccindex_tm_data_tr.enable = true;
+        //实时轨迹数据
+        ccindex_tm_data_trajectories_tr.timeoutSecond = 3;
+        ccindex_tm_data_trajectories_tr.recoverySecond = 3;
+        ccindex_tm_data_trajectories_tr.enable = true;
+        //实时过车数据
+        ccindex_tm_data_vehiclePass_tr.timeoutSecond = 3;
+        ccindex_tm_data_vehiclePass_tr.recoverySecond = 3;
+        ccindex_tm_data_vehiclePass_tr.enable = true;
+
+        //实时排队数据
+        ccindex_tm_data_queueUp_tr.timeoutSecond = 3;
+        ccindex_tm_data_queueUp_tr.recoverySecond = 3;
+        ccindex_tm_data_queueUp_tr.enable = true;
+        //实时区域状态数据
+        ccindex_tm_data_areaState_tr.timeoutSecond = 3;
+        ccindex_tm_data_areaState_tr.recoverySecond = 3;
+        ccindex_tm_data_areaState_tr.enable = true;
+        //实时溢出数据
+        ccindex_tm_data_overflow_tr.timeoutSecond = 3;
+        ccindex_tm_data_overflow_tr.recoverySecond = 3;
+        ccindex_tm_data_overflow_tr.enable = true;
+        //实时出口通道数据
+        ccindex_tm_data_outlane_tr.timeoutSecond = 3;
+        ccindex_tm_data_outlane_tr.recoverySecond = 3;
+        ccindex_tm_data_outlane_tr.enable = true;
+
+        //定时统计数据
+        ccindex_tm_data_statistics_tr.timeoutSecond = 3;
+        ccindex_tm_data_statistics_tr.recoverySecond = 3;
+        ccindex_tm_data_statistics_tr.enable = true;
+        //定时评价数据
+        ccindex_tm_data_evaluations_tr.timeoutSecond = 3;
+        ccindex_tm_data_evaluations_tr.recoverySecond = 3;
+        ccindex_tm_data_evaluations_tr.enable = true;
+        //定时行人及非机动车数据
+        ccindex_tm_data_nonmotor_tr.timeoutSecond = 40;
+        ccindex_tm_data_nonmotor_tr.recoverySecond = 40;
+        ccindex_tm_data_nonmotor_tr.enable = true;
+        //实时设备状态
+        ccindex_tm_data_deviceStatus_tr.timeoutSecond = 350;
+        ccindex_tm_data_deviceStatus_tr.recoverySecond = 350;
+        ccindex_tm_data_deviceStatus_tr.enable = true;
+
 
         ccindex_st_data_tr.timeoutSecond = 900;
         ccindex_st_data_tr.recoverySecond = 900;
-        // ccindex_st_data_tr.timeoutSecond = 5;
-        // ccindex_st_data_tr.recoverySecond = 5;
         ccindex_st_data_tr.enable = true;
 
 
         cloud_point_data_tr.timeoutSecond = 3600;
         cloud_point_data_tr.recoverySecond = 3600;
-        // cloud_point_data_tr.timeoutSecond = 6;
-        // cloud_point_data_tr.recoverySecond = 6;
         cloud_point_data_tr.enable = true;
 
         sensor_cloud_link_tr.timeoutSecond = 5;
@@ -542,8 +593,6 @@ struct MecSelfCheckTime : public afl::base::SerializableData
 
         cloud_point_link_tr.timeoutSecond = 3600;
         cloud_point_link_tr.recoverySecond = 3600;
-        // cloud_point_link_tr.timeoutSecond = 6;
-        // cloud_point_link_tr.recoverySecond = 6;
         cloud_point_link_tr.enable = true;
     }
 
@@ -551,39 +600,53 @@ private:
     virtual void serialize(json &j) override
     {
         JsonSerialize(spat_src_data_tr,                      "A_spat_src_data_tr",                      j, false);
-        JsonSerialize(sensor_objs_data_tr,                         "B_sensor_objs_tr",                         j, false);
-        JsonSerialize(ccindex_tc_data_tr,                        "C_ccindex_tc_tr",                        j, false);
-        JsonSerialize(ccindex_tm_data_tr,                      "D_ccindex_tm_tr",                      j, false);
-        JsonSerialize(ccindex_st_data_tr,                      "E_ccindex_st_tr",                      j, false);
-        JsonSerialize(cloud_point_data_tr,                    "F_cloud_point_data_tr",                    j, false);
-
-
-        JsonSerialize(sensor_cloud_link_tr,                    "G_sensor_cloud_link_tr",                    j, false);
-
+        JsonSerialize(sensor_objs_data_tr,                   "B_sensor_objs_data_tr",                        j, false);
+        JsonSerialize(ccindex_tc_data_tr,                    "C_ccindex_tc_data_tr",                         j, false);
+        JsonSerialize(ccindex_tm_data_tr,                    "D_ccindex_tm_data_tr",                         j, false);
+        JsonSerialize(ccindex_st_data_tr,                    "E_ccindex_st_data_tr",                         j, false);
+        JsonSerialize(cloud_point_data_tr,                   "F_cloud_point_data_tr",                   j, false);
+        JsonSerialize(sensor_cloud_link_tr,                  "G_sensor_cloud_link_tr",                  j, false);
         JsonSerialize(ccindex_st_link_tr,                    "H_ccindex_st_link_tr",                    j, false);
-
         JsonSerialize(ccindex_tm_link_tr,                    "I_ccindex_tm_link_tr",                    j, false);
-
-        JsonSerialize(cloud_point_link_tr,                    "J_cloud_point_link_tr",                    j, false);
-
+        JsonSerialize(cloud_point_link_tr,                   "J_cloud_point_link_tr",                   j, false);
+
+        // --- 以下为补充的缺失字段 ---
+        JsonSerialize(ccindex_tm_data_trajectories_tr,       "K_ccindex_tm_data_trajectories_tr",       j, false);
+        JsonSerialize(ccindex_tm_data_vehiclePass_tr,        "L_ccindex_tm_data_vehiclePass_tr",        j, false);
+        JsonSerialize(ccindex_tm_data_queueUp_tr,            "M_ccindex_tm_data_queueUp_tr",            j, false);
+        JsonSerialize(ccindex_tm_data_areaState_tr,          "N_ccindex_tm_data_areaState_tr",          j, false);
+        JsonSerialize(ccindex_tm_data_overflow_tr,           "O_ccindex_tm_data_overflow_tr",           j, false);
+        JsonSerialize(ccindex_tm_data_outlane_tr,            "P_ccindex_tm_data_outlane_tr",            j, false);
+        JsonSerialize(ccindex_tm_data_statistics_tr,         "Q_ccindex_tm_data_statistics_tr",         j, false);
+        JsonSerialize(ccindex_tm_data_evaluations_tr,        "R_ccindex_tm_data_evaluations_tr",        j, false);
+        JsonSerialize(ccindex_tm_data_nonmotor_tr,           "S_ccindex_tm_data_nonmotor_tr",           j, false);
+        JsonSerialize(ccindex_tm_data_deviceStatus_tr,       "T_ccindex_tm_data_deviceStatus_tr",       j, false);
     }
 
     virtual void deserialize(const json &j) override
     {
-        JsonDeserialize(spat_src_data_tr,                      "A_spat_src_data_tr",                      j, noUse_isEmptyFlag);
-        JsonDeserialize(sensor_objs_data_tr,                         "B_sensor_objs_tr",                         j, noUse_isEmptyFlag);
-        JsonDeserialize(ccindex_tc_data_tr,                        "C_ccindex_tc_tr",                        j, noUse_isEmptyFlag);
-        JsonDeserialize(ccindex_tm_data_tr,                      "D_ccindex_tm_tr",                      j, noUse_isEmptyFlag);
-        JsonDeserialize(ccindex_st_data_tr,                      "E_ccindex_st_tr",                      j, noUse_isEmptyFlag);
-        JsonDeserialize(cloud_point_data_tr,                    "F_cloud_point_data_tr",                    j, noUse_isEmptyFlag);
-
-        JsonDeserialize(sensor_cloud_link_tr,                    "G_sensor_cloud_link_tr",                    j, noUse_isEmptyFlag);
-
-        JsonDeserialize(ccindex_st_link_tr,                    "H_ccindex_st_link_tr",                    j, noUse_isEmptyFlag);
-
-        JsonDeserialize(ccindex_tm_link_tr,                    "I_ccindex_tm_link_tr",                    j, noUse_isEmptyFlag);
-
-        JsonDeserialize(cloud_point_link_tr,                    "J_cloud_point_link_tr",                    j, noUse_isEmptyFlag);
+        JsonDeserialize(spat_src_data_tr,                    "A_spat_src_data_tr",                      j, noUse_isEmptyFlag);
+        JsonDeserialize(sensor_objs_data_tr,                 "B_sensor_objs_data_tr",                        j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tc_data_tr,                  "C_ccindex_tc_data_tr",                         j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_tr,                  "D_ccindex_tm_data_tr",                         j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_st_data_tr,                  "E_ccindex_st_data_tr",                         j, noUse_isEmptyFlag);
+        JsonDeserialize(cloud_point_data_tr,                 "F_cloud_point_data_tr",                   j, noUse_isEmptyFlag);
+        JsonDeserialize(sensor_cloud_link_tr,                "G_sensor_cloud_link_tr",                  j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_st_link_tr,                  "H_ccindex_st_link_tr",                    j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_link_tr,                  "I_ccindex_tm_link_tr",                    j, noUse_isEmptyFlag);
+        JsonDeserialize(cloud_point_link_tr,                 "J_cloud_point_link_tr",                   j, noUse_isEmptyFlag);
+
+        // --- 以下为补充的缺失字段 ---
+        JsonDeserialize(ccindex_tm_data_trajectories_tr,     "K_ccindex_tm_data_trajectories_tr",       j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_vehiclePass_tr,      "L_ccindex_tm_data_vehiclePass_tr",        j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_queueUp_tr,          "M_ccindex_tm_data_queueUp_tr",            j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_areaState_tr,        "N_ccindex_tm_data_areaState_tr",          j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_overflow_tr,         "O_ccindex_tm_data_overflow_tr",           j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_outlane_tr,          "P_ccindex_tm_data_outlane_tr",            j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_statistics_tr,       "Q_ccindex_tm_data_statistics_tr",         j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_evaluations_tr,      "R_ccindex_tm_data_evaluations_tr",        j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_nonmotor_tr,         "S_ccindex_tm_data_nonmotor_tr",           j, noUse_isEmptyFlag);
+        JsonDeserialize(ccindex_tm_data_deviceStatus_tr,     "T_ccindex_tm_data_deviceStatus_tr",       j, noUse_isEmptyFlag);
     }
 
 public:
@@ -591,15 +654,29 @@ public:
     {
         std::stringstream ss;
         ss << std::left << std::setw(40) << "spat_src_data_tr: "           << spat_src_data_tr.to_string()           << std::endl;
-        ss << std::left << std::setw(40) << "sensor_objs_data_tr: "              << sensor_objs_data_tr.to_string()              << std::endl;
-        ss << std::left << std::setw(40) << "ccindex_tc_data_tr: "             << ccindex_tc_data_tr.to_string()             << std::endl;
-        ss << std::left << std::setw(40) << "ccindex_tm_data_tr: "           << ccindex_tm_data_tr.to_string()           << std::endl;
-        ss << std::left << std::setw(40) << "ccindex_st_data_tr: "           << ccindex_st_data_tr.to_string()           << std::endl;
-        ss << std::left << std::setw(40) << "cloud_point_data_tr: "         << cloud_point_data_tr.to_string()         << std::endl;
-        ss << std::left << std::setw(40) << "sensor_cloud_link_tr: "         << sensor_cloud_link_tr.to_string()         << std::endl;
+        ss << std::left << std::setw(40) << "sensor_objs_data_tr: "        << sensor_objs_data_tr.to_string()        << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tc_data_tr: "         << ccindex_tc_data_tr.to_string()         << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_tr: "         << ccindex_tm_data_tr.to_string()         << std::endl;
+
+        // --- 以下为补充的缺失字段 ---
+        ss << std::left << std::setw(40) << "ccindex_tm_data_trajectories_tr: " << ccindex_tm_data_trajectories_tr.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_vehiclePass_tr: "  << ccindex_tm_data_vehiclePass_tr.to_string()  << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_queueUp_tr: "      << ccindex_tm_data_queueUp_tr.to_string()      << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_areaState_tr: "    << ccindex_tm_data_areaState_tr.to_string()    << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_overflow_tr: "     << ccindex_tm_data_overflow_tr.to_string()     << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_outlane_tr: "      << ccindex_tm_data_outlane_tr.to_string()      << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_statistics_tr: "   << ccindex_tm_data_statistics_tr.to_string()   << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_evaluations_tr: "  << ccindex_tm_data_evaluations_tr.to_string()  << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_nonmotor_tr: "     << ccindex_tm_data_nonmotor_tr.to_string()     << std::endl;
+        ss << std::left << std::setw(40) << "ccindex_tm_data_deviceStatus_tr: " << ccindex_tm_data_deviceStatus_tr.to_string() << std::endl;
+        // ----------------------------
+
+        ss << std::left << std::setw(40) << "ccindex_st_data_tr: "         << ccindex_st_data_tr.to_string()         << std::endl;
+        ss << std::left << std::setw(40) << "cloud_point_data_tr: "        << cloud_point_data_tr.to_string()        << std::endl;
+        ss << std::left << std::setw(40) << "sensor_cloud_link_tr: "       << sensor_cloud_link_tr.to_string()       << std::endl;
         ss << std::left << std::setw(40) << "ccindex_st_link_tr: "         << ccindex_st_link_tr.to_string()         << std::endl;
         ss << std::left << std::setw(40) << "ccindex_tm_link_tr: "         << ccindex_tm_link_tr.to_string()         << std::endl;
-        ss << std::left << std::setw(40) << "cloud_point_link_tr: "         << cloud_point_link_tr.to_string()         << std::endl;
+        ss << std::left << std::setw(40) << "cloud_point_link_tr: "        << cloud_point_link_tr.to_string()        << std::endl;
 
         return ss.str();
     }
@@ -659,6 +736,17 @@ struct ConfigerMec : public afl::base::SerializableData
     TrafficlightDetectDataConfiger     configerTrafficlightDetectData;
     SensorInterExterParamEnablePeriodConfiger configerSensorInterExterParamEnablePeriod;
     MonitorMecConfiger              configerMonitorMec;
+    MonitorMecConfigerSpatSrcData       configerMonitorMecSpatSrcData;
+    MonitorMecConfigerRsapData          configerMonitorMecRsapData;
+    MonitorMecConfigerRsapLink              configerMonitorMecRsapLink;
+    MonitorMecConfigerCcindexV2xData  configerMonitorMecCcindexV2xData;
+    MonitorMecConfigerCcindexTmData  configerMonitorMecTmData;
+    MonitorMecConfigerCcindexTmLink  configerMonitorMecTmLink;
+    MonitorMecConfigerCcindexStData  configerMonitorMecStData ;
+    MonitorMecConfigerCcindexStLink  configerMonitorMecStLink;
+    MonitorMecConfigerCcindexRcp configerMonitorMecRcp;
+    // MonitorMecConfigerCcindexRcpData  configerMonitorMecRcpData;
+    // MonitorMecConfigerCcindexRcpLink  configerMonitorMecRcpLink;
     MecSelfCheckTime                configerMecSelfCheckTime;
     MqttTopicConfigerCCIndex        configerMqttTopicCCIndex;
 
@@ -689,6 +777,20 @@ private:
         JsonSerialize(configerTrafficlightDetectData, "P_configerTrafficlightDetectData", j, false);
         JsonSerialize(configerSensorInterExterParamEnablePeriod, "Q_configerSensorInterExterParamEnablePeriod", j, false);
         JsonSerialize(configerMonitorMec, "R_configerMonitorMec", j, false);
+        JsonSerialize(configerMonitorMec, "R_configerMonitorMec", j, false);
+        JsonSerialize(configerMonitorMecSpatSrcData, "R_configerMonitorMecSpatSrcData", j, false);
+        JsonSerialize(configerMonitorMecRsapData, "R_configerMonitorMecRsapData", j, false);
+        JsonSerialize(configerMonitorMecRsapLink, "R_configerMonitorMecRsapLink", j, false);
+        JsonSerialize(configerMonitorMecCcindexV2xData, "R_configerMonitorMecCcindexV2xData", j, false);
+        JsonSerialize(configerMonitorMecTmData, "R_configerMonitorMecTmData", j, false);
+        JsonSerialize(configerMonitorMecTmLink, "R_configerMonitorMecTmLink", j, false);
+        JsonSerialize(configerMonitorMecStData, "R_configerMonitorMecStData", j, false);
+        JsonSerialize(configerMonitorMecStLink, "R_configerMonitorMecStLink", j, false);
+
+        JsonSerialize(configerMonitorMecRcp, "R_configerMonitorMecRcp", j, false);
+        // JsonSerialize(configerMonitorMecRcpData, "R_configerMonitorMecRcpData", j, false);
+        // JsonSerialize(configerMonitorMecRcpLink, "R_configerMonitorMecRcpLink", j, false);
+
         JsonSerialize(configerMecSelfCheckTime, "S_configerMecSelfCheckTime", j, false);
 
         JsonSerialize(configerMqttTopicCCIndex, "T_configerMqttTopicCCIndex", j, false);
@@ -722,6 +824,18 @@ private:
         JsonDeserialize(configerTrafficlightDetectData, "P_configerTrafficlightDetectData", j, noUse_isEmptyFlag);
         JsonDeserialize(configerSensorInterExterParamEnablePeriod, "Q_configerSensorInterExterParamEnablePeriod", j, noUse_isEmptyFlag);
         JsonDeserialize(configerMonitorMec, "R_configerMonitorMec", j, noUse_isEmptyFlag);
+        JsonDeserialize(configerMonitorMecSpatSrcData, "R_configerMonitorMecSpatSrcData", j, noUse_isEmptyFlag);
+        JsonDeserialize(configerMonitorMecRsapData, "R_configerMonitorMecRsapData", j, noUse_isEmptyFlag);
+        JsonDeserialize(configerMonitorMecRsapLink, "R_configerMonitorMecRsapLink", j, noUse_isEmptyFlag);
+        JsonDeserialize(configerMonitorMecCcindexV2xData, "R_configerMonitorMecCcindexV2xData", j, noUse_isEmptyFlag);
+        JsonDeserialize(configerMonitorMecTmData, "R_configerMonitorMecTmData", j, noUse_isEmptyFlag);
+        JsonDeserialize(configerMonitorMecTmLink, "R_configerMonitorMecTmLink", j, noUse_isEmptyFlag);
+        JsonDeserialize(configerMonitorMecStData, "R_configerMonitorMecStData", j, noUse_isEmptyFlag);
+        JsonDeserialize(configerMonitorMecStLink, "R_configerMonitorMecStLink", j, noUse_isEmptyFlag);
+
+        JsonDeserialize(configerMonitorMecRcp, "R_configerMonitorMecRcp", j, noUse_isEmptyFlag);
+        // JsonDeserialize(configerMonitorMecRcpData, "R_configerMonitorMecRcpData", j, noUse_isEmptyFlag);
+        // JsonDeserialize(configerMonitorMecRcpLink, "R_configerMonitorMecRcpLink", j, noUse_isEmptyFlag);
 
         JsonDeserialize(configerMecSelfCheckTime, "S_configerMecSelfCheckTime", j, noUse_isEmptyFlag);
 
@@ -772,7 +886,19 @@ public:
         ss << "----------------------------------------" << std::endl;
         ss << std::left << std::setw(40) << "configerSensorInterExterParamEnablePeriod: " << std::endl << std::left << configerSensorInterExterParamEnablePeriod.to_string() <<  std::endl;
         ss << "----------------------------------------" << std::endl;
-        ss << std::left << std::setw(40) << "configerMonitorMec: " << std::endl << std::left << configerMonitorMec.to_string() <<  std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMec: " << std::endl << std::left << configerMonitorMec.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMecSpatSrcData: " << std::endl << std::left << configerMonitorMecSpatSrcData.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMecRsapData: " << std::endl << std::left << configerMonitorMecRsapData.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMecRsapLink: " << std::endl << std::left << configerMonitorMecRsapLink.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMecCcindexV2xData: " << std::endl << std::left << configerMonitorMecCcindexV2xData.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMecTmData: " << std::endl << std::left << configerMonitorMecTmData.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMecTmLink: " << std::endl << std::left << configerMonitorMecTmLink.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMecStData: " << std::endl << std::left << configerMonitorMecStData.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMecStLink: " << std::endl << std::left << configerMonitorMecStLink.to_string() << std::endl;
+        // ss << std::left << std::setw(40) << "configerMonitorMecRcpData: " << std::endl << std::left << configerMonitorMecRcpData.to_string() << std::endl;
+        // ss << std::left << std::setw(40) << "configerMonitorMecRcpLink: " << std::endl << std::left << configerMonitorMecRcpLink.to_string() << std::endl;
+        ss << std::left << std::setw(40) << "configerMonitorMecRcp: " << std::endl << std::left << configerMonitorMecRcp.to_string() << std::endl;
+
         ss << "----------------------------------------" << std::endl;
         ss << std::left << std::setw(40) << "configerMqttTopicCCIndex: " << std::endl << std::left << configerMqttTopicCCIndex.to_string() <<  std::endl;
         ss << "----------------------------------------" << std::endl;
diff --git a/middleware/protocol/om/configer_om.h b/middleware/protocol/om/configer_om.h
index 059e7ee..fb87603 100644
--- a/middleware/protocol/om/configer_om.h
+++ b/middleware/protocol/om/configer_om.h
@@ -135,7 +135,7 @@ struct ConfigerOm : public ConfigerData<ConfigerOm>
         configerMqttCloud.mqttSendQos = 1;
         configerMqttCloud.mqttSubScribeQos = 1;
     }
-    bool                                enableDebugPrint = false;
+    bool                                enableDebugPrint = true;
     std::string                         rscuEsn;
     //////////////////////////////////////////////////
     MqttConfigerCommon                  configerMqtt;
diff --git a/middleware/protocol/om/data_model/mec/data_mec_self_check.h b/middleware/protocol/om/data_model/mec/data_mec_self_check.h
index 7e33d1f..5f884dc 100644
--- a/middleware/protocol/om/data_model/mec/data_mec_self_check.h
+++ b/middleware/protocol/om/data_model/mec/data_mec_self_check.h
@@ -36,15 +36,46 @@ struct TopicCCIndexTm
     std::string Topic_DeviceStatus = "deviceStatus/";// mec向平台发送 设备状态数据
     std::string Topic_Pulse         = "pulse/";   // mec向平台发送, 实时脉冲数据
 
-    std::string Topic_Static_Query_Ack = "query/ack/";
-    std::string Topic_Static_Update_Ack = "update/ack/";
+    /**
+ * @brief 将结构体内容转换为字符串用于打印日志
+ * @return 格式化后的字符串
+ */
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << "TopicCCIndexTm Configuration:\n"
+           << "  [Trajectories] : " << Topic_Trajectories << "\n"
+           << "  [VehiclePass]  : " << Topic_VehiclePass << "\n"
+           << "  [QueueUp]      : " << Topic_QueueUp << "\n"
+           << "  [AreaState]    : " << Topic_AreaState << "\n"
+           << "  [Overflow]     : " << Topic_Overflow << "\n"
+           << "  [Outlane]      : " << Topic_Outlane << "\n"
+           << "  [Statistics]   : " << Topic_Statistics << "\n"
+           << "  [Evaluations]  : " << Topic_Evaluations << "\n"
+           << "  [Nonmotor]     : " << Topic_Nonmotor << "\n"
+           << "  [DeviceStatus] : " << Topic_DeviceStatus << "\n"
+           << "  [Pulse]        : " << Topic_Pulse ;
+        return ss.str();
+    }
 };
 
 
 struct TopicCCIndexSt
 {
     std::string Topic_Static_Query_Ack = "query/ack/";
-    std::string Topic_Static_Update_Ack = "update/ack/";
+    std::string Topic_Static_Update = "update";
+    /**
+* @brief 将结构体内容转换为字符串用于打印日志
+* @return 格式化后的字符串
+*/
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << "TopicCCIndexTm Configuration:\n"
+           << "  [Static_Update_Ack] : " << Topic_Static_Query_Ack << "\n"
+           << "  [Static_Update]  : " << Topic_Static_Update ;
+        return ss.str();
+    }
 };
 
 NAMESPACE_ENDED_OM_COMPONENT_MEC
diff --git a/middleware/protocol/om/data_model/mec/data_monitor_mec.h b/middleware/protocol/om/data_model/mec/data_monitor_mec.h
index eed5dd3..7fedc92 100644
--- a/middleware/protocol/om/data_model/mec/data_monitor_mec.h
+++ b/middleware/protocol/om/data_model/mec/data_monitor_mec.h
@@ -19,29 +19,96 @@
 #ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_MONITOR_MEC_H
 #define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_MONITOR_MEC_H
 #include "middleware/protocol/om_common/data_model/data_common.h"
+#include "middleware/protocol/proto/monitor_mec.pb.h"
 NAMESPACE_START_OM_COMPONENT_MEC
 using namespace os::v2x::protocol::om::common;
-// 监控规则定义
-struct MonitorRule
-{
-    std::string name;                   // 规则名称/提示信息
-    MecAlarmTypeErrorCodeEnum alarmType;// 对应的告警类型
-    uint64_t timeoutMs;                 // 超时触发阈值 (毫秒)
-    uint64_t recoveryMs;                // 恢复所需持续时间 (毫秒)
-    bool isLinkCheck;                   // true: 链路状态检查(bool), false: 数据超时检查(时间戳)
-    bool enable;                        //是否检测
-    int selfCheckFaultType;             // [新增] 对应协议表37的故障类型ID (0表示不上报)
-};
 
 // 监控项的运行时状态
 struct MonitorState
 {
-    uint64_t lastReceivedTime = 0;      // 最后一次收到数据的时间
-    uint64_t recoveryStartTime = 0;     // 开始恢复的时间点 (用于判断持续时长)
-    uint64_t alarmOccurredTime = 0;     // 告警发生的时间 (用于消除告警时填入)
-    uint64_t lastAlarmPublishTime = 0;  // 上次告警上报的时间 (用于重复上报)
-    bool isAlarmActive = false;         // 当前是否处于告警状态
-    bool lastLinkStatus = true;         // 上一次的链路状态 (仅用于链路检查)
+    airos::monitor_mec::MonitorMecTag tag;
+    std::string     name;                   // 规则名称/提示信息
+    MecAlarmTypeErrorCodeEnum alarmType;    // 对应的告警类型
+    uint64_t    timeoutMs;                  // 超时触发阈值 (毫秒)
+    uint64_t    recoveryMs;                 // 恢复所需持续时间 (毫秒)
+    bool        isLinkCheck;                // true: 链路状态检查, false: 数据超时检查
+    bool        enable;                     // 是否检测
+    int         selfCheckFaultType;         // 故障类型ID
+
+    // 内部状态变量 (保持默认值)
+    uint64_t lastReceivedTime = 0;
+    uint64_t recoveryStartTime = 0;
+    uint64_t alarmOccurredTime = 0;
+    uint64_t lastAlarmPublishTime = 0;
+    bool isAlarmActive = false;
+    bool lastLinkStatus = true;
+
+    // ==========================================
+    // [修复] 必须添加默认构造函数
+    // ==========================================
+    MonitorState() = default;
+
+    // ==========================================
+    // [修复] 添加带参数的构造函数 (解决编译错误的关键)
+    // ==========================================
+    // ==========================================
+    // 包含所有参数的构造函数
+    // ==========================================
+    MonitorState(airos::monitor_mec::MonitorMecTag t,
+                 std::string n,
+                 MecAlarmTypeErrorCodeEnum at,
+                 uint64_t tm,
+                 uint64_t rm,
+                 bool ilc,
+                 bool en,
+                 int sft,
+                 // 下面是新增的内部状态参数
+                 uint64_t lrt,
+                 uint64_t rst,
+                 uint64_t aot,
+                 uint64_t lapt,
+                 bool iaa,
+                 bool lls)
+        : tag(t),
+          name(std::move(n)),
+          alarmType(at),
+          timeoutMs(tm),
+          recoveryMs(rm),
+          isLinkCheck(ilc),
+          enable(en),
+          selfCheckFaultType(sft),
+          // 使用传入的参数初始化内部状态
+          lastReceivedTime(lrt),
+          recoveryStartTime(rst),
+          alarmOccurredTime(aot),
+          lastAlarmPublishTime(lapt),
+          isAlarmActive(iaa),
+          lastLinkStatus(lls)
+    {
+    }
+
+
+    // to_string 保持不变
+    std::string to_string() const
+    {
+        std::ostringstream oss;
+        oss << "MonitorState {"
+            << "name: '" << name << "', "
+            << "alarmType: " << static_cast<int>(alarmType) << ", "
+            << "timeoutMs: " << timeoutMs << ", "
+            << "recoveryMs: " << recoveryMs << ", "
+            << "isLinkCheck: " << (isLinkCheck ? "true" : "false") << ", "
+            << "enable: " << (enable ? "true" : "false") << ", "
+            << "selfCheckFaultType: " << selfCheckFaultType << ", "
+            << "lastReceivedTime: " << lastReceivedTime << ", "
+            << "recoveryStartTime: " << recoveryStartTime << ", "
+            << "alarmOccurredTime: " << alarmOccurredTime << ", "
+            << "lastAlarmPublishTime: " << lastAlarmPublishTime << ", "
+            << "isAlarmActive: " << (isAlarmActive ? "true" : "false") << ", "
+            << "lastLinkStatus: " << (lastLinkStatus ? "true" : "false")
+            << "}";
+        return oss.str();
+    }
 };
 /**
  * @brief 故障类型定义
diff --git a/middleware/protocol/om/om_component.cc b/middleware/protocol/om/om_component.cc
index 34fc033..790748c 100644
--- a/middleware/protocol/om/om_component.cc
+++ b/middleware/protocol/om/om_component.cc
@@ -48,6 +48,8 @@ bool OM_COMPONENT::Init()
     m_TaskCloudPubFirst.reset(new std::thread([&](){ initEventLoopCloudPubFirst(); }));
     m_TaskCloudPubSecond.reset(new std::thread([&](){ initEventLoopCloudPubSecond(); }));
     m_TaskOmPing.reset(new std::thread([&](){ initEventLoopOmPing(); }));
+    m_TaskOmCheck.reset(new std::thread([&](){ initEventLoopOmCheck(); }));
+
     if(!getConfiger().configerMec.enableRegister)
     {
         m_RegisterFlag = true;
@@ -71,7 +73,18 @@ bool OM_COMPONENT::doBaiscWork()
         m_TaskProcessV2xData.reset(new std::thread([&](){ initProcessV2xData(); }));
         m_TaskProcessTrafficlightDetectData.reset(new std::thread([&](){ initProcessTrafficlightDetectData(); }));
   		m_TaskProcessTiming.reset(new std::thread([&](){ getTimingInfo(); }));;
-        m_TaskProcessMonitorMecData.reset(new std::thread([&](){ initProcessMonitorMecOutputResult(); }));
+
+        //mec自检
+        m_TaskProcessMecCheckSpatSrcData.reset(new std::thread([&](){ initProcessMecCheckSpatSrcData(); }));
+        m_TaskProcessMecCheckRsapData.reset(new std::thread([&](){ initProcessMecCheckRsapData(); }));
+        m_TaskProcessMecCheckRsapLink.reset(new std::thread([&](){ initProcessMecCheckRsapLink(); }));
+        m_TaskProcessMecCheckCcindexTcData.reset(new std::thread([&](){ initProcessMecCheckCcindexTcData(); }));
+        m_TaskProcessMecCheckCcindexTmData.reset(new std::thread([&](){ initProcessMecCheckCcindexTmData(); }));
+        m_TaskProcessMecCheckCcindexTmLink.reset(new std::thread([&](){ initProcessMecCheckCcindexTmLink(); }));
+        m_TaskProcessMecCheckCcindexStData.reset(new std::thread([&](){ initProcessMecCheckCcindexStData(); }));
+        m_TaskProcessMecCheckCcindexStLink.reset(new std::thread([&](){ initProcessMecCheckCcindexStLink(); }));
+        m_TaskProcessMecCheckCcindexCcindexRcpData.reset(new std::thread([&](){ initProcessMecCheckCcindexRcpData(); }));
+        m_TaskProcessMecCheckCcindexCcindexRcpLink.reset(new std::thread([&](){ initProcessMecCheckCcindexRcpLink(); }));
         m_TaskProcessMonitorSpatData.reset(new std::thread([&](){ initProcessMonitorSpat(); }));
     }
 
@@ -213,6 +226,12 @@ bool OM_COMPONENT::doBaiscWork()
 }
 
 
+bool OM_COMPONENT::initEventLoopOmCheck()
+{
+    m_EventloopOmCheck = std::make_shared<afl::net::EventLoop>();
+    m_EventloopOmCheck->loop();
+    return true;
+}
 bool OM_COMPONENT::initEventLoopOmPing()
 {
     m_EventloopOmPing = std::make_shared<afl::net::EventLoop>();
@@ -355,6 +374,10 @@ void  OM_COMPONENT::initProcessMonitorSpat()
 //信号机数据状态
 void OM_COMPONENT::processMonitorSpatDataStatus(const std::shared_ptr<const airos::monitor_mec::MonitorSpat> &recvData)
 {
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
+    }
     if (!getConfiger().configerMec.configerEnable.Enable_Spat_Data_Status)
     {
         return;
@@ -407,16 +430,22 @@ void OM_COMPONENT::processMonitorSpatDataStatus(const std::shared_ptr<const airo
 
     return;
 }
-//MEC自检
-void  OM_COMPONENT::initProcessMonitorMecOutputResult()
+
+//////////////////////////////////////////////////////////////////
+//MEC自检 - SpatSrcData
+void OM_COMPONENT::initProcessMecCheckSpatSrcData()
 {
     auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
-            m_MqttClientConfig.configerMec.configerMonitorMec.channel_readers,
-            std::bind(&OM_COMPONENT::processMonitorMecOutputResult, this, std::placeholders::_1));
+            m_MqttClientConfig.configerMec.configerMonitorMecSpatSrcData.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckSpatSrcData, this, std::placeholders::_1));
 }
-//MEC自检
-void OM_COMPONENT::processMonitorMecOutputResult(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+//MEC自检 - SpatSrcData
+void OM_COMPONENT::processMecCheckSpatSrcData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
 {
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
+    }
     if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
     {
         return;
@@ -433,7 +462,33 @@ void OM_COMPONENT::processMonitorMecOutputResult(const std::shared_ptr<const air
         {
             OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]has_md_spat_src_data" ;
         }
-        handleMonitorDataUpdate(mecDeviceData->md_spat_src_data().tag(), mecDeviceData->md_spat_src_data().timestamp());
+        handleMonitorDataUpdate(std::ref(m_MonitorStateSpatSrcData), mecDeviceData->md_spat_src_data().tag(), mecDeviceData->md_spat_src_data().timestamp());
+    }
+
+}
+
+//MEC自检 - RsapData
+void OM_COMPONENT::initProcessMecCheckRsapData()
+{
+    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
+            m_MqttClientConfig.configerMec.configerMonitorMecRsapData.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckRsapData, this, std::placeholders::_1));
+}
+//MEC自检 - RsapData
+void OM_COMPONENT::processMecCheckRsapData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
+    }
+    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+    {
+        return;
+    }
+    // 1. 处理数据类消息
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]" << mecDeviceData->DebugString() ;
     }
     //感知数据
     if (mecDeviceData->has_md_sensor_objs()  && getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.enable)
@@ -442,8 +497,69 @@ void OM_COMPONENT::processMonitorMecOutputResult(const std::shared_ptr<const air
         {
             OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]has_md_sensor_objs" ;
         }
-        handleMonitorDataUpdate(mecDeviceData->md_sensor_objs().tag(), mecDeviceData->md_sensor_objs().timestamp());
+        handleMonitorDataUpdate(std::ref(m_MonitorStateRsapData), mecDeviceData->md_sensor_objs().tag(), mecDeviceData->md_sensor_objs().timestamp());
+    }
+}
+
+//MEC自检 - RsapLink
+void OM_COMPONENT::initProcessMecCheckRsapLink()
+{
+    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
+            m_MqttClientConfig.configerMec.configerMonitorMecRsapLink.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckRsapLink, this, std::placeholders::_1));
+}
+//MEC自检 - RsapLink
+void OM_COMPONENT::processMecCheckRsapLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
+    }
+    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+    {
+        return;
+    }
+    // 1. 处理数据类消息
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]" << mecDeviceData->DebugString() ;
     }
+    // 2. 处理链路类消息
+    //云控链路状态
+    if (mecDeviceData->has_ml_rsap() && getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.enable)
+    {
+        if(getConfiger().enableDebugPrint)
+        {
+            OM_CHECK_LINK_DEBUG_PRINT << "[monitor_link]has_ml_rsap" ;
+        }
+        handleMonitorLinkUpdate(std::ref(m_MonitorStateRsapLink), mecDeviceData->ml_rsap().tag(), mecDeviceData->ml_rsap().con_flag(), mecDeviceData->ml_rsap().timestamp());
+    }
+}
+
+//MEC自检 - CcindexV2xData
+void OM_COMPONENT::initProcessMecCheckCcindexTcData()
+{
+    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
+            m_MqttClientConfig.configerMec.configerMonitorMecCcindexV2xData.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckCcindexTcData, this, std::placeholders::_1));
+}
+//MEC自检 - CcindexV2xData
+void OM_COMPONENT::processMecCheckCcindexTcData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
+    }
+    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+    {
+        return;
+    }
+    // 1. 处理数据类消息
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]" << mecDeviceData->DebugString() ;
+    }
+
     //v2x信控指标
     if (mecDeviceData->has_md_ccindex_tc()  && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.enable)
     {
@@ -451,7 +567,32 @@ void OM_COMPONENT::processMonitorMecOutputResult(const std::shared_ptr<const air
         {
             OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]has_md_ccindex_tc" ;
         }
-        handleMonitorDataUpdate(mecDeviceData->md_ccindex_tc().tag(), mecDeviceData->md_ccindex_tc().timestamp());
+        handleMonitorDataUpdate(std::ref(m_MonitorStateCcindexTcData), mecDeviceData->md_ccindex_tc().tag(), mecDeviceData->md_ccindex_tc().timestamp());
+    }
+}
+
+//MEC自检 - CcindexTmData
+void OM_COMPONENT::initProcessMecCheckCcindexTmData()
+{
+    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
+            m_MqttClientConfig.configerMec.configerMonitorMecTmData.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckCcindexTmData, this, std::placeholders::_1));
+}
+//MEC自检 - CcindexTmData
+void OM_COMPONENT::processMecCheckCcindexTmData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
+    }
+    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+    {
+        return;
+    }
+    // 1. 处理数据类消息
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]" << mecDeviceData->DebugString() ;
     }
     //雷达信控指标-动态
     if (mecDeviceData->has_md_ccindex_tm() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable)
@@ -460,37 +601,109 @@ void OM_COMPONENT::processMonitorMecOutputResult(const std::shared_ptr<const air
         {
             OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]has_md_ccindex_tm" ;
         }
-        handleMonitorDataUpdateCcindexTm(mecDeviceData->md_ccindex_tm().topic(), mecDeviceData->md_ccindex_tm().tag(), mecDeviceData->md_ccindex_tm().timestamp());
+        handleMonitorDataUpdateCcindexTm(mecDeviceData->md_ccindex_tm().device_id(),
+            mecDeviceData->md_ccindex_tm().topic(), mecDeviceData->md_ccindex_tm().tag(), mecDeviceData->md_ccindex_tm().timestamp());
     }
-    //雷达信控指标-静态
-    if (mecDeviceData->has_md_ccindex_st() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable)
+
+}
+
+//MEC自检 - CcindexTmLink
+void OM_COMPONENT::initProcessMecCheckCcindexTmLink()
+{
+    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
+            m_MqttClientConfig.configerMec.configerMonitorMecTmLink.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckCcindexTmLink, this, std::placeholders::_1));
+}
+//MEC自检 - CcindexTmLink
+void OM_COMPONENT::processMecCheckCcindexTmLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
     {
-        if(getConfiger().enableDebugPrint)
-        {
-            OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]has_md_ccindex_st" ;
-        }
-        handleMonitorDataUpdateCcindexSt(mecDeviceData->md_ccindex_st().topic(), mecDeviceData->md_ccindex_st().tag(), mecDeviceData->md_ccindex_st().timestamp());
+        return;
     }
-    //毫米波雷达原始数据
-    if (mecDeviceData->has_md_radar_cp() && getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable)
+    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+    {
+        return;
+    }
+    // 1. 处理数据类消息
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]" << mecDeviceData->DebugString() ;
+    }
+
+    //信控链路状态-动态
+    if (mecDeviceData->has_ml_ccindex_tm() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.enable)
     {
         if(getConfiger().enableDebugPrint)
         {
-            OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]has_md_radar_cp" ; ;
+            OM_CHECK_LINK_DEBUG_PRINT << "[monitor_link]has_ml_ccindex_tm" ;
         }
-        handleMonitorDataUpdate(mecDeviceData->md_radar_cp().tag(), mecDeviceData->md_radar_cp().timestamp());
+        handleMonitorLinkUpdate(std::ref(m_MonitorStateCcindexTmLink), mecDeviceData->ml_ccindex_tm().tag(), mecDeviceData->ml_ccindex_tm().con_flag(), mecDeviceData->ml_ccindex_tm().timestamp());
     }
+}
 
-    // 2. 处理链路类消息
-    //云控链路状态
-    if (mecDeviceData->has_ml_rsap() && getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.enable)
+//MEC自检 - CcindexStData
+void OM_COMPONENT::initProcessMecCheckCcindexStData()
+{
+    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
+            m_MqttClientConfig.configerMec.configerMonitorMecStData.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckCcindexStData, this, std::placeholders::_1));
+}
+//MEC自检 - CcindexStData
+void OM_COMPONENT::processMecCheckCcindexStData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
+    }
+    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+    {
+        return;
+    }
+    // 1. 处理数据类消息
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]" << mecDeviceData->DebugString() ;
+    }
+
+    //雷达信控指标-静态
+    if (mecDeviceData->has_md_ccindex_st() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable)
     {
         if(getConfiger().enableDebugPrint)
         {
-            OM_CHECK_LINK_DEBUG_PRINT << "[monitor_link]has_ml_rsap" ;
+            OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]has_md_ccindex_st" ;
         }
-        handleMonitorLinkUpdate(mecDeviceData->ml_rsap().tag(), mecDeviceData->ml_rsap().con_flag(), mecDeviceData->ml_rsap().timestamp());
+        handleMonitorDataUpdateCcindexSt(mecDeviceData->md_ccindex_st().device_id(),
+            mecDeviceData->md_ccindex_st().topic(), mecDeviceData->md_ccindex_st().tag(), mecDeviceData->md_ccindex_st().timestamp());
+    }
+
+
+}
+
+//MEC自检 - CcindexStLink
+void OM_COMPONENT::initProcessMecCheckCcindexStLink()
+{
+    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
+            m_MqttClientConfig.configerMec.configerMonitorMecStLink.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckCcindexStLink, this, std::placeholders::_1));
+}
+//MEC自检 - CcindexStLink
+void OM_COMPONENT::processMecCheckCcindexStLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
     }
+    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+    {
+        return;
+    }
+    // 1. 处理数据类消息
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]" << mecDeviceData->DebugString() ;
+    }
+
     //信控链路状态-静态
     if (mecDeviceData->has_ml_ccindex_st() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.enable)
     {
@@ -498,17 +711,70 @@ void OM_COMPONENT::processMonitorMecOutputResult(const std::shared_ptr<const air
         {
             OM_CHECK_LINK_DEBUG_PRINT << "[monitor_link]has_ml_ccindex_st" ;
         }
-        handleMonitorLinkUpdate(mecDeviceData->ml_ccindex_st().tag(), mecDeviceData->ml_ccindex_st().con_flag(), mecDeviceData->ml_ccindex_st().timestamp());
+        handleMonitorLinkUpdate(std::ref(m_MonitorStateCcindexStLink), mecDeviceData->ml_ccindex_st().tag(), mecDeviceData->ml_ccindex_st().con_flag(), mecDeviceData->ml_ccindex_st().timestamp());
     }
-    //信控链路状态-动态
-    if (mecDeviceData->has_ml_ccindex_tm() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.enable)
+
+}
+
+//MEC自检 - CcindexRcpData
+void OM_COMPONENT::initProcessMecCheckCcindexRcpData()
+{
+    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
+            m_MqttClientConfig.configerMec.configerMonitorMecRcp.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckCcindexRcpData, this, std::placeholders::_1));
+}
+//MEC自检 - CcindexRcpData
+void OM_COMPONENT::processMecCheckCcindexRcpData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
+    }
+    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+    {
+        return;
+    }
+    // 1. 处理数据类消息
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]" << mecDeviceData->DebugString() ;
+    }
+    //毫米波雷达原始数据
+    if (mecDeviceData->has_md_radar_cp() && getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable)
     {
         if(getConfiger().enableDebugPrint)
         {
-            OM_CHECK_LINK_DEBUG_PRINT << "[monitor_link]has_ml_ccindex_tm" ;
+            OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]has_md_radar_cp" ; ;
         }
-        handleMonitorLinkUpdate(mecDeviceData->ml_ccindex_tm().tag(), mecDeviceData->ml_ccindex_tm().con_flag(), mecDeviceData->ml_ccindex_tm().timestamp());
+        // handleMonitorDataUpdate(std::ref(m_MonitorStateCcindexRcp), mecDeviceData->md_radar_cp().tag(), mecDeviceData->md_radar_cp().timestamp());
+    }
+
+}
+
+//MEC自检 - CcindexRcpData
+void OM_COMPONENT::initProcessMecCheckCcindexRcpLink()
+{
+    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
+            m_MqttClientConfig.configerMec.configerMonitorMecRcp.channel_readers,
+            std::bind(&OM_COMPONENT::processMecCheckCcindexRcpLink, this, std::placeholders::_1));
+}
+//MEC自检 - CcindexRcpData
+void OM_COMPONENT::processMecCheckCcindexRcpLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
     }
+    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+    {
+        return;
+    }
+    // 1. 处理数据类消息
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data]" << mecDeviceData->DebugString() ;
+    }
+
     //点云链路状态
     if (mecDeviceData->has_ml_radar_cp() && getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.enable)
     {
@@ -516,34 +782,29 @@ void OM_COMPONENT::processMonitorMecOutputResult(const std::shared_ptr<const air
         {
             OM_CHECK_LINK_DEBUG_PRINT << "[monitor_link]has_ml_radar_cp" ;
         }
-        handleMonitorLinkUpdate(mecDeviceData->ml_radar_cp().tag(), mecDeviceData->ml_radar_cp().con_flag(), mecDeviceData->ml_radar_cp().timestamp());
+        // handleMonitorLinkUpdate(std::ref(m_MonitorStateCcindexRcp), mecDeviceData->ml_radar_cp().tag(), mecDeviceData->ml_radar_cp().con_flag(), mecDeviceData->ml_radar_cp().timestamp());
     }
+
 }
 
 //mec自检：处理数据更新逻辑
-void OM_COMPONENT::handleMonitorDataUpdate(int tag, uint64_t timestamp)
+void OM_COMPONENT::handleMonitorDataUpdate(MonitorState &state, int tag, uint64_t timestamp)
 {
     if(getConfiger().enableDebugPrint)
     {
-        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data][tag]" << tag;
-        for (auto pair: m_MonitorRulesData)
-        {
-            OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data][m_MonitorRulesData]tag:" << pair.first;
-        }
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data][tag]" << tag << "[state]" << state.to_string();
     }
 
-    if (m_MonitorRulesData.find(tag) == m_MonitorRulesData.end())
+    if (tag != state.tag)
     {
         OM_CHECK_DATA_ERROR_PRINT << "[monitor_data]m_MonitorRulesData.find(tag) == m_MonitorRulesData.end()" ;
         return;
     }
 
-    auto& rule = m_MonitorRulesData[tag];
-    if (!rule.enable)
+    if (!state.enable)
     {
         return;
     }
-    auto& state = m_MonitorStatesData[tag];
     uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
 
     // 1. 喂狗：更新最后收到时间
@@ -566,53 +827,63 @@ void OM_COMPONENT::handleMonitorDataUpdate(int tag, uint64_t timestamp)
         else
         {
             // 检查数据是否持续了足够长的时间
-            if (now - state.recoveryStartTime >= rule.recoveryMs)
+            if (now - state.recoveryStartTime >= state.recoveryMs)
             {
                 // 满足恢复条件 -> 消除告警
-                if (tag == MONITOR_TAG_MONITOR_IN_SPAT_SRC_DATA)
+                if (tag == MONITOR_TAG_DATA_SPAT_SRC)
                 {
                     m_SpatDataStatusFromTLDeviceFlag.store(true, std::memory_order_relaxed);
                 }
-                alarmDisappeared(rule.name, rule.alarmType, getConfiger().rscuEsn, rule.name, state.alarmOccurredTime);
+                alarmDisappeared(state.name, state.alarmType, getConfiger().rscuEsn, state.name, state.alarmOccurredTime);
                 state.isAlarmActive = false;
                 state.recoveryStartTime = 0;
                 state.lastAlarmPublishTime = 0;  // 重置告警上报时间
                 if(getConfiger().enableDebugPrint)
                 {
-                    OM_CHECK_DATA_WARN_PRINT << "[monitor_data] RECOVERED: " << rule.name;
+                    OM_CHECK_DATA_WARN_PRINT << "[monitor_data] RECOVERED: " << state.name;
                 }
             }
         }
     }
 }
-
+//////////////////////////
+//
 //雷达信控指标-动态
-void OM_COMPONENT::handleMonitorDataUpdateCcindexTm(const std::string& topic, int tag, uint64_t timestamp)
+void OM_COMPONENT::handleMonitorDataUpdateCcindexTm(const std::string & deviceId, const std::string& topic, int tag, uint64_t timestamp)
 {
     if(getConfiger().enableDebugPrint)
     {
-        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data_ccindex_tm][tag]" << tag << "[topic]" << topic;
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data_ccindex_tm][tag]" << tag << "[topic]" << topic << "[device_id]" << deviceId;
     }
-    if (m_MonitorRulesCcindexTm.find(tag) == m_MonitorRulesCcindexTm.end())
+    for (auto& pair : m_MonitorStatesCcindexTmData)
     {
-        OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_tm]m_MonitorRulesCcindexTm.find(tag) == m_MonitorRulesCcindexTm.end()";
-        return;
-    }
 
-    auto& rule = m_MonitorRulesCcindexTm[tag];
-    if (!rule.enable)
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data_ccindex_tm][id]" << pair.first ;
+    }
+    // 使用 find 查找
+    auto it = m_MonitorStatesCcindexTmData.find(deviceId);
+    // 判断是否找到
+    if (it == m_MonitorStatesCcindexTmData.end())
     {
+        OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_tm]m_MonitorRulesCcindexTm.find(deviceid) == m_MonitorRulesCcindexTm.end()";
         return;
     }
 
+    auto& devices = m_MonitorStatesCcindexTmData[deviceId];
+
     // 检查该topic是否在监控状态表中
-    if (m_MonitorStatesCcindexTm[tag].find(topic) == m_MonitorStatesCcindexTm[tag].end())
+    if (devices.find(topic) == devices.end())
     {
         OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_tm]topic not found in m_MonitorStatesCcindexTm: " << topic;
         return;
     }
 
-    auto& state = m_MonitorStatesCcindexTm[tag][topic];
+    auto& state = m_MonitorStatesCcindexTmData[deviceId][topic];
+    if (! state.enable)
+    {
+        return;
+    }
+
     uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
 
     // 1. 喂狗：更新最后收到时间
@@ -635,51 +906,62 @@ void OM_COMPONENT::handleMonitorDataUpdateCcindexTm(const std::string& topic, in
         else
         {
             // 检查数据是否持续了足够长的时间
-            if (now - state.recoveryStartTime >= rule.recoveryMs)
+            if (now - state.recoveryStartTime >=  state.recoveryMs)
             {
                 // 满足恢复条件 -> 消除告警
-                std::string errorDetail = rule.name + " [topic: " + topic + "]";
-                alarmDisappeared(errorDetail, rule.alarmType, getConfiger().rscuEsn, topic, state.alarmOccurredTime);
+                std::string errorDetail = state.name + " [topic: " + topic + "]";
+                alarmDisappeared(errorDetail, state.alarmType, getConfiger().rscuEsn, topic, state.alarmOccurredTime);
                 state.isAlarmActive = false;
                 state.recoveryStartTime = 0;
                 state.lastAlarmPublishTime = 0;  // 重置告警上报时间
                 if(getConfiger().enableDebugPrint)
                 {
-                     OM_CHECK_DATA_WARN_PRINT << "[monitor_data_ccindex_tm] RECOVERED: " << rule.name << " [topic: " << topic << "]";
+                     OM_CHECK_DATA_WARN_PRINT << "[monitor_data_ccindex_tm] RECOVERED: " << state.name << " [topic: " << topic << "]";
                  }
             }
         }
     }
 }
-
-//雷达信控指标-静态
-void OM_COMPONENT::handleMonitorDataUpdateCcindexSt(const std::string& topic, int tag, uint64_t timestamp)
+//////////////////////////////////////////////
+//【数据更新】雷达信控指标-静态
+void OM_COMPONENT::handleMonitorDataUpdateCcindexSt(const std::string & deviceId, const std::string& topic, int tag, uint64_t timestamp)
 {
     if(getConfiger().enableDebugPrint)
     {
-        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data_ccindex_st][tag]" << tag << "[topic]" << topic;
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data_ccindex_st][device_id]" << deviceId  << "[tag]" << tag << "[topic]" << topic;
     }
 
-    if (m_MonitorRulesCcindexSt.find(tag) == m_MonitorRulesCcindexSt.end())
+    // 使用 find 查找
+    auto it = m_MonitorStatesCcindexStData.find(deviceId);
+    for (auto& pair:m_MonitorStatesCcindexStData)
     {
-        OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_st]m_MonitorRulesCcindexTm.find(tag) == m_MonitorRulesCcindexTm.end()";
-        return;
+        OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_st][id]" << pair.first;
     }
-
-    auto& rule = m_MonitorRulesCcindexSt[tag];
-    if (!rule.enable)
+    for (auto& pair: m_MonitorStatesCcindexStData[deviceId])
     {
+        OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_st][topic]" << pair.first;
+    }
+    // 判断是否找到
+    if (it == m_MonitorStatesCcindexStData.end())
+    {
+        OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_st]m_MonitorRulesCcindexTm.find(deviceid) == m_MonitorRulesCcindexTm.end()";
         return;
     }
 
+    auto& devices = m_MonitorStatesCcindexStData[deviceId];
+
     // 检查该topic是否在监控状态表中
-    if (m_MonitorStatesCcindexSt[tag].find(topic) == m_MonitorStatesCcindexSt[tag].end())
+    if (devices.find(topic) == devices.end())
     {
         OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_st]topic not found in m_MonitorStatesCcindexTm: " << topic;
         return;
     }
 
-    auto& state = m_MonitorStatesCcindexSt[tag][topic];
+    auto& state = m_MonitorStatesCcindexStData[deviceId][topic];
+    if (! state.enable)
+    {
+        return;
+    }
     uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
 
     // 1. 喂狗：更新最后收到时间
@@ -705,20 +987,20 @@ void OM_COMPONENT::handleMonitorDataUpdateCcindexSt(const std::string& topic, in
             if (getConfiger().enableDebugPrint)
             {
                 OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data_ccindex_st][recovery-start] state.recoveryStartTime != 0  [now - state.recoveryStartTime ] "
-                    << (now - state.recoveryStartTime ) << rule.recoveryMs;
+                    << (now - state.recoveryStartTime ) << state.recoveryMs;
             }
             // 检查数据是否持续了足够长的时间
-            if (now - state.recoveryStartTime >= rule.recoveryMs)
+            if (now - state.recoveryStartTime >= state.recoveryMs)
             {
                 // 满足恢复条件 -> 消除告警
-                std::string errorDetail = rule.name + " [topic: " + topic + "]";
-                alarmDisappeared(errorDetail, rule.alarmType, getConfiger().rscuEsn, topic, state.alarmOccurredTime);
+                std::string errorDetail = state.name + " [topic: " + topic + "]";
+                alarmDisappeared(errorDetail, state.alarmType, getConfiger().rscuEsn, topic, state.alarmOccurredTime);
                 state.isAlarmActive = false;
                 state.recoveryStartTime = 0;
                 state.lastAlarmPublishTime = 0;  // 重置告警上报时间
                 if(getConfiger().enableDebugPrint)
                 {
-                    OM_CHECK_DATA_WARN_PRINT << "[monitor_data_ccindex_st] RECOVERED: " << rule.name << " [topic: " << topic << "]";
+                    OM_CHECK_DATA_WARN_PRINT << "[monitor_data_ccindex_st] RECOVERED: " << state.name << " [topic: " << topic << "]";
                 }
             }
         }
@@ -728,32 +1010,26 @@ void OM_COMPONENT::handleMonitorDataUpdateCcindexSt(const std::string& topic, in
         OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data_ccindex_st] !state.isAlarmActive";
     }
 }
+//////////////////////////////////////////////////////////////
+////////////////////////////////////////////////////////////////
 //MEC自检结果 ：监控链路状态
-void OM_COMPONENT::handleMonitorLinkUpdate(int tag, bool status, uint64_t timestamp)
+void OM_COMPONENT::handleMonitorLinkUpdate(MonitorState &state, int tag, bool status, uint64_t timestamp)
 {
     if(getConfiger().enableDebugPrint)
     {
-        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data][tag]" << tag;
-        for (auto pair: m_MonitorRulesLink)
-        {
-            OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data][m_MonitorRules]tag:" << pair.first;
-        }
-
+        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_data][tag]" << tag << "[state]" << state.to_string();
     }
 
-
-    if (m_MonitorRulesLink.find(tag) == m_MonitorRulesLink.end())
+    if (tag != state.tag)
     {
-        OM_CHECK_LINK_ERROR_PRINT << "[monitor_link]m_MonitorRulesLink.find(tag) == m_MonitorRulesLink.end()" ;
+        OM_CHECK_DATA_ERROR_PRINT << "[monitor_data]m_MonitorRulesData.find(tag) == m_MonitorRulesData.end()" ;
         return;
     }
 
-    auto& rule = m_MonitorRulesLink[tag];
-    if (!rule.enable)
+    if (!state.enable)
     {
         return;
     }
-    auto& state = m_MonitorStatesDataLink[tag];
     uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
     // 1. 喂狗：只要收到消息，说明模块在线，更新心跳时间
     state.lastReceivedTime = now;
@@ -773,7 +1049,7 @@ void OM_COMPONENT::handleMonitorLinkUpdate(int tag, bool status, uint64_t timest
             state.isAlarmActive = true;
             state.alarmOccurredTime = now;
             state.lastAlarmPublishTime = now;  // 记录首次告警上 报时间
-            alarmOccurred(rule.name + " [link down]", rule.alarmType, getConfiger().rscuEsn, rule.name,
+            alarmOccurred(state.name + " [link down]", state.alarmType, getConfiger().rscuEsn, state.name,
                 "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime );
 
         }
@@ -787,22 +1063,22 @@ void OM_COMPONENT::handleMonitorLinkUpdate(int tag, bool status, uint64_t timest
                 if (getConfiger().enableDebugPrint)
                 {
                     OM_CHECK_LINK_WARN_PRINT << "[monitor_link][link-alarm-repeat][tag]" << tag
-                                       << "[name]" << rule.name
+                                       << "[name]" << state.name
                                        << "[timeDiff(ms)]" << alarmPublishTimeDiff
-                                       << "[interval(ms)]" << rule.timeoutMs;
+                                       << "[interval(ms)]" << state.timeoutMs;
                 }
 
                 // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
-                if (alarmPublishTimeDiff >= rule.timeoutMs)
+                if (alarmPublishTimeDiff >= state.timeoutMs)
                 {
                     state.lastAlarmPublishTime = now;  // 更新上次上报时间
-                    alarmOccurred(rule.name + " [link down]", rule.alarmType, getConfiger().rscuEsn, rule.name,
+                    alarmOccurred(state.name + " [link down]", state.alarmType, getConfiger().rscuEsn, state.name,
                         "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
                     if(getConfiger().enableDebugPrint)
                     {
-                        OM_CHECK_LINK_WARN_PRINT << "[monitor_link] LINK ALARM REPEATED: " << rule.name
+                        OM_CHECK_LINK_WARN_PRINT << "[monitor_link] LINK ALARM REPEATED: " << state.name
                                           << " (repeat interval: "
-                                          << rule.timeoutMs << "ms)";
+                                          << state.timeoutMs << "ms)";
                     }
                 }
             }
@@ -814,13 +1090,13 @@ void OM_COMPONENT::handleMonitorLinkUpdate(int tag, bool status, uint64_t timest
         if (state.isAlarmActive) {
             // 链路恢复通常不需要持续时间，收到 true 即认为恢复
             // 如果需要持续时间，可以在这里加类似 handleMonitorDataUpdate 的逻辑
-            alarmDisappeared(rule.name, rule.alarmType, getConfiger().rscuEsn, rule.name, state.alarmOccurredTime);
+            alarmDisappeared(state.name, state.alarmType, getConfiger().rscuEsn, state.name, state.alarmOccurredTime);
             state.isAlarmActive = false;
             state.recoveryStartTime = 0;
             state.lastAlarmPublishTime = 0;  // 重置告警上报时间
             if(getConfiger().enableDebugPrint)
             {
-                OM_CHECK_LINK_SUCCESS_PRINT << "[monitor_link] LINK RECOVERED: " << rule.name;
+                OM_CHECK_LINK_SUCCESS_PRINT << "[monitor_link] LINK RECOVERED: " << state.name;
             }
         }
     }
@@ -831,353 +1107,665 @@ void OM_COMPONENT::handleMonitorLinkUpdate(int tag, bool status, uint64_t timest
 //MEC自检结果：初始化监控规则
 void OM_COMPONENT::initMonitorRules()
 {
-
-
-    // 1. 数据异常规则 (超时触发，持续恢复)
+    // 1. 信号机原始数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.enable)
     {
-        m_MonitorRulesData[MONITOR_TAG_MONITOR_IN_SPAT_SRC_DATA] =
-            {"[data ] spat src ", MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS,
+        m_MonitorStateSpatSrcData =os::v2x::protocol::om::mec::MonitorState
+        {
+            MONITOR_TAG_DATA_SPAT_SRC,
+            "[data ] spat src ",
+            MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS,
             getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.recoverySecond * m_Second2MSUnit,
-            false, getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.enable,
-            OmFaultType::OFT_SPAT_SRC_DATA
-            };
+            false,
+            getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.enable,
+            OmFaultType::OFT_SPAT_SRC_DATA,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
+        };
     }
-
+    //感知数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.enable)
     {
-        m_MonitorRulesData[MONITOR_TAG_MONITOR_IN_SENSOR_OBJ_DATA] =
-            {"[data ] sensor obj ", MEC_ALARM_TYPE_MONITOR_SENSOR_OBJ_DATA_LOSS,
+        m_MonitorStateRsapData =os::v2x::protocol::om::mec::MonitorState
+        {
+            MONITOR_TAG_DATA_SENSOR_OBJ,
+            "[data ] sensor obj ", MEC_ALARM_TYPE_MONITOR_SENSOR_OBJ_DATA_LOSS,
             getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.enable,
-            OmFaultType::OFT_SENSOR_OBJS_DATA
-            };
+            OmFaultType::OFT_SENSOR_OBJS_DATA,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
+        };
     }
+    //v2x信控数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.enable)
     {
-        m_MonitorRulesData[MONITOR_TAG_MONITOR_IN_CCINDEX_TC] =
-            {"[data ] cc tc ", MEC_ALARM_TYPE_MONITOR_V2X_CC_DATA_LOSS,
+        m_MonitorStateCcindexTcData =os::v2x::protocol::om::mec::MonitorState
+        {
+            MONITOR_TAG_DATA_CCINDEX_TC,
+            "[data ] cc tc ", MEC_ALARM_TYPE_MONITOR_V2X_CC_DATA_LOSS,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.timeoutSecond * m_Second2MSUnit,
-           getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.recoverySecond * m_Second2MSUnit,
+            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.enable,
-            OmFaultType::OFT_CCINDEX_TC_DATA
-            };
+            OmFaultType::OFT_CCINDEX_TC_DATA,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
+        };
     }
 
+    //雷达动态数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable)
     {
-        m_MonitorRulesCcindexTm[MONITOR_TAG_MONITOR_IN_CCINDEX_TM] =
-            {"[data ]cc dynamic ", MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
-            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.timeoutSecond * m_Second2MSUnit,
-            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.recoverySecond * m_Second2MSUnit,
-            false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable,
-            OmFaultType::OFT_CCINDEX_TM_DATA
-            };
-    }
+        for (auto pair:m_MecSelfCheckTmDeviceIdTopicMap)
+        {
+
+            std::string id = pair.first;
+            OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_tm]id: " << id  ;
+            TopicCCIndexTm topicCCIndexTm = pair.second;
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.enable)
+            {
+                //实时轨迹数据
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Trajectories] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_TRAJECTORIES,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+            //实时过车数
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable)
+            {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_VehiclePass] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_VEHICLEPASS,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+            //实时排队数
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.enable)
+            {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_QueueUp] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_QUEUEUP,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+            //实时区域状态数据
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.enable)
+            {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_AreaState] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_AREASTATE,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+
+            //实时溢出数据
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.enable)
+            {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Overflow] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_OVERFLOW,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+            //实时出口通道数据
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.enable)
+            {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Outlane] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_OUTLANE,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+
+            //    定时统计数据
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.enable)
+            {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Statistics] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_STATISTICS,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+
+            // 定时评价数据
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.enable)
+            {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Evaluations] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_EVALUATIONS,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+            //定时行人及非机动车数据
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.enable)
+            {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Nonmotor] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_NONMOTOR,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+            //设备状态数据
+            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable)
+            {
+                m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_DeviceStatus] =os::v2x::protocol::om::mec::MonitorState
+                {
+                    MONITOR_TAG_DATA_CCINDEX_TM_DEVICESTATUS,
+                    "[data ]cc dynamic ",
+                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.timeoutSecond * m_Second2MSUnit,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.recoverySecond * m_Second2MSUnit,
+                    false,
+                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable,
+                    OmFaultType::OFT_CCINDEX_TM_DATA,
+                0,
+                0,
+                0,
+                0,
+                false,
+                true
+                };
+            }
+            // //实时脉冲数据
+            // m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Pulse] = {
+            //     "[data ]cc dynamic ",
+            //     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
+            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.timeoutSecond * m_Second2MSUnit,
+            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.recoverySecond * m_Second2MSUnit,
+            //     false,
+            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable,
+            //     OmFaultType::OFT_CCINDEX_TM_DATA
+            // };
+
+        }
+    }
+    //静态数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable)
     {
-        m_MonitorRulesCcindexSt[MONITOR_TAG_MONITOR_IN_CCINDEX_ST] =
-            {"[data ] cc static ", MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS,
-            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.timeoutSecond * m_Second2MSUnit,
-            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.recoverySecond * m_Second2MSUnit,
-            false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable,
-            OmFaultType::OFT_CCINDEX_ST_DATA
+
+        for (auto pair:m_MecSelfCheckStDeviceIdTopicMap)
+        {
+            std::string id = pair.first;
+
+            TopicCCIndexSt topicCCIndexSt = pair.second;
+            OM_CHECK_DATA_ERROR_PRINT << "[monitor_data_ccindex_st]id: " << id << "[topicCCIndexSt]" << topicCCIndexSt.to_string() ;
+            m_MonitorStatesCcindexStData[id][topicCCIndexSt.Topic_Static_Query_Ack] =os::v2x::protocol::om::mec::MonitorState
+            {
+                MONITOR_TAG_DATA_CCINDEX_ST_QUERY_ACK,
+                "[data ] cc static ",
+                MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS,
+                getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.timeoutSecond * m_Second2MSUnit,
+                getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.recoverySecond * m_Second2MSUnit,
+                false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable,
+                OmFaultType::OFT_CCINDEX_ST_DATA,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
+            };
+            m_MonitorStatesCcindexStData[id][topicCCIndexSt.Topic_Static_Update] =os::v2x::protocol::om::mec::MonitorState
+            {
+                MONITOR_TAG_DATA_CCINDEX_ST_UPDATE,
+                "[data ] cc static ", MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS,
+                getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.timeoutSecond * m_Second2MSUnit,
+                getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.recoverySecond * m_Second2MSUnit,
+                false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable,
+                OmFaultType::OFT_CCINDEX_ST_DATA,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
             };
+        }
     }
+    //点云数据
     if(getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable)
     {
-        m_MonitorRulesData[MONITOR_TAG_MONITOR_IN_RADAR_DATA] =
-            {"[data ] radar raw ", MEC_ALARM_TYPE_MONITOR_RADAR_RAW_DATA_LOSS,
+        m_MonitorStateCcindexRcpData =os::v2x::protocol::om::mec::MonitorState
+        {
+            MONITOR_TAG_DATA_RADAR_DATA,
+            "[data ] radar raw ", MEC_ALARM_TYPE_MONITOR_RADAR_RAW_DATA_LOSS,
             getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable,
-            OmFaultType::OFT_RADAR_RAW_DATA
-            };
+            OmFaultType::OFT_RADAR_RAW_DATA,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
+        };
     }
 
     // 2. 链路故障规则 (状态触发，状态恢复)
+    //感知数据上云：链路
     if(getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.enable)
     {
-        m_MonitorRulesLink[MONITOR_TAG_MONITOR_OUT_SENSOR_CLOUD] =
-            {"[link ] sensor cloud ", MEC_ALARM_TYPE_MONITOR_CLOUD_LINK_ERROR,
+        m_MonitorStateRsapLink =os::v2x::protocol::om::mec::MonitorState
+        {
+            MONITOR_TAG_LINK_RSAP,
+            "[link ] sensor cloud ", MEC_ALARM_TYPE_MONITOR_CLOUD_LINK_ERROR,
             getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.enable,
-            OmFaultType::OFT_SENSOR_CLOUD_LINK_STATUS
-            };
+            OmFaultType::OFT_SENSOR_CLOUD_LINK_STATUS,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
+        };
     }
+    //雷达信控指标：静态链路
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.enable)
     {
-        m_MonitorRulesLink[MONITOR_TAG_MONITOR_OUT_CCINDEX_ST] =
-            {"[link ] cc static ", MEC_ALARM_TYPE_MONITOR_CC_STAT_LINK_ERROR,
+       m_MonitorStateCcindexStLink =os::v2x::protocol::om::mec::MonitorState
+        {
+           MONITOR_TAG_LINK_CCINDEX_ST,
+            "[link ] cc static ", MEC_ALARM_TYPE_MONITOR_CC_STAT_LINK_ERROR,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.enable,
-            OmFaultType::OFT_CCINDEX_ST_LINK_STATUS
-            };
+            OmFaultType::OFT_CCINDEX_ST_LINK_STATUS,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
+        };
     }
+        //动态数据：链路
     if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.enable)
     {
-        m_MonitorRulesLink[MONITOR_TAG_MONITOR_OUT_CCINDEX_TM] =
-            {"[link ] cc dynamic ", MEC_ALARM_TYPE_MONITOR_CC_DYN_LINK_ERROR,
+        m_MonitorStateCcindexTmLink =os::v2x::protocol::om::mec::MonitorState
+        {
+            MONITOR_TAG_LINK_CCINDEX_TM,
+            "[link ] cc dynamic ", MEC_ALARM_TYPE_MONITOR_CC_DYN_LINK_ERROR,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.enable,
-            OmFaultType::OFT_CCINDEX_TM_LINK_STATUS
-            };
+            OmFaultType::OFT_CCINDEX_TM_LINK_STATUS,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
+        };
     }
     if(getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.enable)
     {
-        m_MonitorRulesLink[MONITOR_TAG_MONITOR_OUT_RADAR_POINT_CLOUD] =
-            {"[link ] point cloud", MEC_ALARM_TYPE_MONITOR_POINT_CLOUD_LINK_ERROR,
+        m_MonitorStateCcindexRcpLink =os::v2x::protocol::om::mec::MonitorState
+        {
+            MONITOR_TAG_LINK_DAR,
+            "[link ] point cloud", MEC_ALARM_TYPE_MONITOR_POINT_CLOUD_LINK_ERROR,
             getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.timeoutSecond * m_Second2MSUnit,
             getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.recoverySecond * m_Second2MSUnit,
             false, getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.enable,
-            OmFaultType::OFT_CLOUD_POINT_LINK_STATUS
+            OmFaultType::OFT_CLOUD_POINT_LINK_STATUS,
+            0,
+            0,
+            0,
+            0,
+            false,
+            true
             };
     }
-    // 初始化状态，将最后接收时间设为当前，避免启动即告警
-    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
-    for(auto& rule : m_MonitorRulesData)
-    {
-        m_MonitorStatesData[rule.first].lastReceivedTime = now;
-        m_MonitorStatesData[rule.first].isAlarmActive = false;
-        m_MonitorStatesData[rule.first].lastLinkStatus = true;
-    }
-    for(auto& rule : m_MonitorRulesLink)
-    {
-        m_MonitorStatesDataLink[rule.first].lastReceivedTime = now;
-        m_MonitorStatesDataLink[rule.first].isAlarmActive = false;
-        m_MonitorStatesDataLink[rule.first].lastLinkStatus = true;
-    }
-
-    // 初始化多雷达动态数据的监控状态
-    for (auto& pair : m_MecSelfCheckTmDataMap)
-    {
-        const std::string& radarID = pair.first;
-        const TopicCCIndexTm& topicCCIndexTm = pair.second;
-        
-        // 为每个雷达的每个动态topic初始化监控状态
-        std::vector<std::string> dynamicTopics = {
-            topicCCIndexTm.Topic_Trajectories,
-            topicCCIndexTm.Topic_VehiclePass,
-            topicCCIndexTm.Topic_QueueUp,
-            topicCCIndexTm.Topic_AreaState,
-            topicCCIndexTm.Topic_Overflow,
-            topicCCIndexTm.Topic_Outlane,
-            topicCCIndexTm.Topic_Statistics,
-            topicCCIndexTm.Topic_Evaluations,
-            topicCCIndexTm.Topic_Nonmotor,
-            topicCCIndexTm.Topic_DeviceStatus,
-            topicCCIndexTm.Topic_Pulse
-        };
-        
-        for (const auto& topic : dynamicTopics)
-        {
-            if (!topic.empty())
-            {
-                m_MonitorStatesCcindexTm[MONITOR_TAG_MONITOR_IN_CCINDEX_TM][topic].lastReceivedTime = now;
-                m_MonitorStatesCcindexTm[MONITOR_TAG_MONITOR_IN_CCINDEX_TM][topic].isAlarmActive = false;
-                m_MonitorStatesCcindexTm[MONITOR_TAG_MONITOR_IN_CCINDEX_TM][topic].lastLinkStatus = true;
-            }
-        }
-    }
-    // 初始化多雷达静态数据的监控状态
-    for (auto& pair : m_MecSelfCheckStDataMap)
-    {
-        const std::string& radarID = pair.first;
-        const TopicCCIndexSt& topicCCIndexSt = pair.second;
 
-        // 为每个雷达的每个动态topic初始化监控状态
-        std::vector<std::string> staticTopics = {
-            topicCCIndexSt.Topic_Static_Query_Ack,
-            topicCCIndexSt.Topic_Static_Update_Ack
-        };
 
-        for (const auto& topic : staticTopics)
-        {
-            if (!topic.empty())
-            {
-                m_MonitorStatesCcindexSt[MONITOR_TAG_MONITOR_IN_CCINDEX_ST][topic].lastReceivedTime = now;
-                m_MonitorStatesCcindexSt[MONITOR_TAG_MONITOR_IN_CCINDEX_ST][topic].isAlarmActive = false;
-                m_MonitorStatesCcindexSt[MONITOR_TAG_MONITOR_IN_CCINDEX_ST][topic].lastLinkStatus = true;
-            }
-        }
-    }
-    //MEC自检结果: 启动定时检查器，建议 1秒一次
-    m_TimerMonitorMecCheck = m_EventloopOmMec->addTimer(
-        std::bind(&OM_COMPONENT::checkMonitorMecTimeouts, this), getConfiger().configerMec.configerPublishPeriod.periodCheckMecStatus, true);
+    // 获取检查周期配置
+    auto checkPeriod = getConfiger().configerMec.configerPublishPeriod.periodCheckMecStatus;
+    // MEC自检结果: 信号机原始数据 - SpatSrcData
+    m_TimerMonitorSpatSrcData = m_EventloopOmMec->addTimer(
+    std::bind(&OM_COMPONENT::checkMonitorSpatSrcDataTimeouts, this), checkPeriod, true);
+
+
+    // MEC自检结果:感知数据 - RsapData
+    m_TimerMonitorRsapData = m_EventloopOmMec->addTimer(
+    std::bind(&OM_COMPONENT::checkMonitorRsapDataTimeouts, this), checkPeriod, true);
+
+
+    // MEC自检结果: V2x信控数据 - CcindexTcData
+    m_TimerMonitorCcindexTcData = m_EventloopOmMec->addTimer(
+        std::bind(&OM_COMPONENT::checkMonitorCcindexTcDataTimeouts, this), checkPeriod, true);
+
+    // MEC自检结果: 雷达动态数据 - CcindexTmData
+    m_TimerMonitorCcindexTmData = m_EventloopOmMec->addTimer(
+        std::bind(&OM_COMPONENT::checkMonitorCcindexTmDataTimeouts, this), checkPeriod, true);
+
+    // MEC自检结果: 雷达静态数据 - CcindexStData
+    m_TimerMonitorCcindexStData = m_EventloopOmMec->addTimer(
+    std::bind(&OM_COMPONENT::checkMonitorCcindexStDataTimeouts, this), checkPeriod, true);
+
+    // MEC自检结果: 雷达点云数据 - CcindexRcpData
+    m_TimerMonitorCcindexRcp = m_EventloopOmMec->addTimer(
+    std::bind(&OM_COMPONENT::checkMonitorCcindexRcpDataTimeouts, this), checkPeriod, true);
+
 }
-//MEC自检结果： 监控是否超时
-void OM_COMPONENT::checkMonitorMecTimeouts()
+
+// MEC自检结果： 监控是否超时 - SpatSrcData
+void OM_COMPONENT::checkMonitorSpatSrcDataTimeouts()
+{
+    checkMonitorMecDataTimeouts(m_MonitorStateSpatSrcData, MONITOR_TAG_DATA_SPAT_SRC);
+}
+
+// MEC自检结果： 监控是否超时 - RsapData
+void OM_COMPONENT::checkMonitorRsapDataTimeouts()
 {
+    checkMonitorMecDataTimeouts(m_MonitorStateRsapData, MONITOR_TAG_DATA_SENSOR_OBJ);
+}
+
+
+// MEC自检结果： 监控是否超时 - CcindexTcData
+void OM_COMPONENT::checkMonitorCcindexTcDataTimeouts()
+{
+    checkMonitorMecDataTimeouts(m_MonitorStateCcindexTcData, MONITOR_TAG_DATA_CCINDEX_TC);
+}
+
+// MEC自检结果： 监控是否超时 - CcindexRcp
+void OM_COMPONENT::checkMonitorCcindexRcpDataTimeouts()
+{
+    checkMonitorMecDataTimeouts(m_MonitorStateCcindexRcpData, MONITOR_TAG_DATA_RADAR_DATA);
+}
+
+//MEC自检结果： 监控数据是否超时
+void OM_COMPONENT::checkMonitorMecDataTimeouts(MonitorState& state, int tag)
+{
+    if (!m_MqttClientCloud || !m_RegisterFlag)
+    {
+        return;
+    }
     uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
     /////////////////////////////////////////////////////////////////////////////////////////
-    for (auto& pair : m_MonitorRulesData)
-    {
-        int tag = pair.first;
-        auto& rule = pair.second;
-        auto& state = m_MonitorStatesData[tag];
 
+    uint64_t timeDiff = now - state.lastReceivedTime;
+    if(getConfiger().enableDebugPrint)
+    {
+        OM_CHECK_WARN_PRINT << "[om-check]t" << state.to_string();
+    }
 
-        uint64_t timeDiff = now - state.lastReceivedTime;
+    // --- 1. 检查超时 (故障触发) ---
+    // 无论是数据流断了，还是链路心跳断了，只要超时都是故障
+    if ((timeDiff+1000) >= state.timeoutMs)
+    {
         if(getConfiger().enableDebugPrint)
         {
-            OM_CHECK_WARN_PRINT << "[monitor]tag:" << tag << "[timeDiff]" << timeDiff << "[timeoutMs]"
-                                << rule.timeoutMs << "[now]"  << now  << "[lastReceivedTime]" <<  state.lastReceivedTime ;
+            OM_CHECK_WARN_PRINT << "[monitor] timeDiff > rule.timeoutMs" ;
         }
+        if (!state.isAlarmActive)
+        {
+            if(getConfiger().enableDebugPrint)
+            {
+                OM_CHECK_WARN_PRINT << "[monitor] !state.isAlarmActive " ;
+            }
+            std::string errorDetail;
+            if (state.isLinkCheck)
+            {
+                // 链路检查模式超时 = 模块挂了/CyberRT不通 (静默故障)
+                errorDetail = state.name + " [module offline/timeout]";
+            }
+            else
+            {
+                // 数据检查模式超时 = 业务数据没上来
+                errorDetail = state.name + " [no data]";
+            }
+            state.isAlarmActive = true;
+            state.alarmOccurredTime = now;
+            state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
+            state.recoveryStartTime = 0;
+            if (tag == MONITOR_TAG_DATA_SPAT_SRC)
+            {
+                //没有信号机原始数据
+                m_SpatDataStatusFromTLDeviceFlag.store(false, std::memory_order_relaxed);
+            }
+            alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, state.name,
+                "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
 
-        // --- 1. 检查超时 (故障触发) ---
-        // 无论是数据流断了，还是链路心跳断了，只要超时都是故障
-        if ((timeDiff+1000) >= rule.timeoutMs)
+            if(getConfiger().enableDebugPrint)
+            {
+                OM_CHECK_WARN_PRINT << "[monitor] " << errorDetail << " detected after "
+                                   << timeDiff << "ms (timeout: " << state.timeoutMs << "ms) "
+                        << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
+                 << "[alarmOccurredTime]" << state.alarmOccurredTime;
+            }
+        }
+        // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
+        else
         {
             if(getConfiger().enableDebugPrint)
             {
-                OM_CHECK_WARN_PRINT << "[monitor] timeDiff > rule.timeoutMs" ;
+                OM_CHECK_WARN_PRINT << "[monitor] state.isAlarmActive " ;
             }
-            if (!state.isAlarmActive)
+            // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
+            // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
+            if (!state.isLinkCheck && state.recoveryStartTime != 0)
             {
                 if(getConfiger().enableDebugPrint)
                 {
-                    OM_CHECK_WARN_PRINT << "[monitor] !state.isAlarmActive " ;
-                }
-                std::string errorDetail;
-                if (rule.isLinkCheck)
-                {
-                    // 链路检查模式超时 = 模块挂了/CyberRT不通 (静默故障)
-                    errorDetail = rule.name + " [module offline/timeout]";
+                    OM_CHECK_WARN_PRINT << "[monitor] recovery reset for " << state.name
+                                       << " (data stopped again for " << timeDiff << "ms)";
                 }
-                else
-                {
-                    // 数据检查模式超时 = 业务数据没上来
-                    errorDetail = rule.name + " [no data]";
-                }
-                state.isAlarmActive = true;
-                state.alarmOccurredTime = now;
-                state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
                 state.recoveryStartTime = 0;
-                if (tag == MONITOR_TAG_MONITOR_IN_SPAT_SRC_DATA)
-                {
-                    //没有信号机原始数据
-                    m_SpatDataStatusFromTLDeviceFlag.store(false, std::memory_order_relaxed);
-                }
-                alarmOccurred(errorDetail, rule.alarmType, getConfiger().rscuEsn, rule.name,
-                    "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
-
-                if(getConfiger().enableDebugPrint)
-                {
-                    OM_CHECK_WARN_PRINT << "[monitor] " << errorDetail << " detected after "
-                                       << timeDiff << "ms (timeout: " << rule.timeoutMs << "ms) "
-                            << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
-                     << "[alarmOccurredTime]" << state.alarmOccurredTime;
-                }
             }
-            // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
-            else
+
+            // 检查是否需要重复上报告警
+            // 如果上次上报时间为0，说明是首次告警，已经在上面的分支处理了
+            if (state.lastAlarmPublishTime > 0)
             {
-                if(getConfiger().enableDebugPrint)
+                uint64_t alarmPublishTimeDiff = (now - state.lastAlarmPublishTime) ;
+                if (getConfiger().enableDebugPrint)
                 {
-                    OM_CHECK_WARN_PRINT << "[monitor] state.isAlarmActive " ;
+                    OM_CHECK_WARN_PRINT << "[monitor][alarm-repeat][tag]" << tag
+                                       << "[name]" << state.name
+                                       << "[timeDiff(ms)]" << alarmPublishTimeDiff
+                                       << "[interval(ms)]" << state.timeoutMs
+                                        << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
+                                        << "[alarmOccurredTime]" << state.alarmOccurredTime;
                 }
-                // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
-                // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
-                if (!rule.isLinkCheck && state.recoveryStartTime != 0)
+
+                // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
+                if (alarmPublishTimeDiff >= state.timeoutMs)
                 {
-                    if(getConfiger().enableDebugPrint)
+                    std::string errorDetail;
+                    if (state.isLinkCheck)
                     {
-                        OM_CHECK_WARN_PRINT << "[monitor] recovery reset for " << rule.name
-                                           << " (data stopped again for " << timeDiff << "ms)";
+                        errorDetail = state.name + " [module offline/timeout]";
                     }
-                    state.recoveryStartTime = 0;
-                }
-
-                // 检查是否需要重复上报告警
-                // 如果上次上报时间为0，说明是首次告警，已经在上面的分支处理了
-                if (state.lastAlarmPublishTime > 0)
-                {
-                    uint64_t alarmPublishTimeDiff = (now - state.lastAlarmPublishTime) ;
-                    if (getConfiger().enableDebugPrint)
+                    else
                     {
-                        OM_CHECK_WARN_PRINT << "[monitor][alarm-repeat][tag]" << tag
-                                           << "[name]" << rule.name
-                                           << "[timeDiff(ms)]" << alarmPublishTimeDiff
-                                           << "[interval(ms)]" << rule.timeoutMs
-                                            << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
-                                            << "[alarmOccurredTime]" << state.alarmOccurredTime;
+                        errorDetail = state.name + " [no data]";
                     }
-
-                    // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
-                    if (alarmPublishTimeDiff >= rule.timeoutMs)
+                    // state.lastAlarmPublishTime = now;  // 更新上次上报时间
+                    if (tag == MONITOR_TAG_DATA_SPAT_SRC)
                     {
-                        std::string errorDetail;
-                        if (rule.isLinkCheck)
-                        {
-                            errorDetail = rule.name + " [module offline/timeout]";
-                        }
-                        else
-                        {
-                            errorDetail = rule.name + " [no data]";
-                        }
-                        state.lastAlarmPublishTime = now;  // 更新上次上报时间
-                        if (tag == MONITOR_TAG_MONITOR_IN_SPAT_SRC_DATA)
-                        {
-                            //没有信号机原始数据
-                            m_SpatDataStatusFromTLDeviceFlag.store(false, std::memory_order_relaxed);
-                        }
-                        alarmOccurred(errorDetail, rule.alarmType, getConfiger().rscuEsn, rule.name,
-                            "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
+                        //没有信号机原始数据
+                        m_SpatDataStatusFromTLDeviceFlag.store(false, std::memory_order_relaxed);
+                    }
+                    alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, state.name,
+                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
 
-                        if (getConfiger().enableDebugPrint)
-                        {
-                            OM_CHECK_WARN_PRINT << "[monitor] mec-self check repeated: " << errorDetail
-                                              << " (timeout: " << timeDiff << "ms, repeat interval: "
-                                              << rule.timeoutMs << "ms)";
-                        }
+                    if (getConfiger().enableDebugPrint)
+                    {
+                        OM_CHECK_WARN_PRINT << "[monitor] mec-self check repeated: " << errorDetail
+                                          << " (timeout: " << timeDiff << "ms, repeat interval: "
+                                          << state.timeoutMs << "ms)";
                     }
                 }
             }
         }
-        else
-        {
-            // OM_CHECK_DEBUG_PRINT << "[monitor] timeDiff <= rule.timeoutMs" ;
-            // timeDiff <= rule.timeoutMs
-            // 说明数据正常或链路正常。
-            // 此时什么都不用做！
-            // 恢复逻辑（Recovery）是在 handleMonitorDataUpdate 中处理的，不要在这里重置！
+    }
+    else
+    {
+        // OM_CHECK_DEBUG_PRINT << "[monitor] timeDiff <= rule.timeoutMs" ;
+        // timeDiff <= rule.timeoutMs
+        // 说明数据正常或链路正常。
+        // 此时什么都不用做！
+        // 恢复逻辑（Recovery）是在 handleMonitorDataUpdate 中处理的，不要在这里重置！
 
-            // 【错误代码删除】不要在这里写 state.recoveryStartTime = 0;
-            // 否则只要有数据来，恢复计时器就被清零，永远无法满足 "持续N秒" 的条件。
-        }
+        // 【错误代码删除】不要在这里写 state.recoveryStartTime = 0;
+        // 否则只要有数据来，恢复计时器就被清零，永远无法满足 "持续N秒" 的条件。
     }
+}
+// MEC自检结果： 监控是否超时 - CcindexTmData
+void OM_COMPONENT::checkMonitorCcindexTmDataTimeouts()
+{
+    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
     /////////////////////////////////////////////////////////////////////////////////////////
     //检查多雷达动态数据的超时
-    for (auto& rulePair : m_MonitorRulesCcindexTm)
+    for (auto& statesPair : m_MonitorStatesCcindexTmData)
     {
-        int tag = rulePair.first;
-        auto& rule = rulePair.second;
-        
-        if (!rule.enable)
-        {
-            continue;
-        }
+        std::string deviceId = statesPair.first;
+        auto& statesMap = statesPair.second;
 
         // 遍历每个雷达的每个topic
-        for (auto& statePair : m_MonitorStatesCcindexTm[tag])
+        for (auto& statePair : statesMap)
         {
             const std::string& topic = statePair.first;
             auto& state = statePair.second;
-            
+            if (!state.enable)
+            {
+                continue;
+            }
+
             uint64_t timeDiff = now - state.lastReceivedTime;
             if (getConfiger().enableDebugPrint)
             {
-                OM_CHECK_WARN_PRINT << "[monitor_ccindex_tm]tag:" << tag << "[topic]" << topic
-                                   << "[timeDiff]" << timeDiff << "[timeoutMs]" << rule.timeoutMs
+                OM_CHECK_WARN_PRINT << "[monitor_ccindex_tm]tag:" << state.tag << "[topic]" << topic
+                                   << "[timeDiff]" << timeDiff << "[timeoutMs]" << state.timeoutMs
                                    << "[now]" << now << "[lastReceivedTime]" << state.lastReceivedTime;
             }
             // --- 1. 检查超时 (故障触发) ---
-            if ((timeDiff + 1000) >= rule.timeoutMs)
+            if ((timeDiff + 1000) >= state.timeoutMs)
             {
                 if (getConfiger().enableDebugPrint)
                 {
@@ -1189,18 +1777,18 @@ void OM_COMPONENT::checkMonitorMecTimeouts()
                     {
                         OM_CHECK_WARN_PRINT << "[monitor_ccindex_tm] !state.isAlarmActive for topic: " << topic;
                     }
-                    std::string errorDetail = rule.name + " [topic: " + topic + "] [no data]";
+                    std::string errorDetail = state.name + " [topic: " + topic + "] [no data]";
                     state.isAlarmActive = true;
                     state.alarmOccurredTime = now;
                     state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
                     state.recoveryStartTime = 0;
-                    alarmOccurred(errorDetail, rule.alarmType, getConfiger().rscuEsn, topic,
+                    alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, topic,
                         "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.lastAlarmPublishTime);
 
                     if (getConfiger().enableDebugPrint)
                     {
                         OM_CHECK_WARN_PRINT << "[monitor_ccindex_tm] " << errorDetail << " detected after "
-                                           << timeDiff << "ms (timeout: " << rule.timeoutMs << "ms) "
+                                           << timeDiff << "ms (timeout: " << state.timeoutMs << "ms) "
                                            << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime;
                     }
                 }
@@ -1213,11 +1801,11 @@ void OM_COMPONENT::checkMonitorMecTimeouts()
                     }
                     // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
                     // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
-                    if (!rule.isLinkCheck && state.recoveryStartTime != 0)
+                    if (!state.isLinkCheck && state.recoveryStartTime != 0)
                     {
                         if(getConfiger().enableDebugPrint)
                         {
-                            OM_CHECK_WARN_PRINT << "[monitor_ccindex_tm] recovery reset for " << rule.name
+                            OM_CHECK_WARN_PRINT << "[monitor_ccindex_tm] recovery reset for " << state.name
                                                << " [topic: " << topic << "]"
                                                << " (data stopped again for " << timeDiff << "ms)";
                         }
@@ -1230,27 +1818,27 @@ void OM_COMPONENT::checkMonitorMecTimeouts()
                         uint64_t alarmPublishTimeDiff = (now - state.lastAlarmPublishTime);
                         if (getConfiger().enableDebugPrint)
                         {
-                            OM_CHECK_WARN_PRINT << "[monitor_ccindex_tm][alarm-repeat][tag]" << tag
+                            OM_CHECK_WARN_PRINT << "[monitor_ccindex_tm][alarm-repeat][tag]" << state.tag
                                                << "[topic]" << topic
-                                               << "[name]" << rule.name
+                                               << "[name]" << state.name
                                                << "[timeDiff(ms)]" << alarmPublishTimeDiff
-                                               << "[interval(ms)]" << rule.timeoutMs
+                                               << "[interval(ms)]" << state.timeoutMs
                                                << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime;
                         }
 
                         // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
-                        if (alarmPublishTimeDiff >= rule.timeoutMs)
+                        if (alarmPublishTimeDiff >= state.timeoutMs)
                         {
-                            std::string errorDetail = rule.name + " [topic: " + topic + "] [no data]";
-                            state.lastAlarmPublishTime = now;  // 更新上次上报时间
-                            alarmOccurred(errorDetail, rule.alarmType, getConfiger().rscuEsn, topic,
+                            std::string errorDetail = state.name + " [topic: " + topic + "] [no data]";
+                            // state.lastAlarmPublishTime = now;  // 更新上次上报时间
+                            alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, topic,
                                 "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.lastAlarmPublishTime);
 
                             if (getConfiger().enableDebugPrint)
                             {
                                 OM_CHECK_WARN_PRINT << "[monitor_ccindex_tm] mec-self check repeated: " << errorDetail
                                                   << " (timeout: " << timeDiff << "ms, repeat interval: "
-                                                  << rule.timeoutMs << "ms)";
+                                                  << state.timeoutMs << "ms)";
                             }
                         }
                     }
@@ -1258,37 +1846,43 @@ void OM_COMPONENT::checkMonitorMecTimeouts()
             }
         }
     }
+}
+
+// MEC自检结果： 监控是否超时 - CcindexStData
+void OM_COMPONENT::checkMonitorCcindexStDataTimeouts()
+{
+    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
   /////////////////////////////////////////////////////////////////////////////////////////
     //检查多雷达静态数据的超时
-    for (auto& rulePair : m_MonitorRulesCcindexSt)
+    for (auto& statesPair : m_MonitorStatesCcindexStData)
     {
-        int tag = rulePair.first;
-        auto& rule = rulePair.second;
+        std::string deviceId = statesPair.first;
+        auto& statesMap = statesPair.second;
 
-        if (!rule.enable)
-        {
-            continue;
-        }
 
         // 遍历每个雷达的每个topic
-        for (auto& statePair : m_MonitorStatesCcindexSt[tag])
+        for (auto& statePair : statesMap)
         {
             const std::string& topic = statePair.first;
             auto& state = statePair.second;
+            if (!state.enable)
+            {
+                continue;
+            }
 
             uint64_t timeDiff = now - state.lastReceivedTime;
             if (getConfiger().enableDebugPrint)
             {
-                OM_CHECK_WARN_PRINT << "[monitor_ccindex_st]tag:" << tag << "[topic]" << topic
-                                   << "[timeDiff]" << timeDiff << "[timeoutMs]" << rule.timeoutMs
+                OM_CHECK_WARN_PRINT << "[monitor_ccindex_st]tag:" << state.tag << "[topic]" << topic
+                                   << "[timeDiff]" << timeDiff << "[timeoutMs]" << state.timeoutMs
                                    << "[now]" << now << "[lastReceivedTime]" << state.lastReceivedTime;
             }
             // --- 1. 检查超时 (故障触发) ---
-            if ((timeDiff + 1000) >= rule.timeoutMs)
+            if ((timeDiff + 1000) >= state.timeoutMs)
             {
                 if (getConfiger().enableDebugPrint)
                 {
-                    OM_CHECK_WARN_PRINT << "[monitor_ccindex_st] timeDiff >= rule.timeoutMs for topic: " << topic;
+                    OM_CHECK_WARN_PRINT << "[monitor_ccindex_st] timeDiff >= state.timeoutMs for topic: " << topic;
                 }
                 if (!state.isAlarmActive)
                 {
@@ -1296,18 +1890,18 @@ void OM_COMPONENT::checkMonitorMecTimeouts()
                     {
                         OM_CHECK_WARN_PRINT << "[monitor_ccindex_st] !state.isAlarmActive for topic: " << topic;
                     }
-                    std::string errorDetail = rule.name + " [topic: " + topic + "] [no data]";
+                    std::string errorDetail = state.name + " [topic: " + topic + "] [no data]";
                     state.isAlarmActive = true;
                     state.alarmOccurredTime = now;
                     state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
                     state.recoveryStartTime = 0;
-                    alarmOccurred(errorDetail, rule.alarmType, getConfiger().rscuEsn, topic,
+                    alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, topic,
                     "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.lastAlarmPublishTime);
 
                     if (getConfiger().enableDebugPrint)
                     {
                         OM_CHECK_WARN_PRINT << "[monitor_ccindex_st] " << errorDetail << " detected after "
-                                           << timeDiff << "ms (timeout: " << rule.timeoutMs << "ms) "
+                                           << timeDiff << "ms (timeout: " << state.timeoutMs << "ms) "
                                            << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime;
                     }
                 }
@@ -1320,11 +1914,11 @@ void OM_COMPONENT::checkMonitorMecTimeouts()
                     }
                     // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
                     // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
-                    if (!rule.isLinkCheck && state.recoveryStartTime != 0)
+                    if (!state.isLinkCheck && state.recoveryStartTime != 0)
                     {
                         if(getConfiger().enableDebugPrint)
                         {
-                            OM_CHECK_WARN_PRINT << "[monitor_ccindex_st] recovery reset for " << rule.name
+                            OM_CHECK_WARN_PRINT << "[monitor_ccindex_st] recovery reset for " << state.name
                                                << " [topic: " << topic << "]"
                                                << " (data stopped again for " << timeDiff << "ms)";
                         }
@@ -1337,27 +1931,27 @@ void OM_COMPONENT::checkMonitorMecTimeouts()
                         uint64_t alarmPublishTimeDiff = (now - state.lastAlarmPublishTime);
                         if (getConfiger().enableDebugPrint)
                         {
-                            OM_CHECK_WARN_PRINT << "[monitor_ccindex_st][alarm-repeat][tag]" << tag
+                            OM_CHECK_WARN_PRINT << "[monitor_ccindex_st][alarm-repeat][tag]" << state.tag
                                                << "[topic]" << topic
-                                               << "[name]" << rule.name
+                                               << "[name]" << state.name
                                                << "[timeDiff(ms)]" << alarmPublishTimeDiff
-                                               << "[interval(ms)]" << rule.timeoutMs
+                                               << "[interval(ms)]" << state.timeoutMs
                                                << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime;
                         }
 
                         // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
-                        if (alarmPublishTimeDiff >= rule.timeoutMs)
+                        if (alarmPublishTimeDiff >= state.timeoutMs)
                         {
-                            std::string errorDetail = rule.name + " [topic: " + topic + "] [no data]";
-                            state.lastAlarmPublishTime = now;  // 更新上次上报时间
-                            alarmOccurred(errorDetail, rule.alarmType, getConfiger().rscuEsn, topic,
+                            std::string errorDetail = state.name + " [topic: " + topic + "] [no data]";
+                            // state.lastAlarmPublishTime = now;  // 更新上次上报时间
+                            alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, topic,
                             "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.lastAlarmPublishTime);
 
                             if (getConfiger().enableDebugPrint)
                             {
                                 OM_CHECK_WARN_PRINT << "[monitor_ccindex_st] mec-self check repeated: " << errorDetail
                                                   << " (timeout: " << timeDiff << "ms, repeat interval: "
-                                                  << rule.timeoutMs << "ms)";
+                                                  << state.timeoutMs << "ms)";
                             }
                         }
                     }
@@ -1365,8 +1959,8 @@ void OM_COMPONENT::checkMonitorMecTimeouts()
             }
         }
     }
-
 }
+
 //MEC自检结果：错误码映射
 int OM_COMPONENT::getSelfCheckFaultType(MecAlarmTypeErrorCodeEnum alarmType)
 {
@@ -1446,7 +2040,9 @@ void OM_COMPONENT::sendMecSelfCheckMsg(MecAlarmTypeErrorCodeEnum alarmType, int
         mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_MecSelfCheckResult_Up, data, desc);
     }
 }
-
+//////////////////////////////////////////////////////////////////////////////////////////////
+/////////////////////////////////////////////////////////////////////////////////////////////////
+///
 //告警：摄像头角度偏移
 void  OM_COMPONENT::initProcessEventOutputResult()
 {
@@ -2217,6 +2813,7 @@ bool OM_COMPONENT::V2xPbAsnMessageFrame2StrBroad(const std::shared_ptr<const v2x
 #endif
     return true;
 }
+//监控om运维状态：路侧监控工具使用
 void OM_COMPONENT::periodMonitorOmStatus()
 {
     if (!getConfiger().configerMec.configerEnable.Enable_Monitor_Mec_Om_Status)
@@ -2845,14 +3442,13 @@ bool OM_COMPONENT::periodMonitorSensorDeviceActive()
 bool OM_COMPONENT::periodMonitorSpatDataStatus()
 {
     std::string hint = "Monitor-Spat-Data-Status";
-    if (!m_MqttConnectedCloud)
+    if (!m_MqttConnectedCloud||!m_RegisterFlag)
     {
         OM_CLOUD_DEBUG_PRINT << "[hint]" << hint ;
         return false;
     }
-    if (!m_RegisterFlag)
+    if (!getConfiger().configerMec.configerEnable.Enable_Spat_Data_Status)
     {
-        OM_CLOUD_DEBUG_PRINT << "[hint]" << hint ;
         return false;
     }
     // bool spatDataStatusFromTLDeviceFlag = m_SpatDataStatusFromTLDeviceFlag.load(std::memory_order_relaxed);
@@ -2887,6 +3483,7 @@ bool OM_COMPONENT::periodMonitorSpatDataStatus()
 
 
 ///////////////////////////////////////////////////////////////////////////
+//获取云控的的配置内容从工参配置文件中
 bool OM_COMPONENT::getWorkParamFromFileCloud()
 {
    OmWorkParamConfiger  omWorkParamConfiger;
@@ -2932,6 +3529,7 @@ bool OM_COMPONENT::getWorkParamFromFileCloud()
 //    getConfiger().enableDebugPrint = omWorkParamConfiger.hasedInit;
     saveConfiger();
 }
+//从工参中获取配置文件
 bool OM_COMPONENT::getWorkParamFromFile()
 {
     WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, m_OmWorkParamConfiger);
@@ -3246,7 +3844,7 @@ bool OM_COMPONENT::getWorkParamFromFile()
     }
     return true;
 }
-
+//获取软件版本
 std::string OM_COMPONENT::getSoftwareVersion()
 {
     std::string softwareVersionFromFile;
@@ -3812,7 +4410,7 @@ void OM_COMPONENT::subscribeOtaCancelData(const json &contentJson)
     {
         return;
     }
-    std::string hint;
+
     OtaUpdateCancelData otaUpdateCancelData;
     try
     {
@@ -3833,7 +4431,19 @@ void OM_COMPONENT::subscribeOtaCancelData(const json &contentJson)
     // 2. 执行核心取消操作
     m_CancelDownloadFlag.store(true, std::memory_order_relaxed);
     m_UpdateVersionFromCancleMsg = otaUpdateCancelData.updateVersion;
-    OM_MEC_SUCCESS_PRINT << "[cancel] OTA Cancelled Successfully.";
+
+    std::string hint = "[cancel] OTA Cancelled Successfully.";
+    OM_MEC_SUCCESS_PRINT << hint;
+    OTAUpdateStatusUpData otaUpdateStatusUpData;
+    otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
+    otaUpdateStatusUpData.seqNum =  otaUpdateCancelData.seqNum;
+    otaUpdateStatusUpData.rscuEsn = otaUpdateCancelData.rscuEsn;
+    otaUpdateStatusUpData.code  = FAILURE;
+    otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
+    otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;
+    otaUpdateStatusUpData.description = hint;
+
+    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status ,otaUpdateStatusUpData, hint);
 }
 
 
@@ -4022,6 +4632,7 @@ void  OM_COMPONENT::monitorDeviceNeedUpdate()
 
         //取消升级
         bool val = m_CancelDownloadFlag.load(std::memory_order_relaxed);
+        OM_MEC_ERROR_PRINT << "[notice] " << (val?"true":"false");
         if (val)
         {
             if (m_UpdateVersionFromDownMsg !=  m_UpdateVersionFromCancleMsg)
@@ -4088,6 +4699,19 @@ void  OM_COMPONENT::monitorDeviceNeedUpdate()
                                     getConfiger().configerMec.configerOtaParam.airosVersionTarFlag = true;
                                     getConfiger().configerMec.configerOtaParam.airosVersionSeqNum = m_OtaDownSeqnum;
                                     saveConfiger();
+
+                                    OTAUpdateStatusUpData otaUpdateStatusUpData;
+                                    otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
+                                    otaUpdateStatusUpData.seqNum =  m_OtaDownSeqnum;
+                                    otaUpdateStatusUpData.rscuEsn = getConfiger().rscuEsn;
+                                    otaUpdateStatusUpData.code  = SUCCESS;
+                                    otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
+                                    otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;
+
+                                    std::string hint = "[success] upgrade success!";
+                                    otaUpdateStatusUpData.description = hint;
+                                    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
+
                                     std::string command_start = "cd /home/airos/ && launch all";
                                     int ret = system(command_start.c_str());
                                     if (ret == 0)
@@ -4577,11 +5201,11 @@ void OM_COMPONENT::updateMecStatusInfo()
 }
 void OM_COMPONENT::alarmMonitor()
 {
-    if (!m_MqttConnected)
+    if (!m_MqttConnected|| !m_RegisterFlag)
     {
         return;
     }
-    if(!getConfiger().configerMec.configerEnable.Enable_Alarm_Monitor || !m_RegisterFlag)
+    if(!getConfiger().configerMec.configerEnable.Enable_Alarm_Monitor )
     {
         return;
     }
@@ -5272,6 +5896,11 @@ void OM_COMPONENT::publishDeviceVersionData(DeviceBaseInfoQueryData& deviceBaseI
         return;
     }
 
+    if (!m_RegisterFlag)
+    {
+		return;		
+
+    }
     if(!m_MqttClientConfig.configerMec.configerEnable.Enable_Ota_Version || !m_RegisterFlag)
     {
         return;
@@ -7099,6 +7728,12 @@ bool OM_COMPONENT::getMqttConfigerMsg()
         for(uint32_t i = 0; i < getConfiger().configerRadar.configerTopicRadarID.radarIDs.size(); i++)
         {
             std::string radarID = getConfiger().configerMec.configerTopicRadarID.radarIDs[i];
+            std::string device_id_str;
+            size_t last_slash = radarID.rfind('/');
+            if (last_slash != std::string::npos) {
+                // 提取最后一个 '/' 之后的所有字符 (例如 "{device_id}")
+                device_id_str = radarID.substr(last_slash + 1);
+            }
             m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix =   m_MqttClientConfig.rscuEsn + getConfiger().configerMec.configerTopicRadarID.radarIDs[i];
 
             if(getConfiger().enableDebugPrint)
@@ -7118,7 +7753,7 @@ bool OM_COMPONENT::getMqttConfigerMsg()
             topicCCIndexTm.Topic_Nonmotor         = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_Nonmotor     + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
             topicCCIndexTm.Topic_DeviceStatus     = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_DeviceStatus     + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
             topicCCIndexTm.Topic_Pulse            = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_Pulse            + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
-            m_MecSelfCheckTmDataMap[radarID] = topicCCIndexTm;
+            m_MecSelfCheckTmDeviceIdTopicMap[device_id_str] = topicCCIndexTm;
             //静态
             TopicCCIndexSt            topicCCIndexSt;
             // if (!m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Query_Ack.empty() && m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Query_Ack.back() == '/')
@@ -7133,10 +7768,10 @@ bool OM_COMPONENT::getMqttConfigerMsg()
             // {
             //     m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Update_Ack.pop_back();
             // }
-            topicCCIndexSt.Topic_Static_Update_Ack = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Profix +
-                                                            m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Update_Ack +  m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
+            topicCCIndexSt.Topic_Static_Update = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Profix +
+                                                            m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Update +  m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
 
-            m_MecSelfCheckStDataMap[radarID] = topicCCIndexSt;
+            m_MecSelfCheckStDeviceIdTopicMap[device_id_str] = topicCCIndexSt;
         }
     }
 
@@ -8269,9 +8904,13 @@ bool OM_COMPONENT::alarmOccurred(std::string hint, MecAlarmTypeErrorCodeEnum mec
         alarmManagementData.alarm.addition = sensorDeviceNo;
         alarmManagementData.alarm.additionEmpty = false;
     }
-    if (mecAlarmTypeErrorCode < MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS)
+    if (mecAlarmTypeErrorCode < MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS )
     {
-        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Alarm, alarmManagementData, hint);
+        if (getConfiger().configerMec.configerEnable.Enable_Alarm)
+        {
+            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Alarm, alarmManagementData, hint);
+        }
+
     }
     else
     {
@@ -8280,7 +8919,11 @@ bool OM_COMPONENT::alarmOccurred(std::string hint, MecAlarmTypeErrorCodeEnum mec
         if (faultType != 0) {
             // 故障产生：status=1, startTime=now, stopTime=0(或now)
             // uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
-            sendMecSelfCheckMsg(mecAlarmTypeErrorCode, faultType, 1, faultStartTime, 0, addition);
+            if (getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
+            {
+                OM_MEC_DEBUG_PRINT  << "[alarm][occurred][type]" << mecAlarmTypeErrorCode << "[msg]" << hint;
+                sendMecSelfCheckMsg(mecAlarmTypeErrorCode, faultType, 1, faultStartTime, 0, addition);
+            }
         }
     }
 
diff --git a/middleware/protocol/om/om_component.h b/middleware/protocol/om/om_component.h
index 1c81123..ff028fd 100644
--- a/middleware/protocol/om/om_component.h
+++ b/middleware/protocol/om/om_component.h
@@ -254,6 +254,7 @@ public:
     bool initEventLoopOmMec();
     bool initEventLoopOmPing();
     bool initEventLoopOmSpat();
+    bool initEventLoopOmCheck();
     bool getMqttConfigerMsg();
     bool getWorkParamFromFile();
     std::string getGatewayAddress();
@@ -575,6 +576,8 @@ private:
     long long getFileSize(const std::string& path);
     void handleErrorResponse(CURLcode code);
     bool checkOtaUpdateVersion();
+    //ota升级取消
+    void subscribeOtaCancelData(const json &contentJson);
     //设备重启
     void subscribeRebootData(const json &contentJson);
     void monitorRebootTimeConfiger();
@@ -683,6 +686,7 @@ private:
     std::unique_ptr<std::thread>    m_TaskOtaDown;
     std::unique_ptr<std::thread>    m_TaskOmPing;
     std::unique_ptr<std::thread>    m_TaskOmSpat;
+    std::unique_ptr<std::thread>    m_TaskOmCheck;
     /////////////////////////////////////////////////////////////
     std::shared_ptr<afl::net::EventLoop>    m_Eventloop;
     std::shared_ptr<afl::net::EventLoop>    m_EventloopCloud;
@@ -694,6 +698,9 @@ private:
     std::shared_ptr<afl::net::EventLoop>    m_EventloopOmPing;
     std::shared_ptr<afl::net::EventLoop>    m_EventloopOmSpat;
 
+    std::shared_ptr<afl::net::EventLoop>    m_EventloopOmCheck;
+
+
     std::unique_ptr<afl::net::Channel>      m_Chnl;
     MQTTAsync_connectOptions        m_MqttConnOpts;
     MQTTAsync                       m_MqttClient;
@@ -759,7 +766,7 @@ private:
     std::unique_ptr<std::thread>    m_TaskProcessV2xData;
     std::unique_ptr<std::thread>    m_TaskProcessTrafficlightDetectData;
 	std::unique_ptr<std::thread>    m_TaskProcessTiming;
-    std::unique_ptr<std::thread>    m_TaskProcessMonitorMecData;
+    std::unique_ptr<std::thread>    m_TaskProcessMecCheck;
     std::unique_ptr<std::thread>    m_TaskProcessMonitorSpatData;  //信号机数据状态
     EnAsnType                               m_AsnType{EnAsnType::YDT_3709_2020_EXT};
     int                                     m_MsgType;
@@ -843,55 +850,116 @@ private:
     int m_TimerPingSensorDevice = -1;
 	bool m_isTrafficlightNormal = true;
 	
-    void subscribeOtaCancelData(const json &contentJson);
+    ///////////////////////////////////////////////////////////////////////////////////////////////////////
     std::string m_UsingVersion = "";
-
-    // 新增：用于控制取消的共享标志
     std::atomic<bool>  m_CancelDownloadFlag{false};
     std::string m_UpdateVersionFromCancleMsg = ""; //取消升级的版本号
     std::string m_UpdateVersionFromDownMsg = ""; //升级版本号来自云端下发
+    ///////////////////////////////////////////////////////////////////////////////////////////////////////
+
+    void  initProcessMecCheckSpatSrcData();
+    void  processMecCheckSpatSrcData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+
+    void  initProcessMecCheckRsapData();
+    void  processMecCheckRsapData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+
+    void  initProcessMecCheckRsapLink();
+    void  processMecCheckRsapLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+
+    void  initProcessMecCheckCcindexTcData();
+    void  processMecCheckCcindexTcData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+
+    void  initProcessMecCheckCcindexTmData();
+    void  processMecCheckCcindexTmData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+
+    void  initProcessMecCheckCcindexTmLink();
+    void  processMecCheckCcindexTmLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+
+    void  initProcessMecCheckCcindexStData();
+    void  processMecCheckCcindexStData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+
+    void  initProcessMecCheckCcindexStLink();
+    void  processMecCheckCcindexStLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+
+    void  initProcessMecCheckCcindexRcpData();
+    void  processMecCheckCcindexRcpData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+
+    void  initProcessMecCheckCcindexRcpLink();
+    void  processMecCheckCcindexRcpLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
+    //mec自检 ：信号机原始数据
+    std::unique_ptr<std::thread>    m_TaskProcessMecCheckSpatSrcData;
+    MonitorState                    m_MonitorStateSpatSrcData;
+
+    //mec自检： 感知数据
+    std::unique_ptr<std::thread>    m_TaskProcessMecCheckRsapData;
+    MonitorState                    m_MonitorStateRsapData;
+
+
+    //mec自检： V2X信控数据
+    std::unique_ptr<std::thread>    m_TaskProcessMecCheckCcindexTcData;
+    MonitorState                    m_MonitorStateCcindexTcData;
+
+    //mec自检： 雷达动态数据
+    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexTmData;
+    std::map<std::string, std::map<std::string, MonitorState>>     m_MonitorStatesCcindexTmData;  // <id, <topic, state>>
+    std::map<std::string, TopicCCIndexTm>  m_MecSelfCheckTmDeviceIdTopicMap;
+
+    //mec自检： 雷达静态数据
+    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexStData;
+    std::map<std::string, std::map<std::string, MonitorState>>     m_MonitorStatesCcindexStData;  // <id, <topic, state>>
+    std::map<std::string, TopicCCIndexSt>  m_MecSelfCheckStDeviceIdTopicMap;
+
+
+    //mec自检： 点云数据
+    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexCcindexRcpData;
+    MonitorState                            m_MonitorStateCcindexRcpData;
+
+    /////////////////////////////////////////////////////////////////////////
+    //mec自检： 云控链路
+    std::unique_ptr<std::thread>    m_TaskProcessMecCheckRsapLink;
+    MonitorState                    m_MonitorStateRsapLink;
+
+    //mec自检： 雷达动态链路
+    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexTmLink;
+    MonitorState                            m_MonitorStateCcindexTmLink;
+    //mec自检： 雷达静态链路
+    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexStLink;
+    MonitorState                            m_MonitorStateCcindexStLink;
+
+    //mec自检： 点云链路
+    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexCcindexRcpLink;
+    MonitorState     m_MonitorStateCcindexRcpLink;
+
+
+    void handleMonitorDataUpdateCcindexTm(const std::string & deviceId, const std::string& topic, int tag, uint64_t timestamp);
+    void handleMonitorDataUpdateCcindexSt(const std::string & deviceId, const std::string& topic, int tag, uint64_t timestamp);
+
+    // MEC自检结果： 监控是否超时
+    void checkMonitorSpatSrcDataTimeouts();
+    void checkMonitorRsapDataTimeouts();
+    void checkMonitorCcindexTcDataTimeouts();
+    void checkMonitorCcindexTmDataTimeouts();
+    void checkMonitorCcindexStDataTimeouts();
+    void checkMonitorCcindexRcpDataTimeouts();
+    void checkMonitorMecDataTimeouts(MonitorState& state, int tag);
+    long m_TimerMonitorSpatSrcData = -1;
+    long m_TimerMonitorRsapData = -1;
+    long m_TimerMonitorRsapLink = -1;
+    long m_TimerMonitorCcindexTcData = -1;
+    long m_TimerMonitorCcindexTmData = -1;
+    long m_TimerMonitorCcindexStData = -1;
+    long m_TimerMonitorCcindexTmLink = -1;
+    long m_TimerMonitorCcindexStLink = -1;
+    long m_TimerMonitorCcindexRcp = -1;
 
-    void  initProcessMonitorMecOutputResult();
-    void  processMonitorMecOutputResult(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData);
-    // 监控规则表: Tag -> Rule
-    std::map<int, MonitorRule> m_MonitorRulesData;
-    // 监控状态表: Tag -> State
-    std::map<int, MonitorState> m_MonitorStatesData;
-
-    // 监控规则表: Tag -> Rule
-    std::map<int, MonitorRule> m_MonitorRulesLink;
-    // 监控状态表: Tag -> State
-    std::map<int, MonitorState> m_MonitorStatesDataLink;
-
-    // [新增] 存储多路雷达设备的动态数据
-    // 监控规则表: Tag -> Rule
-    std::map<int, MonitorRule> m_MonitorRulesCcindexTm;
-    // Key: topic, Value: 监控状态
-    std::map<int, std::map<std::string, MonitorState>> m_MonitorStatesCcindexTm;
-    std::map<std::string, TopicCCIndexTm>  m_MecSelfCheckTmDataMap;
-    // [新增] 处理多设备雷达数据的更新逻辑
-    void handleMonitorDataUpdateCcindexTm(const std::string& topic, int tag, uint64_t timestamp);
-
-    // [新增] 存储多路雷达设备的静态数据
-    // 监控规则表: Tag -> Rule
-    std::map<int, MonitorRule> m_MonitorRulesCcindexSt;
-    // Key: topic, Value: 监控状态
-    std::map<int, std::map<std::string, MonitorState>> m_MonitorStatesCcindexSt;
-    std::map<std::string, TopicCCIndexSt>  m_MecSelfCheckStDataMap;
-    // [新增] 处理多设备雷达数据的更新逻辑
-    void handleMonitorDataUpdateCcindexSt(const std::string& topic, int tag, uint64_t timestamp);
-
-    // 定时器ID
-    long m_TimerMonitorMecCheck = -1;
     int m_Second2MSUnit = 1000;
     // 初始化监控规则
     void initMonitorRules();
     // 处理单个数据类型的监控逻辑
-    void handleMonitorDataUpdate(int tag, uint64_t timestamp);
-
+    void handleMonitorDataUpdate(MonitorState &state, int tag, uint64_t timestamp);
 
     // 处理单个链路类型的监控逻辑
-    void handleMonitorLinkUpdate(int tag, bool status, uint64_t timestamp);
+    void handleMonitorLinkUpdate(MonitorState &state, int tag, bool status, uint64_t timestamp);
     // 定时检查超时函数
     void checkMonitorMecTimeouts();
 
@@ -900,7 +968,7 @@ private:
 
     // 发送自检结果消息
     void sendMecSelfCheckMsg(MecAlarmTypeErrorCodeEnum alarmType,  int faultType, int faultStatus, uint64_t startTime, uint64_t stopTime, const std::string& desc);
-
+    ///////////////////////////////////////////////////////////////////////////////////////////////////////
     std::string getTLFaultDesc(TrafficlightDetectStatus tlds);
     //信号灯状态检测
     std::atomic<bool> m_SpatDataStatusFromTLAdapterFlag{false}; //是否接收到信号机适配器的信号机数据状态,
diff --git a/middleware/protocol/om_common/configer_bs.h b/middleware/protocol/om_common/configer_bs.h
index b32f5f7..4b05653 100644
--- a/middleware/protocol/om_common/configer_bs.h
+++ b/middleware/protocol/om_common/configer_bs.h
@@ -27,6 +27,308 @@ using namespace airos::base::workparam;
     #define SUBSCRIBE_NUM 1000
 #endif
 
+///////////////////////////////////////////////////////////////////////////////////
+/////信号机原始数据
+struct MonitorMecConfigerSpatSrcData : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/spat_src/data_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+///////////////////////////////////////////////////////////////////////////////////
+///感知数据
+struct MonitorMecConfigerRsapData : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/rsap/data_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+struct MonitorMecConfigerRsapLink : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/rsap/link_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+///////////////////////////////////////////////////////////////////////////////////
+///V2x信控指标数据
+struct MonitorMecConfigerCcindexV2xData : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/ccindex/tc/data_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+///////////////////////////////////////////////////////////////////////////////////
+///雷达信控指标：动态数据及链路
+struct MonitorMecConfigerCcindexTmData : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/ccindex/tm/data_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+
+///雷达信控指标：动态数据及链路
+struct MonitorMecConfigerCcindexTmLink : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/ccindex/tm/link_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+///雷达信控指标：静态数据及链路
+struct MonitorMecConfigerCcindexStData : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/ccindex/st/data_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+struct MonitorMecConfigerCcindexStLink : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/ccindex/st/link_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+
+///毫米波雷达原始数据：数据及链路
+struct MonitorMecConfigerCcindexRcp : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/rcp_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+///毫米波雷达原始数据：数据及链路
+struct MonitorMecConfigerCcindexRcpData : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/ccindex/rcp/data_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+struct MonitorMecConfigerCcindexRcpLink : public afl::base::SerializableData
+{
+    std::string channel_readers = "/v2x/mec/om/check/ccindex/rcp/link_";
+
+private:
+    virtual void serialize(afl::base::json &j)
+    {
+        JsonSerialize(channel_readers, "A_channel_readers", j, false);
+
+    }
+
+    virtual void deserialize(const afl::base::json &j) override
+    {
+        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);
+
+    }
+public:
+    std::string to_string() const
+    {
+        std::stringstream ss;
+        ss << std::endl;
+        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
+
+        return ss.str();
+    }
+};
+///////////////////////////////////////////////////////////////////////////////////
+
 struct MonitorMecConfiger : public afl::base::SerializableData
 {
     std::string channel_readers = "/v2x/monitor/mec";
@@ -138,10 +440,10 @@ private:
     }
 };
 
-
+//信号灯检测
 struct TrafficlightDetectDataConfiger : public afl::base::SerializableData
 {
-    std::string channel_readers = "/om/spat/check";
+    std::string channel_readers = "/v2x/mec/om/spat/detect/data_";
 private:
     virtual void serialize(json &j) override
     {
@@ -164,10 +466,10 @@ public:
         return ss.str();
     }
 };
-
+//信号灯数据状态
 struct MonitorSpatConfiger : public afl::base::SerializableData
 {
-    std::string channel_readers = "/v2x/monitor/spat";
+    std::string channel_readers = "/v2x/mec/om/spat/status_";
 
 private:
     virtual void serialize(afl::base::json &j)
diff --git a/middleware/protocol/proto/monitor_mec.proto b/middleware/protocol/proto/monitor_mec.proto
index 4094655..f272680 100644
--- a/middleware/protocol/proto/monitor_mec.proto
+++ b/middleware/protocol/proto/monitor_mec.proto
@@ -2,18 +2,31 @@ syntax = "proto2";
 package airos.monitor_mec;
 enum MonitorMecTag
 {
-  MONITOR_TAG_MONITOR_IN_SPAT_SRC_DATA = 3024;  //信号机原始数据, [异常]超过5秒没有数据上报
-  MONITOR_TAG_MONITOR_IN_SENSOR_OBJ_DATA = 3025; //感知数据异常, [异常]超过10秒没有数据上报
-  MONITOR_TAG_MONITOR_IN_SENSOR_EVENTS_DATA = 3026; //感知事件异常, [异常]超过10秒没有数据上报
-  MONITOR_TAG_MONITOR_IN_CCINDEX_TC = 3027; //v2x信控指标, 最小周期5s, [异常]超过15秒没有数据上报
-  MONITOR_TAG_MONITOR_IN_CCINDEX_TM = 3028; //雷达信控指标-动态: 最小周期1s, [异常]超过3s没有上报
-  MONITOR_TAG_MONITOR_IN_CCINDEX_ST = 3029; //雷达信控指标-静态: 周期5分钟，[异常]超过15分钟没有上报
-  MONITOR_TAG_MONITOR_IN_RADAR_DATA = 3030; //毫米波雷达原始数据: 周期目前接口没有要求，如果定义推送周期最大不超过30分钟，则1小时未推送为故障 [恢复]1个小时内恢复推送
-  MONITOR_TAG_MONITOR_IN_LIDAR_DATA = 3031; //激光雷达原始数据
-  MONITOR_TAG_MONITOR_OUT_SENSOR_CLOUD = 3032; //云控链路状态:TCP	TCP连接状态异常
-  MONITOR_TAG_MONITOR_OUT_CCINDEX_ST = 3033; // 信控链路状态-静态	HTTP	http连接异常
-  MONITOR_TAG_MONITOR_OUT_CCINDEX_TM = 3034; //  信控链路状态-动态	MQTT	mqtt链路异常
-  MONITOR_TAG_MONITOR_OUT_RADAR_POINT_CLOUD = 3035;  //FTP	ftp链路异常
+  MONITOR_TAG_DATA_SPAT_SRC = 3024;  //信号机原始数据, [异常]超过5秒没有数据上报
+  MONITOR_TAG_DATA_SENSOR_OBJ = 3025; //感知数据异常, [异常]超过10秒没有数据上报
+  MONITOR_TAG_DATA_SENSOR_EVENTS = 3026; //感知事件异常, [异常]超过10秒没有数据上报
+  MONITOR_TAG_DATA_CCINDEX_TC = 3027; //v2x信控指标, 最小周期5s, [异常]超过15秒没有数据上报
+
+  MONITOR_TAG_DATA_CCINDEX_TM_TRAJECTORIES = 3028; //雷达信控指标-动态: 实时轨迹数据 最小周期1s, [异常]超过3s没有上报
+  MONITOR_TAG_DATA_CCINDEX_TM_VEHICLEPASS = 3029; //雷达信控指标-动态:实时过车数据  最小周期1s, [异常]超过3s没有上报
+  MONITOR_TAG_DATA_CCINDEX_TM_QUEUEUP = 3030; //雷达信控指标-动态: 实时排队数据 最小周期1s, [异常]超过3s没有上报
+  MONITOR_TAG_DATA_CCINDEX_TM_AREASTATE = 3031; //雷达信控指标-动态:实时区域状态数据  最小周期1s, [异常]超过3s没有上报
+  MONITOR_TAG_DATA_CCINDEX_TM_OVERFLOW = 3032; //雷达信控指标-动态: 实时溢出数据 最小周期1s, [异常]超过3s没有上报
+  MONITOR_TAG_DATA_CCINDEX_TM_OUTLANE = 3033; //雷达信控指标-动态:实时出口通道数据 最小周期1s, [异常]超过3s没有上报
+  MONITOR_TAG_DATA_CCINDEX_TM_STATISTICS = 3034; //雷达信控指标-动态: 定时统计数据 最小周期1s, [异常]超过3s没有上报
+  MONITOR_TAG_DATA_CCINDEX_TM_EVALUATIONS = 3035; //雷达信控指标-动态: 定时评价数据 最小周期1s, [异常]超过3s没有上报
+  MONITOR_TAG_DATA_CCINDEX_TM_NONMOTOR = 3036; //雷达信控指标-动态: 定时行人及非机动车数据 最小周期1s, [异常]超过3s没有上报
+  MONITOR_TAG_DATA_CCINDEX_TM_DEVICESTATUS = 3037; //雷达信控指标-动态: 实时设备状态 最小周期1s, [异常]超过3s没有上报
+
+  MONITOR_TAG_DATA_CCINDEX_ST_QUERY_ACK = 3038; //雷达信控指标-静态: 周期5分钟，[异常]超过15分钟没有上报
+  MONITOR_TAG_DATA_CCINDEX_ST_UPDATE = 3039; //雷达信控指标-静态: 周期5分钟，[异常]超过15分钟没有上报
+
+  MONITOR_TAG_DATA_RADAR_DATA = 3040; //毫米波雷达原始数据: 周期目前接口没有要求，如果定义推送周期最大不超过30分钟，则1小时未推送为故障 [恢复]1个小时内恢复推送
+  MONITOR_TAG_DATA_LIDAR_DATA = 3041; //激光雷达原始数据
+  MONITOR_TAG_LINK_RSAP = 3042; //云控链路状态:TCP	TCP连接状态异常
+  MONITOR_TAG_LINK_CCINDEX_ST = 3043; // 信控链路状态-静态	HTTP	http连接异常
+  MONITOR_TAG_LINK_CCINDEX_TM = 3044; //  信控链路状态-动态	MQTT	mqtt链路异常
+  MONITOR_TAG_LINK_DAR = 3045;  //点云：FTP	ftp链路异常
 };
 
 ///////////////////////////////////////////////////////
@@ -21,17 +34,20 @@ enum MonitorMecTag
 message MonitorData { //数据监控
   required MonitorMecTag tag = 1;  //消息类型
   required uint64 timestamp = 2;  //utc时间戳: 发送监控数据包时的时间戳
+  optional string device_id = 3;  // [新增] 用于区分不同的雷达设备
 }
 message MonitorDataCcindexTm { //数据监控
   required MonitorMecTag tag = 1;  //消息类型
   required uint64 timestamp = 2;  //utc时间戳: 发送监控数据包时的时间戳
-  optional string topic = 3;  // [新增] 增加 topic 字段，用于区分不同的雷达设备 (device_id)
+  optional string device_id = 3;  // [新增] 用于区分不同的雷达设备
+  optional string topic = 4;  // [新增] 增加 topic 字段，用于区分不同的雷达设备 (device_id)
 }
 
 message MonitorDataCcindexSt { //数据监控
   required MonitorMecTag tag = 1;  //消息类型
   required uint64 timestamp = 2;  //utc时间戳: 发送监控数据包时的时间戳
-  optional string topic = 3;  // [新增] 增加 topic 字段，用于区分不同的雷达设备 (device_id)
+  optional string device_id = 3;  // [新增] 用于区分不同的雷达设备
+  optional string topic = 4;  // [新增] 增加 topic 字段，用于区分不同的雷达设备 (device_id)
 }
 
 message MonitorLink { // 链路状态-静态
diff --git a/middleware/protocol/radar_point_cloud/DianYunRecorder.py b/middleware/protocol/radar_point_cloud/DianYunRecorder.py
index 027fd9d..bb1fd3a 100644
--- a/middleware/protocol/radar_point_cloud/DianYunRecorder.py
+++ b/middleware/protocol/radar_point_cloud/DianYunRecorder.py
@@ -65,7 +65,7 @@ def cyber_talker_init():
     try:
         cyber.init("monitor_mec_radar_point_cloud_py")
         node = cyber.Node("node_monitor_mec_radar_point_cloud")
-        writer = node.create_writer("/v2x/monitor/mec", MonitorMec, 6)
+        writer = node.create_writer("/v2x/mec/om/check/rcp_", MonitorMec, 6)
         return node, writer
     except Exception as e:
         print(f"Cyber init failed: {e}")
@@ -247,7 +247,8 @@ class TCPReceiver:
                             msg = MonitorMec()
                             msg.md_lidar_cp.tag = MonitorMecTag.MONITOR_TAG_MONITOR_IN_LIDAR_DATA
                             msg.md_lidar_cp.timestamp = int(now * 1000)
-                            with CYBER_WRITE_LOCK:
+                            msg.md_lidar_cp.device_id = self.sn
+                        with CYBER_WRITE_LOCK:
                                 self.cyber_writer.write(msg)
                             self.last_cyber_msg_time = now
                         except Exception as e:
diff --git a/middleware/protocol/rsap/rsap_component.cc b/middleware/protocol/rsap/rsap_component.cc
index 858fa71..0118e17 100644
--- a/middleware/protocol/rsap/rsap_component.cc
+++ b/middleware/protocol/rsap/rsap_component.cc
@@ -2722,20 +2722,20 @@ bool RSAP_COMPONENT::writeDeviceNoInfoToFile(std::string mecSn, std::string mecE
 void RSAP_COMPONENT::pushMonitorMecLinkStatus(bool con_flag)
 {
     auto* ml_rsap =  mec_monitor_->mutable_ml_rsap();
-    ml_rsap->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_SENSOR_CLOUD);
+    ml_rsap->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_RSAP);
     ml_rsap->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
     ml_rsap->set_con_flag(con_flag);
     if (m_CloudServiceConfig.enablePrintInfoLog)
     {
-        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_->ShortDebugString();
+        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/rsap/data_,[data]" << mec_monitor_->ShortDebugString();
     }
-    Send("/v2x/monitor/mec", mec_monitor_);
+    Send("/v2x/mec/om/check/rsap/data_", mec_monitor_);
 }
 
 void RSAP_COMPONENT::monitorMecLinkStatus()
 {
     auto* ml_rsap =  mec_monitor_->mutable_ml_rsap();
-    ml_rsap->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_SENSOR_CLOUD);
+    ml_rsap->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_RSAP);
     ml_rsap->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
     if (m_ConnetedFlag  && m_CertVerifyFlag)
     {
@@ -2747,8 +2747,8 @@ void RSAP_COMPONENT::monitorMecLinkStatus()
     }
     if (m_CloudServiceConfig.enablePrintInfoLog)
     {
-        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_->ShortDebugString();
+        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/rsap/link_,[data]" << mec_monitor_->ShortDebugString();
     }
-    Send("/v2x/monitor/mec", mec_monitor_);
+    Send("/v2x/mec/om/check/rsap/link_", mec_monitor_);
 }
 NAMESPACE_PROTOCOL_THREAD_END
```
