# fix(运维):【bug24320】【顺义车网][智路os运维3.4) ota不能进行升级

- 作者：zhangenwei
- 日期：2026-03-05
- 完整 hash：`f1fc634643a01affeba57ac38d0de85438728218`
- 改动：+201 / -212 行

```diff
@@@C	zhangenwei	f1fc634643a01affeba57ac38d0de85438728218	2026-03-05	fix(运维):【bug24320】【顺义车网][智路os运维3.4) ota不能进行升级
diff --git a/middleware/protocol/om/configer/configer_om_mec.h b/middleware/protocol/om/configer/configer_om_mec.h
index 679bda9..bc3e657 100644
--- a/middleware/protocol/om/configer/configer_om_mec.h
+++ b/middleware/protocol/om/configer/configer_om_mec.h
@@ -372,12 +372,18 @@ struct OtaParamConfiger : public afl::base::SerializableData
     bool                airosVersionTarFlag = false;
     std::string         airosVersionSeqNum = "";
     std::string         airosVersionFromFile = "";
+    // std::string         airosVersionFromOtaDownData = ""; //ota版本：来自ota升级命令中
+    // std::string         airosVersionUsing = ""; //ota版本：正在使用中，第一次等于airosVersionFromFile，
+    // //后期等于升级成功后的airosVersionFromOtaDownData
 private:
     virtual void serialize(json &j) override
     {
         JsonSerialize(airosVersionTarFlag, "A_airosVersionTarFlag", j, false);
         JsonSerialize(airosVersionSeqNum, "B_airosVersionSeqNum", j, false);
         JsonSerialize(airosVersionFromFile, "C_airosVersionFromFile", j, false);
+
+        // JsonSerialize(airosVersionFromOtaDownData, "D_airosVersionFromOtaDownData", j, false);
+        // JsonSerialize(airosVersionUsing, "E_airosVersionUsing", j, false);
     }
 
     virtual void deserialize(const json &j) override
@@ -385,6 +391,9 @@ private:
         JsonDeserialize(airosVersionTarFlag, "A_airosVersionTarFlag", j, noUse_isEmptyFlag);
         JsonDeserialize(airosVersionSeqNum, "B_airosVersionSeqNum", j, noUse_isEmptyFlag);
         JsonDeserialize(airosVersionFromFile, "C_airosVersionFromFile", j, noUse_isEmptyFlag);
+
+        // JsonDeserialize(airosVersionFromOtaDownData, "D_airosVersionFromOtaDownData", j, noUse_isEmptyFlag);
+        // JsonDeserialize(airosVersionUsing, "E_airosVersionUsing", j, noUse_isEmptyFlag);
     }
 public:
     std::string to_string() const {
diff --git a/middleware/protocol/om/data_model/mec/data_mec_self_check.h b/middleware/protocol/om/data_model/mec/data_mec_self_check.h
new file mode 100644
index 0000000..7e33d1f
--- /dev/null
+++ b/middleware/protocol/om/data_model/mec/data_mec_self_check.h
@@ -0,0 +1,51 @@
+/*********************************************************************************
+* @file		maintenance_management_ota_data.h
+ * @brief		maintenance_management_ota_data belongs to CICTCI
+ * @details
+ * @author		alfred
+ * @email       zhangenwei64@gmail.com
+ * @date		24-4-21
+ * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
+ * @verbatim
+ *
+ *  Change History:
+ *  Date      Author    Version  ChangeId           Description
+ *  ------------------------------------------------------------------------------
+ *  24-4-21 alfred       1.0       ————             Create this file
+ *
+ * @endverbatim
+ ********************************************************************************/
+
+#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_MEC_SELF_CHECK_H
+#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_MEC_SELF_CHECK_H
+#include "middleware/protocol/om_common/data_model/data_common.h"
+NAMESPACE_START_OM_COMPONENT_MEC
+using namespace os::v2x::protocol::om::common;
+
+struct TopicCCIndexTm
+{
+    std::string Topic_Trajectories  = "trajectories/"; // mec向平台发送 实时轨迹数据
+    std::string Topic_VehiclePass   = "vehiclePass/";   // mec向平台发送 实时过车数据
+    std::string Topic_QueueUp       = "queueUp/";       // mec向平台发送 实时排队数据
+    std::string Topic_AreaState     = "areaState/";// mec向平台发送 实时区域状态数据
+    std::string Topic_Overflow      = "overflow/";// mec向平台发送 实时溢出数据
+    std::string Topic_Outlane       = "outlane/";// mec向平台发送 实时出口通道数据
+    std::string Topic_Statistics    = "statistics/";// mec向平台发送 定时统计数据
+    std::string Topic_Evaluations   = "evaluations/";// mec向平台发送 定时评价数据
+    std::string Topic_Nonmotor      = "nonmotor/";// mec向平台发送 定时评价数据
+    std::string Topic_DeviceStatus = "deviceStatus/";// mec向平台发送 设备状态数据
+    std::string Topic_Pulse         = "pulse/";   // mec向平台发送, 实时脉冲数据
+
+    std::string Topic_Static_Query_Ack = "query/ack/";
+    std::string Topic_Static_Update_Ack = "update/ack/";
+};
+
+
+struct TopicCCIndexSt
+{
+    std::string Topic_Static_Query_Ack = "query/ack/";
+    std::string Topic_Static_Update_Ack = "update/ack/";
+};
+
+NAMESPACE_ENDED_OM_COMPONENT_MEC
+#endif
\ No newline at end of file
diff --git a/middleware/protocol/om/om_component.cc b/middleware/protocol/om/om_component.cc
index 4411591..d4fbd1b 100644
--- a/middleware/protocol/om/om_component.cc
+++ b/middleware/protocol/om/om_component.cc
@@ -1936,7 +1936,12 @@ bool OM_COMPONENT::V2xPbAsnMessageFrame2StrBroad(const std::shared_ptr<const v2x
 }
 void OM_COMPONENT::periodMonitorOmStatus()
 {
-    if (output_monitor_) {
+    if (!getConfiger().configerMec.configerEnable.Enable_Monitor_Mec_Om_Status)
+    {
+        return;
+    }
+    if (output_monitor_)
+    {
         output_monitor_->Clear();
     }
     auto* om_response =  output_monitor_->mutable_om_response();
@@ -3172,14 +3177,18 @@ void OM_COMPONENT::onConnectedPushMsgOnce()
  */
 bool OM_COMPONENT::checkOtaUpdateVersion()
 {
-    if (!m_MqttConnected)
+    if (!m_MqttConnectedCloud)
     {
 //        OM_MEC_ERROR_PRINT << "[error]mqtt not connected!";
         return false;
     }
+    if (!getConfiger().configerMec.configerEnable.Enable_Ota_Down)
+    {
+        return false;
+    }
     if(getConfiger().enableDebugPrint)
     {
-        OM_MEC_DEBUG_PRINT << "[update][softwareVersion]" << getConfiger().configerMec.configerOmCommon.softwareVersion
+         OM_MEC_DEBUG_PRINT << "[update][softwareVersion]" << getConfiger().configerMec.configerOmCommon.softwareVersion
         <<  "[airosVersionFromFile]"  << getConfiger().configerMec.configerOtaParam.airosVersionFromFile
         <<  "[airosVersionTarFlag]" << getConfiger().configerMec.configerOtaParam.airosVersionTarFlag
          << "[airosVersionSeqNum]" << getConfiger().configerMec.configerOtaParam.airosVersionSeqNum;
@@ -3618,11 +3627,6 @@ void OM_COMPONENT::subscribeOtaCancelData(const json &contentJson)
     }
     std::string hint;
     OtaUpdateCancelData otaUpdateCancelData;
-    OTAUpdateStatusUpData otaUpdateStatusUpData;
-    otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
-    otaUpdateStatusUpData.code  = FAILURE;
-    otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
-    otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;
     try
     {
         otaUpdateCancelData = contentJson;
@@ -3630,50 +3634,19 @@ void OM_COMPONENT::subscribeOtaCancelData(const json &contentJson)
     catch (afl::base::json::exception &e)
     {
         OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
-        hint = "parse ota-cancel-json error！";
-        otaUpdateStatusUpData.seqNum =  otaUpdateCancelData.seqNum;
-        otaUpdateStatusUpData.rscuEsn = otaUpdateCancelData.rscuEsn;
-        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status ,otaUpdateStatusUpData, hint);
         return;
     }
-    // ---------------------------------------------------------
-    // 检测：是否正在升级 & 执行取消逻辑
-    // ---------------------------------------------------------
 
-    // 1. 校验 ESN 
+    // 1. 校验 ESN
     if (otaUpdateCancelData.rscuEsn != getConfiger().rscuEsn)
     {
         OM_MEC_WARN_PRINT << "[cancel] ESN mismatch. Recv: " << otaUpdateCancelData.rscuEsn;
         return;
     }
     // 2. 执行核心取消操作
-
-    // A. 通知下载线程停止 (利用之前定义的 shared_ptr<atomic>)
-    // 只要设置了这个，线程里的 curl 回调就会返回非0，线程就会退出，且不会去改成功标志
-    if (m_CancelDownloadFlag)
-    {
-        *m_CancelDownloadFlag = true;
-    }
-    // B. 清理本地状态
-    {
-        std::unique_lock<std::mutex> locker(m_MutexDownload);
-        m_IsGetDownResponseSucess = false;
-    }
-    // C. (可选) 删除可能下载了一半的文件
-    std::string downloadTarRootPath = getConfiger().configerProjectPath.projectRoot + "/" + getConfiger().configerProjectPath.otaDirName;
-    std::string tarFilePath = downloadTarRootPath + "/" + getConfiger().configerProjectPath.otaFileName;
-    remove(tarFilePath.c_str());
-
-    // 3. 上报“已取消”状态给云端
-    otaUpdateStatusUpData.seqNum = otaUpdateCancelData.seqNum;
-    otaUpdateStatusUpData.rscuEsn = otaUpdateCancelData.rscuEsn;
-    otaUpdateStatusUpData.code = FAILURE; // 建议：如果协议支持，最好定义一个 CANCELED = 2 之类的状态
-    hint = "Update canceled by user command.";
-    otaUpdateStatusUpData.description = hint;
-
-    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
-
-    OM_MEC_SUCESS_PRINT << "[cancel] OTA Cancelled Successfully.";
+    m_CancelDownloadFlag.store(true, std::memory_order_relaxed);
+    m_UpdateVersionFromCancleMsg = otaUpdateCancelData.updateVersion;
+    OM_MEC_SUCCESS_PRINT << "[cancel] OTA Cancelled Successfully.";
 }
 
 
@@ -3743,7 +3716,7 @@ void OM_COMPONENT::subscribeOtaDownData(const json &contentJson)
         OM_MEC_DEBUG_PRINT << hint;
         return;
     }
-
+    m_UpdateVersionFromDownMsg =  otaUpdateDownData.updateVersion;
     m_DownloadHttpHandle = curl_easy_init();
     m_DownloadChkPara = otaUpdateDownData.downloadChkPara;
     uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
@@ -3757,7 +3730,7 @@ void OM_COMPONENT::subscribeOtaDownData(const json &contentJson)
         {
             OM_MEC_DEBUG_PRINT << "[updateTime]start update thread!";
         }
-        std::thread th(std::bind(&OM_COMPONENT::threadDownloadOtaFile, this, this, otaUpdateDownData, m_CancelDownloadFlag));
+        std::thread th(std::bind(&OM_COMPONENT::threadDownloadOtaFile, this, this, otaUpdateDownData));
         th.detach();
     }
     else if(otaUpdateDownData.updateTime <  0)
@@ -3795,14 +3768,8 @@ bool OM_COMPONENT::processOtaUpdateDownData(OTAUpdateDownData& otaUpdateDownData
         m_EventloopOmMec->cancelTimer(m_TimerOtaUpdateDownData);
         m_TimerOtaUpdateDownData = -1;
     }
-    // std::thread th(std::bind(&OM_COMPONENT::threadDownloadOtaFile, this, this, otaUpdateDownData));
-    // th.detach();
-    // 【新增】初始化取消标志
-    m_CancelDownloadFlag = std::make_shared<std::atomic<bool>>(false);
-
-    // 【修改】将 m_CancelDownloadFlag 传递给线程
-    // 注意：这里必须按值传递 shared_ptr，增加引用计数，防止主线程释放后悬空
-    std::thread th(std::bind(&OM_COMPONENT::threadDownloadOtaFile, this, this, otaUpdateDownData, m_CancelDownloadFlag));
+    std::thread th(std::bind(&OM_COMPONENT::threadDownloadOtaFile, this, this, otaUpdateDownData));
+    th.detach();
     return true;
 }
 std::string OM_COMPONENT::calculateMD5(const std::string& filename)
@@ -3847,11 +3814,11 @@ void  OM_COMPONENT::monitorDeviceNeedUpdate()
     {
         return;
     }
-    // 【新增】双重保险：如果当前正在取消中，不要处理任何逻辑
-    if (m_CancelDownloadFlag && *m_CancelDownloadFlag)
+    if (!getConfiger().configerMec.configerEnable.Enable_Ota_Down)
     {
-        return;
+        return ;
     }
+
     bool m_IsGetDownResponseSucessMain;
     std::string otaSeqNumTemp = "";
     std::unique_lock<std::mutex> locker(m_MutexDownload);
@@ -3865,82 +3832,129 @@ void  OM_COMPONENT::monitorDeviceNeedUpdate()
                                            + "/" +  getConfiger().configerProjectPath.otaDirName;
         std::string tarFilePath = downloadTarRootPath + "/" +  getConfiger().configerProjectPath.otaFileName;
 
-        std::string tarXvfPath = "/home/airos";
 
-        std::string tarMd5Value = calculateMD5(tarFilePath.c_str());
-        if(getConfiger().enableDebugPrint)
-        {
-            OM_MEC_DEBUG_PRINT << "[update][md5]" << tarMd5Value << "[m_DownloadChkPara]" << m_DownloadChkPara;
-        }
-        if(m_DownloadChkPara == tarMd5Value)
+        //取消升级
+        bool val = m_CancelDownloadFlag.load(std::memory_order_relaxed);
+        if (val)
         {
-            m_Md5NoEqualPushOnced = false;
-            OM_MEC_DEBUG_PRINT << "[update]md5 equal!";
-            if(m_OtaIsExecBashOnce == false)
+            if (m_UpdateVersionFromDownMsg !=  m_UpdateVersionFromCancleMsg)
+            {
+                OM_MEC_ERROR_PRINT << "[error] UpdateVersion is not equal!";
+                return ;
+            }
+            if (-1 == remove(tarFilePath.c_str()))
+            {
+                OM_MEC_ERROR_PRINT << "[error] Failed to delete existing file!";
+            }
+            else
             {
                 if(getConfiger().enableDebugPrint)
                 {
-                    OM_MEC_DEBUG_PRINT << "[notice]just exec once!";
+                    OM_MEC_DEBUG_PRINT << "[success] Successfully deleted existing file!";
+                    return ;
                 }
-                m_OtaIsExecBashOnce = true;
-                std::string command_tar = "cd /home/airos/ && tar xvf " + tarFilePath;
-                int ret = system(command_tar.c_str());
-                if (ret == 0)
-                {
+            }
+        }
+        else
+        {
+            //正常升级
+            std::string tarXvfPath = "/home/airos";
 
-                    if( afl::FileUtil::isFileExist(getConfiger().configerWorkParam.configerVersionFilePath.c_str()))
+            std::string tarMd5Value = calculateMD5(tarFilePath.c_str());
+            if(getConfiger().enableDebugPrint)
+            {
+                OM_MEC_DEBUG_PRINT << "[update][md5]" << tarMd5Value << "[m_DownloadChkPara]" << m_DownloadChkPara;
+            }
+            if(m_DownloadChkPara == tarMd5Value)
+            {
+                m_Md5NoEqualPushOnced = false;
+                OM_MEC_DEBUG_PRINT << "[update]md5 equal!";
+                if(m_OtaIsExecBashOnce == false)
+                {
+                    if(getConfiger().enableDebugPrint)
+                    {
+                        OM_MEC_DEBUG_PRINT << "[notice]just exec once!";
+                    }
+                    m_OtaIsExecBashOnce = true;
+                    std::string command_tar = "cd /home/airos/ && tar xvf " + tarFilePath;
+                    int ret = system(command_tar.c_str());
+                    if (ret == 0)
                     {
-                        std::ifstream softwareVersionFile(getConfiger().configerWorkParam.configerVersionFilePath.c_str());
-                        if (softwareVersionFile)
+
+                        if( afl::FileUtil::isFileExist(getConfiger().configerWorkParam.configerVersionFilePath.c_str()))
                         {
-                            // 读取第一行内容
-                            std::string line;
-                            if (std::getline(softwareVersionFile, line))
+                            std::ifstream softwareVersionFile(getConfiger().configerWorkParam.configerVersionFilePath.c_str());
+                            if (softwareVersionFile)
                             {
-                                if(getConfiger().enableDebugPrint)
-                                {
-                                    OM_MEC_DEBUG_PRINT << "[update][notice]read sucess!";
-                                }
-                                OM_MEC_DEBUG_PRINT <<  "[update][airosVersionFromFile]" <<  line
-                                << "[airosVersionTarFlag]" <<  getConfiger().configerMec.configerOtaParam.airosVersionTarFlag;
-                                getConfiger().configerMec.configerOtaParam.airosVersionFromFile = line;
-                                getConfiger().configerMec.configerOtaParam.airosVersionTarFlag = true;
-                                getConfiger().configerMec.configerOtaParam.airosVersionSeqNum = m_OtaDownSeqnum;
-                                saveConfiger();
-                                std::string command_start = "cd /home/airos/ && launch all";
-                                int ret = system(command_start.c_str());
-                                if (ret == 0)
+                                // 读取第一行内容
+                                std::string line;
+                                if (std::getline(softwareVersionFile, line))
                                 {
+                                    if(getConfiger().enableDebugPrint)
+                                    {
+                                        OM_MEC_DEBUG_PRINT << "[update][notice]read success!";
+                                        OM_MEC_DEBUG_PRINT <<  "[update][airosVersionFromFile]" <<  line
+                                        << "[airosVersionTarFlag]" <<  getConfiger().configerMec.configerOtaParam.airosVersionTarFlag;
+                                    }
+
+                                    getConfiger().configerMec.configerOtaParam.airosVersionFromFile = line;
+                                    getConfiger().configerMec.configerOtaParam.airosVersionTarFlag = true;
+                                    getConfiger().configerMec.configerOtaParam.airosVersionSeqNum = m_OtaDownSeqnum;
+                                    saveConfiger();
+                                    std::string command_start = "cd /home/airos/ && launch all";
+                                    int ret = system(command_start.c_str());
+                                    if (ret == 0)
+                                    {
                                         if(getConfiger().enableDebugPrint)
                                         {
                                             OM_MEC_DEBUG_PRINT <<  "[update][success] Sucess to execute command: " << command_start;
                                         }
-                                    m_OtaIsExecBashOnce = true;
-                                }
-                                else
-                                {
-                                    OM_MEC_ERROR_PRINT <<  "[update][error] Failed to execute command: " << command_start;
+                                        m_OtaIsExecBashOnce = true;
+                                    }
+                                    else
+                                    {
+                                        OM_MEC_ERROR_PRINT <<  "[update][error] Failed to execute command: " << command_start;
+                                    }
                                 }
                             }
+                            else
+                            {
+                                OM_MEC_ERROR_PRINT << "[update][error]read file failure!";
+                            }
                         }
                         else
                         {
-                            OM_MEC_ERROR_PRINT << "[update][error]read file failure!";
+                            OM_MEC_ERROR_PRINT << "[update][error]file not exsit!";
                         }
+
                     }
                     else
                     {
-                        OM_MEC_ERROR_PRINT << "[update][error]file not exsit!";
+                        OM_MEC_ERROR_PRINT <<  "[update][error] Failed to execute command: " << command_tar;
+                        if(getConfiger().enableDebugPrint)
+                        {
+                            OM_MEC_SUCCESS_PRINT << "[update][error] Failed to execute command: tar";
+                        }
+                        OTAUpdateStatusUpData otaUpdateStatusUpData;
+                        otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
+                        otaUpdateStatusUpData.seqNum =  m_OtaDownSeqnum;
+                        otaUpdateStatusUpData.rscuEsn = getConfiger().rscuEsn;
+                        otaUpdateStatusUpData.code  = FAILURE;
+                        otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
+                        otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;
+
+                        std::string hint = "tar xvf failure!";
+                        otaUpdateStatusUpData.description = hint;
+                        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
                     }
 
                 }
-                else
+            }
+            else
+            {
+                if(!m_Md5NoEqualPushOnced)
                 {
-                    OM_MEC_ERROR_PRINT <<  "[update][error] Failed to execute command: " << command_tar;
-                    if(getConfiger().enableDebugPrint)
-                    {
-                            OM_MEC_SUCCESS_PRINT << "[update][error] Failed to execute command: tar";
-                    }
+                    OM_MEC_ERROR_PRINT << "[update][error] md5 is not equal！";
                     OTAUpdateStatusUpData otaUpdateStatusUpData;
                     otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
                     otaUpdateStatusUpData.seqNum =  m_OtaDownSeqnum;
@@ -3949,34 +3963,16 @@ void  OM_COMPONENT::monitorDeviceNeedUpdate()
                     otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
                     otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;
 
-                    std::string hint = "tar xvf failure!";
+                    std::string hint = "md5 no equal!";
                     otaUpdateStatusUpData.description = hint;
                     mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
+                    m_Md5NoEqualPushOnced = true;
+                    return ;
                 }
 
             }
         }
-        else
-        {
-            if(!m_Md5NoEqualPushOnced)
-            {
-                OM_MEC_ERROR_PRINT << "[update][error] md5 is not equal！";
-                OTAUpdateStatusUpData otaUpdateStatusUpData;
-                otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
-                otaUpdateStatusUpData.seqNum =  m_OtaDownSeqnum;
-                otaUpdateStatusUpData.rscuEsn = getConfiger().rscuEsn;
-                otaUpdateStatusUpData.code  = FAILURE;
-                otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
-                otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;
-
-                std::string hint = "md5 no equal!";
-                otaUpdateStatusUpData.description = hint;
-                mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
-                m_Md5NoEqualPushOnced = true;
-                return ;
-            }
 
-        }
         m_IsGetDownResponseSucessMain = false;
     }
     else
@@ -4008,14 +4004,9 @@ void OM_COMPONENT::handleErrorResponse(CURLcode code)
             OM_MEC_ERROR_PRINT << "[error] Request failed! Response code: [" << code << "]";
     }
 }
-// void OM_COMPONENT::threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUpdateDownData)
-void OM_COMPONENT::threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUpdateDownData, std::shared_ptr<std::atomic<bool>> cancelFlag)
+void OM_COMPONENT::threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUpdateDownData)
 {
     try {
-
-        // 0. 刚进来就检查一下
-        if (*cancelFlag) return;
-
         struct curl_slist *headers = NULL;
         std::string httpQueryResponseStr;
         OTAUpdateDownData otaUpdateDownDataTemp = otaUpdateDownData;
@@ -4034,21 +4025,11 @@ void OM_COMPONENT::threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUp
         {
             curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_WRITEFUNCTION, curlWriteCallback);
             curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_WRITEDATA, (void *)&httpQueryResponseStr);
-            // 【新增】设置进度回调，用于打断下载
-            curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_NOPROGRESS, 0L); // 开启进度功能
-            curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_PROGRESSFUNCTION, curlProgressCallback);
-            curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_PROGRESSDATA, cancelFlag.get()); // 传入 atomic 指针
+
             // Retry loop for 3 attempts
             const int maxAttempts = 3;
             for (int attempt = 0; attempt < maxAttempts; ++attempt)
             {
-                // 【新增】每次重试前检查是否已取消
-                if (*cancelFlag)
-                {
-                    OM_MEC_DEBUG_PRINT << "[notice] Download cancelled by user.";
-                    curl_slist_free_all(headers); // 记得清理资源
-                    return;
-                }
                 // Set the Authorization header
                 std::string tokenDataHead = "Authorization: " + otaUpdateDownDataTemp.token;
                 headers = curl_slist_append(headers, tokenDataHead.c_str());
@@ -4060,14 +4041,6 @@ void OM_COMPONENT::threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUp
 
                 // Perform the request
                 m_DownloadRepCode = curl_easy_perform(m_DownloadHttpHandle);
-
-                // 【新增】如果是被用户取消导致的失败，直接退出循环，不要重试
-                if (m_DownloadRepCode == CURLE_ABORTED_BY_CALLBACK || *cancelFlag)
-                {
-                    OM_MEC_DEBUG_PRINT << "[notice] Download aborted.";
-                    curl_slist_free_all(headers);
-                    return;
-                }
                 if (CURLE_OK == m_DownloadRepCode)
                 {
                     break; // Exit loop on success
@@ -4086,6 +4059,7 @@ void OM_COMPONENT::threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUp
                 curl_easy_cleanup(m_DownloadHttpHandle);
                 httpQueryResponseStr.clear();
                 std::unique_lock<std::mutex> locker(m_MutexDownload);
+
                 m_IsGetDownResponseSucess = false;
                 if(getConfiger().enableDebugPrint)
                 {
@@ -4094,13 +4068,7 @@ void OM_COMPONENT::threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUp
                 m_OtaSeqNum = otaUpdateDownDataTemp.seqNum;
                 return;
             }
-            // 【新增】在写文件之前，再次检查取消标志
-            // 防止下载刚完成的那一瞬间用户点了取消，却还是把文件写进去了
-            if (*cancelFlag)
-            {
-                curl_slist_free_all(headers);
-                return;
-            }
+
             // File storage logic
             std::string downloadTarRootPath = getConfiger().configerProjectPath.projectRoot + "/" + getConfiger().configerProjectPath.otaDirName;
             std::string tarFilePath = downloadTarRootPath + "/" + getConfiger().configerProjectPath.otaFileName;
@@ -4142,30 +4110,19 @@ void OM_COMPONENT::threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUp
             // Check file size and set status
             if (getFileSize(tarFilePath) > 100 * 1024 * 1024)
             {
-                // 【关键修改】最后一步，只有没取消才通知主进程
-                if (!*cancelFlag)
-                {
-                    if(getConfiger().enableDebugPrint) {
-                        OM_MEC_DEBUG_PRINT << "[success] File stored successfully!";
-                    }
-                    std::unique_lock<std::mutex> locker(m_MutexDownload);
-                    m_IsGetDownResponseSucess = true; // 只有这里设为 true，定时器才会工作
-                }
-                else
+                if(getConfiger().enableDebugPrint)
                 {
-                    // 如果取消了，最好把刚才写了一半或者刚写完的文件删掉
-                    remove(tarFilePath.c_str());
+                    OM_MEC_DEBUG_PRINT << "[success] File stored successfully!";
                 }
-                // if(getConfiger().enableDebugPrint)
-                // {
-                //     OM_MEC_DEBUG_PRINT << "[success] File stored successfully!";
-                // }
-                // std::unique_lock<std::mutex> locker(m_MutexDownload);
-                // m_IsGetDownResponseSucess = true;
+                std::unique_lock<std::mutex> locker(m_MutexDownload);
+                m_IsGetDownResponseSucess = true;
             }
             else
             {
-                OM_MEC_DEBUG_PRINT << "[error] File size too small, please check the response content!";
+                if(getConfiger().enableDebugPrint)
+                {
+                    OM_MEC_DEBUG_PRINT << "[error] File size too small, please check the response content!";
+                }
             }
 
             // Free the headers list
@@ -6802,6 +6759,10 @@ bool OM_COMPONENT::getMqttConfigerMsg()
     //camera
     if(getConfiger().configerEnable.Enable_Camera)
     {
+        if(getConfiger().enableDebugPrint)
+        {
+            OM_CLOUD_SUCCESS_PRINT << "Get om-camera Topic!";
+        }
         std::pair<std::string, CameraRegisterMapConfiger> registerPair;
         for(uint32_t i = 0; i < getConfiger().configerCamera.configerTopicRcId.RcIds.size(); i++)
         {
@@ -10768,32 +10729,7 @@ void  OM_COMPONENT::getTimingInfo()
     close(sockfd);
 
 }
-// 静态回调函数
-int OM_COMPONENT::curlProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
-{
-    auto pCancelFlag = static_cast<std::atomic<bool>*>(clientp);
-    if (pCancelFlag && *pCancelFlag)
-    {
-        // 返回非0值，通知 curl 终止传输
-        return 1;
-    }
-    return 0;
-}
-void OM_COMPONENT::cancelDownload()
-{
-    // 1. 设置标志位，通知线程停止
-    if (m_CancelDownloadFlag) {
-        *m_CancelDownloadFlag = true;
-    }
 
-    // 2. 逻辑上重置成功标志，防止定时器误判
-    {
-        std::unique_lock<std::mutex> locker(m_MutexDownload);
-        m_IsGetDownResponseSucess = false;
-    }
-
-    OM_MEC_DEBUG_PRINT << "[cmd] Cancel download command received.";
-}
 
 std::string OM_COMPONENT::getTLFaultDesc(TrafficlightDetectStatus tlds)
 {
@@ -10835,6 +10771,4 @@ std::string OM_COMPONENT::getTLFaultDesc(TrafficlightDetectStatus tlds)
     }
     return tldsStr;
 }
-
-
 NAMESPACE_ENDED_OM
diff --git a/middleware/protocol/om/om_component.h b/middleware/protocol/om/om_component.h
index 04fdb33..f167d1c 100644
--- a/middleware/protocol/om/om_component.h
+++ b/middleware/protocol/om/om_component.h
@@ -564,9 +564,7 @@ private:
     void subscribeOtaDownData(const json &contentJson);
     void publishOtaStatusData();
     void subscribeOtaStatusAckData();
-    // void threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUpdateDownData);
-    // 修改：线程函数签名，增加 cancelFlag 参数
-    void threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUpdateDownData, std::shared_ptr<std::atomic<bool>> cancelFlag);
+    void threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUpdateDownData);
     void monitorDeviceNeedUpdate();
     long long getFileSize(const std::string& path);
     void handleErrorResponse(CURLcode code);
@@ -839,13 +837,10 @@ private:
     void subscribeOtaCancelData(const json &contentJson);
     std::string m_UsingVersion = "";
 
-    // 新增：取消下载的接口
-    void cancelDownload();
     // 新增：用于控制取消的共享标志
-    std::shared_ptr<std::atomic<bool>> m_CancelDownloadFlag;
-
-    // 新增：Curl的进度回调函数 (必须是 static 或全局函数)
-    static int curlProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
+    std::atomic<bool>  m_CancelDownloadFlag{false};
+    std::string m_UpdateVersionFromCancleMsg = ""; //取消升级的版本号
+    std::string m_UpdateVersionFromDownMsg = ""; //升级版本号来自云端下发
 
     void  initProcessMonitorMecOutputResult();
     void  processMonitorMecOutputResult(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData);
```
