# feat(设备接入)【FAR202505-00064】智路OS网络相机接入工厂类定义实现

- 作者：changxuhui
- 日期：2025-07-04
- 完整 hash：`14bb558aa3e87b80ac79f8fec89e4fe5bab4a477`
- 改动：+444 / -0 行

```diff
@@@C	changxuhui	14bb558aa3e87b80ac79f8fec89e4fe5bab4a477	2025-07-04	feat(设备接入)【FAR202505-00064】智路OS网络相机接入工厂类定义实现
diff --git a/base/device_connect/ipcamera/README.md b/base/device_connect/ipcamera/README.md
new file mode 100644
index 0000000..271e90f
--- /dev/null
+++ b/base/device_connect/ipcamera/README.md
@@ -0,0 +1,209 @@
+### ipcamera设备
+模块提供了接入AirOS-edge ipcamera设备所需要的标准接口，定义了AirOS-edge结构化的ipcamera设备输出数据类型，提供ipcamera设备的注册工厂。支持多路RTSP流接入。
+
+用户需要将ipcamera设备采集的数据封装成如下的AirOS-edge结构化的ipcamera输出数据类型
+```protobuf
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
+```
+详细的字段定义请参阅：[ipcamera.proto](../proto/ipcamera.proto)
+
+### 接口含义 
+用户需要实现4个接口：`Init`、`Start`、`WriteToDevice`、`GetState`
+
+### 多路流支持
+模块支持多路RTSP流接入，每路流独立管理：
+- 支持配置文件中的`rtsp_urls`列表配置多路流
+- 支持配置文件中的`output_topics`列表指定输出话题
+- 每路流独立线程拉流，自动重连
+- 数据发布到配置文件中指定的输出话题
+- 支持SEI时间戳解析和关键帧检测
+
+#### `Init`接口
+用于读入ipcamera设备的初始化配置文件，实现ipcamera设备的初始化。
+
+#### `Start` 接口
+用于启动ipcamera设备，获取AirOS-edge结构化的标准ipcamera设备输出数据。
+
+#### `WriteToDevice` 接口
+用于将控制信息写入ipcamera设备，接收 `IpCameraReceiveData` 类型的控制数据
+
+#### `GetState` 接口
+用于实现ipcamera设备的状态查询，返回ipcamera设备的运行状态。
+
+### 使用方式
+1. 在ipcamera目录下建立具体ipcamera设备的目录（建议），添加具体设备的`.h`头文件，引用ipcamera接口头文件，并继承ipcamera接口抽象类（以`dummy_ipcamera`为例）
+    
+    引入ipcamera接口头文件
+    ```c++
+    #include "ipcamera/device_base.h"
+    ```
+
+    继承ipcamera接口抽象类
+    ```c++
+    class DummyIpCamera : public IpCameraDevice {
+    public:
+        // 构造函数，由AirOS-edge框架调用，并提供多路流ipcamera数据回调函数
+        // 回调函数签名：void(const std::string& stream_id, const IpCameraDataType& data)
+        DummyIpCamera(const IpCameraCallBack& cb): IpCameraDevice(cb) {}
+        ~DummyIpCamera() = default;
+        
+        // ipcamera设备初始化接口，config_file文件内容由用户定义
+        bool Init(const std::string& config_file) override;
+        
+        // ipcamera设备启动接口
+        void Start() override;
+
+        // 控制信息写入ipcamera设备接口
+        void WriteToDevice(const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) override;
+
+        // 设备状态查询接口
+        IpCameraDeviceState GetState() override;
+    };
+    ```
+2. 添加具体设备的`.cpp`文件，引入ipcamera设备注册工厂，实现相应的接口，用注册宏将具体设备注册给ipcamera工厂
+    
+    引入ipcamera设备注册工厂：
+    ```c++
+    #include "dummy_ipcamera.h"
+    // 引入ipcamera设备注册工厂
+    #include "ipcamera/device_factory.h"
+    ```
+
+    实现ipcamera初始化接口：
+    ```c++
+    bool DummyIpCamera::Init(const std::string& config_file) {
+        /*
+            解析config_file参数，初始化ipcamera各项参数...
+        */
+
+        /*
+            初始化ipcamera设备...
+        */
+
+        return true;
+    }
+    ```
+    
+    实现ipcamera启动接口
+    ```c++
+    void DummyIpCamera::Start() {
+        // 多路流示例：为每个流启动独立线程
+        for (const auto& stream_info : streams_) {
+            std::thread([this, stream_info]() {
+                while (!stop_.load()) {
+                    /*
+                    制备AirOS-edge结构化的ipcamera输出数据...
+                    auto data = std::make_shared<os::v2x::device::ipcamera::CompressedImage>();
+                    ...
+                    */
+
+                    /*
+                    将结构化的ipcamera输出数据传递给AirOS-edge框架提供的回调函数
+                    sender_(stream_info.first, data);  // 多路流回调，包含stream_id
+                    ...
+                    */
+                }
+            }).detach();
+        }
+    }
+    ```
+
+    实现ipcamera控制信息写入接口
+    ```c++
+    void DummyIpCamera::WriteToDevice(const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) {
+       /*
+        写入receive_data内容
+        ...
+        */
+    }
+    ```
+
+    实现ipcamera状态查询接口
+    ```c++
+    IpCameraDeviceState GetState() override {
+        /*
+            返回ipcamera当前运行状态
+        */
+    }
+    ```
+
+    将ipcamera设备注册给ipcamera设备工厂
+    ```c++
+    // "dummy_ipcamera"为注册的具体ipcamera设备名称
+    V2XOS_IPCAMERA_REG_FACTORY(DummyIpCamera, "dummy_ipcamera");
+    ```
+3. AirOS-edge框架将会以如下方式构造和启动`dummy_ipcamera`设备
+
+    基于注册设备时的key，利用设备工厂构建指定的设备
+    ```c++
+    ...
+    // 多路流回调函数
+    auto multi_stream_callback = [this](const std::string& stream_id, const IpCameraDataType& data) {
+        // 处理多路流数据，按stream_id分发到不同话题
+        std::string topic = "/sensor/ipcamera/h264/" + stream_id;
+        Send(topic, data);
+    };
+    
+    device_ = IpCameraDeviceFactory::Instance().GetUnique("dummy_ipcamera", multi_stream_callback);
+    if (device_ == nullptr) {
+        return 0;
+    }
+    ...
+    ```
+
+    初始化设备
+    ```c++
+    ...
+    if (!device_->Init("xxx/config.yaml")) {
+        ...
+        return 0;
+    }
+    ...
+    ```
+
+    启动设备
+    ```c++
+    ...
+    task_.reset(new std::thread([&](){device_->Start();}));
+    ...
+    ```
+    *`device_` 与 `task_` 为AirOS-edge框架内部持有的成员变量*
+
+    *`multi_stream_callback` 为AirOS-edge框架内部定义的多路流ipcamera结构化输出数据处理函数*
+
+### 配置文件示例
+支持多路流配置：
+
+```yaml
+frame_id: "camera_frame"
+# 多路RTSP拉流地址配置
+rtsp_urls:
+  - "rtsp://192.168.1.100:554/stream1"
+  - "rtsp://192.168.1.101:554/stream2"
+  - "rtsp://192.168.1.102:554/stream3"
+# 输出话题配置（与rtsp_urls一一对应）
+output_topics:
+  - "/sensor/ipcamera/h264/192_168_1_100"
+  - "/sensor/ipcamera/h264/192_168_1_101"
+  - "/sensor/ipcamera/h264/192_168_1_102"
+reconnect_interval: 5  # 重连间隔（秒）
+buffer_size: 1024000   # 缓冲区大小
+connection_timeout: 10 # 连接超时时间（秒）
+enable_sei_timestamp: true  # 是否启用SEI时间戳解析
+```
+
+**话题发布规则：**
+- 话题：使用配置文件中指定的 `output_topics` 列表
+- 每个RTSP URL对应一个输出话题
diff --git a/base/device_connect/ipcamera/device_base.h b/base/device_connect/ipcamera/device_base.h
new file mode 100644
index 0000000..f0dfc36
--- /dev/null
+++ b/base/device_connect/ipcamera/device_base.h
@@ -0,0 +1,93 @@
+/*********************************************************************************
+ * @file		device_base.h
+ * @brief		IP Camera设备基类
+ * @details		IP Camera设备接口定义，支持多路RTSP流接入
+ * @author		ChangXuhui
+ * @date		2025/7/4
+ * @copyright	Copyright (c) 2025 The Airos Authors.
+ * @verbatim
+ *
+ *  Change History:
+ *  Date         Author        Version      ChangeId           Description
+ *  ------------------------------------------------------------------------------
+ *  2025/1/13  ChangXuhui      1.0          ————               IP Camera设备基类开发
+ *                                                             
+ *
+ * @endverbatim
+ ********************************************************************************/
+#pragma once
+#include <functional>
+#include <memory>
+
+#include "base/device_connect/proto/ipcamera.pb.h"
+
+namespace os {
+namespace v2x {
+namespace device {
+
+/**
+ * @struct  IpCameraDeviceState
+ * @brief   IP Camera设备运行状态
+ * @details
+ */
+enum class IpCameraDeviceState
+{
+    UNKNOWN,   //未知
+    RUNNING,  //运行
+    STOP
+};
+
+using IpCameraDataType = std::shared_ptr<const os::v2x::device::ipcamera::CompressedImage>;
+// 多路流回调函数类型
+using IpCameraCallBack = std::function<void(const std::string& stream_id, const IpCameraDataType &)>;
+
+/**
+ * @brief  IP Camera设备接口类
+ */
+class IpCameraDevice
+{
+public:
+    IpCameraDevice() = default;
+
+    /**
+     * @brief      IP Camera设备构造函数
+     * @param[in]  cb AirOS-edge框架注册的多路流回调函数
+     */
+    explicit IpCameraDevice(const IpCameraCallBack &cb) : sender_(cb)
+    {
+    }
+
+    virtual ~IpCameraDevice() = default;
+
+    /**
+     * @brief      用于IP Camera设备初始化
+     * @param[in]  config_file IP Camera设备初始化参数配置文件
+     * @retval     初始化是否成功
+     */
+    virtual bool Init(const std::string &config_file) = 0;
+
+    /**
+     * @brief 用于启动IP Camera设备，产出AirOS-edge结构化的标准图像采集输出数据
+     */
+    virtual void Start() = 0;
+
+    /**
+     * @brief      用于获取IP Camera设备状态
+     * @retval     设备状态码图像采集DeviceState
+     */
+    virtual IpCameraDeviceState GetState() = 0;
+
+    /**
+     * @brief      用于将数据写入IP Camera设备状态
+     * @param[in]  receive_data AirOS-edge结构化的IP Camera接收数据
+     */
+    virtual void WriteToDevice(
+        const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) = 0;
+
+   protected:
+    IpCameraCallBack sender_;
+};
+
+}  // namespace device
+}  // namespace v2x
+}  // namespace os
diff --git a/base/device_connect/ipcamera/device_factory.cc b/base/device_connect/ipcamera/device_factory.cc
new file mode 100644
index 0000000..b798680
--- /dev/null
+++ b/base/device_connect/ipcamera/device_factory.cc
@@ -0,0 +1,50 @@
+/*********************************************************************************
+ * @file		device_factory.cc
+ * @brief		IP Camera设备工厂类实现
+ * @details		IP Camera设备工厂类的具体实现，提供单例模式和设备实例化功能
+ * @author		ChangXuhui
+ * @date		2025/7/4
+ * @copyright	Copyright (c) 2025 The Airos Authors.
+ * @verbatim
+ *
+ *  Change History:
+ *  Date         Author        Version      ChangeId           Description
+ *  ------------------------------------------------------------------------------
+ *  2025/1/13  ChangXuhui      1.0          ————               IP Camera设备工厂类实现
+ *
+ * @endverbatim
+ ********************************************************************************/
+#include "base/device_connect/ipcamera/device_factory.h"
+
+namespace os {
+namespace v2x {
+namespace device {
+
+IpCameraDeviceFactory &IpCameraDeviceFactory::Instance()
+{
+    static IpCameraDeviceFactory instance_;
+    return instance_;
+}
+
+std::unique_ptr<IpCameraDevice> IpCameraDeviceFactory::GetUnique(const std::string &key, const IpCameraCallBack &cb)
+{
+    return std::unique_ptr<IpCameraDevice>(Produce(key, cb));
+}
+
+std::shared_ptr<IpCameraDevice> IpCameraDeviceFactory::GetShared(const std::string &key, const IpCameraCallBack &cb)
+{
+    return std::shared_ptr<IpCameraDevice>(Produce(key, cb));
+}
+
+IpCameraDevice *IpCameraDeviceFactory::Produce(const std::string &key, const IpCameraCallBack &cb)
+{
+    if (map_.find(key) == map_.end())
+    {
+        return nullptr;
+    }
+    return map_[key](cb);
+}
+
+}  // namespace device
+}  // namespace v2x
+}  // namespace os 
diff --git a/base/device_connect/ipcamera/device_factory.h b/base/device_connect/ipcamera/device_factory.h
new file mode 100644
index 0000000..148d702
--- /dev/null
+++ b/base/device_connect/ipcamera/device_factory.h
@@ -0,0 +1,91 @@
+/*********************************************************************************
+ * @file		device_factory.h
+ * @brief		IP Camera设备工厂类
+ * @details		IP Camera设备注册工厂，支持动态设备注册和实例化
+ * @author		ChangXuhui
+ * @date		2025/7/4
+ * @copyright	Copyright (c) 2025 The Airos Authors.
+ * @verbatim
+ *
+ *  Change History:
+ *  Date         Author        Version      ChangeId           Description
+ *  ------------------------------------------------------------------------------
+ *  2025/1/13  ChangXuhui      1.0          ————               IP Camera设备工厂类开发
+ *
+ * @endverbatim
+ ********************************************************************************/
+#pragma once
+
+#include <functional>
+#include <map>
+#include "device_base.h"
+
+namespace os {
+namespace v2x {
+namespace device {
+
+/**
+ * @brief  IP Camera设备注册工厂
+ */
+class IpCameraDeviceFactory {
+ public:
+  using CONSTRUCT = std::function<IpCameraDevice *(const IpCameraCallBack &cb)>;
+
+  template <typename Inherit>
+  class Register_t {
+   public:
+    Register_t(const std::string &key) {
+      IpCameraDeviceFactory::Instance().map_.emplace(
+          key, [](const IpCameraCallBack &cb) { return new Inherit(cb); });
+    }
+  };
+
+  /**
+   * @brief      用于获取指定IP Camera设备实例的unique指针
+   * @param[in]  key 指定IP Camera设备名
+   * @retval     指定IP Camera设备实例的unique指针
+   */
+  std::unique_ptr<IpCameraDevice> GetUnique(const std::string &key,
+                                           const IpCameraCallBack &cb);
+
+  /**
+   * @brief      用于获取指定IP Camera设备实例的shared指针
+   * @param[in]  key 指定IP Camera设备名
+   * @retval     指定IP Camera设备实例的shared指针
+   */
+  std::shared_ptr<IpCameraDevice> GetShared(const std::string &key,
+                                           const IpCameraCallBack &cb);
+
+
+  /**
+   * @brief      用于获取IP Camera设备工厂实例（单例）
+   * @retval     IP Camera设备工厂实例
+   */
+  static IpCameraDeviceFactory &Instance();
+
+ private:
+  IpCameraDevice *Produce(const std::string &key, const IpCameraCallBack &cb);
+
+  IpCameraDeviceFactory(){};
+
+  IpCameraDeviceFactory(const IpCameraDeviceFactory &) = delete;
+
+  IpCameraDeviceFactory(IpCameraDeviceFactory &&) = delete;
+
+ private:
+  std::map<std::string, CONSTRUCT> map_;
+};
+
+#define V2XOS_IPCAMERA_REG(T) v2xos_reg_func_str_##T##_
+/**
+ * @brief      用于注册指定IP Camera设备
+ * @param[in]  T 具体IP Camera设备子类型
+ * @param[in]  key 注册的IP Camera设备名字
+ */
+#define V2XOS_IPCAMERA_REG_FACTORY(T, key)                       \
+  static IpCameraDeviceFactory::Register_t<T> V2XOS_IPCAMERA_REG(T)(  \
+      key);
+
+}  // namespace device
+}  // namespace v2x
+}  // namespace os 
diff --git a/base/device_connect/proto/BUILD b/base/device_connect/proto/BUILD
index cb12ef7..12221af 100644
--- a/base/device_connect/proto/BUILD
+++ b/base/device_connect/proto/BUILD
@@ -17,5 +17,6 @@ proto_library(
         "radar_data.proto",
         "rsu_data.proto",
         "traffic_light_data.proto",
+        "ipcamera.proto",
     ],
 )
```
