# fix(运维):【20260305-zhangenwei-3】：【顺义车网】【智路os运维3.4】    1.修改配置下发中日志级别为4导致系统重启后，运行失败的问题

- 作者：zhangenwei
- 日期：2026-03-05
- 完整 hash：`aeced6d69e9a62798731d30a335dd3053090082e`
- 改动：+3 / -3 行

```diff
@@@C	zhangenwei	aeced6d69e9a62798731d30a335dd3053090082e	2026-03-05	fix(运维):【20260305-zhangenwei-3】：【顺义车网】【智路os运维3.4】    1.修改配置下发中日志级别为4导致系统重启后，运行失败的问题
diff --git a/middleware/protocol/om/om_component.cc b/middleware/protocol/om/om_component.cc
index 5403f1d..bab3f31 100644
--- a/middleware/protocol/om/om_component.cc
+++ b/middleware/protocol/om/om_component.cc
@@ -2737,7 +2737,7 @@ bool OM_COMPONENT::getWorkParamFromFile()
             logLevel = LOG_LEVEL_ERROR;
             break;
         case 4: //No-log
-            logLevel = LOG_LEVEL_NO_LOG;
+            logLevel = LOG_LEVEL_ERROR;
             break;
         default:
             break;
@@ -6305,7 +6305,7 @@ bool OM_COMPONENT::processConfigUpdate(ConfigUpdateData& configUpdateData,  Conf
                     logLevelStr="3";
                     break;
                 case LOG_LEVEL_NO_LOG:
-                    logLevelStr="4";
+                    logLevelStr="3";
                     break;
                 default:
                     logLevelStr="0";
diff --git a/middleware/protocol/proto/BUILD b/middleware/protocol/proto/BUILD
index 8f1991b..50e4e3f 100644
--- a/middleware/protocol/proto/BUILD
+++ b/middleware/protocol/proto/BUILD
@@ -38,7 +38,7 @@ proto_library(
 
 py_proto_library(
     name = "monitor_mec_py_pb2",
-    srcs = ["monitor_mec.proto"], # ??????? .proto ???
+    srcs = ["monitor_mec.proto"], # ����д��ʵ�ʵ� .proto �ļ���
     deps = [
         "@com_google_protobuf//:protobuf_python",
     ],
```
