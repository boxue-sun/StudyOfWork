# fix(信控):【20260910-libo-1】修复ccindex三处崩溃：收到非法JSON时double free、启动时EventLoop空指针、退出时卡死/段错误

- 作者：libo
- 日期：2026-09-10
- 完整 hash：`994ac529031aefd65ef0ef9c9afeee0b474a784a`
- 改动：+192 / -134 行

```diff
@@@C	libo	994ac529031aefd65ef0ef9c9afeee0b474a784a	2026-09-10	fix(信控):【20260910-libo-1】修复ccindex三处崩溃：收到非法JSON时double free、启动时EventLoop空指针、退出时卡死/段错误
diff --git a/middleware/protocol/ccindex/ccindex_component.cc b/middleware/protocol/ccindex/ccindex_component.cc
index f1d1ddc..f19f0d1 100644
--- a/middleware/protocol/ccindex/ccindex_component.cc
+++ b/middleware/protocol/ccindex/ccindex_component.cc
@@ -16,8 +16,52 @@
  * @endverbatim
  ********************************************************************************/
 #include "ccindex_component.h"
+#include <cstdio>
+#include <cstring>
 NAMESPACE_START_RADAR_CCINDEX
 
+// 把字节串（MQTT payload、json异常信息）转成可打印字符串用于日志：可见ASCII原样输出，
+// 其它字节（含反斜杠）转成\xHH，这样非法UTF-8、控制字符、截断等都能看出来，也不会把日志文件变成二进制；
+// 过长时只保留头尾
+static std::string escapeForLog(const void* data, int len)
+{
+    const int kHead = 1024;
+    const int kTail = 512;
+    std::string out;
+    const unsigned char* p = static_cast<const unsigned char*>(data);
+    if (p == nullptr || len <= 0)
+    {
+        return out;
+    }
+    auto append = [&out, p](int begin, int end)
+    {
+        char hex[8];
+        for (int i = begin; i < end; ++i)
+        {
+            if (p[i] >= 0x20 && p[i] < 0x7f && p[i] != '\\')
+            {
+                out += static_cast<char>(p[i]);
+            }
+            else
+            {
+                snprintf(hex, sizeof(hex), "\\x%02x", p[i]);
+                out += hex;
+            }
+        }
+    };
+    if (len <= kHead + kTail)
+    {
+        append(0, len);
+    }
+    else
+    {
+        append(0, kHead);
+        out += "...(" + std::to_string(len - kHead - kTail) + " bytes omitted)...";
+        append(len - kTail, len);
+    }
+    return out;
+}
+
 int CCINDEX_COMPONENT::MAX_MESSAGE_COUNT_TC_CHANNEL = 1000; // 设置您的最大限制
 bool CCINDEX_COMPONENT::Init()
 {
@@ -57,6 +101,9 @@ bool CCINDEX_COMPONENT::Init()
 
     m_TaskUseCase.reset(new std::thread([&](){ initUseCase();}));
 	
+    // EventLoop构造时绑定当前线程，只能在各自线程里创建；这里等上面7个EventLoop都创建好，
+    // 避免doBaiscWork()和随后到达的MQTT回调访问到还没创建的EventLoop（空指针）
+    waitEventLoopsReady(7);
     doBaiscWork();
     return true;
 }
@@ -89,6 +136,84 @@ bool CCINDEX_COMPONENT::initEventLoopNoTrajectoriesTwo()
 
 }
 
+// EventLoop只能在创建它的线程里析构（~EventLoop里会assertInLoopThread），线程退出后已无法满足。
+// 组件只在进程退出时才析构，所以EventLoop和挂在上面的UDP Channel有意不释放，交给进程退出回收
+static void leakAtExit(std::shared_ptr<afl::net::EventLoop>& loop)
+{
+    new std::shared_ptr<afl::net::EventLoop>(std::move(loop));
+}
+
+void CCINDEX_COMPONENT::shutdownWorkers()
+{
+    // 1. 置退出标志（之后MQTT回调直接丢弃新消息），让所有EventLoop退出loop()并join，之后不再有定时器/任务在执行
+    //    注意：正在执行中的回调（如连不上的HTTP请求）要等它返回，EventLoop才会退出
+    m_StopWorkers = true;
+    std::shared_ptr<afl::net::EventLoop>* loops[] = {&m_Eventloop, &m_EventloopTrafficMetrics, &m_EventloopTrafficMetricsPubFirst,
+                                                     &m_EventloopTrafficMetricsPubSecond, &m_EventloopTc, &m_EventloopTcPubFirst,
+                                                     &m_EventloopTcPubSecond, &m_EventloopStatic, &m_EventloopCloud};
+    for (auto* loop : loops)
+    {
+        if (*loop)
+        {
+            (*loop)->quit();
+        }
+    }
+    auto joinTask = [](std::unique_ptr<std::thread>& task)
+    {
+        if (task && task->joinable())
+        {
+            task->join();
+        }
+    };
+    for (auto* task : {&m_Task, &m_TaskTrafficMetrics, &m_TaskTrafficMetricsPubFirst, &m_TaskTrafficMetricsPubSecond,
+                       &m_TaskTc, &m_TaskTcPubFirst, &m_TaskTcPubSecond, &m_TaskStatic, &m_TaskStaticCloud, &m_TaskUseCase})
+    {
+        joinTask(*task);
+    }
+    // 2. 唤醒并join上传队列线程（它们由m_EventloopTrafficMetrics上的定时器创建，此时已不会再创建）
+    {
+        std::lock_guard<std::mutex> lock(cv_m1);
+    }
+    cv1.notify_all();
+    {
+        std::lock_guard<std::mutex> lock(cv_m2);
+    }
+    cv2.notify_all();
+    joinTask(m_TaskQueue1);
+    joinTask(m_TaskQueue2);
+    // 3. 断开MQTT，之后不再有回调访问EventLoop
+    mqttDeinit();
+    mqttDeinitCloud();
+    // 4. EventLoop和UDP Channel不在这里析构，见leakAtExit的说明
+    for (auto* loop : loops)
+    {
+        leakAtExit(*loop);
+    }
+    (void)m_UdpChnl.release();
+}
+
+void CCINDEX_COMPONENT::notifyEventLoopReady()
+{
+    std::lock_guard<std::mutex> lock(m_EventloopReadyMutex);
+    ++m_EventloopReadyCount;
+    m_EventloopReadyCv.notify_all();
+}
+
+void CCINDEX_COMPONENT::waitEventLoopsReady(int count)
+{
+    std::unique_lock<std::mutex> lock(m_EventloopReadyMutex);
+    if (!m_EventloopReadyCv.wait_for(lock, std::chrono::seconds(10),
+                                     [this, count]() { return m_EventloopReadyCount >= count; }))
+    {
+        CCINDEX_INTER_ERROR_PRINT << "[error]wait eventloop ready timeout! [ready]" << m_EventloopReadyCount << " [expect]" << count;
+        return;
+    }
+    if(getConfiger().enableDebugPrint)
+    {
+        CCINDEX_INTER_DEBUG_PRINT << "[notice]all eventloop ready! [count]" << m_EventloopReadyCount;
+    }
+}
+
 bool CCINDEX_COMPONENT::initEventLoop()
 {
     m_Eventloop = std::make_shared<afl::net::EventLoop>();
@@ -104,12 +229,14 @@ bool CCINDEX_COMPONENT::initEventLoop()
                                                         getConfiger().configerPeriod.periodMonitorMecLinkStatusCcindexSt, true);
 
     }
+    notifyEventLoopReady();
     m_Eventloop->loop();
     return true;
 }
 bool CCINDEX_COMPONENT::initEventLoopTrafficMetrics()
 {
     m_EventloopTrafficMetrics = std::make_shared<afl::net::EventLoop>();
+    notifyEventLoopReady();
     m_EventloopTrafficMetrics->loop();
     return true;
 }
@@ -117,12 +244,14 @@ bool CCINDEX_COMPONENT::initEventLoopTrafficMetrics()
 bool CCINDEX_COMPONENT::initEventLoopTrafficMetricsPubFirst()
 {
     m_EventloopTrafficMetricsPubFirst = std::make_shared<afl::net::EventLoop>();
+    notifyEventLoopReady();
     m_EventloopTrafficMetricsPubFirst->loop();
     return true;
 }
 bool CCINDEX_COMPONENT::initEventLoopTrafficMetricsPubSecond()
 {
     m_EventloopTrafficMetricsPubSecond = std::make_shared<afl::net::EventLoop>();
+    notifyEventLoopReady();
     m_EventloopTrafficMetricsPubSecond->loop();
     return true;
 }
@@ -147,6 +276,7 @@ bool CCINDEX_COMPONENT::initEventLoopTc()
         }
 
     }
+    notifyEventLoopReady();
     m_EventloopTc->loop();
     return true;
 }
@@ -233,12 +363,14 @@ bool CCINDEX_COMPONENT::initEventLoopTcPubSecond()
 bool CCINDEX_COMPONENT::initEventLoopStatic()
 {
     m_EventloopStatic = std::make_shared<afl::net::EventLoop>();
+    notifyEventLoopReady();
     m_EventloopStatic->loop();
     return true;
 }
 bool CCINDEX_COMPONENT::initEventLoopCloud()
 {
     m_EventloopCloud = std::make_shared<afl::net::EventLoop>();
+    notifyEventLoopReady();
     m_EventloopCloud->loop();
     return true;
 }
@@ -1007,7 +1139,11 @@ bool CCINDEX_COMPONENT::uploadQueue1()
     while (true)
     {
         std::unique_lock<std::mutex> lock(cv_m1);
-        cv1.wait(lock, [&] { return queue1_ready; }); // 等待通知
+        cv1.wait(lock, [&] { return queue1_ready || m_StopWorkers; }); // 等待通知
+        if (m_StopWorkers)
+        {
+            return true;
+        }
         if(getConfiger().enableDebugPrint)
         {
             RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------>[queue1]revice notify queue1!";
@@ -1158,7 +1294,11 @@ bool CCINDEX_COMPONENT::uploadQueue2()
     while (true)
     {
         std::unique_lock<std::mutex> lock(cv_m2);
-        cv2.wait(lock, [&] { return queue2_ready; });
+        cv2.wait(lock, [&] { return queue2_ready || m_StopWorkers; });
+        if (m_StopWorkers)
+        {
+            return true;
+        }
         if(getConfiger().enableDebugPrint)
         {
             RADAR_TRAFFIC_METRICS_ERROR_PRINT << "------>[queue2]revice notify queue2!";
@@ -2343,10 +2483,19 @@ int CCINDEX_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int
 {
     int ret = 1;
     auto thiz = (CCINDEX_COMPONENT *) context;
+    if (thiz->m_StopWorkers)
+    {
+        // 进程正在退出（EventLoop已停）：新消息直接丢弃，不再处理/转发
+        MQTTAsync_freeMessage(&message);
+        MQTTAsync_free(topicName);
+        return 1;
+    }
     if (message->payloadlen)
     {
         if(topicLen <= 0)
         {
+            MQTTAsync_freeMessage(&message);
+            MQTTAsync_free(topicName);
             return ret;
         }
         std::string topic(topicName, topicLen);
@@ -2374,12 +2523,14 @@ int CCINDEX_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int
                 {
                     trafficMetricsJson = afl::base::json::parse((char *) message->payload, (char *) message->payload + message->payloadlen);
                 }
-                catch (...)
+                catch (const std::exception& e)
                 {
+                    // 坏消息直接丢弃：释放后必须返回1。返回0会让paho用已释放的指针重投，导致double free
+                    RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt-inter " << "parse json failure! [topic]" << topic
+                                                      << " [payloadlen]" << message->payloadlen << " [what]" << escapeForLog(e.what(), strlen(e.what())) << " [payload]" << escapeForLog(message->payload, message->payloadlen);
                     MQTTAsync_freeMessage(&message);
                     MQTTAsync_free(topicName);
-                    RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt-inter " << "parse json failure!";
-                    return false;
+                    return 1;
                 }
                 if(thiz->getConfiger().enableDebugPrint)
                 {
@@ -2442,10 +2593,14 @@ int CCINDEX_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int
                 try {
                     staticJson = afl::base::json::parse((char *) message->payload, (char *) message->payload + message->payloadlen);
                 }
-                catch (...)
+                catch (const std::exception& e)
                 {
-                    RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt-inter " << "parse json failure!";
-                    return false;
+                    // 坏消息直接丢弃：释放后返回1。返回0会让paho不停重投同一条坏消息
+                    RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt-inter " << "parse json failure! [topic]" << topic
+                                                      << " [payloadlen]" << message->payloadlen << " [what]" << escapeForLog(e.what(), strlen(e.what())) << " [payload]" << escapeForLog(message->payload, message->payloadlen);
+                    MQTTAsync_freeMessage(&message);
+                    MQTTAsync_free(topicName);
+                    return 1;
                 }
                 if(thiz->getConfiger().enableDebugPrint)
                 {
@@ -3595,18 +3750,27 @@ int CCINDEX_COMPONENT::mqttSubscribeMsgArrvdCloud(void *context, char *topicName
 {
     int ret = 1;
     auto thiz = (CCINDEX_COMPONENT *) context;
+    if (thiz->m_StopWorkers)
+    {
+        // 进程正在退出（EventLoop已停）：新消息直接丢弃，不再处理/转发
+        MQTTAsync_freeMessage(&message);
+        MQTTAsync_free(topicName);
+        return 1;
+    }
     if (message->payloadlen)
     {
         std::string topic(topicName, topicLen);
         std::string topicProfix = topic.substr(0, topic.find("/"));
         if(thiz->getConfiger().enableDebugPrint)
         {
-            CCINDEX_CLOUD_DEBUG_PRINT << "[notice]cloud >>> ccindex" << " [topic]" << topicName << "[msg]" << (char *) (message->payload);
+            CCINDEX_CLOUD_DEBUG_PRINT << "[notice]cloud >>> ccindex" << " [topic]" << topicName << "[msg]" << std::string((char *) message->payload, message->payloadlen);
         }
         if(topicProfix != "upload")
         {
             CCINDEX_CLOUD_DEBUG_PRINT << "[error]mqtt-cloud topic profix is not upload!";
-            return -1;
+            MQTTAsync_freeMessage(&message);
+            MQTTAsync_free(topicName);
+            return 1;
         }
         if(topicProfix == "upload")
         {
@@ -3661,12 +3825,14 @@ int CCINDEX_COMPONENT::mqttSubscribeMsgArrvdCloud(void *context, char *topicName
                 {
                     trafficMetricsJson = afl::base::json::parse((char *) message->payload, (char *) message->payload + message->payloadlen);
                 }
-                catch (...)
+                catch (const std::exception& e)
                 {
+                    // 坏消息直接丢弃：释放后必须返回1。返回0会让paho用已释放的指针重投，导致double free
+                    CCINDEX_CLOUD_ERROR_PRINT << "[Cloud]radar-cloud <<<<<<< clod[error] parse json failure! [topic]" << topic
+                                              << " [payloadlen]" << message->payloadlen << " [what]" << escapeForLog(e.what(), strlen(e.what())) << " [payload]" << escapeForLog(message->payload, message->payloadlen);
                     MQTTAsync_freeMessage(&message);
                     MQTTAsync_free(topicName);
-                    CCINDEX_CLOUD_ERROR_PRINT << "[Cloud]radar-cloud <<<<<<< clod[error] parse json failure!";
-                    return false;
+                    return 1;
                 }
                 if (thiz->getConfiger().enableDebugPrint)
                 {
diff --git a/middleware/protocol/ccindex/ccindex_component.h b/middleware/protocol/ccindex/ccindex_component.h
index 34101ff..9733ff6 100644
--- a/middleware/protocol/ccindex/ccindex_component.h
+++ b/middleware/protocol/ccindex/ccindex_component.h
@@ -26,6 +26,8 @@
 #include <cstdint>
 #include <vector>
 #include <atomic>
+#include <condition_variable>
+#include <mutex>
 #include "middleware/protocol/proto/monitor.pb.h"
 #include "base/device_connect/proto/cloud_data.pb.h"
 #include "middleware/protocol/proto/monitor_mec.pb.h"
@@ -383,127 +385,9 @@ public:
 
     virtual ~CCINDEX_COMPONENT()
     {
-
-        if(m_TimerMonitorMqttConnected > 0)
-        {
-            m_Eventloop->cancelTimer(m_TimerMonitorMqttConnected);
-            m_TimerMonitorMqttConnected = -1;
-        }
-        if(m_TimerMqttReconnectInter > 0)
-        {
-            m_EventloopCloud->cancelTimer(m_TimerMqttReconnectInter);
-            m_TimerMqttReconnectInter = -1;
-        }
-        if(m_TimerMqttReconnectCloud > 0)
-        {
-            m_EventloopCloud->cancelTimer(m_TimerMqttReconnectCloud);
-            m_TimerMqttReconnectCloud = -1;
-        }
-        if(m_temptimer > 0) {
-            m_EventloopTc->cancelTimer(m_temptimer);
-            m_temptimer = -1;
-        }
-        if(m_timer_mec_radar_cloud_fd > 0) {
-            m_Eventloop->cancelTimer(m_timer_mec_radar_cloud_fd);
-            m_timer_mec_radar_cloud_fd = -1;
-        }
-        if(m_TimerTrajectoriesMonitorStart > 0)
-        {
-            m_EventloopTrafficMetrics->cancelTimer(m_TimerTrajectoriesMonitorStart);
-            m_TimerTrajectoriesMonitorStart = -1;
-        }
-        if(m_TimerConfigQueryMonitor > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerConfigQueryMonitor);
-            m_TimerConfigQueryMonitor = -1;
-        }
-        if(m_TimerConfigQueryMonitorPushOnce > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerConfigQueryMonitorPushOnce);
-            m_TimerConfigQueryMonitorPushOnce = -1;
-        }
-
-        if(m_TimerConfigQueryInstantly > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerConfigQueryInstantly);
-            m_TimerConfigQueryInstantly = -1;
-        }
-        if(m_TimerConfigQuery2HttpServer > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerConfigQuery2HttpServer);
-            m_TimerConfigQuery2HttpServer = -1;
-        }
-
-        if(m_TimerConfigQuery2HttpServerTsmtc > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerConfigQuery2HttpServerTsmtc);
-            m_TimerConfigQuery2HttpServerTsmtc = -1;
-        }
-        if(m_TimerConfigQuery2HttpServerDesaysv > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerConfigQuery2HttpServerDesaysv);
-            m_TimerConfigQuery2HttpServerDesaysv = -1;
-        }
-        if(m_TimerConfigUpdateMonitor > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerConfigUpdateMonitor);
-            m_TimerConfigUpdateMonitor = -1;
-        }
-        if(m_TimerConfigUpdateMonitorTsmtc > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerConfigUpdateMonitorTsmtc);
-            m_TimerConfigUpdateMonitorTsmtc = -1;
-        }
-
-        if(m_TimerConfigUpdateMonitorDesaysv > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerConfigUpdateMonitorDesaysv);
-            m_TimerConfigUpdateMonitorDesaysv = -1;
-        }
-
-        mqttDeinit();
-        mqttDeinitCloud();
-        // 1. 停止并等待所有线程结束
-        if (m_Task && m_Task->joinable()) {
-            m_Task->join();
-        }
-        if (m_TaskTrafficMetrics && m_TaskTrafficMetrics->joinable()) {
-            m_TaskTrafficMetrics->join();
-        }
-        if (m_TaskTrafficMetricsPubFirst && m_TaskTrafficMetricsPubFirst->joinable()) {
-            m_TaskTrafficMetricsPubFirst->join();
-        }
-        if (m_TaskTrafficMetricsPubSecond && m_TaskTrafficMetricsPubSecond->joinable()) {
-            m_TaskTrafficMetricsPubSecond->join();
-        }
-        if (m_TaskTc && m_TaskTc->joinable()) {
-            m_TaskTc->join();
-        }
-        if (m_TaskTcPubFirst && m_TaskTcPubFirst->joinable()) {
-            m_TaskTcPubFirst->join();
-        }
-        if (m_TaskTcPubSecond && m_TaskTcPubSecond->joinable()) {
-            m_TaskTcPubSecond->join();
-        }
-        if (m_TaskStatic && m_TaskStatic->joinable()) {
-            m_TaskStatic->join();
-        }
-        if (m_TaskStaticCloud && m_TaskStaticCloud->joinable()) {
-            m_TaskStaticCloud->join();
-        }
-        if (m_TaskUseCase && m_TaskUseCase->joinable()) {
-            m_TaskUseCase->join();
-        }
-        if(m_TimerMecLinkStatusCcindexTm > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerMecLinkStatusCcindexTm);
-            m_TimerMecLinkStatusCcindexTm = -1;
-        }
-        if(m_TimerMecLinkStatusCcindexSt > 0)
-        {
-            m_EventloopStatic->cancelTimer(m_TimerMecLinkStatusCcindexSt);
-            m_TimerMecLinkStatusCcindexSt = -1;
-        }
+        // 依次停EventLoop/上传队列线程、断开MQTT。原来这里先断MQTT再join，而EventLoop从不退出，
+        // join一直卡住、定时器还在析构中的对象上运行，重复节点/Ctrl+C退出时卡死或段错误
+        shutdownWorkers();
     };
 
     bool Init() override;
@@ -528,6 +412,9 @@ public:
     bool getMqttConfigerMsg();
     bool getWorkParamFromFile();
     bool initUseCase();
+    void notifyEventLoopReady();
+    void waitEventLoopsReady(int count);
+    void shutdownWorkers();
     void processProcessEventOutputResult(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData);
 
 private:
@@ -986,6 +873,10 @@ private:
     std::shared_ptr<afl::net::EventLoop>    m_EventloopTcPubSecond;
     std::shared_ptr<afl::net::EventLoop>    m_EventloopStatic;
     std::shared_ptr<afl::net::EventLoop>    m_EventloopCloud;
+    // 各EventLoop在各自线程中创建，Init()通过以下变量等待全部创建完成
+    std::mutex                              m_EventloopReadyMutex;
+    std::condition_variable                 m_EventloopReadyCv;
+    int                                     m_EventloopReadyCount = 0;
     std::unique_ptr<afl::net::Channel>      m_Chnl;
     /////////////////////////////////////////////////////////////
     std::shared_ptr<afl::net::EventLoop>    m_EventloopTrajectoriesTmstc;
@@ -1021,6 +912,7 @@ private:
     std::mutex cv_m2;                      // 条件变量的互斥锁
     bool queue1_ready = false;            // 标志队列 1 是否准备好
     bool queue2_ready = false;            // 标志队列 2 是否准备好
+    std::atomic<bool> m_StopWorkers{false};  // 析构时通知EventLoop/上传队列线程退出
     std::unique_ptr<std::thread>    m_TaskQueue1;
     std::unique_ptr<std::thread>    m_TaskQueue2;
     int m_TimerTrajectoriesMonitorStart = -1;
```
