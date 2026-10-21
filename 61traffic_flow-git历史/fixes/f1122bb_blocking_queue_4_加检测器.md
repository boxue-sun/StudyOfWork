# f1122bb  add traffic flow detector & perfect blocking queue

- 作者：fengwenxi
- 日期：2024-10-21
- 完整 hash：`f1122bb3203b8cc83eff413e8e1d62dcef13dff5`
- 改动：+298 / -82 行，涉及 13 个文件

涉及文件：
- `src/base/blocking_queue.hpp`
- `src/base/copyable.hpp`
- `src/base/noncopyable.hpp`
- `src/base/singleton.hpp`
- `src/common/config.h`
- `src/plugin/traffic_flow_service_impl.cpp`
- `src/plugin/traffic_flow_service_impl.h`
- `src/traffic_flow/detector_types.h`
- `src/traffic_flow/track_info.h`
- `src/traffic_flow/traffic_flow_detector.cpp`
- `src/traffic_flow/traffic_flow_detector.h`
- `src/traffic_flow/traffic_flow_factory.cpp`
- `src/traffic_flow/traffic_flow_factory.h`

```diff
diff --git a/src/base/blocking_queue.hpp b/src/base/blocking_queue.hpp
index de7152f..eb1a949 100644
--- a/src/base/blocking_queue.hpp
+++ b/src/base/blocking_queue.hpp
@@ -3,9 +3,10 @@
  * @author FengWenxi (ThorfromAsgard@outlook.com)
  * @brief An implementation of blocking queue using std::condition_variable #NOTE This only works on C++11 standard.
  * @version 0.1
- * @date 2024-10-08
+ * @date 2024-06-08
  *
  * <========================================================================>
+ *          github page: https://github.com/ThorfromAsgard/cybertron
  *                   © 2024 FengWenxi. All Rights Reserved.
  * <========================================================================>
  *
@@ -39,11 +40,12 @@ public:
      * set it to 0, if you are not sure, then you are not aware !!!!!!
      *
      */
-    explicit BlockingQueue(bool push_block = false, size_t capacity_limit = 0)
+    explicit BlockingQueue(size_t capacity_limit = 0, bool push_block = false)
         : push_block_(push_block),
           capacity_limit_(capacity_limit),
           mutex_(),
-          consumer_(mutex_),
+          producer_(),
+          consumer_(),
           deque_(),
           active_(true) {
         std::cout << "Blocking Queue capacity: " << capacity_limit_ << std::endl;
@@ -60,6 +62,10 @@ public:
      * #TODO: Do tests to make sure this works fine!
      */
     ~BlockingQueue() {
+        Close();
+    }
+
+    void Close() {
         {
             std::lock_guard<std::mutex> lock(mutex_);
             deque_.clear();
@@ -82,17 +88,18 @@ public:
     bool PushBack(const T& element, const int64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
-            std::cv_status is_timeout = std::cv_status::no_timeout;
+            bool is_woken_up = false;
             if (timeout) {
-                is_timeout = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
+                is_woken_up = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
                     return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
             } else {
                 producer_.wait(lock, [&] {
                     return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
+                is_woken_up = true;
             }
-            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+            if ((!active_) || (!is_woken_up)) {
                 return false;
             }
         } else {
@@ -115,20 +122,21 @@ public:
      * to the back of the queue,
      * @return false if it fails.
      */
-    bool PushBack(T&& element, const uint64_t& timeout = 0) {
+    bool PushBack(T&& element, const int64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
-            std::cv_status is_timeout = std::cv_status::no_timeout;
+            bool is_woken_up = false;
             if (timeout) {
-                is_timeout = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
+                is_woken_up = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
                     return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
             } else {
                 producer_.wait(lock, [&] {
                     return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
+                is_woken_up = true;
             }
-            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+            if ((!active_) || (!is_woken_up)) {
                 return false;
             }
         } else {
@@ -136,7 +144,7 @@ public:
                 deque_.pop_front();
             }
         }
-        deque_.push_back(std::move(item));
+        deque_.push_back(std::move(element));
         consumer_.notify_one();
         return true;
     }
@@ -151,20 +159,21 @@ public:
      * to the front of the queue,
      * @return false if it fails.
      */
-    bool PushFront(const T& element, const uint64_t& timeout = 0) {
+    bool PushFront(const T& element, const int64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
-            std::cv_status is_timeout = std::cv_status::no_timeout;
+            bool is_woken_up = false;
             if (timeout) {
-                is_timeout = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
+                is_woken_up = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
                     return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
             } else {
                 producer_.wait(lock, [&] {
                     return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
+                is_woken_up = true;
             }
-            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+            if ((!active_) || (!is_woken_up)) {
                 return false;
             }
         } else {
@@ -186,20 +195,21 @@ public:
      * to the front of the queue,
      * @return false if it fails.
      */
-    bool PushFront(T&& item, const uint64_t& timeout = 0) {
+    bool PushFront(T&& item, const int64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
-            std::cv_status is_timeout = std::cv_status::no_timeout;
+            bool is_woken_up = false;
             if (timeout) {
-                is_timeout = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
+                is_woken_up = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
                     return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
             } else {
                 producer_.wait(lock, [&] {
                     return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
+                is_woken_up = true;
             }
-            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+            if ((!active_) || (!is_woken_up)) {
                 return false;
             }
         } else {
@@ -221,16 +231,17 @@ public:
      * from the front of the queue,
      * @return false if it fails.
      */
-    bool PopFront(T& element, const int& timeout = 0) {
+    bool PopFront(T& element, const int64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
-        std::cv_status is_timeout = std::cv_status::no_timeout;
+        bool is_woken_up = false;
         if (timeout) {
-            is_timeout = consumer_.wait_for(lock, std::chrono::microseconds(timeout),
-                                            [&] { return ((!active_) || (!deque_.empty())); });
+            is_woken_up = consumer_.wait_for(lock, std::chrono::microseconds(timeout),
+                                             [&] { return ((!active_) || (!deque_.empty())); });
         } else {
             consumer_.wait(lock, [&] { return ((!active_) || (!deque_.empty())); });
+            is_woken_up = true;
         }
-        if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+        if ((!active_) || (!is_woken_up)) {
             return false;
         };
         element = deque_.front();
@@ -251,16 +262,17 @@ public:
      * from the back of the queue,
      * @return false if it fails.
      */
-    bool PopBack(T& element, const int& timeout = 0) {
+    bool PopBack(T& element, const int64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
-        std::cv_status is_timeout = std::cv_status::no_timeout;
+        bool is_woken_up = false;
         if (timeout) {
-            is_timeout = consumer_.wait_for(lock, std::chrono::microseconds(timeout),
-                                            [&] { return ((!active_) || (!deque_.empty())); });
+            is_woken_up = consumer_.wait_for(lock, std::chrono::microseconds(timeout),
+                                             [&] { return ((!active_) || (!deque_.empty())); });
         } else {
             consumer_.wait(lock, [&] { return ((!active_) || (!deque_.empty())); });
+            is_woken_up = true;
         }
-        if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+        if ((!active_) || (!is_woken_up)) {
             return false;
         };
         element = deque_.back();
@@ -271,15 +283,12 @@ public:
         return true;
     }
 
-    size_t Size() const {
+    size_t Size() {
         std::lock_guard<std::mutex> lock(mutex_);
         return deque_.size();
     }
 
-    size_t Capacity() {
-        std::lock_guard<std::mutex> lock(mutex_);
-        return capacity_limit_;
-    }
+    size_t Capacity() { return capacity_limit_; }
 
     bool Empty() {
         std::lock_guard<std::mutex> lock(mutex_);
@@ -303,6 +312,7 @@ public:
 
     void Clear() {
         std::lock_guard<std::mutex> lock(mutex_);
+        while (deque_.size()) {}
         deque_.clear();
     }
 
diff --git a/src/base/copyable.hpp b/src/base/copyable.hpp
index f70582f..c4d3a76 100644
--- a/src/base/copyable.hpp
+++ b/src/base/copyable.hpp
@@ -3,9 +3,10 @@
  * @author FengWenxi (ThorfromAsgard@outlook.com)
  * @brief A copyable base class
  * @version 1.0
- * @date 2024-10-08
+ * @date 2024-06-08
  *
  * <========================================================================>
+ *          github page: https://github.com/ThorfromAsgard/cybertron         
  *                   © 2024 FengWenxi. All Rights Reserved.
  * <========================================================================>
  *
diff --git a/src/base/noncopyable.hpp b/src/base/noncopyable.hpp
index de50feb..a130d3d 100644
--- a/src/base/noncopyable.hpp
+++ b/src/base/noncopyable.hpp
@@ -3,9 +3,10 @@
  * @author FengWenxi (ThorfromAsgard@outlook.com)
  * @brief A noncopyable base class
  * @version 1.0
- * @date 2024-10-08
+ * @date 2024-06-08
  *
  * <========================================================================>
+ *          github page: https://github.com/ThorfromAsgard/cybertron         
  *                   © 2024 FengWenxi. All Rights Reserved.
  * <========================================================================>
  *
diff --git a/src/base/singleton.hpp b/src/base/singleton.hpp
index 1a02fc9..bc91d02 100644
--- a/src/base/singleton.hpp
+++ b/src/base/singleton.hpp
@@ -3,9 +3,10 @@
  * @author FengWenxi (ThorfromAsgard@outlook.com)
  * @brief A simple implementation of singleton class using template and perfect forwarding.
  * @version 1.0
- * @date 2024-10-08
+ * @date 2024-06-08
  *
  * <========================================================================>
+ *          github page: https://github.com/ThorfromAsgard/cybertron         
  *                   © 2024 FengWenxi. All Rights Reserved.
  * <========================================================================>
  *
diff --git a/src/common/config.h b/src/common/config.h
index 054a9c1..81d2a91 100644
--- a/src/common/config.h
+++ b/src/common/config.h
@@ -14,16 +14,20 @@
 #define TRAFFIC_FLOW_CONFIG_H
 
 #include "base/singleton.hpp"
-#include "traffic_flow/detector_types.h"
+#include "traffic_flow/detector_type.h"
+#include "traffic_flow/track_info.h"
 
 #include <map>
 #include <string>
+#include <vector>
 
 namespace fusion_perception {
-typedef struct DetectorConfig_ {
+
+struct DetectorConfig {
     std::string name;
     TrafficFlowDetecotorType type = TrafficFlowDetecotorType::kDefault;
-} DetectorConfig;
+    std::vector<GeoPosition> section_line;
+};
 
 class GlobalConfig final : public cybertron::base::Singleton<GlobalConfig> {
     friend class cybertron::base::Singleton<GlobalConfig>;
diff --git a/src/plugin/traffic_flow_service_impl.cpp b/src/plugin/traffic_flow_service_impl.cpp
index a2636c9..0f54932 100644
--- a/src/plugin/traffic_flow_service_impl.cpp
+++ b/src/plugin/traffic_flow_service_impl.cpp
@@ -22,6 +22,7 @@
 #include <iostream>
 #include <sstream>
 #include <string>
+#include <functional>
 
 namespace fusion_perception::plugin {
 
@@ -53,7 +54,8 @@ int TrafficFlowServiceImpl::Init(const char* config_path) {
         return CC_FAIL;
     }
     TrafficFlowFactory& traffic_flow_factory = TrafficFlowFactory::GetInstance();
-
+    traffic_flow_factory.RegisterPublishCallback(
+        std::bind(&TrafficFlowServiceImpl::Publish, this, std::placeholders::_1));
     return CC_OK;
 }
 
@@ -169,7 +171,7 @@ void TrafficFlowServiceImpl::OnPublish(uint8_t id, void* message) {
     return;
 }
 
-void TrafficFlowServiceImpl::Publish(const std::string& result) {
+bool TrafficFlowServiceImpl::Publish(const std::string& result) {
     /* Initialize output message */
     FusionMsg message;
     message.version = 1;
@@ -180,16 +182,14 @@ void TrafficFlowServiceImpl::Publish(const std::string& result) {
 
     std::vector<uint8_t> message_buffer;
     message_buffer.clear();
-    if (!result.empty()) {
-        message_buffer.assign(result.begin(), result.end());
-        message_buffer.emplace_back('\0');
-        return;
-    } else {
+    if (result.empty()) {
         if (s_fs_api_->TrafficFlowService_Log) {
             s_fs_api_->TrafficFlowService_Log(LogLevel_Warn, "Warning! Result string empty!");
         }
-        return;
+        return false;
     }
+    message_buffer.assign(result.begin(), result.end());
+    message_buffer.emplace_back('\0');
     message.body_size = message_buffer.size();
     message.body = message_buffer.data();
 
@@ -199,7 +199,9 @@ void TrafficFlowServiceImpl::Publish(const std::string& result) {
         if (s_fs_api_->TrafficFlowService_Log) {
             s_fs_api_->TrafficFlowService_Log(LogLevel_Error, "[Publish] error code: %d", code);
         }
+        return false;
     }
+    return true;
 }
 
 uint64_t TrafficFlowServiceImpl::GetTimestamp() {
diff --git a/src/plugin/traffic_flow_service_impl.h b/src/plugin/traffic_flow_service_impl.h
index 1166a09..9569417 100644
--- a/src/plugin/traffic_flow_service_impl.h
+++ b/src/plugin/traffic_flow_service_impl.h
@@ -90,11 +90,13 @@ public:
 
 protected:
     /**
-     * @brief Send message spontaneously
+     * @brief Send message spontaneously.
      *
-     * @param message that we want to publish
+     * @param message that we want to publish.
+     * @return true if it successes,
+     * @return false if it fails.
      */
-    void Publish(const std::string& message);
+    bool Publish(const std::string& message);
     uint64_t GetTimestamp();
 
 private:
diff --git a/src/traffic_flow/detector_types.h b/src/traffic_flow/detector_type.h
similarity index 57%
rename from src/traffic_flow/detector_types.h
rename to src/traffic_flow/detector_type.h
index 1353ad6..e54f58f 100644
--- a/src/traffic_flow/detector_types.h
+++ b/src/traffic_flow/detector_type.h
@@ -1,5 +1,5 @@
 /**
- * @file detector_types.h
+ * @file detector_type.h
  * @author FengWenxi (ThorfromAsgard@outlook.com)
  * @brief Defination enum class TrafficFlowDetectorType.
  * @version 0.1
@@ -17,7 +17,20 @@
 
 namespace fusion_perception {
 
-enum class TrafficFlowDetecotorType : uint8_t { kDefault = 255 };
+enum class TrafficFlowDetecotorType : uint8_t {
+    kDefault = 0,
+    kMotorVehicleVolume = 1,
+    kNonMotorVehicleVolume = 2,
+    kPedestrianVolume = 3,
+    kPassengerCarUnit = 4,
+    kAverageSpeed = 5,
+    kQueueLength = 6,
+    kQueueSize = 7,
+    kTimeHeadway = 8,
+    kTimeGap = 9,
+    kTimeOccupancy = 10,
+    kSpaceOccupancy = 11
+};
 }
 
 #endif
\ No newline at end of file
diff --git a/src/traffic_flow/track_info.h b/src/traffic_flow/track_info.h
new file mode 100644
index 0000000..c635f83
--- /dev/null
+++ b/src/traffic_flow/track_info.h
@@ -0,0 +1,33 @@
+/**
+ * @file track_info.h
+ * @author FengWenxi (ThorfromAsgard@outlook.com)
+ * @brief TrackInfo struct
+ * @version 0.1
+ * @date 2024-10-20
+ *
+ * <========================================================================>
+ *                   © 2024 FengWenxi. All Rights Reserved.
+ * <========================================================================>
+ *
+ */
+#ifndef TRACK_INFO_H
+#define TRACK_INFO_H
+
+#include <vector>
+
+namespace fusion_perception {
+
+struct GeoPosition {
+    double longitude = 0.0;
+    double latitude = 0.0;
+};
+
+struct TrackInfo {
+    uint64_t id = 0;
+    std::vector<GeoPosition> track;
+    uint64_t last_update_time = 0; 
+};
+
+}  // namespace fusion_perception
+
+#endif
\ No newline at end of file
diff --git a/src/traffic_flow/traffic_flow_detector.cpp b/src/traffic_flow/traffic_flow_detector.cpp
new file mode 100644
index 0000000..1384575
--- /dev/null
+++ b/src/traffic_flow/traffic_flow_detector.cpp
@@ -0,0 +1,25 @@
+/**
+ * @file traffic_flow_detector.cpp
+ * @author FengWenxi (ThorfromAsgard@outlook.com)
+ * @brief
+ * @version 0.1
+ * @date 2024-10-11
+ *
+ * <========================================================================>
+ *                   © 2024 FengWenxi. All Rights Reserved.
+ * <========================================================================>
+ *
+ */
+#include "traffic_flow_detector.h"
+
+namespace fusion_perception {
+
+void TrafficFlowDetector::RegisterDeliverCallback(const ResultDeliverCallback& callback) {
+    deliver_callback_ = callback;
+}
+
+VolumeDetector::VolumeDetector(const DetectorConfig& config){
+    // type = config.type
+}
+
+}  // namespace fusion_perception
\ No newline at end of file
diff --git a/src/traffic_flow/traffic_flow_detector.h b/src/traffic_flow/traffic_flow_detector.h
index 300b97b..7356d7a 100644
--- a/src/traffic_flow/traffic_flow_detector.h
+++ b/src/traffic_flow/traffic_flow_detector.h
@@ -1,7 +1,7 @@
 /**
  * @file traffic_flow_detector.h
  * @author FengWenxi (ThorfromAsgard@outlook.com)
- * @brief TrafficFlowDetector classes.
+ * @brief TrafficFlowDetector class.
  * @version 0.1
  * @date 2024-10-11
  *
@@ -13,23 +13,42 @@
 #ifndef TRAFFIC_FLOW_DETECTOR_H
 #define TRAFFIC_FLOW_DETECTOR_H
 
+#include <nlohmann/json.hpp>
+
 #include "base/noncopyable.hpp"
 #include "common/config.h"
+#include "common/perception_data.h"
+#include "track_info.h"
 
 namespace fusion_perception {
-struct PerceptionResult;
 class TrafficFlowDetector : public cybertron::base::Noncopyable {
-    using PublishCallback = std::function<void(const std::string&)>;
+    using ResultDeliverCallback = std::function<bool(const std::string&)>;
 
 public:
-    TrafficFlowDetector(const DetectorConfig& config);
-    virtual ~TrafficFlowDetector();
+    TrafficFlowDetector() = default;
+    TrafficFlowDetector(const DetectorConfig& config) {};
+    virtual ~TrafficFlowDetector() = default;
 
-    void RegisterPublishCallback(const PublishCallback& callback);
-    virtual void UpdateNewFrame(const PerceptionResult& new_frame_result);
+    void RegisterDeliverCallback(const ResultDeliverCallback& callback);
+    virtual void Detect(const PerceptionResult& new_frame_result) = 0;
 
 protected:
-    PublishCallback publish_callback_;
+    ResultDeliverCallback deliver_callback_;
+};
+
+class VolumeDetector final : public TrafficFlowDetector {
+
+public:
+    VolumeDetector(const DetectorConfig& config);
+    virtual ~VolumeDetector() = default;
+
+    virtual void Detect(const PerceptionResult& new_frame_result) override;
+
+private:
+    const TrafficFlowDetecotorType detector_type;
+    const InnerType object_type;
+    std::map<uint64_t, TrackInfo> tracks;
+    std::vector<GeoPosition> section_line;
 };
 
 }  // namespace fusion_perception
diff --git a/src/traffic_flow/traffic_flow_factory.cpp b/src/traffic_flow/traffic_flow_factory.cpp
index 937ddad..1baea7b 100644
--- a/src/traffic_flow/traffic_flow_factory.cpp
+++ b/src/traffic_flow/traffic_flow_factory.cpp
@@ -17,45 +17,112 @@
 
 #include <memory>
 #include <iostream>
+#include <functional>
 
 namespace fusion_perception {
 
-TrafficFlowFactory::TrafficFlowFactory() {
+TrafficFlowFactory::TrafficFlowFactory()
+    : status_(FactoryStatus::kNew),
+      detectors_(),
+      working_thread_(new std::thread(&TrafficFlowFactory::Detect, this)),
+      publish_callback_(),
+      input_buffer_(kBufferCapacity, false) {
     auto& global_config = GlobalConfig::GetInstance();
     for (const auto& one_detector_config : global_config.GetDetectorConfig()) {
         // TODO: change to derived class.
-        std::shared_ptr<TrafficFlowDetector> one_detector =
-            std::make_shared<TrafficFlowDetector>(one_detector_config.second);
-        detectors_.emplace(std::make_pair(one_detector_config.second.name, one_detector));
+        // std::shared_ptr<TrafficFlowDetector> one_detector =
+        //     std::make_shared<TrafficFlowDetector>(one_detector_config.second);
+        // detectors_.emplace(std::make_pair(one_detector_config.second.name, one_detector));
     }
 }
 
-TrafficFlowFactory::~TrafficFlowFactory() {}
+TrafficFlowFactory::~TrafficFlowFactory() {
+    status_ = FactoryStatus::kExit;
+    input_buffer_.Close();
+    working_thread_->join();
+}
+
+FactoryStatus TrafficFlowFactory::GetStatus() {
+    return status_;
+}
+
+bool TrafficFlowFactory::StartRunning() {
+    std::cout << "TrafficFlowFactory current status: " << FactoryStatusToString(status_) << std::endl;
+    if (status_ != FactoryStatus::kReady && status_ != FactoryStatus::kWait) {
+        std::cerr << "Not in ready or wait status, can not start running!" << std::endl;
+        return false;
+    }
+    status_ = FactoryStatus::kRunning;
+    std::cout << "TrafficFlowFactory start working." << std::endl;
+    return true;
+}
 
 void TrafficFlowFactory::RegisterPublishCallback(const PublishCallback& callback) {
     std::cout << "Register publish_callback_ for TrafficFlowFactory." << std::endl;
     publish_callback_ = callback;
     for (const auto& one_detector : detectors_) {
-        one_detector.second->RegisterPublishCallback(callback);
+        one_detector.second->RegisterDeliverCallback(
+            std::bind(&TrafficFlowFactory::CollectResult, this, std::placeholders::_1));
     }
+    status_ = FactoryStatus::kReady;
 }
 
-void TrafficFlowFactory::UpdateNewFrame(const std::string& new_frame_data) {
-    if (new_frame_data.empty()) {
-        std::cerr << "The json content from the FusionMsg body was empty." << std::endl;
-        return;
+bool TrafficFlowFactory::CollectResult(const std::string& result) {
+    if(result.empty()){
+        std::cerr << "Empty result." << std::endl;
+        return false;
     }
-    PerceptionResult detected_result;
-    nlohmann::json j;
-    try {
-        detected_result = nlohmann::json::parse(new_frame_data);
-    } catch (std::exception& e) {
-        std::cerr << "Unable to parse JSON from message: " << e.what() << std::endl;
-        return;
+    publish_callback_(result);
+    return true;
+}
+
+bool TrafficFlowFactory::UpdateNewFrame(const std::string& new_frame_data) {
+    if (status_ == FactoryStatus::kRunning) {
+        if (new_frame_data.empty()) {
+            std::cerr << "The json content from the FusionMsg body was empty." << std::endl;
+            return false;
+        }
+        PerceptionResult detected_result;
+        nlohmann::json j;
+        try {
+            detected_result = nlohmann::json::parse(new_frame_data);
+        } catch (std::exception& e) {
+            std::cerr << "Unable to parse JSON from message: " << e.what() << std::endl;
+            return false;
+        }
+        size_t buffer_size = input_buffer_.Size();
+        if (buffer_size >= input_buffer_.Capacity() * kBufferSizeWarningRatio) {
+            std::cerr << "Warning, buffer size reaches to " << buffer_size << std::endl;
+        }
+        bool ret = input_buffer_.PushBack(detected_result);
+        return ret;
+    } else {
+        std::cerr << "Warning, TrafficFlowFactory not running, current status: " << FactoryStatusToString(status_) << std::endl;
+        return false;
+    }
+}
+
+void TrafficFlowFactory::Detect() {
+    while (status_ != FactoryStatus::kExit) {
+        PerceptionResult detected_result;
+        bool ret = input_buffer_.PopFront(detected_result);
+        if (!ret) {
+            std::cerr << "Buffer pop error!" << std::endl;
+            return;
+        }
+        for (const auto& it : detectors_) {
+            it.second->Detect(detected_result);
+        }
     }
-    for (const auto& it : detectors_) {
-        it.second->UpdateNewFrame(detected_result);
+}
+
+bool TrafficFlowFactory::StopRunning() {
+    if (status_ != FactoryStatus::kRunning) {
+        std::cerr << "TrafficFlowFactory not running!" << std::endl;
+        return false;
     }
+    status_ = FactoryStatus::kWait;
+    return true;
 }
 
 }  // namespace fusion_perception
\ No newline at end of file
diff --git a/src/traffic_flow/traffic_flow_factory.h b/src/traffic_flow/traffic_flow_factory.h
index 9b2f7d5..d3e6917 100644
--- a/src/traffic_flow/traffic_flow_factory.h
+++ b/src/traffic_flow/traffic_flow_factory.h
@@ -14,31 +14,69 @@
 #define TRAFFIC_FLOW_FACTORY_H
 
 #include "base/singleton.hpp"
+#include "base/blocking_queue.hpp"
 
 #include <memory>
+#include <atomic>
 #include <string>
+#include <thread>
 #include <unordered_map>
+#include <map>
 #include <functional>
+#include <condition_variable>
 
 namespace fusion_perception {
 
+constexpr int64_t kBufferTimeout = 200000;  // in us.
+constexpr size_t kBufferCapacity = 60;
+constexpr float kBufferSizeWarningRatio = 0.8;
+
 class TrafficFlowDetector;
 struct PerceptionResult;
+
+enum class FactoryStatus : uint8_t { kNew = 0, kReady = 1, kRunning = 2, kWait = 3, kExit = 4 };
+
+static std::map<FactoryStatus, std::string> s_factory_status_to_string_map = {{FactoryStatus::kNew, "New"},
+                                                                              {FactoryStatus::kReady, "Ready"},
+                                                                              {FactoryStatus::kRunning, "Running"},
+                                                                              {FactoryStatus::kWait, "Wait"},
+                                                                              {FactoryStatus::kExit, "Exit"}};
+
+inline std::string FactoryStatusToString(const FactoryStatus& status) {
+    std::string status_str = "";
+    auto find_it = s_factory_status_to_string_map.find(status);
+    if (find_it != s_factory_status_to_string_map.end()) {
+        status_str = find_it->second;
+    }
+    return status_str;
+}
+
 class TrafficFlowFactory final : public cybertron::base::Singleton<TrafficFlowFactory> {
     using PublishCallback = std::function<void(const std::string&)>;
     friend class cybertron::base::Singleton<TrafficFlowFactory>;
 
 public:
     void RegisterPublishCallback(const PublishCallback& callback);
-    void UpdateNewFrame(const std::string& new_frame_data);
+    bool UpdateNewFrame(const std::string& new_frame_data);
 
-protected:
+    FactoryStatus GetStatus();
+    bool StartRunning();
+    bool StopRunning();
+
+private:
     TrafficFlowFactory();
     virtual ~TrafficFlowFactory();
 
+    bool CollectResult(const std::string& result);
+    void Detect();
+
 private:
+    std::atomic<FactoryStatus> status_;
     std::unordered_map<std::string, std::shared_ptr<TrafficFlowDetector>> detectors_;
+    std::unique_ptr<std::thread> working_thread_;
     PublishCallback publish_callback_;
+    cybertron::base::BlockingQueue<PerceptionResult> input_buffer_;
 };
+
 }  // namespace fusion_perception
 #endif
\ No newline at end of file
```
