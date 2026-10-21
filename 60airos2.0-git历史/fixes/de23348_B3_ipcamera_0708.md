# perf(设备接入)【FAR202505-00064】相机接入模块优化

- 作者：changxuhui
- 日期：2025-07-08
- 完整 hash：`de233482b285c46924ea305cc52b889e8c6ae31d`
- 改动：+1081 / -50 行

```diff
@@@C	changxuhui	de233482b285c46924ea305cc52b889e8c6ae31d	2025-07-08	perf(设备接入)【FAR202505-00064】相机接入模块优化
diff --git a/base/device_connect/ipcamera/device_base.h b/base/device_connect/ipcamera/device_base.h
index f0dfc36..4a2b209 100644
--- a/base/device_connect/ipcamera/device_base.h
+++ b/base/device_connect/ipcamera/device_base.h
@@ -10,7 +10,7 @@
  *  Change History:
  *  Date         Author        Version      ChangeId           Description
  *  ------------------------------------------------------------------------------
- *  2025/1/13  ChangXuhui      1.0          ————               IP Camera设备基类开发
+ *  2025/7/4  ChangXuhui      1.0          ————               IP Camera设备基类开发
  *                                                             
  *
  * @endverbatim
diff --git a/base/device_connect/ipcamera/device_factory.cc b/base/device_connect/ipcamera/device_factory.cc
index a87a7e6..a90837d 100644
--- a/base/device_connect/ipcamera/device_factory.cc
+++ b/base/device_connect/ipcamera/device_factory.cc
@@ -10,7 +10,7 @@
  *  Change History:
  *  Date         Author        Version      ChangeId           Description
  *  ------------------------------------------------------------------------------
- *  2025/1/13  ChangXuhui      1.0          ————               IP Camera设备工厂类实现
+ *  2025/7/4  ChangXuhui      1.0          ————               IP Camera设备工厂类实现
  *
  * @endverbatim
  ********************************************************************************/
diff --git a/base/device_connect/ipcamera/device_factory.h b/base/device_connect/ipcamera/device_factory.h
index 148d702..6d8933a 100644
--- a/base/device_connect/ipcamera/device_factory.h
+++ b/base/device_connect/ipcamera/device_factory.h
@@ -10,7 +10,7 @@
  *  Change History:
  *  Date         Author        Version      ChangeId           Description
  *  ------------------------------------------------------------------------------
- *  2025/1/13  ChangXuhui      1.0          ————               IP Camera设备工厂类开发
+ *  2025/7/4  ChangXuhui      1.0          ————               IP Camera设备工厂类开发
  *
  * @endverbatim
  ********************************************************************************/
diff --git a/data/20250708153829.record.00000 b/data/20250708153829.record.00000
new file mode 100644
index 0000000..8631182
Binary files /dev/null and b/data/20250708153829.record.00000 differ
diff --git a/middleware/device_service/framework/ipcamera/conf/ipcamera_config.yaml b/middleware/device_service/framework/ipcamera/conf/ipcamera_config.yaml
index 55ec502..ca549b5 100644
--- a/middleware/device_service/framework/ipcamera/conf/ipcamera_config.yaml
+++ b/middleware/device_service/framework/ipcamera/conf/ipcamera_config.yaml
@@ -1,5 +1,4 @@
 # IP Camera 配置文件
-frame_id: "camera_frame"
 # 多路RTSP拉流地址配置
 rtsp_urls:
   - "rtsp://root:root@172.20.65.193:8554/stream1"
@@ -16,3 +15,4 @@ reconnect_interval: 5  # 重连间隔（秒）
 buffer_size: 1024000   # 缓冲区大小
 connection_timeout: 10 # 连接超时时间（秒）
 enable_sei_timestamp: true  # 是否启用SEI时间戳解析
+enable_h264_save: false  # 是否保存H.264文件到磁盘（默认关闭） 
\ No newline at end of file
diff --git a/middleware/device_service/framework/ipcamera/ipcamera_component.cc b/middleware/device_service/framework/ipcamera/ipcamera_component.cc
index b027a9c..128d2a6 100644
--- a/middleware/device_service/framework/ipcamera/ipcamera_component.cc
+++ b/middleware/device_service/framework/ipcamera/ipcamera_component.cc
@@ -10,7 +10,7 @@
  *  Change History:
  *  Date         Author        Version      ChangeId           Description
  *  ------------------------------------------------------------------------------
- *  2025/1/13  ChangXuhui      1.0          ————               IP Camera组件实现
+ *  2025/7/4  ChangXuhui      1.0          ————               IP Camera组件实现
  *
  * @endverbatim
  ********************************************************************************/
diff --git a/middleware/device_service/framework/ipcamera/ipcamera_component.h b/middleware/device_service/framework/ipcamera/ipcamera_component.h
index 057bbbf..a9a496d 100644
--- a/middleware/device_service/framework/ipcamera/ipcamera_component.h
+++ b/middleware/device_service/framework/ipcamera/ipcamera_component.h
@@ -10,7 +10,7 @@
  *  Change History:
  *  Date         Author        Version      ChangeId           Description
  *  ------------------------------------------------------------------------------
- *  2025/1/13  ChangXuhui      1.0          ————               IP Camera组件头文件
+ *  2025/7/4  ChangXuhui      1.0          ————               IP Camera组件头文件
  *
  * @endverbatim
  ********************************************************************************/
diff --git a/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.cc b/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.cc
index 7e1b8ff..bd6e196 100644
--- a/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.cc
+++ b/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.cc
@@ -10,7 +10,7 @@
  *  Change History:
  *  Date         Author        Version      ChangeId           Description
  *  ------------------------------------------------------------------------------
- *  2025/1/13  ChangXuhui      1.0          ————               标准IP Camera设备实现
+ *  2025/7/4  ChangXuhui      1.0          ————               标准IP Camera设备实现
  *
  * @endverbatim
  ********************************************************************************/
@@ -25,11 +25,21 @@
 #include <iomanip>
 #include <regex>
 #include <string>
+#include <fstream>
+#include <map>
+#include <mutex>
+#include <sys/stat.h>
+#include <sys/types.h>
 
 namespace os {
 namespace v2x {
 namespace device {
 
+// 文件写入相关全局变量
+static std::map<std::string, std::ofstream> g_h264_files;
+static std::mutex g_file_mutex;
+static std::map<std::string, int64_t> g_frame_counts;
+
 bool StandardIpCamera::Init(const std::string& conf) {
   IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera::Init config file: " << conf;
   
@@ -37,11 +47,10 @@ bool StandardIpCamera::Init(const std::string& conf) {
     YAML::Node config = YAML::LoadFile(conf);
     
     // 读取配置参数
-    frame_id_ = config["frame_id"].as<std::string>("camera_frame");
     reconnect_interval_ = config["reconnect_interval"].as<int>(5);
-    buffer_size_ = config["buffer_size"].as<int>(1024000);
     connection_timeout_ = config["connection_timeout"].as<int>(10);
     enable_sei_timestamp_ = config["enable_sei_timestamp"].as<bool>(true);
+    enable_h264_save_ = config["enable_h264_save"].as<bool>(false);
     
     // 读取多路RTSP URL
     if (config["rtsp_urls"] && config["rtsp_urls"].IsSequence()) {
@@ -64,16 +73,15 @@ bool StandardIpCamera::Init(const std::string& conf) {
       return false;
     }
 
-    IPCAMERA_SERVICE_LOG_INFO << "Frame ID: " << frame_id_;
     IPCAMERA_SERVICE_LOG_INFO << "RTSP URLs count: " << rtsp_urls_.size();
     for (size_t i = 0; i < rtsp_urls_.size(); ++i) {
       IPCAMERA_SERVICE_LOG_INFO << "RTSP URL " << i << ": " << rtsp_urls_[i];
       IPCAMERA_SERVICE_LOG_INFO << "Output Topic " << i << ": " << output_topics_[i];
     }
     IPCAMERA_SERVICE_LOG_INFO << "Reconnect interval: " << reconnect_interval_ << "s";
-    IPCAMERA_SERVICE_LOG_INFO << "Buffer size: " << buffer_size_;
     IPCAMERA_SERVICE_LOG_INFO << "Connection timeout: " << connection_timeout_ << "s";
     IPCAMERA_SERVICE_LOG_INFO << "SEI timestamp parsing: " << (enable_sei_timestamp_ ? "enabled" : "disabled");
+    IPCAMERA_SERVICE_LOG_INFO << "H.264 file save: " << (enable_h264_save_ ? "enabled" : "disabled");
     
     // 初始化 FFmpeg
     if (!InitFFmpeg()) {
@@ -177,6 +185,11 @@ void StandardIpCamera::StopStream(const std::string& stream_id) {
     stream_info->format_ctx = nullptr;
   }
   
+  // 关闭H.264文件
+  if (enable_h264_save_) {
+    CloseH264File(stream_id);
+  }
+  
   IPCAMERA_SERVICE_LOG_INFO << "Stopped stream: " << stream_id;
 }
 
@@ -249,69 +262,81 @@ void StandardIpCamera::TaskPullRtspStream(const std::string& stream_id) {
       }
       
       // 只处理视频流
-      if (pkt.stream_index == stream_info->video_stream_index) {
+      if (pkt.stream_index == stream_info->video_stream_index) {  
         // 创建压缩图像数据
         auto compressed_image = std::make_shared<os::v2x::device::ipcamera::CompressedImage>();
         
-        // 设置头部信息
-        auto header = compressed_image->mutable_header();
-        header->set_timestamp_sec(airos::base::TimeUtil::GetCurrentTime());
-        header->set_module_name("ipcamera_service");
-        header->set_sequence_num(++stream_info->sequence_num);
-        header->set_camera_timestamp(GetCurrentTimestamp());
-        
-        // 设置图像信息
-        compressed_image->set_frame_id(frame_id_);
+        // 设置图像信息 - 使用每路流的独立递增帧ID (64位整数)
+        uint64_t current_frame_id = ++stream_info->frame_counter;
+        compressed_image->set_frame_id(std::to_string(current_frame_id));
         compressed_image->set_format("h264");  // H264 格式
-        compressed_image->set_measurement_time(airos::base::TimeUtil::GetCurrentTime());
         
         // 检测是否为关键帧
         bool is_key_frame = IsKeyFrame(pkt.data, pkt.size);
-        IPCAMERA_SERVICE_LOG_INFO << "Size " << pkt.size;
         compressed_image->set_frame_type(is_key_frame ? 1 : 0);  // 1表示关键帧，0表示普通帧
-        IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " frame type: " << (is_key_frame ? "key frame" : "normal frame");
         
         // 从SEI数据中解析曝光时间戳
         if (enable_sei_timestamp_) {
           int64_t exposure_timestamp = ExtractExposureTimestamp(pkt.data, pkt.size);
           if (exposure_timestamp > 0) {
             compressed_image->set_exposure_time(exposure_timestamp);
-            IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " using extracted exposure timestamp: " << exposure_timestamp;
           } else {
             // 使用系统当前时间作为曝光时间戳
             int64_t current_time = GetCurrentTimestamp();
             compressed_image->set_exposure_time(current_time);
-            IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " using system current time: " << current_time;
           }
         } else {
           // 使用系统当前时间作为曝光时间戳
           int64_t current_time = GetCurrentTimestamp();
           compressed_image->set_exposure_time(current_time);
         }
-        
         // 设置 H264 NALU 数据
         std::string h264_data(reinterpret_cast<const char*>(pkt.data), pkt.size);
         compressed_image->set_data(h264_data);
+      
+        // 在发送前设置header信息（确保时间戳更准确）
+        auto header = compressed_image->mutable_header();
+        header->set_timestamp_sec(airos::base::TimeUtil::GetCurrentTime());
+        header->set_module_name("ipcamera_service");
+        header->set_sequence_num(++stream_info->sequence_num);
+        header->set_camera_timestamp(GetCurrentTimestamp());
+        compressed_image->set_measurement_time(airos::base::TimeUtil::GetCurrentTime());
         
-        // 打印前16位数据
-        std::stringstream ss;
-        ss << "Stream " << stream_id << " Frame " << stream_info->sequence_num << " first 16 bytes: ";
-        for (int i = 0; i < std::min(16, pkt.size); ++i) {
-          ss << std::hex << std::setw(2) << std::setfill('0') 
-             << static_cast<int>(static_cast<unsigned char>(pkt.data[i])) << " ";
+        // 计算发送间隔
+        double current_time = airos::base::TimeUtil::GetCurrentTime();
+        if (stream_info->last_send_time > 0.0) {
+            double interval = current_time - stream_info->last_send_time;
+            
+            // 更新统计信息
+            stream_info->total_interval_sec += interval;
+            stream_info->interval_count++;
+            if (interval < stream_info->min_interval_sec) {
+                stream_info->min_interval_sec = interval;
+            }
+            if (interval > stream_info->max_interval_sec) {
+                stream_info->max_interval_sec = interval;
+            }
         }
-        IPCAMERA_SERVICE_LOG_INFO << ss.str();
+        stream_info->last_send_time = current_time;
         
         // 发送数据，使用多路流回调
         if (sender_) {
-          // 使用输出话题作为stream_id
           sender_(stream_info->output_topic, compressed_image);
-        }
-        
-        // 更新统计信息
-        if (stream_info->sequence_num % 100 == 0) {
-          IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " received " << stream_info->sequence_num 
-                    << " frames, data size: " << pkt.size << " bytes";
+          
+          // 如果启用H.264保存，则保存到文件
+          if (enable_h264_save_) {
+            SaveH264ToFile(stream_id, pkt.data, pkt.size);
+          }
+          
+          // 每100帧打印间隔统计信息
+          if (stream_info->sequence_num % 100 == 0 && stream_info->interval_count > 0) {
+            double avg_interval = stream_info->total_interval_sec / stream_info->interval_count;
+            IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " send interval stats: "
+                      << "avg=" << std::fixed << std::setprecision(3) << avg_interval << "s, "
+                      << "min=" << std::fixed << std::setprecision(3) << stream_info->min_interval_sec << "s, "
+                      << "max=" << std::fixed << std::setprecision(3) << stream_info->max_interval_sec << "s, "
+                      << "count=" << stream_info->interval_count;
+          }
         }
       }
       
@@ -409,8 +434,6 @@ int64_t StandardIpCamera::ExtractExposureTimestamp(const uint8_t* data, size_t s
         }
         
         if (sei_payload_start + sei_payload_size < size) {
-          IPCAMERA_SERVICE_LOG_INFO << "Found SEI data, size: " << sei_payload_size << " bytes";
-          
           // 假设时间戳从 SEI 数据的固定偏移开始
           const size_t timestamp_offset = 20; // 根据实际 SEI 数据调整
           const size_t timestamp_size = 14;    // 时间戳长度（字节）
@@ -423,7 +446,6 @@ int64_t StandardIpCamera::ExtractExposureTimestamp(const uint8_t* data, size_t s
             try {
               int64_t timestamp_ms = std::stoll(timestamp);
               if (timestamp_ms > 1000000000000) { // 验证时间戳合理性
-                IPCAMERA_SERVICE_LOG_INFO << "Extracted exposure timestamp: " << timestamp_ms;
                 return timestamp_ms;
               }
             } catch (const std::exception& e) {
@@ -460,7 +482,6 @@ bool StandardIpCamera::IsKeyFrame(const uint8_t* data, size_t size) {
       
       // I帧的NALU类型是5 (IDR帧)
       if (nal_type == 5) {
-        IPCAMERA_SERVICE_LOG_INFO << "Found key frame (IDR frame)";
         return true;
       }
       
@@ -479,7 +500,6 @@ bool StandardIpCamera::IsKeyFrame(const uint8_t* data, size_t size) {
     pos++;
   }
   
-  IPCAMERA_SERVICE_LOG_INFO << "No key frame found";
   return false;
 }
 
@@ -511,6 +531,79 @@ IpCameraDeviceState StandardIpCamera::GetState() {
   return IpCameraDeviceState::UNKNOWN;
 }
 
+void StandardIpCamera::SaveH264ToFile(const std::string& stream_id, const uint8_t* data, size_t size) {
+  auto it = streams_.find(stream_id);
+  if (it == streams_.end()) {
+    return;
+  }
+  
+  auto& stream_info = it->second;
+  
+  // 检查文件是否已打开，如果没有则创建新文件
+  if (!stream_info->h264_file.is_open()) {
+    // 生成文件名：stream_id_YYYYMMDD_HHMMSS.h264
+    auto now = std::chrono::system_clock::now();
+    auto time_t = std::chrono::system_clock::to_time_t(now);
+    std::tm tm = *std::localtime(&time_t);
+    
+    // 从stream_id中提取IP地址（格式：/sensor/ipcamera/h264/172_20_65_193）
+    std::string ip_address = stream_id;
+    size_t pos = ip_address.find_last_of("/");
+    if (pos != std::string::npos) {
+        ip_address = ip_address.substr(pos + 1);  // 提取IP部分：172_20_65_193
+    }
+
+    // 生成包含完整IP的文件名
+    std::stringstream filename_ss;
+    filename_ss << ip_address << "_"
+                << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".h264";
+    stream_info->h264_filename = "/home/airos/data/" + filename_ss.str();
+    
+    // 确保目录存在
+    std::string dir_path = "/home/airos/data";
+    struct stat st = {0};
+    if (stat(dir_path.c_str(), &st) == -1) {
+        mkdir(dir_path.c_str(), 0755);
+    }
+    
+    // 打开文件
+    stream_info->h264_file.open(stream_info->h264_filename, std::ios::binary | std::ios::app);
+    if (!stream_info->h264_file.is_open()) {
+      IPCAMERA_SERVICE_LOG_ERROR << "Failed to open H.264 file: " << stream_info->h264_filename;
+      return;
+    }
+    
+    IPCAMERA_SERVICE_LOG_INFO << "Created H.264 file: " << stream_info->h264_filename << " for stream: " << stream_id;
+  }
+  
+  // 写入H.264数据
+  stream_info->h264_file.write(reinterpret_cast<const char*>(data), size);
+  stream_info->h264_file.flush();  // 确保数据立即写入磁盘
+  
+  // 更新统计信息
+  stream_info->saved_frame_count++;
+  
+  // 每1000帧打印一次统计信息
+  if (stream_info->saved_frame_count % 1000 == 0) {
+    IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " saved " 
+              << stream_info->saved_frame_count << " frames to H.264 file: " << stream_info->h264_filename;
+  }
+}
+
+void StandardIpCamera::CloseH264File(const std::string& stream_id) {
+  auto it = streams_.find(stream_id);
+  if (it == streams_.end()) {
+    return;
+  }
+  
+  auto& stream_info = it->second;
+  if (stream_info->h264_file.is_open()) {
+    stream_info->h264_file.close();
+    IPCAMERA_SERVICE_LOG_INFO << "Closed H.264 file for stream: " << stream_id 
+              << " (saved " << stream_info->saved_frame_count << " frames): " << stream_info->h264_filename;
+  }
+}
+
 // 注册设备到工厂
 V2XOS_IPCAMERA_REG_FACTORY(StandardIpCamera, "standard_ipcamera");
 
diff --git a/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.h b/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.h
index e14c9d9..286ab01 100644
--- a/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.h
+++ b/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.h
@@ -10,7 +10,7 @@
  *  Change History:
  *  Date         Author        Version      ChangeId           Description
  *  ------------------------------------------------------------------------------
- *  2025/1/13  ChangXuhui      1.0          ————               标准IP Camera设备头文件
+ *  2025/7/4  ChangXuhui      1.0          ————               标准IP Camera设备头文件
  *
  * @endverbatim
  ********************************************************************************/
@@ -24,6 +24,8 @@
 #include <map>
 #include <mutex>
 #include <vector>
+#include <cfloat>
+#include <fstream>
 
 #include "base/device_connect/ipcamera/device_base.h"
 #include "glog/logging.h"
@@ -63,13 +65,12 @@ class StandardIpCamera : public IpCameraDevice {
   std::atomic<bool> stop_ = ATOMIC_VAR_INIT(false);
   
   // 配置参数
-  std::string frame_id_;
   std::vector<std::string> rtsp_urls_;  // 多路RTSP URL
   std::vector<std::string> output_topics_;  // 输出话题列表
   int reconnect_interval_;  // 重连间隔（秒）
-  int buffer_size_;         // 缓冲区大小
   int connection_timeout_;  // 连接超时时间（秒）
   bool enable_sei_timestamp_;  // 是否启用SEI时间戳解析
+  bool enable_h264_save_;  // 是否保存H.264文件到磁盘
   
   // 单个流信息结构体
   struct StreamInfo {
@@ -81,9 +82,24 @@ class StandardIpCamera : public IpCameraDevice {
     AVFormatContext* format_ctx;
     int video_stream_index;
     uint32_t sequence_num;
+    uint64_t frame_counter;  // 每路流的独立帧计数器 (0-18446744073709551615)
+    double last_send_time;   // 上一帧发送时间（秒）
+    
+    // 发送间隔统计
+    double total_interval_sec;  // 总间隔时间（秒）
+    double min_interval_sec;    // 最小间隔（秒）
+    double max_interval_sec;    // 最大间隔（秒）
+    int64_t interval_count;     // 间隔统计次数
+    
+    // H.264文件保存相关
+    std::ofstream h264_file;  // H.264文件流
+    std::string h264_filename;  // H.264文件名
+    int64_t saved_frame_count;  // 已保存帧数
     
     StreamInfo() : stop(false), connected(false), format_ctx(nullptr), 
-                   video_stream_index(-1), sequence_num(0) {}
+                   video_stream_index(-1), sequence_num(0), frame_counter(0),
+                   last_send_time(0.0), total_interval_sec(0.0), min_interval_sec(DBL_MAX), 
+                   max_interval_sec(0.0), interval_count(0), saved_frame_count(0) {}
   };
   
   // 多路流相关
@@ -100,6 +116,10 @@ class StandardIpCamera : public IpCameraDevice {
   
   // 关键帧检测
   bool IsKeyFrame(const uint8_t* data, size_t size);
+  
+  // H.264文件保存
+  void SaveH264ToFile(const std::string& stream_id, const uint8_t* data, size_t size);
+  void CloseH264File(const std::string& stream_id);
 };
 
 }  // namespace device
diff --git a/middleware/device_service/modules/ipcamera/test_tool/README.md b/middleware/device_service/modules/ipcamera/test_tool/README.md
new file mode 100644
index 0000000..9961f08
--- /dev/null
+++ b/middleware/device_service/modules/ipcamera/test_tool/README.md
@@ -0,0 +1,60 @@
+# extract_h264.py
+
+从record文件中提取H.264二进制数据的工具。
+
+## 功能
+
+- 自动识别IP Camera话题（格式：`/sensor/ipcamera/h264/{ip_address}`）
+- 提取H.264二进制数据到独立的.h264文件
+- 支持多路流数据分离保存
+- 提供详细的提取统计信息
+
+## 使用方法
+
+```bash
+# 基本用法
+python3 extract_h264.py <record_file>
+
+# 指定输出目录
+python3 extract_h264.py <record_file> --output-dir /path/to/output
+```
+
+## 参数
+
+- `record_file`: record文件路径
+- `--output-dir`, `-o`: 输出目录（可选，默认使用输入文件所在目录）
+
+## 输出文件
+
+- 文件名格式：`{ip_address}_{timestamp}_record.h264`
+- 示例：`172_20_65_193_20250708_145242_record.h264`
+
+## 示例
+
+```bash
+# 提取record文件中的H.264数据
+python3 extract_h264.py /home/airos/data/record.record
+
+# 输出示例
+读取文件: /home/airos/data/record.record
+输出目录: /home/airos/data
+发现话题: ['/sensor/ipcamera/h264/172_20_65_193', '/sensor/ipcamera/h264/172_20_65_194']
+创建文件: /home/airos/data/172_20_65_193_20250708_145242_record.h264
+创建文件: /home/airos/data/172_20_65_194_20250708_145242_record.h264
+已提取 100 帧，2.5 MB
+已提取 200 帧，5.1 MB
+话题 /sensor/ipcamera/h264/172_20_65_193: 提取 1500 帧，2.5 MB -> 172_20_65_193_20250708_145242_record.h264
+话题 /sensor/ipcamera/h264/172_20_65_194: 提取 1480 帧，2.3 MB -> 172_20_65_194_20250708_145242_record.h264
+
+提取完成!
+总帧数: 2980
+总字节: 5,038,592 (4.81 MB)
+输出目录: /home/airos/data
+```
+
+## 依赖
+
+- Python 3.6+
+- Apollo Cyber RT环境
+- protobuf库
+
diff --git a/middleware/device_service/modules/ipcamera/test_tool/extract_h264.py b/middleware/device_service/modules/ipcamera/test_tool/extract_h264.py
new file mode 100644
index 0000000..e4af2af
--- /dev/null
+++ b/middleware/device_service/modules/ipcamera/test_tool/extract_h264.py
@@ -0,0 +1,150 @@
+#!/usr/bin/env python3
+# -*- coding: utf-8 -*-
+"""
+从record文件中提取H.264二进制数据
+使用Apollo Cyber RT record API读取record文件
+"""
+
+import os
+import sys
+import argparse
+from datetime import datetime
+
+try:
+    # 导入Apollo Cyber RT record API
+    from cyber.python.cyber_py3 import record
+    import ipcamera_pb2
+except ImportError as e:
+    print(f"Error: 无法导入必要的模块: {e}")
+    print("请确保Apollo Cyber RT环境已正确配置")
+    sys.exit(1)
+
+
+def extract_h264_data(record_file, output_dir=None):
+    """提取H.264二进制数据，自动匹配IP Camera话题
+    
+    Args:
+        record_file: record文件路径
+        output_dir: 输出目录路径（如果为None，则使用输入文件所在目录）
+    """
+    if not os.path.exists(record_file):
+        print(f"Error: Record文件 {record_file} 不存在")
+        return False
+    
+    # 如果未指定输出目录，使用输入文件所在目录
+    if output_dir is None:
+        output_dir = os.path.dirname(os.path.abspath(record_file))
+        if not output_dir:  # 如果是当前目录
+            output_dir = "."
+    
+    print(f"读取文件: {record_file}")
+    print(f"输出目录: {output_dir}")
+    
+    # 确保输出目录存在
+    os.makedirs(output_dir, exist_ok=True)
+    
+    # 统计信息
+    total_extracted_frames = 0
+    total_extracted_bytes = 0
+    topic_stats = {}
+    
+    try:
+        # 使用Apollo Cyber RT record reader
+        reader = record.RecordReader(record_file)
+        
+        # 获取所有话题列表
+        channels = reader.get_channellist()
+        print(f"发现话题: {channels}")
+        
+        # 过滤IP Camera话题
+        ipcamera_topics = []
+        for channel in channels:
+            if channel.startswith("/sensor/ipcamera/h264/"):
+                ipcamera_topics.append(channel)
+        
+        if not ipcamera_topics:
+            print("警告: 未发现IP Camera话题")
+            return False
+        
+        print(f"发现IP Camera话题: {ipcamera_topics}")
+        
+        # 为每个话题创建文件
+        topic_files = {}
+        # 生成时间戳
+        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
+        
+        for topic in ipcamera_topics:
+            # 提取IP地址部分
+            ip_part = topic.split("/")[-1]  # 获取最后一段作为IP地址
+            filename = f"{ip_part}_{timestamp}_record.h264"
+            filepath = os.path.join(output_dir, filename)
+            
+            topic_files[topic] = open(filepath, 'wb')
+            topic_stats[topic] = {"frames": 0, "bytes": 0}
+            print(f"创建文件: {filepath} 用于话题: {topic}")
+        
+        # 读取所有消息
+        for channel_name, msg, data_type, timestamp in reader.read_messages():
+            # 只处理IP Camera话题
+            if channel_name not in topic_files:
+                continue
+            
+            try:
+                # 解析protobuf消息
+                compressed_image = ipcamera_pb2.CompressedImage()
+                compressed_image.ParseFromString(msg)
+                
+                # 检查是否有data字段
+                if compressed_image.HasField('data'):
+                    # 获取对应的文件
+                    out_f = topic_files[channel_name]
+                    h264_data = compressed_image.data
+                    out_f.write(h264_data)
+                    
+                    # 更新统计信息
+                    topic_stats[channel_name]["frames"] += 1
+                    topic_stats[channel_name]["bytes"] += len(h264_data)
+                    total_extracted_frames += 1
+                    total_extracted_bytes += len(h264_data)
+                    
+                    # 显示进度
+                    if total_extracted_frames % 100 == 0:
+                        print(f"已提取 {total_extracted_frames} 帧，{total_extracted_bytes/1024/1024:.2f} MB")
+            
+            except Exception as e:
+                print(f"解析消息失败: {e}")
+                continue
+        
+        # 关闭所有文件
+        for topic, file_obj in topic_files.items():
+            file_obj.close()
+            ip_part = topic.split("/")[-1]
+            filename = f"{ip_part}_{timestamp}_record.h264"
+            stats = topic_stats[topic]
+            print(f"话题 {topic}: 提取 {stats['frames']} 帧，{stats['bytes']/1024/1024:.2f} MB -> {filename}")
+    
+    except Exception as e:
+        print(f"Error: {e}")
+        return False
+    
+    print(f"\n提取完成!")
+    print(f"总帧数: {total_extracted_frames}")
+    print(f"总字节: {total_extracted_bytes:,} ({total_extracted_bytes/1024/1024:.2f} MB)")
+    print(f"输出目录: {output_dir}")
+    
+    return True
+
+
+def main():
+    parser = argparse.ArgumentParser(description='从record文件提取H.264二进制数据，自动匹配IP Camera话题')
+    parser.add_argument('record_file', help='record文件路径')
+    parser.add_argument('--output-dir', '-o', help='输出目录 (默认: 使用输入文件所在目录)')
+    
+    args = parser.parse_args()
+    
+    if not extract_h264_data(args.record_file, args.output_dir):
+        sys.exit(1)
+
+
+if __name__ == '__main__':
+    main() 
\ No newline at end of file
diff --git a/middleware/device_service/modules/ipcamera/test_tool/ipcamera_pb2.py b/middleware/device_service/modules/ipcamera/test_tool/ipcamera_pb2.py
new file mode 100644
index 0000000..abe67f0
--- /dev/null
+++ b/middleware/device_service/modules/ipcamera/test_tool/ipcamera_pb2.py
@@ -0,0 +1,708 @@
+# -*- coding: utf-8 -*-
+# Generated by the protocol buffer compiler.  DO NOT EDIT!
+# source: ipcamera.proto
+"""Generated protocol buffer code."""
+from google.protobuf.internal import enum_type_wrapper
+from google.protobuf import descriptor as _descriptor
+from google.protobuf import message as _message
+from google.protobuf import reflection as _reflection
+from google.protobuf import symbol_database as _symbol_database
+# @@protoc_insertion_point(imports)
+
+_sym_db = _symbol_database.Default()
+
+
+
+
+DESCRIPTOR = _descriptor.FileDescriptor(
+  name='ipcamera.proto',
+  package='os.v2x.device.ipcamera',
+  syntax='proto2',
+  serialized_options=None,
+  create_key=_descriptor._internal_create_key,
+  serialized_pb=b'\n\x0eipcamera.proto\x12\x16os.v2x.device.ipcamera\"\x9a\x02\n\x06Header\x12\x15\n\rtimestamp_sec\x18\x01 \x01(\x01\x12\x13\n\x0bmodule_name\x18\x02 \x01(\t\x12\x14\n\x0csequence_num\x18\x03 \x01(\r\x12\x17\n\x0flidar_timestamp\x18\x04 \x01(\x04\x12\x18\n\x10\x63\x61mera_timestamp\x18\x05 \x01(\x04\x12\x17\n\x0fradar_timestamp\x18\x06 \x01(\x04\x12\x12\n\nstart_time\x18\x07 \x01(\x01\x12\x13\n\x0b\x66inish_time\x18\x08 \x01(\x01\x12\x38\n\x10\x64\x65pended_modules\x18\t \x03(\x0b\x32\x1e.os.v2x.device.ipcamera.Module\x12\x10\n\x08\x66rame_id\x18\n \x01(\t\x12\r\n\x05stamp\x18\x0b \x01(\x04\"3\n\x06Module\x12\x13\n\x0bmodule_name\x18\x01 \x01(\t\x12\x14\n\x0csequence_num\x18\x02 \x01(\r\"\xc7\x01\n\x05Image\x12.\n\x06header\x18\x01 \x01(\x0b\x32\x1e.os.v2x.device.ipcamera.Header\x12\x10\n\x08\x66rame_id\x18\x02 \x01(\t\x12\x18\n\x10measurement_time\x18\x03 \x02(\x01\x12\x0e\n\x06height\x18\x04 \x01(\r\x12\r\n\x05width\x18\x05 \x01(\r\x12\x10\n\x08\x65ncoding\x18\x06 \x01(\t\x12\x0c\n\x04step\x18\x07 \x01(\r\x12\x0c\n\x04\x64\x61ta\x18\x08 \x01(\x0c\x12\x15\n\rexposure_time\x18\t \x01(\r\"\xb6\x01\n\x0f\x43ompressedImage\x12.\n\x06header\x18\x01 \x01(\x0b\x32\x1e.os.v2x.device.ipcamera.Header\x12\x10\n\x08\x66rame_id\x18\x02 \x01(\t\x12\x0e\n\x06\x66ormat\x18\x03 \x01(\t\x12\x0c\n\x04\x64\x61ta\x18\x04 \x01(\x0c\x12\x18\n\x10measurement_time\x18\x05 \x01(\x01\x12\x12\n\nframe_type\x18\x06 \x01(\r\x12\x15\n\rexposure_time\x18\x07 \x01(\r\"#\n\x13IpCameraReceiveData\x12\x0c\n\x04\x64\x61ta\x18\x01 \x01(\x0c*\xfb\x05\n\x0bPixelFormat\x12\t\n\x04RGB8\x10\xe9\x07\x12\n\n\x05RGBA8\x10\xea\x07\x12\n\n\x05RGB16\x10\xeb\x07\x12\x0b\n\x06RGBA16\x10\xec\x07\x12\t\n\x04\x42GR8\x10\xed\x07\x12\n\n\x05\x42GRA8\x10\xee\x07\x12\n\n\x05\x42GR16\x10\xef\x07\x12\x0b\n\x06\x42GRA16\x10\xf0\x07\x12\n\n\x05MONO8\x10\xf1\x07\x12\x0b\n\x06MONO16\x10\xf2\x07\x12\x0e\n\tTYPE_8UC1\x10\xd1\x0f\x12\x0e\n\tTYPE_8UC2\x10\xd2\x0f\x12\x0e\n\tTYPE_8UC3\x10\xd3\x0f\x12\x0e\n\tTYPE_8UC4\x10\xd4\x0f\x12\x0e\n\tTYPE_8SC1\x10\xd5\x0f\x12\x0e\n\tTYPE_8SC2\x10\xd6\x0f\x12\x0e\n\tTYPE_8SC3\x10\xd7\x0f\x12\x0e\n\tTYPE_8SC4\x10\xd8\x0f\x12\x0f\n\nTYPE_16UC1\x10\xd9\x0f\x12\x0f\n\nTYPE_16UC2\x10\xda\x0f\x12\x0f\n\nTYPE_16UC3\x10\xdb\x0f\x12\x0f\n\nTYPE_16UC4\x10\xdc\x0f\x12\x0f\n\nTYPE_16SC1\x10\xdd\x0f\x12\x0f\n\nTYPE_16SC2\x10\xde\x0f\x12\x0f\n\nTYPE_16SC3\x10\xdf\x0f\x12\x0f\n\nTYPE_16SC4\x10\xe0\x0f\x12\x0f\n\nTYPE_32SC1\x10\xe1\x0f\x12\x0f\n\nTYPE_32SC2\x10\xe2\x0f\x12\x0f\n\nTYPE_32SC3\x10\xe3\x0f\x12\x0f\n\nTYPE_32SC4\x10\xe4\x0f\x12\x0f\n\nTYPE_32FC1\x10\xe5\x0f\x12\x0f\n\nTYPE_32FC2\x10\xe6\x0f\x12\x0f\n\nTYPE_32FC3\x10\xe7\x0f\x12\x0f\n\nTYPE_32FC4\x10\xe8\x0f\x12\x0f\n\nTYPE_64FC1\x10\xe9\x0f\x12\x0f\n\nTYPE_64FC2\x10\xea\x0f\x12\x0f\n\nTYPE_64FC3\x10\xeb\x0f\x12\x0f\n\nTYPE_64FC4\x10\xec\x0f\x12\x10\n\x0b\x42\x41YER_RGGB8\x10\xb9\x17\x12\x10\n\x0b\x42\x41YER_BGGR8\x10\xba\x17\x12\x10\n\x0b\x42\x41YER_GBRG8\x10\xbb\x17\x12\x10\n\x0b\x42\x41YER_GRBG8\x10\xbc\x17\x12\x11\n\x0c\x42\x41YER_RGGB16\x10\xbd\x17\x12\x11\n\x0c\x42\x41YER_BGGR16\x10\xbe\x17\x12\x11\n\x0c\x42\x41YER_GBRG16\x10\xbf\x17\x12\x11\n\x0c\x42\x41YER_GRBG16\x10\xc0\x17\x12\x0b\n\x06YUV422\x10\xa1\x1f'
+)
+
+_PIXELFORMAT = _descriptor.EnumDescriptor(
+  name='PixelFormat',
+  full_name='os.v2x.device.ipcamera.PixelFormat',
+  filename=None,
+  file=DESCRIPTOR,
+  create_key=_descriptor._internal_create_key,
+  values=[
+    _descriptor.EnumValueDescriptor(
+      name='RGB8', index=0, number=1001,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='RGBA8', index=1, number=1002,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='RGB16', index=2, number=1003,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='RGBA16', index=3, number=1004,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BGR8', index=4, number=1005,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BGRA8', index=5, number=1006,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BGR16', index=6, number=1007,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BGRA16', index=7, number=1008,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='MONO8', index=8, number=1009,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='MONO16', index=9, number=1010,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_8UC1', index=10, number=2001,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_8UC2', index=11, number=2002,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_8UC3', index=12, number=2003,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_8UC4', index=13, number=2004,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_8SC1', index=14, number=2005,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_8SC2', index=15, number=2006,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_8SC3', index=16, number=2007,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_8SC4', index=17, number=2008,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_16UC1', index=18, number=2009,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_16UC2', index=19, number=2010,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_16UC3', index=20, number=2011,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_16UC4', index=21, number=2012,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_16SC1', index=22, number=2013,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_16SC2', index=23, number=2014,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_16SC3', index=24, number=2015,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_16SC4', index=25, number=2016,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_32SC1', index=26, number=2017,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_32SC2', index=27, number=2018,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_32SC3', index=28, number=2019,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_32SC4', index=29, number=2020,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_32FC1', index=30, number=2021,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_32FC2', index=31, number=2022,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_32FC3', index=32, number=2023,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_32FC4', index=33, number=2024,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_64FC1', index=34, number=2025,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_64FC2', index=35, number=2026,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_64FC3', index=36, number=2027,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='TYPE_64FC4', index=37, number=2028,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BAYER_RGGB8', index=38, number=3001,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BAYER_BGGR8', index=39, number=3002,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BAYER_GBRG8', index=40, number=3003,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BAYER_GRBG8', index=41, number=3004,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BAYER_RGGB16', index=42, number=3005,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BAYER_BGGR16', index=43, number=3006,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BAYER_GBRG16', index=44, number=3007,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='BAYER_GRBG16', index=45, number=3008,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+    _descriptor.EnumValueDescriptor(
+      name='YUV422', index=46, number=4001,
+      serialized_options=None,
+      type=None,
+      create_key=_descriptor._internal_create_key),
+  ],
+  containing_type=None,
+  serialized_options=None,
+  serialized_start=805,
+  serialized_end=1568,
+)
+_sym_db.RegisterEnumDescriptor(_PIXELFORMAT)
+
+PixelFormat = enum_type_wrapper.EnumTypeWrapper(_PIXELFORMAT)
+RGB8 = 1001
+RGBA8 = 1002
+RGB16 = 1003
+RGBA16 = 1004
+BGR8 = 1005
+BGRA8 = 1006
+BGR16 = 1007
+BGRA16 = 1008
+MONO8 = 1009
+MONO16 = 1010
+TYPE_8UC1 = 2001
+TYPE_8UC2 = 2002
+TYPE_8UC3 = 2003
+TYPE_8UC4 = 2004
+TYPE_8SC1 = 2005
+TYPE_8SC2 = 2006
+TYPE_8SC3 = 2007
+TYPE_8SC4 = 2008
+TYPE_16UC1 = 2009
+TYPE_16UC2 = 2010
+TYPE_16UC3 = 2011
+TYPE_16UC4 = 2012
+TYPE_16SC1 = 2013
+TYPE_16SC2 = 2014
+TYPE_16SC3 = 2015
+TYPE_16SC4 = 2016
+TYPE_32SC1 = 2017
+TYPE_32SC2 = 2018
+TYPE_32SC3 = 2019
+TYPE_32SC4 = 2020
+TYPE_32FC1 = 2021
+TYPE_32FC2 = 2022
+TYPE_32FC3 = 2023
+TYPE_32FC4 = 2024
+TYPE_64FC1 = 2025
+TYPE_64FC2 = 2026
+TYPE_64FC3 = 2027
+TYPE_64FC4 = 2028
+BAYER_RGGB8 = 3001
+BAYER_BGGR8 = 3002
+BAYER_GBRG8 = 3003
+BAYER_GRBG8 = 3004
+BAYER_RGGB16 = 3005
+BAYER_BGGR16 = 3006
+BAYER_GBRG16 = 3007
+BAYER_GRBG16 = 3008
+YUV422 = 4001
+
+
+
+_HEADER = _descriptor.Descriptor(
+  name='Header',
+  full_name='os.v2x.device.ipcamera.Header',
+  filename=None,
+  file=DESCRIPTOR,
+  containing_type=None,
+  create_key=_descriptor._internal_create_key,
+  fields=[
+    _descriptor.FieldDescriptor(
+      name='timestamp_sec', full_name='os.v2x.device.ipcamera.Header.timestamp_sec', index=0,
+      number=1, type=1, cpp_type=5, label=1,
+      has_default_value=False, default_value=float(0),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='module_name', full_name='os.v2x.device.ipcamera.Header.module_name', index=1,
+      number=2, type=9, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"".decode('utf-8'),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='sequence_num', full_name='os.v2x.device.ipcamera.Header.sequence_num', index=2,
+      number=3, type=13, cpp_type=3, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='lidar_timestamp', full_name='os.v2x.device.ipcamera.Header.lidar_timestamp', index=3,
+      number=4, type=4, cpp_type=4, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='camera_timestamp', full_name='os.v2x.device.ipcamera.Header.camera_timestamp', index=4,
+      number=5, type=4, cpp_type=4, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='radar_timestamp', full_name='os.v2x.device.ipcamera.Header.radar_timestamp', index=5,
+      number=6, type=4, cpp_type=4, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='start_time', full_name='os.v2x.device.ipcamera.Header.start_time', index=6,
+      number=7, type=1, cpp_type=5, label=1,
+      has_default_value=False, default_value=float(0),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='finish_time', full_name='os.v2x.device.ipcamera.Header.finish_time', index=7,
+      number=8, type=1, cpp_type=5, label=1,
+      has_default_value=False, default_value=float(0),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='depended_modules', full_name='os.v2x.device.ipcamera.Header.depended_modules', index=8,
+      number=9, type=11, cpp_type=10, label=3,
+      has_default_value=False, default_value=[],
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='frame_id', full_name='os.v2x.device.ipcamera.Header.frame_id', index=9,
+      number=10, type=9, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"".decode('utf-8'),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='stamp', full_name='os.v2x.device.ipcamera.Header.stamp', index=10,
+      number=11, type=4, cpp_type=4, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+  ],
+  extensions=[
+  ],
+  nested_types=[],
+  enum_types=[
+  ],
+  serialized_options=None,
+  is_extendable=False,
+  syntax='proto2',
+  extension_ranges=[],
+  oneofs=[
+  ],
+  serialized_start=43,
+  serialized_end=325,
+)
+
+
+_MODULE = _descriptor.Descriptor(
+  name='Module',
+  full_name='os.v2x.device.ipcamera.Module',
+  filename=None,
+  file=DESCRIPTOR,
+  containing_type=None,
+  create_key=_descriptor._internal_create_key,
+  fields=[
+    _descriptor.FieldDescriptor(
+      name='module_name', full_name='os.v2x.device.ipcamera.Module.module_name', index=0,
+      number=1, type=9, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"".decode('utf-8'),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='sequence_num', full_name='os.v2x.device.ipcamera.Module.sequence_num', index=1,
+      number=2, type=13, cpp_type=3, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+  ],
+  extensions=[
+  ],
+  nested_types=[],
+  enum_types=[
+  ],
+  serialized_options=None,
+  is_extendable=False,
+  syntax='proto2',
+  extension_ranges=[],
+  oneofs=[
+  ],
+  serialized_start=327,
+  serialized_end=378,
+)
+
+
+_IMAGE = _descriptor.Descriptor(
+  name='Image',
+  full_name='os.v2x.device.ipcamera.Image',
+  filename=None,
+  file=DESCRIPTOR,
+  containing_type=None,
+  create_key=_descriptor._internal_create_key,
+  fields=[
+    _descriptor.FieldDescriptor(
+      name='header', full_name='os.v2x.device.ipcamera.Image.header', index=0,
+      number=1, type=11, cpp_type=10, label=1,
+      has_default_value=False, default_value=None,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='frame_id', full_name='os.v2x.device.ipcamera.Image.frame_id', index=1,
+      number=2, type=9, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"".decode('utf-8'),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='measurement_time', full_name='os.v2x.device.ipcamera.Image.measurement_time', index=2,
+      number=3, type=1, cpp_type=5, label=2,
+      has_default_value=False, default_value=float(0),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='height', full_name='os.v2x.device.ipcamera.Image.height', index=3,
+      number=4, type=13, cpp_type=3, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='width', full_name='os.v2x.device.ipcamera.Image.width', index=4,
+      number=5, type=13, cpp_type=3, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='encoding', full_name='os.v2x.device.ipcamera.Image.encoding', index=5,
+      number=6, type=9, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"".decode('utf-8'),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='step', full_name='os.v2x.device.ipcamera.Image.step', index=6,
+      number=7, type=13, cpp_type=3, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='data', full_name='os.v2x.device.ipcamera.Image.data', index=7,
+      number=8, type=12, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"",
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='exposure_time', full_name='os.v2x.device.ipcamera.Image.exposure_time', index=8,
+      number=9, type=13, cpp_type=3, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+  ],
+  extensions=[
+  ],
+  nested_types=[],
+  enum_types=[
+  ],
+  serialized_options=None,
+  is_extendable=False,
+  syntax='proto2',
+  extension_ranges=[],
+  oneofs=[
+  ],
+  serialized_start=381,
+  serialized_end=580,
+)
+
+
+_COMPRESSEDIMAGE = _descriptor.Descriptor(
+  name='CompressedImage',
+  full_name='os.v2x.device.ipcamera.CompressedImage',
+  filename=None,
+  file=DESCRIPTOR,
+  containing_type=None,
+  create_key=_descriptor._internal_create_key,
+  fields=[
+    _descriptor.FieldDescriptor(
+      name='header', full_name='os.v2x.device.ipcamera.CompressedImage.header', index=0,
+      number=1, type=11, cpp_type=10, label=1,
+      has_default_value=False, default_value=None,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='frame_id', full_name='os.v2x.device.ipcamera.CompressedImage.frame_id', index=1,
+      number=2, type=9, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"".decode('utf-8'),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='format', full_name='os.v2x.device.ipcamera.CompressedImage.format', index=2,
+      number=3, type=9, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"".decode('utf-8'),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='data', full_name='os.v2x.device.ipcamera.CompressedImage.data', index=3,
+      number=4, type=12, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"",
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='measurement_time', full_name='os.v2x.device.ipcamera.CompressedImage.measurement_time', index=4,
+      number=5, type=1, cpp_type=5, label=1,
+      has_default_value=False, default_value=float(0),
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='frame_type', full_name='os.v2x.device.ipcamera.CompressedImage.frame_type', index=5,
+      number=6, type=13, cpp_type=3, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+    _descriptor.FieldDescriptor(
+      name='exposure_time', full_name='os.v2x.device.ipcamera.CompressedImage.exposure_time', index=6,
+      number=7, type=13, cpp_type=3, label=1,
+      has_default_value=False, default_value=0,
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+  ],
+  extensions=[
+  ],
+  nested_types=[],
+  enum_types=[
+  ],
+  serialized_options=None,
+  is_extendable=False,
+  syntax='proto2',
+  extension_ranges=[],
+  oneofs=[
+  ],
+  serialized_start=583,
+  serialized_end=765,
+)
+
+
+_IPCAMERARECEIVEDATA = _descriptor.Descriptor(
+  name='IpCameraReceiveData',
+  full_name='os.v2x.device.ipcamera.IpCameraReceiveData',
+  filename=None,
+  file=DESCRIPTOR,
+  containing_type=None,
+  create_key=_descriptor._internal_create_key,
+  fields=[
+    _descriptor.FieldDescriptor(
+      name='data', full_name='os.v2x.device.ipcamera.IpCameraReceiveData.data', index=0,
+      number=1, type=12, cpp_type=9, label=1,
+      has_default_value=False, default_value=b"",
+      message_type=None, enum_type=None, containing_type=None,
+      is_extension=False, extension_scope=None,
+      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
+  ],
+  extensions=[
+  ],
+  nested_types=[],
+  enum_types=[
+  ],
+  serialized_options=None,
+  is_extendable=False,
+  syntax='proto2',
+  extension_ranges=[],
+  oneofs=[
+  ],
+  serialized_start=767,
+  serialized_end=802,
+)
+
+_HEADER.fields_by_name['depended_modules'].message_type = _MODULE
+_IMAGE.fields_by_name['header'].message_type = _HEADER
+_COMPRESSEDIMAGE.fields_by_name['header'].message_type = _HEADER
+DESCRIPTOR.message_types_by_name['Header'] = _HEADER
+DESCRIPTOR.message_types_by_name['Module'] = _MODULE
+DESCRIPTOR.message_types_by_name['Image'] = _IMAGE
+DESCRIPTOR.message_types_by_name['CompressedImage'] = _COMPRESSEDIMAGE
+DESCRIPTOR.message_types_by_name['IpCameraReceiveData'] = _IPCAMERARECEIVEDATA
+DESCRIPTOR.enum_types_by_name['PixelFormat'] = _PIXELFORMAT
+_sym_db.RegisterFileDescriptor(DESCRIPTOR)
+
+Header = _reflection.GeneratedProtocolMessageType('Header', (_message.Message,), {
+  'DESCRIPTOR' : _HEADER,
+  '__module__' : 'ipcamera_pb2'
+  # @@protoc_insertion_point(class_scope:os.v2x.device.ipcamera.Header)
+  })
+_sym_db.RegisterMessage(Header)
+
+Module = _reflection.GeneratedProtocolMessageType('Module', (_message.Message,), {
+  'DESCRIPTOR' : _MODULE,
+  '__module__' : 'ipcamera_pb2'
+  # @@protoc_insertion_point(class_scope:os.v2x.device.ipcamera.Module)
+  })
+_sym_db.RegisterMessage(Module)
+
+Image = _reflection.GeneratedProtocolMessageType('Image', (_message.Message,), {
+  'DESCRIPTOR' : _IMAGE,
+  '__module__' : 'ipcamera_pb2'
+  # @@protoc_insertion_point(class_scope:os.v2x.device.ipcamera.Image)
+  })
+_sym_db.RegisterMessage(Image)
+
+CompressedImage = _reflection.GeneratedProtocolMessageType('CompressedImage', (_message.Message,), {
+  'DESCRIPTOR' : _COMPRESSEDIMAGE,
+  '__module__' : 'ipcamera_pb2'
+  # @@protoc_insertion_point(class_scope:os.v2x.device.ipcamera.CompressedImage)
+  })
+_sym_db.RegisterMessage(CompressedImage)
+
+IpCameraReceiveData = _reflection.GeneratedProtocolMessageType('IpCameraReceiveData', (_message.Message,), {
+  'DESCRIPTOR' : _IPCAMERARECEIVEDATA,
+  '__module__' : 'ipcamera_pb2'
+  # @@protoc_insertion_point(class_scope:os.v2x.device.ipcamera.IpCameraReceiveData)
+  })
+_sym_db.RegisterMessage(IpCameraReceiveData)
+
+
+# @@protoc_insertion_point(module_scope)
```
