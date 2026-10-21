# fix(运维):【20260316-zhangenwei-2】修改ccindex中关于mec自检的channel

- 作者：zhangenwei
- 日期：2026-03-16
- 完整 hash：`e1abc59f4bac30e5e9f3bf55b293d29eb4baeaee`
- 改动：+100 / -3 行

```diff
@@@C	zhangenwei	e1abc59f4bac30e5e9f3bf55b293d29eb4baeaee	2026-03-16	fix(运维):【20260316-zhangenwei-2】修改ccindex中关于mec自检的channel
diff --git a/middleware/protocol/ccindex/ccindex_component.cc b/middleware/protocol/ccindex/ccindex_component.cc
index 447aefa..376db2f 100644
--- a/middleware/protocol/ccindex/ccindex_component.cc
+++ b/middleware/protocol/ccindex/ccindex_component.cc
@@ -7580,12 +7580,12 @@ void CCINDEX_COMPONENT::pushMonitorMecDataStatus(airos::monitor_mec::MonitorMecT
     md_ccindex_tc->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
     if(getConfiger().enableDebugPrint)
     {
-        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << mec_monitor_->ShortDebugString();
+        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/tc/data_,[data]" << mec_monitor_->ShortDebugString();
     }
-    Send("/v2x/monitor/mec", mec_monitor_);
+    Send("/v2x/mec/om/check/ccindex/tc/data_", mec_monitor_);
 }
 
-void CCINDEX_COMPONENT::pushMonitorMecDataStatusTm(airos::monitor_mec::MonitorMecTag tag, std::string topic)
+void CCINDEX_COMPONENT::pushMonitorMecDataStatusTm(airos::monitor_mec::MonitorMecTag tag, std::string topic, std::string device_id)
 {
     //airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_TC
     auto* md_ccindex_tm =  mec_monitor_->mutable_md_ccindex_tm();
diff --git a/middleware/protocol/ccindex/configer_ccindex.h b/middleware/protocol/ccindex/configer_ccindex.h
index 7151e98..5ac6a5a 100644
--- a/middleware/protocol/ccindex/configer_ccindex.h
+++ b/middleware/protocol/ccindex/configer_ccindex.h
@@ -53,6 +53,10 @@ struct HttpConfiger : public afl::base::SerializableData
     uint16_t    mecMqttClientQueryConfigDataPeriod = 10;  //mqtt-client 推送查询数据周期
     //////////////////////////
     //本机作为client
+
+
+
+
     std::string httpCloudServerIp = "172.30.1.161";
     uint16_t    httpCloudServerPort = 8298;
     uint16_t    httpHostClientPostConfigUpdateDataPeriod = 10;  //http-client-post方法推送更新数据周期
diff --git a/middleware/protocol/om/configer/configer_om_mec.h b/middleware/protocol/om/configer/configer_om_mec.h
index c5c5ce9..41cc111 100644
--- a/middleware/protocol/om/configer/configer_om_mec.h
+++ b/middleware/protocol/om/configer/configer_om_mec.h
@@ -510,6 +510,94 @@ struct MecSelfCheckTime : public afl::base::SerializableData
     TimeRule  cloud_point_link_tr;
     MecSelfCheckTime()
     {
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
+
 		spat_src_data_tr.timeoutSecond = 5;
         spat_src_data_tr.recoverySecond = 5;
         spat_src_data_tr.enable = true;
@@ -594,6 +682,8 @@ struct MecSelfCheckTime : public afl::base::SerializableData
         cloud_point_link_tr.timeoutSecond = 3600;
         cloud_point_link_tr.recoverySecond = 3600;
         cloud_point_link_tr.enable = true;
+
+
     }
 
 private:
diff --git a/middleware/protocol/om_common/configer_common.h b/middleware/protocol/om_common/configer_common.h
index 8cd80dd..3d8b828 100644
--- a/middleware/protocol/om_common/configer_common.h
+++ b/middleware/protocol/om_common/configer_common.h
@@ -26,6 +26,7 @@ using namespace afl::util;
 using namespace airos::base::workparam;
 #ifndef SUBSCRIBE_NUM
 #define SUBSCRIBE_NUM 1000
+
 #endif
 struct MqttConfigerCommon : public afl::base::SerializableData
 {
diff --git a/middleware/protocol/om_common/configer_db_device_status.h b/middleware/protocol/om_common/configer_db_device_status.h
index ab6b097..3d32834 100644
--- a/middleware/protocol/om_common/configer_db_device_status.h
+++ b/middleware/protocol/om_common/configer_db_device_status.h
@@ -30,6 +30,8 @@ public:
     std::string DbFileName = "device-status.db";
     std::string DbFilePath = "/home/airos/protocol/device-status.db";
     std::string DbDeviceStatusTableName = "sensor-status";
+
+
     std::string DbSynchronizeTableName = "sensor-synchronize";
     std::string DbMecDeviceStatusTableName = "mec-status";
     std::string DbMecDeviceAlarmTableName = "mec-alarm";
```
