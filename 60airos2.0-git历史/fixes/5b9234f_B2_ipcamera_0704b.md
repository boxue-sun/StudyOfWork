# feat(设备接入)【FAR202505-00064】设备接入网络相机framework和module实现

- 作者：changxuhui
- 日期：2025-07-04
- 完整 hash：`5b9234f531f52880863cd1cb57ab97c11c491d26`
- 改动：+1020 / -8 行

```diff
@@@C	changxuhui	5b9234f531f52880863cd1cb57ab97c11c491d26	2025-07-04	feat(设备接入)【FAR202505-00064】设备接入网络相机framework和module实现
diff --git a/base/common/log.h b/base/common/log.h
index 1ea395e..d56b427 100644
--- a/base/common/log.h
+++ b/base/common/log.h
@@ -88,12 +88,12 @@
 #define TRAFFIC_LIFGT_SERVICE_LOG_ERROR LOG_ERROR_IF << LOG_KEY_TRAFFIC_LIFGT_SERVICE
 #define TRAFFIC_LIFGT_SERVICE_LOG_FATAL LOG_FATAL_IF << LOG_KEY_TRAFFIC_LIFGT_SERVICE
 
-// 接入层 camera_cictci 模块
-#define LOG_KEY_CAMERA_CICTCI_SERVICE "[camera_cictci_service]"
-#define CAMERA_CICTCI_SERVICE_LOG_INFO LOG_INFO_IF << LOG_KEY_CAMERA_CICTCI_SERVICE
-#define CAMERA_CICTCI_SERVICE_LOG_WARN LOG_WARN_IF << LOG_KEY_CAMERA_CICTCI_SERVICE
-#define CAMERA_CICTCI_SERVICE_LOG_ERROR LOG_ERROR_IF << LOG_KEY_CAMERA_CICTCI_SERVICE
-#define CAMERA_CICTCI_SERVICE_LOG_FATAL LOG_FATAL_IF << LOG_KEY_CAMERA_CICTCI_SERVICE
+// 接入层 device_service ipcamera 模块
+#define LOG_KEY_IPCAMERA_SERVICE "[ipcamera_service]"
+#define IPCAMERA_SERVICE_LOG_INFO LOG_INFO_IF << LOG_KEY_IPCAMERA_SERVICE
+#define IPCAMERA_SERVICE_LOG_WARN LOG_WARN_IF << LOG_KEY_IPCAMERA_SERVICE
+#define IPCAMERA_SERVICE_LOG_ERROR LOG_ERROR_IF << LOG_KEY_IPCAMERA_SERVICE
+#define IPCAMERA_SERVICE_LOG_FATAL LOG_FATAL_IF << LOG_KEY_IPCAMERA_SERVICE
 
 
 #define LOG_KEY_RASP "[rsap]"
diff --git a/base/device_connect/ipcamera/device_factory.cc b/base/device_connect/ipcamera/device_factory.cc
index b798680..a87a7e6 100644
--- a/base/device_connect/ipcamera/device_factory.cc
+++ b/base/device_connect/ipcamera/device_factory.cc
@@ -14,6 +14,7 @@
  *
  * @endverbatim
  ********************************************************************************/
+
 #include "base/device_connect/ipcamera/device_factory.h"
 
 namespace os {
diff --git a/base/device_connect/proto/ipcamera.proto b/base/device_connect/proto/ipcamera.proto
new file mode 100644
index 0000000..c472817
--- /dev/null
+++ b/base/device_connect/proto/ipcamera.proto
@@ -0,0 +1,138 @@
+syntax = "proto2";
+
+package os.v2x.device.ipcamera;
+
+message Header {
+  // Message publishing time in seconds. It is recommended to obtain timestamp_sec from
+  // ros::Time::now(), right before calling SerializeToString() and publish().
+  optional double timestamp_sec = 1;
+
+  // Module name.
+  optional string module_name = 2;
+
+  // Sequence number for each message. Each module maintains its own counter for sequence_num,
+  // always starting from 0 on boot.
+  optional uint32 sequence_num = 3;
+
+  // Lidar Sensor timestamp for nano-second.
+  optional uint64 lidar_timestamp = 4;
+
+  // Camera Sensor timestamp for nano-second.
+  optional uint64 camera_timestamp = 5;
+
+  // Radar Sensor timestamp for nano-second.
+  optional uint64 radar_timestamp = 6;
+
+  // Module Start timestamp, shall be consistant with finish timestamp
+  optional double start_time = 7;
+
+  // Module Finish timestamp
+  optional double finish_time = 8;
+
+  // Depended Module ID
+  repeated Module depended_modules = 9;
+
+  // The frame id of message
+  optional string frame_id = 10;
+
+  // The stamp when message is publishing
+  optional uint64 stamp = 11;
+
+}
+
+message Module {
+  optional string module_name = 1;
+  optional uint32 sequence_num = 2;
+}
+
+// Encoding of pixels -- channel meaning, ordering, size
+enum PixelFormat {
+    RGB8    = 1001;
+    RGBA8   = 1002;
+    RGB16   = 1003;
+    RGBA16  = 1004;
+    BGR8    = 1005;
+    BGRA8   = 1006;
+    BGR16   = 1007;
+    BGRA16  = 1008;
+    MONO8   = 1009;
+    MONO16  = 1010;
+
+    // OpenCV CvMat types
+    TYPE_8UC1   = 2001;
+    TYPE_8UC2   = 2002;
+    TYPE_8UC3   = 2003;
+    TYPE_8UC4   = 2004;
+    TYPE_8SC1   = 2005;
+    TYPE_8SC2   = 2006;
+    TYPE_8SC3   = 2007;
+    TYPE_8SC4   = 2008;
+    TYPE_16UC1  = 2009;
+    TYPE_16UC2  = 2010;
+    TYPE_16UC3  = 2011;
+    TYPE_16UC4  = 2012;
+    TYPE_16SC1  = 2013;
+    TYPE_16SC2  = 2014;
+    TYPE_16SC3  = 2015;
+    TYPE_16SC4  = 2016;
+    TYPE_32SC1  = 2017;
+    TYPE_32SC2  = 2018;
+    TYPE_32SC3  = 2019;
+    TYPE_32SC4  = 2020;
+    TYPE_32FC1  = 2021;
+    TYPE_32FC2  = 2022;
+    TYPE_32FC3  = 2023;
+    TYPE_32FC4  = 2024;
+    TYPE_64FC1  = 2025;
+    TYPE_64FC2  = 2026;
+    TYPE_64FC3  = 2027;
+    TYPE_64FC4  = 2028;
+
+    // Bayer encodings
+    BAYER_RGGB8     = 3001;
+    BAYER_BGGR8     = 3002;
+    BAYER_GBRG8     = 3003;
+    BAYER_GRBG8     = 3004;
+    BAYER_RGGB16    = 3005;
+    BAYER_BGGR16    = 3006;
+    BAYER_GBRG16    = 3007;
+    BAYER_GRBG16    = 3008;
+
+    // Miscellaneous
+    // This is the UYVY version of YUV422 codec http://www.fourcc.org/yuv.php#UYVY
+    // with an 8-bit depth
+    YUV422 = 4001;
+}
+
+message Image {
+    optional Header header = 1;
+    optional string frame_id = 2;
+    required double measurement_time = 3;
+
+    optional uint32 height = 4; // image height, that is, number of rows
+    optional uint32 width = 5; // image width, that is, number of columns
+
+    optional string encoding = 6;
+    optional uint32 step = 7; // Full row length in bytes
+    optional bytes data = 8; // actual matrix data, size is (step * rows)
+    optional uint32 exposure_time = 9; // image exposure time, unit is us, 0 means invalid
+}
+
+message CompressedImage {
+    optional Header header = 1;
+    optional string frame_id = 2;
+
+    // Specifies the format of the data
+    //  Acceptable values: jpeg, png, h265, h264
+    optional string format = 3; 
+
+    optional bytes data = 4; // Compressed image buffer
+    optional double measurement_time = 5; //frame  gps times from Image messag
+    optional uint32 frame_type = 6; //IDR:1, others:0
+    optional uint32 exposure_time = 7; // image exposure time, unit is us, 0 means invalid
+}
+
+// 设备接入层的红绿灯设备写入
+message IpCameraReceiveData {
+  optional bytes data = 1; // 数据内容
+};
diff --git a/build_airos.sh b/build_airos.sh
index 051bfaa..152b2de 100644
--- a/build_airos.sh
+++ b/build_airos.sh
@@ -6,7 +6,7 @@ TOP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)" # 得到当前脚本
 ARCH="$(uname -m)"
 SUPPORTED_ARCHS=" x86_64 aarch64 "
 
-SUPPORTED_DEVICE_SERVICE=(traffic_light rsu camera lidar mec )
+SUPPORTED_DEVICE_SERVICE=(traffic_light rsu camera lidar mec ipcamera )
 #############################################################
 protocol_version="v3.3"
 RELEASE_ROOT="/home/airos"
@@ -158,7 +158,7 @@ function env_setup() {
 }
 
 function run_base_device_connect_ut() {
-    local support_device=(rsu traffic_light ins radar lidar mec)
+    local support_device=(rsu traffic_light ins radar lidar mec ipcamera)
     for device in ${support_device[@]}
     do
         pushd ./bazel-bin/${device_modules_path}/${device} > /dev/null
diff --git a/middleware/device_service/framework/ipcamera/BUILD b/middleware/device_service/framework/ipcamera/BUILD
new file mode 100644
index 0000000..349797f
--- /dev/null
+++ b/middleware/device_service/framework/ipcamera/BUILD
@@ -0,0 +1,31 @@
+load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
+
+cc_binary(
+    name = "libipcamera_component.so",
+    linkshared = True,
+    linkstatic = False,
+    deps = [":ipcamera_component_lib"],
+)
+
+cc_library(
+    name = "ipcamera_component_lib",
+    srcs = glob(["*.cc"]),
+    hdrs = glob(["*.h"]),
+    linkopts = [
+        "-pthread",
+    ],
+    deps = [
+        "//base",
+        "//middleware/device_service/framework/proto:ipcamera_config_cc_pb",
+        "//middleware/runtime:airosrt",
+        "@local//:fastrtps",
+        "@local//:gflags",
+        "@local//:glog",
+        "@local//:protobuf",
+        "@local//:yaml-cpp",
+        "//middleware/device_service/modules/ipcamera:libipcamera_device",
+    ],
+    copts = [
+        "-DMODULE_NAME=\\\"ipcamera_service\\\"",
+    ],
+) 
\ No newline at end of file
diff --git a/middleware/device_service/framework/ipcamera/conf/config.pb.txt b/middleware/device_service/framework/ipcamera/conf/config.pb.txt
new file mode 100644
index 0000000..f933bd6
--- /dev/null
+++ b/middleware/device_service/framework/ipcamera/conf/config.pb.txt
@@ -0,0 +1,2 @@
+device: "standard_ipcamera"
+config_file: "/home/airos/param/device/ipcamera/ipcamera_config.yaml" 
\ No newline at end of file
diff --git a/middleware/device_service/framework/ipcamera/conf/ipcamera_config.yaml b/middleware/device_service/framework/ipcamera/conf/ipcamera_config.yaml
new file mode 100644
index 0000000..55ec502
--- /dev/null
+++ b/middleware/device_service/framework/ipcamera/conf/ipcamera_config.yaml
@@ -0,0 +1,18 @@
+# IP Camera 配置文件
+frame_id: "camera_frame"
+# 多路RTSP拉流地址配置
+rtsp_urls:
+  - "rtsp://root:root@172.20.65.193:8554/stream1"
+  - "rtsp://root:root@172.20.65.193:8554/stream2"
+  - "rtsp://root:root@172.20.65.193:8554/stream3"
+  - "rtsp://root:root@172.20.65.193:8554/stream4"
+# 输出话题配置（与rtsp_urls一一对应）
+output_topics:
+  - "/sensor/ipcamera/h264/172_20_65_193"
+  - "/sensor/ipcamera/h264/172_20_65_194"
+  - "/sensor/ipcamera/h264/172_20_65_195"
+  - "/sensor/ipcamera/h264/172_20_65_196"
+reconnect_interval: 5  # 重连间隔（秒）
+buffer_size: 1024000   # 缓冲区大小
+connection_timeout: 10 # 连接超时时间（秒）
+enable_sei_timestamp: true  # 是否启用SEI时间戳解析
diff --git a/middleware/device_service/framework/ipcamera/dag/ipcamera.dag b/middleware/device_service/framework/ipcamera/dag/ipcamera.dag
new file mode 100644
index 0000000..7a56c59
--- /dev/null
+++ b/middleware/device_service/framework/ipcamera/dag/ipcamera.dag
@@ -0,0 +1,15 @@
+# proto-file: dag_conf.proto
+# proto-message: DagConfig
+module_config {
+    module_library: "/home/airos/os/lib/libipcamera_component.so"
+    components{
+        class_name: "IpCameraComponent"
+        config{
+            name: "IpCameraComponent"
+            readers{
+                channel: "/v2x/device/ipcamera/receive_data"
+            }
+            config_file_path: "/home/airos/os/conf/ipcamera/config.pb.txt"
+        }
+    }
+}
\ No newline at end of file
diff --git a/middleware/device_service/framework/ipcamera/ipcamera_component.cc b/middleware/device_service/framework/ipcamera/ipcamera_component.cc
new file mode 100644
index 0000000..b027a9c
--- /dev/null
+++ b/middleware/device_service/framework/ipcamera/ipcamera_component.cc
@@ -0,0 +1,82 @@
+/*********************************************************************************
+ * @file		ipcamera_component.cc
+ * @brief		IP Camera组件实现
+ * @details		IP Camera组件的具体实现，提供设备管理和数据流处理功能
+ * @author		ChangXuhui
+ * @date		2025/7/4
+ * @copyright	Copyright (c) 2025 The Airos Authors.
+ * @verbatim
+ *
+ *  Change History:
+ *  Date         Author        Version      ChangeId           Description
+ *  ------------------------------------------------------------------------------
+ *  2025/1/13  ChangXuhui      1.0          ————               IP Camera组件实现
+ *
+ * @endverbatim
+ ********************************************************************************/
+
+#include "ipcamera_component.h"
+
+#include "base/common/log.h"
+#include "base/common/time_util.h"
+#include "base/device_connect/ipcamera/device_factory.h"
+#include "base/plugin/modules_loader/dynamic_loader.h"
+
+namespace os {
+namespace v2x {
+namespace device {
+
+bool AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)::Init() {
+  // 加载设备库
+  airos::base::DynamicLoader::GetInstance().LoadDevicePackage(
+      "device/lib/ipcamera/");
+      
+  if (!LoadConfig(&conf_)) {
+    IPCAMERA_SERVICE_LOG_ERROR << "load component proto config error";
+    return false;
+  }
+
+  IPCAMERA_SERVICE_LOG_INFO << "device:" << conf_.device();
+  
+  // 创建设备实例，使用多路流回调
+  device_ = IpCameraDeviceFactory::Instance().GetUnique(
+      conf_.device(),
+      std::bind(
+          &AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)::MultiStreamCallBack,
+          this,
+          std::placeholders::_1,
+          std::placeholders::_2));
+
+  if (device_ == nullptr || !device_->Init(conf_.config_file())) {
+    IPCAMERA_SERVICE_LOG_ERROR << "device_ init error";
+    return false;
+  }
+
+  IPCAMERA_SERVICE_LOG_INFO << "device init success!";
+
+  // 启动设备线程
+  task_.reset(new std::thread([&]() {
+    device_->Start();
+  }));
+
+  return true;
+}
+
+bool AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)::Proc(
+    const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& recv_data) {
+  // 将接收到的数据写入设备（用于控制命令等）
+  if (device_ && recv_data) {
+    device_->WriteToDevice(recv_data);
+  }
+  return true;
+}
+
+void AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)::MultiStreamCallBack(
+    const std::string& stream_id, const IpCameraDataType& data) {
+  // 发送压缩图像数据到对应的话题
+  Send(stream_id, data);
+}
+
+}  // namespace device
+}  // namespace v2x
+}  // namespace os 
\ No newline at end of file
diff --git a/middleware/device_service/framework/ipcamera/ipcamera_component.h b/middleware/device_service/framework/ipcamera/ipcamera_component.h
new file mode 100644
index 0000000..057bbbf
--- /dev/null
+++ b/middleware/device_service/framework/ipcamera/ipcamera_component.h
@@ -0,0 +1,53 @@
+/*********************************************************************************
+ * @file		ipcamera_component.h
+ * @brief		IP Camera组件头文件
+ * @details		IP Camera组件的头文件定义，包含组件类声明和接口定义
+ * @author		ChangXuhui
+ * @date		2025/7/4
+ * @copyright	Copyright (c) 2025 The Airos Authors.
+ * @verbatim
+ *
+ *  Change History:
+ *  Date         Author        Version      ChangeId           Description
+ *  ------------------------------------------------------------------------------
+ *  2025/1/13  ChangXuhui      1.0          ————               IP Camera组件头文件
+ *
+ * @endverbatim
+ ********************************************************************************/
+
+#pragma once
+
+#include <thread>
+
+#include "base/device_connect/ipcamera/device_base.h"
+#include "middleware/runtime/src/air_middleware_component.h"
+#include "middleware/device_service/framework/proto/ipcamera_config.pb.h"
+
+namespace os {
+namespace v2x {
+namespace device {
+
+class AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)
+    : public airos::middleware::ComponentAdapter<os::v2x::device::ipcamera::IpCameraReceiveData> {
+ public:
+  AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)()           = default;
+  ~AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)() override = default;
+
+  bool Init() override;
+  bool Proc(const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& recv_data)
+      override;
+
+ private:
+  void MultiStreamCallBack(const std::string& stream_id, const IpCameraDataType& data);
+
+ private:
+  std::unique_ptr<IpCameraDevice> device_;
+  std::unique_ptr<std::thread> task_;
+  os::v2x::device::ipcamera::Config conf_;
+};
+
+REGISTER_AIROS_COMPONENT_CLASS(IpCameraComponent, os::v2x::device::ipcamera::IpCameraReceiveData);
+
+}  // namespace device
+}  // namespace v2x
+}  // namespace os 
\ No newline at end of file
diff --git a/middleware/device_service/framework/proto/BUILD b/middleware/device_service/framework/proto/BUILD
index f4f3344..9dfd319 100644
--- a/middleware/device_service/framework/proto/BUILD
+++ b/middleware/device_service/framework/proto/BUILD
@@ -53,3 +53,13 @@ cc_so_proto_library(
     name = "camera_cictci_config_cc_pb",
     srcs = [":camera_cictci_config_pb"],
 )
+
+proto_library(
+    name = "ipcamera_config_pb",
+    srcs = ["ipcamera_config.proto"],
+)
+
+cc_so_proto_library(
+    name = "ipcamera_config_cc_pb",
+    srcs = [":ipcamera_config_pb"],
+)
diff --git a/middleware/device_service/framework/proto/ipcamera_config.proto b/middleware/device_service/framework/proto/ipcamera_config.proto
new file mode 100644
index 0000000..1ac36fb
--- /dev/null
+++ b/middleware/device_service/framework/proto/ipcamera_config.proto
@@ -0,0 +1,8 @@
+syntax = "proto2";
+
+package os.v2x.device.ipcamera;
+
+message Config {
+  optional string device = 1;
+  optional string config_file = 2;
+} 
\ No newline at end of file
diff --git a/middleware/device_service/modules/ipcamera/BUILD b/middleware/device_service/modules/ipcamera/BUILD
new file mode 100644
index 0000000..46bf5f1
--- /dev/null
+++ b/middleware/device_service/modules/ipcamera/BUILD
@@ -0,0 +1,26 @@
+load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
+
+cc_library(
+    name = "libipcamera_device",
+    srcs = ["libipcamera_device.so"],
+    visibility = ["//visibility:public"],
+)
+
+cc_binary(
+    name = "libipcamera_device.so",
+    srcs = glob([
+        "standard_ipcamera/*",
+    ]),
+    linkshared = True,
+    linkstatic = False,
+    copts = [
+        "-DMODULE_NAME=\\\"ipcamera_service\\\"",
+    ],
+    deps = [
+        "//base",
+        "//base/device_connect/proto:airos_device_pb",
+        "@local//:glog",
+        "@local//:yaml-cpp",
+        "@local//:ffmpeg",
+    ],
+) 
\ No newline at end of file
diff --git a/middleware/device_service/modules/ipcamera/conf/device_lib_cfg.pb b/middleware/device_service/modules/ipcamera/conf/device_lib_cfg.pb
new file mode 100644
index 0000000..8e2368b
--- /dev/null
+++ b/middleware/device_service/modules/ipcamera/conf/device_lib_cfg.pb
@@ -0,0 +1,2 @@
+so_name: "libipcamera_device.so"
+device_name: "standard_ipcamera" 
\ No newline at end of file
diff --git a/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.cc b/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.cc
new file mode 100644
index 0000000..7e1b8ff
--- /dev/null
+++ b/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.cc
@@ -0,0 +1,519 @@
+/*********************************************************************************
+ * @file		standard_ipcamera.cc
+ * @brief		标准IP Camera设备实现
+ * @details		标准IP Camera设备的具体实现，支持多路RTSP流处理和H264解码
+ * @author		ChangXuhui
+ * @date		2025/7/4
+ * @copyright	Copyright (c) 2025 The Airos Authors.
+ * @verbatim
+ *
+ *  Change History:
+ *  Date         Author        Version      ChangeId           Description
+ *  ------------------------------------------------------------------------------
+ *  2025/1/13  ChangXuhui      1.0          ————               标准IP Camera设备实现
+ *
+ * @endverbatim
+ ********************************************************************************/
+
+#include "standard_ipcamera.h"
+#include "base/device_connect/ipcamera/device_factory.h"
+#include "base/common/time_util.h"
+#include "base/common/log.h"
+#include <chrono>
+#include <memory>
+#include <sstream>
+#include <iomanip>
+#include <regex>
+#include <string>
+
+namespace os {
+namespace v2x {
+namespace device {
+
+bool StandardIpCamera::Init(const std::string& conf) {
+  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera::Init config file: " << conf;
+  
+  try {
+    YAML::Node config = YAML::LoadFile(conf);
+    
+    // 读取配置参数
+    frame_id_ = config["frame_id"].as<std::string>("camera_frame");
+    reconnect_interval_ = config["reconnect_interval"].as<int>(5);
+    buffer_size_ = config["buffer_size"].as<int>(1024000);
+    connection_timeout_ = config["connection_timeout"].as<int>(10);
+    enable_sei_timestamp_ = config["enable_sei_timestamp"].as<bool>(true);
+    
+    // 读取多路RTSP URL
+    if (config["rtsp_urls"] && config["rtsp_urls"].IsSequence()) {
+      for (const auto& url : config["rtsp_urls"]) {
+        rtsp_urls_.push_back(url.as<std::string>());
+      }
+    }
+    
+    // 读取输出话题配置
+    if (config["output_topics"] && config["output_topics"].IsSequence()) {
+      for (const auto& topic : config["output_topics"]) {
+        output_topics_.push_back(topic.as<std::string>());
+      }
+    }
+    
+    // 验证配置一致性
+    if (rtsp_urls_.size() != output_topics_.size()) {
+      IPCAMERA_SERVICE_LOG_ERROR << "RTSP URLs count (" << rtsp_urls_.size() 
+                 << ") does not match output topics count (" << output_topics_.size() << ")";
+      return false;
+    }
+
+    IPCAMERA_SERVICE_LOG_INFO << "Frame ID: " << frame_id_;
+    IPCAMERA_SERVICE_LOG_INFO << "RTSP URLs count: " << rtsp_urls_.size();
+    for (size_t i = 0; i < rtsp_urls_.size(); ++i) {
+      IPCAMERA_SERVICE_LOG_INFO << "RTSP URL " << i << ": " << rtsp_urls_[i];
+      IPCAMERA_SERVICE_LOG_INFO << "Output Topic " << i << ": " << output_topics_[i];
+    }
+    IPCAMERA_SERVICE_LOG_INFO << "Reconnect interval: " << reconnect_interval_ << "s";
+    IPCAMERA_SERVICE_LOG_INFO << "Buffer size: " << buffer_size_;
+    IPCAMERA_SERVICE_LOG_INFO << "Connection timeout: " << connection_timeout_ << "s";
+    IPCAMERA_SERVICE_LOG_INFO << "SEI timestamp parsing: " << (enable_sei_timestamp_ ? "enabled" : "disabled");
+    
+    // 初始化 FFmpeg
+    if (!InitFFmpeg()) {
+      IPCAMERA_SERVICE_LOG_ERROR << "Failed to initialize FFmpeg";
+      return false;
+    }
+    
+    // 为每个URL创建流信息
+    for (size_t i = 0; i < rtsp_urls_.size(); ++i) {
+      auto stream_info = std::make_shared<StreamInfo>();
+      stream_info->rtsp_url = rtsp_urls_[i];
+      stream_info->output_topic = output_topics_[i];
+      
+      std::lock_guard<std::mutex> lock(streams_mutex_);
+      streams_[output_topics_[i]] = stream_info;  // 使用输出话题作为流ID
+      
+      IPCAMERA_SERVICE_LOG_INFO << "Created stream for topic: " << output_topics_[i] 
+                << " with URL: " << rtsp_urls_[i];
+    }
+    
+    IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera initialized successfully";
+    return true;
+    
+  } catch (const std::exception& e) {
+    IPCAMERA_SERVICE_LOG_ERROR << "Failed to parse config file: " << e.what();
+    return false;
+  }
+}
+
+void StandardIpCamera::Start() {
+  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera::Start";
+  
+  if (stop_.load()) {
+    IPCAMERA_SERVICE_LOG_WARN << "Camera already stopped, cannot start";
+    return;
+  }
+  
+  // 启动所有流
+  std::lock_guard<std::mutex> lock(streams_mutex_);
+  for (auto& pair : streams_) {
+    StartStream(pair.first);
+  }
+  
+  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera started with " << streams_.size() << " streams";
+}
+
+void StandardIpCamera::Stop() {
+  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera::Stop";
+  
+  stop_.store(true);
+  
+  // 停止所有流
+  std::lock_guard<std::mutex> lock(streams_mutex_);
+  for (auto& pair : streams_) {
+    StopStream(pair.first);
+  }
+  
+  // 清理 FFmpeg 资源
+  CleanupFFmpeg();
+  
+  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera stopped";
+}
+
+void StandardIpCamera::StartStream(const std::string& stream_id) {
+  auto it = streams_.find(stream_id);
+  if (it == streams_.end()) {
+    IPCAMERA_SERVICE_LOG_ERROR << "Stream not found: " << stream_id;
+    return;
+  }
+  
+  auto& stream_info = it->second;
+  if (stream_info->pull_thread) {
+    IPCAMERA_SERVICE_LOG_WARN << "Stream " << stream_id << " already running";
+    return;
+  }
+  
+  stream_info->stop.store(false);
+  stream_info->pull_thread.reset(new std::thread([this, stream_id]() {
+    this->TaskPullRtspStream(stream_id);
+  }));
+  
+  IPCAMERA_SERVICE_LOG_INFO << "Started stream: " << stream_id;
+}
+
+void StandardIpCamera::StopStream(const std::string& stream_id) {
+  auto it = streams_.find(stream_id);
+  if (it == streams_.end()) {
+    return;
+  }
+  
+  auto& stream_info = it->second;
+  stream_info->stop.store(true);
+  stream_info->connected.store(false);
+  
+  if (stream_info->pull_thread && stream_info->pull_thread->joinable()) {
+    stream_info->pull_thread->join();
+  }
+  
+  if (stream_info->format_ctx) {
+    avformat_close_input(&stream_info->format_ctx);
+    stream_info->format_ctx = nullptr;
+  }
+  
+  IPCAMERA_SERVICE_LOG_INFO << "Stopped stream: " << stream_id;
+}
+
+bool StandardIpCamera::InitFFmpeg() {
+  IPCAMERA_SERVICE_LOG_INFO << "Initializing FFmpeg";
+  
+  // 注册所有格式和编解码器
+  av_register_all();
+  avformat_network_init();
+  
+  IPCAMERA_SERVICE_LOG_INFO << "FFmpeg initialized successfully";
+  return true;
+}
+
+void StandardIpCamera::CleanupFFmpeg() {
+  IPCAMERA_SERVICE_LOG_INFO << "Cleaning up FFmpeg resources";
+  
+  std::lock_guard<std::mutex> lock(streams_mutex_);
+  for (auto& pair : streams_) {
+    auto& stream_info = pair.second;
+    if (stream_info->format_ctx) {
+      avformat_close_input(&stream_info->format_ctx);
+      stream_info->format_ctx = nullptr;
+    }
+    stream_info->connected.store(false);
+  }
+  
+  IPCAMERA_SERVICE_LOG_INFO << "FFmpeg resources cleaned up";
+}
+
+
+
+void StandardIpCamera::TaskPullRtspStream(const std::string& stream_id) {
+  IPCAMERA_SERVICE_LOG_INFO << "RTSP pulling task started for stream: " << stream_id;
+  
+  auto it = streams_.find(stream_id);
+  if (it == streams_.end()) {
+    IPCAMERA_SERVICE_LOG_ERROR << "Stream not found: " << stream_id;
+    return;
+  }
+  
+  auto& stream_info = it->second;
+  
+  while (!stream_info->stop.load() && !stop_.load()) {
+    // 尝试连接 RTSP 流
+    if (!ConnectRtspStream(stream_id)) {
+      IPCAMERA_SERVICE_LOG_ERROR << "Failed to connect to RTSP stream " << stream_id 
+                 << ", retrying in " << reconnect_interval_ << " seconds";
+      std::this_thread::sleep_for(std::chrono::seconds(reconnect_interval_));
+      continue;
+    }
+    
+    IPCAMERA_SERVICE_LOG_INFO << "Successfully connected to RTSP stream: " << stream_id;
+    stream_info->connected.store(true);
+    
+    // 读取数据包
+    AVPacket pkt;
+    av_init_packet(&pkt);
+    
+    while (!stream_info->stop.load() && !stop_.load() && stream_info->connected.load()) {
+      int ret = av_read_frame(stream_info->format_ctx, &pkt);
+      if (ret < 0) {
+        if (ret == AVERROR_EOF) {
+          IPCAMERA_SERVICE_LOG_WARN << "RTSP stream ended: " << stream_id;
+        } else {
+          IPCAMERA_SERVICE_LOG_ERROR << "Error reading frame from RTSP stream " << stream_id << ": " << ret;
+        }
+        stream_info->connected.store(false);
+        break;
+      }
+      
+      // 只处理视频流
+      if (pkt.stream_index == stream_info->video_stream_index) {
+        // 创建压缩图像数据
+        auto compressed_image = std::make_shared<os::v2x::device::ipcamera::CompressedImage>();
+        
+        // 设置头部信息
+        auto header = compressed_image->mutable_header();
+        header->set_timestamp_sec(airos::base::TimeUtil::GetCurrentTime());
+        header->set_module_name("ipcamera_service");
+        header->set_sequence_num(++stream_info->sequence_num);
+        header->set_camera_timestamp(GetCurrentTimestamp());
+        
+        // 设置图像信息
+        compressed_image->set_frame_id(frame_id_);
+        compressed_image->set_format("h264");  // H264 格式
+        compressed_image->set_measurement_time(airos::base::TimeUtil::GetCurrentTime());
+        
+        // 检测是否为关键帧
+        bool is_key_frame = IsKeyFrame(pkt.data, pkt.size);
+        IPCAMERA_SERVICE_LOG_INFO << "Size " << pkt.size;
+        compressed_image->set_frame_type(is_key_frame ? 1 : 0);  // 1表示关键帧，0表示普通帧
+        IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " frame type: " << (is_key_frame ? "key frame" : "normal frame");
+        
+        // 从SEI数据中解析曝光时间戳
+        if (enable_sei_timestamp_) {
+          int64_t exposure_timestamp = ExtractExposureTimestamp(pkt.data, pkt.size);
+          if (exposure_timestamp > 0) {
+            compressed_image->set_exposure_time(exposure_timestamp);
+            IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " using extracted exposure timestamp: " << exposure_timestamp;
+          } else {
+            // 使用系统当前时间作为曝光时间戳
+            int64_t current_time = GetCurrentTimestamp();
+            compressed_image->set_exposure_time(current_time);
+            IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " using system current time: " << current_time;
+          }
+        } else {
+          // 使用系统当前时间作为曝光时间戳
+          int64_t current_time = GetCurrentTimestamp();
+          compressed_image->set_exposure_time(current_time);
+        }
+        
+        // 设置 H264 NALU 数据
+        std::string h264_data(reinterpret_cast<const char*>(pkt.data), pkt.size);
+        compressed_image->set_data(h264_data);
+        
+        // 打印前16位数据
+        std::stringstream ss;
+        ss << "Stream " << stream_id << " Frame " << stream_info->sequence_num << " first 16 bytes: ";
+        for (int i = 0; i < std::min(16, pkt.size); ++i) {
+          ss << std::hex << std::setw(2) << std::setfill('0') 
+             << static_cast<int>(static_cast<unsigned char>(pkt.data[i])) << " ";
+        }
+        IPCAMERA_SERVICE_LOG_INFO << ss.str();
+        
+        // 发送数据，使用多路流回调
+        if (sender_) {
+          // 使用输出话题作为stream_id
+          sender_(stream_info->output_topic, compressed_image);
+        }
+        
+        // 更新统计信息
+        if (stream_info->sequence_num % 100 == 0) {
+          IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " received " << stream_info->sequence_num 
+                    << " frames, data size: " << pkt.size << " bytes";
+        }
+      }
+      
+      av_packet_unref(&pkt);
+    }
+    
+    // 断开连接，准备重连
+    if (stream_info->format_ctx) {
+      avformat_close_input(&stream_info->format_ctx);
+      stream_info->format_ctx = nullptr;
+    }
+    
+    if (!stream_info->stop.load() && !stop_.load()) {
+      IPCAMERA_SERVICE_LOG_INFO << "Reconnecting to RTSP stream " << stream_id 
+                << " in " << reconnect_interval_ << " seconds";
+      std::this_thread::sleep_for(std::chrono::seconds(reconnect_interval_));
+    }
+  }
+  
+  IPCAMERA_SERVICE_LOG_INFO << "RTSP pulling task finished for stream: " << stream_id;
+}
+
+bool StandardIpCamera::ConnectRtspStream(const std::string& stream_id) {
+  auto it = streams_.find(stream_id);
+  if (it == streams_.end()) {
+    IPCAMERA_SERVICE_LOG_ERROR << "Stream not found: " << stream_id;
+    return false;
+  }
+  
+  auto& stream_info = it->second;
+  IPCAMERA_SERVICE_LOG_INFO << "Connecting to RTSP stream: " << stream_info->rtsp_url;
+  
+  // 打开 RTSP 流
+  if (avformat_open_input(&stream_info->format_ctx, stream_info->rtsp_url.c_str(), nullptr, nullptr) < 0) {
+    IPCAMERA_SERVICE_LOG_ERROR << "Could not open RTSP stream: " << stream_info->rtsp_url;
+    return false;
+  }
+  
+  // 获取流信息
+  if (avformat_find_stream_info(stream_info->format_ctx, nullptr) < 0) {
+    IPCAMERA_SERVICE_LOG_ERROR << "Could not find stream information";
+    avformat_close_input(&stream_info->format_ctx);
+    return false;
+  }
+  
+  // 打印流信息
+  av_dump_format(stream_info->format_ctx, 0, stream_info->rtsp_url.c_str(), 0);
+  
+  // 查找视频流索引
+  stream_info->video_stream_index = -1;
+  for (unsigned i = 0; i < stream_info->format_ctx->nb_streams; ++i) {
+    if (stream_info->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
+      stream_info->video_stream_index = i;
+      break;
+    }
+  }
+  
+  if (stream_info->video_stream_index == -1) {
+    IPCAMERA_SERVICE_LOG_ERROR << "No video stream found";
+    avformat_close_input(&stream_info->format_ctx);
+    return false;
+  }
+  
+  IPCAMERA_SERVICE_LOG_INFO << "Successfully connected to RTSP stream " << stream_id 
+            << ", video stream index: " << stream_info->video_stream_index;
+  return true;
+}
+
+uint64_t StandardIpCamera::GetCurrentTimestamp() {
+  auto now = std::chrono::system_clock::now();
+  auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
+  return timestamp.count();
+}
+
+int64_t StandardIpCamera::ExtractExposureTimestamp(const uint8_t* data, size_t size) {
+  if (!data || size == 0) {
+    return -1;
+  }
+  
+  size_t pos = 0;
+  
+  while (pos < size) {
+    // 查找 NALU 起始码 (0x00 0x00 0x01)
+    if (pos + 3 < size && data[pos] == 0x00 && data[pos + 1] == 0x00 && data[pos + 2] == 0x01) {
+      uint8_t nal_type = data[pos + 3] & 0x1F;
+      size_t start_code_len = 3;
+      
+      if (nal_type == 6) { // SEI NALU
+        size_t sei_payload_start = pos + start_code_len;
+        size_t sei_payload_size = 0;
+        
+        // 找到 SEI 数据的结束位置
+        while (sei_payload_start + sei_payload_size < size && data[sei_payload_start + sei_payload_size] != 0x80) {
+          sei_payload_size++;
+        }
+        
+        if (sei_payload_start + sei_payload_size < size) {
+          IPCAMERA_SERVICE_LOG_INFO << "Found SEI data, size: " << sei_payload_size << " bytes";
+          
+          // 假设时间戳从 SEI 数据的固定偏移开始
+          const size_t timestamp_offset = 20; // 根据实际 SEI 数据调整
+          const size_t timestamp_size = 14;    // 时间戳长度（字节）
+          
+          if (sei_payload_start + timestamp_offset + timestamp_size <= size) {
+            char timestamp[timestamp_size + 1] = {0};
+            memcpy(timestamp, data + sei_payload_start + timestamp_offset, timestamp_size);
+            timestamp[timestamp_size] = '\0';
+            
+            try {
+              int64_t timestamp_ms = std::stoll(timestamp);
+              if (timestamp_ms > 1000000000000) { // 验证时间戳合理性
+                IPCAMERA_SERVICE_LOG_INFO << "Extracted exposure timestamp: " << timestamp_ms;
+                return timestamp_ms;
+              }
+            } catch (const std::exception& e) {
+              IPCAMERA_SERVICE_LOG_WARN << "Failed to parse timestamp: " << e.what();
+            }
+          } else {
+            IPCAMERA_SERVICE_LOG_WARN << "Timestamp field out of SEI data range";
+          }
+        }
+        
+        // 跳过整个 SEI 数据块
+        pos += start_code_len + sei_payload_size + 1;
+        continue;
+      }
+    }
+    
+    pos++;
+  }
+  
+  return -1; // 未找到有效时间戳
+}
+
+bool StandardIpCamera::IsKeyFrame(const uint8_t* data, size_t size) {
+  if (!data || size == 0) {
+    return false;
+  }
+  
+  size_t pos = 0;
+  
+  while (pos < size) {
+    // 查找 NALU 起始码 (0x00 0x00 0x01)
+    if (pos + 3 < size && data[pos] == 0x00 && data[pos + 1] == 0x00 && data[pos + 2] == 0x01) {
+      uint8_t nal_type = data[pos + 3] & 0x1F;
+      
+      // I帧的NALU类型是5 (IDR帧)
+      if (nal_type == 5) {
+        IPCAMERA_SERVICE_LOG_INFO << "Found key frame (IDR frame)";
+        return true;
+      }
+      
+      // 跳过当前NALU
+      pos += 4;
+      while (pos < size) {
+        // 查找下一个NALU起始码
+        if (pos + 3 < size && data[pos] == 0x00 && data[pos + 1] == 0x00 && data[pos + 2] == 0x01) {
+          break;
+        }
+        pos++;
+      }
+      continue;
+    }
+    
+    pos++;
+  }
+  
+  IPCAMERA_SERVICE_LOG_INFO << "No key frame found";
+  return false;
+}
+
+void StandardIpCamera::WriteToDevice(
+    const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) {
+  // 对于标准IP Camera，通常不需要写入数据
+  // 这里可以实现一些控制命令，比如调整参数等
+  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera::WriteToDevice - Control command received";
+  
+  if (receive_data) {
+    IPCAMERA_SERVICE_LOG_INFO << "Received control data with size: " << receive_data->data().size();
+    // 这里可以根据需要实现具体的控制逻辑
+  }
+}
+
+IpCameraDeviceState StandardIpCamera::GetState() {
+  if (stop_.load()) {
+    return IpCameraDeviceState::STOP;
+  }
+  
+  // 检查多路流状态
+  std::lock_guard<std::mutex> lock(streams_mutex_);
+  for (const auto& pair : streams_) {
+    if (pair.second->connected.load()) {
+      return IpCameraDeviceState::RUNNING;
+    }
+  }
+  
+  return IpCameraDeviceState::UNKNOWN;
+}
+
+// 注册设备到工厂
+V2XOS_IPCAMERA_REG_FACTORY(StandardIpCamera, "standard_ipcamera");
+
+}  // namespace device
+}  // namespace v2x
+}  // namespace os
diff --git a/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.h b/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.h
new file mode 100644
index 0000000..e14c9d9
--- /dev/null
+++ b/middleware/device_service/modules/ipcamera/standard_ipcamera/standard_ipcamera.h
@@ -0,0 +1,107 @@
+/*********************************************************************************
+ * @file		standard_ipcamera.h
+ * @brief		标准IP Camera设备头文件
+ * @details		标准IP Camera设备的头文件定义，包含设备类声明和接口定义
+ * @author		ChangXuhui
+ * @date		2025/7/4
+ * @copyright	Copyright (c) 2025 The Airos Authors.
+ * @verbatim
+ *
+ *  Change History:
+ *  Date         Author        Version      ChangeId           Description
+ *  ------------------------------------------------------------------------------
+ *  2025/1/13  ChangXuhui      1.0          ————               标准IP Camera设备头文件
+ *
+ * @endverbatim
+ ********************************************************************************/
+
+#pragma once
+
+#include <atomic>
+#include <memory>
+#include <thread>
+#include <string>
+#include <map>
+#include <mutex>
+#include <vector>
+
+#include "base/device_connect/ipcamera/device_base.h"
+#include "glog/logging.h"
+#include "yaml-cpp/yaml.h"
+
+extern "C" {
+#include <libavcodec/avcodec.h>
+#include <libavformat/avformat.h>
+#include <libavutil/time.h>
+}
+
+namespace os {
+namespace v2x {
+namespace device {
+
+class StandardIpCamera : public IpCameraDevice {
+ public:
+  StandardIpCamera(const IpCameraCallBack& cb)
+      : IpCameraDevice(cb) {}
+  ~StandardIpCamera() {
+    Stop();
+  }
+
+  bool Init(const std::string& conf) override;
+  void Start() override;
+  void WriteToDevice(
+      const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) override;
+  IpCameraDeviceState GetState() override;
+
+ private:
+  void Stop();
+  bool InitFFmpeg();
+  void CleanupFFmpeg();
+  uint64_t GetCurrentTimestamp();
+
+ private:
+  std::atomic<bool> stop_ = ATOMIC_VAR_INIT(false);
+  
+  // 配置参数
+  std::string frame_id_;
+  std::vector<std::string> rtsp_urls_;  // 多路RTSP URL
+  std::vector<std::string> output_topics_;  // 输出话题列表
+  int reconnect_interval_;  // 重连间隔（秒）
+  int buffer_size_;         // 缓冲区大小
+  int connection_timeout_;  // 连接超时时间（秒）
+  bool enable_sei_timestamp_;  // 是否启用SEI时间戳解析
+  
+  // 单个流信息结构体
+  struct StreamInfo {
+    std::string rtsp_url;
+    std::string output_topic;  // 输出话题
+    std::atomic<bool> stop;
+    std::atomic<bool> connected;
+    std::unique_ptr<std::thread> pull_thread;
+    AVFormatContext* format_ctx;
+    int video_stream_index;
+    uint32_t sequence_num;
+    
+    StreamInfo() : stop(false), connected(false), format_ctx(nullptr), 
+                   video_stream_index(-1), sequence_num(0) {}
+  };
+  
+  // 多路流相关
+  std::map<std::string, std::shared_ptr<StreamInfo>> streams_;
+  std::mutex streams_mutex_;
+  
+  void StartStream(const std::string& stream_id);
+  void StopStream(const std::string& stream_id);
+  void TaskPullRtspStream(const std::string& stream_id);
+  bool ConnectRtspStream(const std::string& stream_id);
+  
+  // SEI时间戳解析
+  int64_t ExtractExposureTimestamp(const uint8_t* data, size_t size);
+  
+  // 关键帧检测
+  bool IsKeyFrame(const uint8_t* data, size_t size);
+};
+
+}  // namespace device
+}  // namespace v2x
+}  // namespace os
```
