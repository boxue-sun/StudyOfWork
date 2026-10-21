# fix(运维):【20260305-zhangenwei-4】：【顺义车网】【智路os运维3.4】   1.增加相机和雷达升级取消相关逻辑

- 作者：zhangenwei
- 日期：2026-03-05
- 完整 hash：`d4efa1c82eb2f29b6a25f9a08a4199e3862dfa51`
- 改动：+35 / -9 行

```diff
@@@C	zhangenwei	d4efa1c82eb2f29b6a25f9a08a4199e3862dfa51	2026-03-05	fix(运维):【20260305-zhangenwei-4】：【顺义车网】【智路os运维3.4】   1.增加相机和雷达升级取消相关逻辑
diff --git a/middleware/protocol/om/om_component.cc b/middleware/protocol/om/om_component.cc
index bab3f31..e0028db 100644
--- a/middleware/protocol/om/om_component.cc
+++ b/middleware/protocol/om/om_component.cc
@@ -6638,11 +6638,7 @@ bool OM_COMPONENT:: getMqttConfigerMsgCloud()
             topicConfiger.Topic_Config_Down          = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Config_Down        ;
             topicConfiger.Topic_Restar               = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Restar             ;
             topicConfiger.Topic_Upgrade              = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Upgrade            ;
-            // topicConfiger.Topic_InternalExternalParams_Up          = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_InternalExternalParams_Up        ;
-            // topicConfiger.Topic_InternalExternalParams_Up_Ack          = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_InternalExternalParams_Up_Ack        ;
-            // topicConfiger.Topic_InternalExternalParams_Query          = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_InternalExternalParams_Query        ;
-            // topicConfiger.Topic_InternalExternalParams_Query_Down          = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_InternalExternalParams_Query_Down        ;
-
+            topicConfiger.Topic_Upgrade_Cancel              = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Upgrade_Cancel            ;
 
             m_MqttClientConfig.configerCamera.topicUnMap[i] = topicConfiger;
             m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Register_Ack.c_str());
@@ -6650,8 +6646,7 @@ bool OM_COMPONENT:: getMqttConfigerMsgCloud()
             m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Config_Down.c_str());
             m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Restar.c_str());
             m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Upgrade.c_str());
-            // m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_InternalExternalParams_Up_Ack.c_str());
-            // m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_InternalExternalParams_Query_Down.c_str());
+            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Upgrade_Cancel.c_str());
 
         }
     }
@@ -6668,6 +6663,7 @@ bool OM_COMPONENT:: getMqttConfigerMsgCloud()
             topicConfigerOmRadar.Topic_Update_Config         = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Update_Config       ;
             topicConfigerOmRadar.Topic_Restore               = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Restore             ;
             topicConfigerOmRadar.Topic_Ota                   = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Ota                 ;
+            topicConfigerOmRadar.Topic_Ota_Cancel                   = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Ota_Cancel                 ;
             topicConfigerOmRadar.Topic_Reboot                = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Reboot              ;
 
             topicConfigerOmRadar.Topic_InternalExternalParams_Up_Ack           = topicProfix + getConfiger().configerRadar.configerTopic.Topic_InternalExternalParams_Up_Ack         ;
@@ -6682,8 +6678,7 @@ bool OM_COMPONENT:: getMqttConfigerMsgCloud()
             m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Restore.c_str());
             m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Ota.c_str());
             m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Reboot.c_str());
-//            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_InternalExternalParams_Up_Ack.c_str());
-//            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_InternalExternalParams_Query_Down.c_str());
+            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Ota_Cancel.c_str());
 
         }
     }
@@ -6860,6 +6855,7 @@ bool OM_COMPONENT::getMqttConfigerMsg()
             topicConfigerOmCamera.Topic_Restar               = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Restar             ;
             topicConfigerOmCamera.Topic_Restar_Up            = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Restar_Up          ;
             topicConfigerOmCamera.Topic_Upgrade              = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Upgrade            ;
+            topicConfigerOmCamera.Topic_Upgrade_Cancel       = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Upgrade_Cancel            ;
             topicConfigerOmCamera.Topic_UpgradeStatus        = topicProfix + getConfiger().configerCamera.configerTopic.Topic_UpgradeStatus      ;
             topicConfigerOmCamera.Topic_Synchronize          = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Synchronize        ;
 
@@ -6929,6 +6925,7 @@ bool OM_COMPONENT::getMqttConfigerMsg()
             topicConfigerOmRadar.Topic_Warning               = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Warning             ;
             topicConfigerOmRadar.Topic_Heartbeat             = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Heartbeat           ;
             topicConfigerOmRadar.Topic_Ota                   = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Ota                 ;
+            topicConfigerOmRadar.Topic_Ota_Cancel            = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Ota_Cancel                 ;
             topicConfigerOmRadar.Topic_Ota_ACK               = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Ota_ACK             ;
             topicConfigerOmRadar.Topic_Reboot                = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Reboot              ;
             topicConfigerOmRadar.Topic_Reboot_Ack            = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Reboot_Ack          ;
@@ -9556,6 +9553,12 @@ bool OM_COMPONENT::processSubscribeDataRadarFromCloud(const std::string &topic,
             topicPT = mqttTopicConfigerOmRadarTemp.Topic_Ota;
             break;
         }
+        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_Ota_Cancel && m_MqttClientConfig.configerRadar.configerEnable.Enable_Ota)
+        {
+            hint = "ota";
+            topicPT = mqttTopicConfigerOmRadarTemp.Topic_Ota_Cancel;
+            break;
+        }
         else if(topic == mqttTopicConfigerOmRadarTemp.Topic_Reboot && m_MqttClientConfig.configerRadar.configerEnable.Enable_Reboot)
         {
             hint = "reboot";
@@ -9689,6 +9692,12 @@ bool OM_COMPONENT::processSubscribeDataCameraFromCloud(const std::string &topic,
             topicPT = mqttTopicMecCameraTemp.Topic_Upgrade;
             break;
         }
+        else if(topic == mqttTopicMecCameraTemp.Topic_Upgrade_Cancel  && m_MqttClientConfig.configerCamera.configerEnable.Enable_Upgrade_Cancel)
+        {
+            hint = "upgrade";
+            topicPT = mqttTopicMecCameraTemp.Topic_Upgrade_Cancel;
+            break;
+        }
         else if(topic == mqttTopicMecCameraTemp.Topic_Config_Query  && m_MqttClientConfig.configerCamera.configerEnable.Enable_Config_Query)
         {
             hint = "config-query";
diff --git a/middleware/protocol/om_common/configer_topic_om_camera.h b/middleware/protocol/om_common/configer_topic_om_camera.h
index 2b1923b..99481d9 100644
--- a/middleware/protocol/om_common/configer_topic_om_camera.h
+++ b/middleware/protocol/om_common/configer_topic_om_camera.h
@@ -72,6 +72,7 @@ struct MqttTopicConfigerCamera : public afl::base::SerializableData
     std::string Topic_Restar            = "/restart";
     std::string Topic_Restar_Up            = "/restart/up";
     std::string Topic_Upgrade           = "/upgrade";
+    std::string Topic_Upgrade_Cancel           = "/upgrade/cancel";
     std::string Topic_UpgradeStatus     = "/upgrade-status/up";
     std::string Topic_Synchronize       = "/synchronize";
 
@@ -98,6 +99,7 @@ private:
         JsonSerialize(Topic_Restar, "J_Topic_Restar", j, false);
         JsonSerialize(Topic_Restar_Up, "J_Topic_Restar_Up", j, false);
         JsonSerialize(Topic_Upgrade, "K_Topic_Upgrade", j, false);
+        JsonSerialize(Topic_Upgrade_Cancel, "K_Topic_Upgrade_Cancel", j, false);
         JsonSerialize(Topic_UpgradeStatus, "L_Topic_UpgradeStatus", j, false);
         JsonSerialize(Topic_Synchronize, "M_Topic_Synchronize", j, false);
 
@@ -125,6 +127,7 @@ private:
         JsonDeserialize(Topic_Restar, "J_Topic_Restar", j, noUse_isEmptyFlag);
         JsonDeserialize(Topic_Restar_Up, "J_Topic_Restar_Up", j, noUse_isEmptyFlag);
         JsonDeserialize(Topic_Upgrade, "K_Topic_Upgrade", j, noUse_isEmptyFlag);
+        JsonDeserialize(Topic_Upgrade_Cancel, "K_Topic_Upgrade_Cancel", j, noUse_isEmptyFlag);
         JsonDeserialize(Topic_UpgradeStatus, "L_Topic_UpgradeStatus", j, noUse_isEmptyFlag);
         JsonDeserialize(Topic_Synchronize, "M_Topic_Synchronize", j, noUse_isEmptyFlag);
 
@@ -156,6 +159,7 @@ public:
         ss << std::left << std::setw(40) << "J_Topic_Restar: " << Topic_Restar <<  std::endl;
         ss << std::left << std::setw(40) << "J_Topic_Restar_Up: " << Topic_Restar_Up <<  std::endl;
         ss << std::left << std::setw(40) << "K_Topic_Upgrade: " << Topic_Upgrade <<  std::endl;
+        ss << std::left << std::setw(40) << "K_Topic_Upgrade_Cancel: " << Topic_Upgrade_Cancel <<  std::endl;
         ss << std::left << std::setw(40) << "L_Topic_UpgradeStatus: " << Topic_UpgradeStatus <<  std::endl;
         ss << std::left << std::setw(40) << "M_Topic_Synchronize: " << Topic_Synchronize <<  std::endl;
 
@@ -185,6 +189,7 @@ public:
     bool Enable_Keepalive = true;
     bool Enable_Restar = true;
     bool Enable_Upgrade = true;
+    bool Enable_Upgrade_Cancel = true;
     bool Enable_UpgradeStatus = true;
     bool Enable_Synchronize = true;
 
@@ -203,6 +208,7 @@ private:
         JsonSerialize(Enable_Keepalive, "H_Enable_Keepalive", j, false);
         JsonSerialize(Enable_Restar, "I_Enable_Restar", j, false);
         JsonSerialize(Enable_Upgrade, "J_Enable_Upgrade", j, false);
+        JsonSerialize(Enable_Upgrade_Cancel, "J_Enable_Upgrade_Cancel", j, false);
         JsonSerialize(Enable_UpgradeStatus, "K_Enable_UpgradeStatus", j, false);
         JsonSerialize(Enable_Synchronize, "L_Enable_Synchronize", j, false);
 
@@ -221,6 +227,7 @@ private:
         JsonDeserialize(Enable_Keepalive, "H_Enable_Keepalive", j, noUse_isEmptyFlag);
         JsonDeserialize(Enable_Restar, "I_Enable_Restar", j, noUse_isEmptyFlag);
         JsonDeserialize(Enable_Upgrade, "J_Enable_Upgrade", j, noUse_isEmptyFlag);
+        JsonDeserialize(Enable_Upgrade_Cancel, "J_Enable_Upgrade_Cancel", j, noUse_isEmptyFlag);
         JsonDeserialize(Enable_UpgradeStatus, "K_Enable_UpgradeStatus", j, noUse_isEmptyFlag);
         JsonDeserialize(Enable_Synchronize, "L_Enable_Synchronize", j, noUse_isEmptyFlag);
 
@@ -242,6 +249,7 @@ public:
         ss << std::left << std::setw(40) << "H_Enable_Keepalive: " << (Enable_Keepalive ? "True" : "False") <<  std::endl;
         ss << std::left << std::setw(40) << "I_Enable_Restar: " << (Enable_Restar ? "True" : "False") <<  std::endl;
         ss << std::left << std::setw(40) << "J_Enable_Upgrade: " << (Enable_Upgrade ? "True" : "False") <<  std::endl;
+        ss << std::left << std::setw(40) << "J_Enable_Upgrade_Cancel: " << (Enable_Upgrade_Cancel ? "True" : "False") <<  std::endl;
         ss << std::left << std::setw(40) << "K_Enable_UpgradeStatus: " << (Enable_UpgradeStatus ? "True" : "False") <<  std::endl;
         ss << std::left << std::setw(40) << "L_Enable_Synchronize: " << (Enable_Synchronize ? "True" : "False") <<  std::endl;
 
diff --git a/middleware/protocol/om_common/configer_topic_om_radar.h b/middleware/protocol/om_common/configer_topic_om_radar.h
index ad52600..62edc1e 100644
--- a/middleware/protocol/om_common/configer_topic_om_radar.h
+++ b/middleware/protocol/om_common/configer_topic_om_radar.h
@@ -75,6 +75,7 @@ struct MqttTopicConfigerOmRadar : public afl::base::SerializableData
     std::string Topic_Warning               = "/upload/warning";
     std::string Topic_Heartbeat             = "/heartbeat";
     std::string Topic_Ota                   = "/ota";
+    std::string Topic_Ota_Cancel            = "/ota/cancel";
     std::string Topic_Ota_ACK               = "/ack/ota";
     std::string Topic_Reboot                = "/reboot";
     std::string Topic_Reboot_Ack            = "/ack/reboot";
@@ -106,6 +107,7 @@ private:
         JsonSerialize(Topic_Warning,          "L_Topic_Warning",         j, false);
         JsonSerialize(Topic_Heartbeat,        "M_Topic_Heartbeat",       j, false);
         JsonSerialize(Topic_Ota,              "N_Topic_Ota",             j, false);
+        JsonSerialize(Topic_Ota_Cancel,       "N_Topic_Ota_Cancel",             j, false);
         JsonSerialize(Topic_Ota_ACK,          "O_Topic_Ota_ACK",         j, false);
         JsonSerialize(Topic_Reboot,           "P_Topic_Reboot",          j, false);
         JsonSerialize(Topic_Reboot_Ack,       "Q_Topic_Reboot_Ack",      j, false);
@@ -138,6 +140,7 @@ private:
         JsonDeserialize(Topic_Warning,          "L_Topic_Warning",          j, noUse_isEmptyFlag);
         JsonDeserialize(Topic_Heartbeat,        "M_Topic_Heartbeat",        j, noUse_isEmptyFlag);
         JsonDeserialize(Topic_Ota,              "N_Topic_Ota",              j, noUse_isEmptyFlag);
+        JsonDeserialize(Topic_Ota_Cancel,       "N_Topic_Ota_Cancel",             j, noUse_isEmptyFlag);
         JsonDeserialize(Topic_Ota_ACK,          "O_Topic_Ota_ACK",          j, noUse_isEmptyFlag);
         JsonDeserialize(Topic_Reboot,           "P_Topic_Reboot",           j, noUse_isEmptyFlag);
         JsonDeserialize(Topic_Reboot_Ack,       "Q_Topic_Reboot_Ack",       j, noUse_isEmptyFlag);
@@ -171,6 +174,7 @@ public:
         ss << std::left << std::setw(40) << "L_Topic_Warning: " << Topic_Warning <<  std::endl;
         ss << std::left << std::setw(40) << "M_Topic_Heartbeat: " << Topic_Heartbeat <<  std::endl;
         ss << std::left << std::setw(40) << "N_Topic_Ota: " << Topic_Ota <<  std::endl;
+        ss << std::left << std::setw(40) << "N_Topic_Ota_Cancel: " << Topic_Ota_Cancel <<  std::endl;
         ss << std::left << std::setw(40) << "O_Topic_Ota_ACK: " << Topic_Ota_ACK <<  std::endl;
         ss << std::left << std::setw(40) << "P_Topic_Reboot: " << Topic_Reboot <<  std::endl;
         ss << std::left << std::setw(40) << "Q_Topic_Reboot_Ack: " << Topic_Reboot_Ack <<  std::endl;
@@ -202,6 +206,7 @@ public:
     bool Enable_Warning = true;
     bool Enable_Heartbeat = true;
     bool Enable_Ota = true;
+    bool Enable_Ota_Cancel = true;
     bool Enable_Ota_ACK = true;
     bool Enable_Reboot = true;
     bool Enable_Reboot_Ack = true;
@@ -228,6 +233,8 @@ private:
         JsonSerialize(Enable_Warning, "K_Enable_Warning", j, false);
         JsonSerialize(Enable_Heartbeat, "L_Enable_Heartbeat", j, false);
         JsonSerialize(Enable_Ota, "M_Enable_Ota", j, false);
+        JsonSerialize(Enable_Ota_Cancel, "M_Enable_Ota_Cancel", j, false);
+
         JsonSerialize(Enable_Ota_ACK, "N_Enable_Ota_ACK", j, false);
         JsonSerialize(Enable_Reboot, "O_Enable_Reboot", j, false);
         JsonSerialize(Enable_Reboot_Ack, "P_Enable_Reboot_Ack", j, false);
@@ -256,6 +263,7 @@ private:
         JsonDeserialize(Enable_Warning, "K_Enable_Warning", j, noUse_isEmptyFlag);
         JsonDeserialize(Enable_Heartbeat, "L_Enable_Heartbeat", j, noUse_isEmptyFlag);
         JsonDeserialize(Enable_Ota, "M_Enable_Ota", j, noUse_isEmptyFlag);
+        JsonDeserialize(Enable_Ota_Cancel, "M_Enable_Ota_Cancel", j, noUse_isEmptyFlag);
         JsonDeserialize(Enable_Ota_ACK, "N_Enable_Ota_ACK", j, noUse_isEmptyFlag);
         JsonDeserialize(Enable_Reboot, "O_Enable_Reboot", j, noUse_isEmptyFlag);
         JsonDeserialize(Enable_Reboot_Ack, "P_Enable_Reboot_Ack", j, noUse_isEmptyFlag);
@@ -287,6 +295,7 @@ public:
         ss << std::left << std::setw(40) << "K_Enable_Warning: " << (Enable_Warning ? "True" : "False") <<  std::endl;
         ss << std::left << std::setw(40) << "L_Enable_Heartbeat: " << (Enable_Heartbeat ? "True" : "False") <<  std::endl;
         ss << std::left << std::setw(40) << "M_Enable_Ota: " << (Enable_Ota ? "True" : "False") <<  std::endl;
+        ss << std::left << std::setw(40) << "M_Enable_Ota_Cancel: " << (Enable_Ota_Cancel ? "True" : "False") <<  std::endl;
         ss << std::left << std::setw(40) << "N_Enable_Ota_ACK: " << (Enable_Ota_ACK ? "True" : "False") <<  std::endl;
         ss << std::left << std::setw(40) << "O_Enable_Reboot: " << (Enable_Reboot ? "True" : "False") <<  std::endl;
         ss << std::left << std::setw(40) << "P_Enable_Reboot_Ack: " << (Enable_Reboot_Ack ? "True" : "False") <<  std::endl;
```
