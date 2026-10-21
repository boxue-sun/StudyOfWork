# fix(运维):【20260311-zhangenwei-7】增加mec自检初始化字段时，没有初始化的字段

- 作者：zhangenwei
- 日期：2026-03-11
- 完整 hash：`0fc1d6b69d46279d93bc61be178a3188bc584fa7`
- 改动：+1 / -1 行

```diff
@@@C	zhangenwei	0fc1d6b69d46279d93bc61be178a3188bc584fa7	2026-03-11	fix(运维):【20260311-zhangenwei-7】增加mec自检初始化字段时，没有初始化的字段
diff --git a/middleware/protocol/om/configer/configer_om_mec.h b/middleware/protocol/om/configer/configer_om_mec.h
index c9fc132..681fbc3 100644
--- a/middleware/protocol/om/configer/configer_om_mec.h
+++ b/middleware/protocol/om/configer/configer_om_mec.h
@@ -526,7 +526,7 @@ struct MecSelfCheckTime : public afl::base::SerializableData
         cloud_point_data_tr.recoverySecond = 3600;
         // cloud_point_data_tr.timeoutSecond = 6;
         // cloud_point_data_tr.recoverySecond = 6;
-
+        cloud_point_data_tr.enable = true;
 
         sensor_cloud_link_tr.timeoutSecond = 5;
         sensor_cloud_link_tr.recoverySecond = 5;
```
