# a818aa8  change format & modify blocking queue

- 作者：fengwenxi
- 日期：2024-10-17
- 完整 hash：`a818aa81721460bfe27aa60a6e5b38af8339d1c1`
- 改动：+180 / -328 行，涉及 6 个文件

涉及文件：
- `.clang-format`
- `CMakeLists.txt`
- `src/base/blocking_queue.hpp`
- `src/base/copyable.hpp`
- `src/base/noncopyable.hpp`
- `src/base/singleton.hpp`

```diff
diff --git a/.clang-format b/.clang-format
index 03e94dc..94a57e7 100644
--- a/.clang-format
+++ b/.clang-format
@@ -41,7 +41,7 @@ BreakBeforeBinaryOperators: None
 BreakBeforeTernaryOperators: true
 BreakConstructorInitializers: BeforeColon
 BreakInheritanceList: BeforeColon
-ColumnLimit: 80
+ColumnLimit: 120
 CompactNamespaces: false
 ContinuationIndentWidth: 4
 Cpp11BracedListStyle: true
diff --git a/CMakeLists.txt b/CMakeLists.txt
index 8e5ea9d..2a8b643 100644
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@ -1,49 +1,51 @@
-cmake_minimum_required(VERSION 3.0)
+CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
 
-if(NOT DEFINED TF_VERSION_MAJOR)
+# Set package version
+IF(NOT DEFINED TF_VERSION_MAJOR)
     SET(TF_VERSION_MAJOR 0)
-endif()
-if (NOT DEFINED TF_VERSION_MINOR)
+ENDIF()
+IF (NOT DEFINED TF_VERSION_MINOR)
     SET(TF_VERSION_MINOR 1)
-endif()
-if (NOT DEFINED TF_VERSION_PATCH)
+ENDIF()
+IF (NOT DEFINED TF_VERSION_PATCH)
     SET(TF_VERSION_PATCH 0)
-endif()
-if (NOT DEFINED TF_VERSION_BUILD)
+ENDIF()
+IF (NOT DEFINED TF_VERSION_BUILD)
     SET(TF_VERSION_BUILD 0)
-endif()
-if (NOT PACKAGE_VERSION)
+ENDIF()
+IF (NOT PACKAGE_VERSION)
   SET(PACKAGE_VERSION "${TF_VERSION_MAJOR}.${TF_VERSION_MINOR}.${TF_VERSION_PATCH}.${TF_VERSION_BUILD}")
-endif()
+ENDIF()
 
 PROJECT(TrafficFlow VERSION ${TF_VERSION_MAJOR}.${TF_VERSION_MINOR}.${TF_VERSION_PATCH}.${TF_VERSION_BUILD} LANGUAGES C CXX)
-
 SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")
+# Set build options
+OPTION(BUILD_DEBUG "Build debug mode" OFF)
+OPTION(BUILD_SHARED_LIBS "Build shared or static library" ON)
+OPTION(BUILD_DEMO "Build demo or not" OFF)
+OPTION(BUILD_TOOL "Build tools or not" OFF)
+OPTION(BUILD_TEST "Build unit test or not" OFF)
+OPTION(BUILD_PLUGIN "Build plug-ins or not" ON)
+# Set cmake install dir
 SET(CMAKE_INSTALL_LIBDIR ${PROJECT_SOURCE_DIR}/release/lib)
 SET(CMAKE_INSTALL_BINDIR ${PROJECT_SOURCE_DIR}/release/bin)
 SET(CMAKE_INSTALL_CONFIGDIR ${PROJECT_SOURCE_DIR}/release/config)
 SET(CMAKE_INSTALL_DOCDIR ${PROJECT_SOURCE_DIR}/release/doc)
 SET(CMAKE_INSTALL_INCLUDEDIR ${PROJECT_SOURCE_DIR}/release/include)
 SET(TF_EXTRA_DEPENDS "")
-if (CMAKE_SYSTEM_NAME MATCHES "^Linux")
+IF (CMAKE_SYSTEM_NAME MATCHES "^Linux")
 	LIST(APPEND TF_EXTRA_DEPENDS "dl")
 	LIST(APPEND TF_EXTRA_DEPENDS "pthread")
-endif()
+ENDIF()
 
-OPTION(BUILD_DEBUG "Build debug mode" OFF)
-OPTION(BUILD_SHARED_LIBS "Build shared or static library" ON)
-OPTION(BUILD_DEMO "Build demo or not" OFF)
-OPTION(BUILD_TOOL "Build tools or not" OFF)
-OPTION(BUILD_TEST "Build unit test or not" OFF)
-OPTION(BUILD_PLUGIN "Build plug-ins or not" ON)
 
 SET(CMAKE_C_STANDARD 99)
 SET(CMAKE_CXX_STANDARD 17)
 
-if (MSVC OR WIN32)
-    if (${CMAKE_VERSION} VERSION_LESS "3.14.0")
+IF (MSVC OR WIN32)
+    IF (${CMAKE_VERSION} VERSION_LESS "3.14.0")
         MESSAGE(FATAL_ERROR "TrafficFlow Module requires CMake 3.14+ to build on Windows.")
-    endif()
+    ENDIF()
     SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4190 /wd4722 /wd4819")
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4190 /wd4722 /wd4819")
     FOREACH(flag_var
@@ -51,62 +53,63 @@ if (MSVC OR WIN32)
         CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
         CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
         CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
-        if(${flag_var} MATCHES "/MD")
+        IF(${flag_var} MATCHES "/MD")
             STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
-        endif()
+        ENDIF()
     ENDFOREACH()
-elseif(CMAKE_SYSTEM_NAME MATCHES "^Android" OR CMAKE_SYSTEM_NAME MATCHES "^Linux")
+ELSEIF(CMAKE_SYSTEM_NAME MATCHES "^Android" OR CMAKE_SYSTEM_NAME MATCHES "^Linux")
     ADD_DEFINITIONS(-fPIC)
-endif()
+ENDIF()
 
-if (NOT MSVC)
+IF (NOT MSVC)
     SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
 	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
-endif()
+ENDIF()
 
-if (CMAKE_BUILD_TYPE MATCHES Debug OR BUILD_DEBUG)
-    MESSAGE("<=========== Working on debug mode ===========>")
+IF (CMAKE_BUILD_TYPE MATCHES Debug OR BUILD_DEBUG)
+    MESSAGE("<================ Working on debug mode ================>")
     ADD_DEFINITIONS(-DDEBUG)
-    if (MSVC)
+    IF (MSVC)
         SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DEBUG")
         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEBUG")
-    else()
+    ELSE()
         SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
         ADD_DEFINITIONS(-DDEBUG)
-    endif()
-else()
-    if (MSVC)
+    ENDIF()
+ELSE()
+    IF (MSVC)
         SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2")
         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
-    else()
-		if (BUILD_TEST)
+    ELSE()
+		IF (BUILD_TEST)
 			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fvisibility=hidden")
 			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fvisibility-inlines-hidden -fvisibility=hidden")
-		else()
+		ELSE()
 			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ffast-math -fvisibility=hidden")
 			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -fvisibility-inlines-hidden -fvisibility=hidden")
-		endif()
-    endif()
-endif()
+		ENDIF()
+    ENDIF()
+ENDIF()
 
-if(CMAKE_SYSTEM_NAME MATCHES "^Android")
-    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
+IF(CMAKE_SYSTEM_NAME MATCHES "^Android")
+    IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
         ADD_DEFINITIONS(-mfloat-abi=softfp -mfpu=neon)
 		ADD_DEFINITIONS(-DDS_USE_NEON)
-    endif()
-endif()
-	
-ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/traffic_flow)
-ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/common)
+    ENDIF()
+ENDIF()
+
+ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/base)
+# ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/common)
+# ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/traffic_flow)
 
-# if (BUILD_DEMO)
-# 	ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/demo)
-# endif()
+# IF (BUILD_DEMO)
+# 	ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/demo)
+# ENDIF()
 
-if (BUILD_PLUGIN)
+IF (BUILD_PLUGIN)
 	ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/plugin)
-endif()
+ENDIF()
 
 # copy conf files
 FILE(GLOB CONFIG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/*)
diff --git a/src/base/blocking_queue.hpp b/src/base/blocking_queue.hpp
index 2fe910c..c9304e0 100644
--- a/src/base/blocking_queue.hpp
+++ b/src/base/blocking_queue.hpp
@@ -1,8 +1,7 @@
 /**
  * @file blocking_queue.hpp
  * @author FengWenxi (ThorfromAsgard@outlook.com)
- * @brief An implementation of blocking queue using std::condition_variable
- * NOTE: This only works on C++11 standard. 
+ * @brief An implementation of blocking queue using std::condition_variable #NOTE This only works on C++11 standard.
  * @version 0.1
  * @date 2024-10-08
  *
@@ -20,50 +19,46 @@
 #include <deque>
 #include <iostream>
 #include <condition_variable>
-#include <cassert>
 
 namespace cybertron::base {
 template <typename T>
 /**
- * @brief An implementation of blocking queue. //NOTE that the parameter {}
+ * @brief An implementation of blocking queue. The queue works in two modes by specifying the {push_block} parameter.
+ * #NOTE Use it carefully when set the parameter {capacity_limit} to 0 because it may lead to unlimited memory
+ * consumption.
  *
  */
 // TODO: add timeout for push method
 class BlockingQueue : public Noncopyable {
 public:
     /**
-     * @brief Construct a new Blocking Queue object, use it carefully when set
-     * the parameter "size" to 0.
+     * @brief Construct a new Blocking Queue object, use it carefully when set the parameter {capacity_limit} to 0.
      *
-     * @param push_block Whether the push method will work in blocking mode or
-     * not.
+     * @param push_block Whether the push method will work in blocking mode or not.
      *
-     * @param max_capacity Again, you should be very aware of what you are doing
-     * when set it to 0, if you are not sure, then you are not aware !!!!!!
+     * @param capacity_limit The capacity limit of the queue, again, you should be very aware of what you are doing when
+     * set it to 0, if you are not sure, then you are not aware !!!!!!
      *
      */
-    explicit BlockingQueue(bool push_block = false, size_t max_capacity = 0)
+    explicit BlockingQueue(bool push_block = false, size_t capacity_limit = 0)
         : push_block_(push_block),
-          capacity_(max_capacity),
+          capacity_limit_(capacity_limit),
           mutex_(),
           consumer_(mutex_),
           deque_(),
           active_(true) {
-        assert(capacity_ >= 0);
-        std::cout << "Blocking Queue capacity: " << capacity_ << std::endl;
-        if (capacity_ == 0) {
+        std::cout << "Blocking Queue capacity: " << capacity_limit_ << std::endl;
+        if (capacity_limit_ == 0) {
             // TODO: Find another way to warning!
-            std::cout
-                << "Warning! Blocking queue parameter size is set to 0, may "
-                   "cause memory overflow."
-                << std::endl;
+            std::cout << "Warning! Blocking queue parameter {capacity_limit} is set to 0, may cause out of memory."
+                      << std::endl;
         }
     }
 
     /**
      * @brief Destroy the Blocking Queue object.
      *
-     * TODO: Do tests to make sure this works fine!
+     * #TODO: Do tests to make sure this works fine!
      */
     ~BlockingQueue() {
         {
@@ -76,132 +71,69 @@ public:
     }
 
     /**
-     * @brief Push item to the back of the queue in a blocking way.
+     * @brief Push element to the back of the queue within {timeout} microseconds in a blocking way. If the {timeout}
+     * parameter is set to 0, then it will always try to push until success or the queue is closed.
      *
-     * @param item Input elemet that is going to be pushed.
-     * @return true if the function successfully pushes one element to the back
-     * of the queue,
-     * @return false if it fails.
-     */
-    bool PushBack(const T& item) {
-        /**
-         * @brief Use unique_lock instead of lock_guard because of it allows
-         * unlock&relock temporarily while lock_guard only allows unlock after
-         * its scope
-         *
-         */
-        std::unique_lock<std::mutex> lock(mutex_);
-        if (push_block_) {
-            producer_.wait(lock, [&] {
-                return !active_ || !capacity_ ||
-                       (capacity_ && (deque_.size() < capacity_));
-            });
-            if (!active_) {
-                return false;
-            }
-        } else {
-            while (capacity_ && deque_.size() >= capacity_) {
-                deque_.pop_front();
-            }
-        }
-        deque_.push_back(item);
-        consumer_.notify_one();
-        return true;
-    }
-
-    /**
-     * @brief Push the item to the back of the queue within {timeout}
-     * microseconds in a blocking way.If the {timeout} parameter is set to
-     * 0,then it will always try to push until success or the Blocking Queue is
-     * closed
-     *
-     * @param item Input element that is going to be pushed.
+     * @param element Input element that is going to be pushed.
      * @param timeout in microseconds.
-     * @return true if the function is not always empty in {timeout}
-     * microseconds and successfully pushes one element to the back of the
-     * queue,
+     * @return true if the function is not always full during {timeout} microseconds and successfully pushes one element
+     * to the back of the queue,
      * @return false if it fails.
      */
-    bool PushBack(const T& item, const int64_t& timeout = 0) {
+    bool PushBack(const T& element, const int64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
             std::cv_status is_timeout = std::cv_status::no_timeout;
             if (timeout) {
-                is_timeout = producer_.wait_for(
-                    lock, std::chrono::microseconds(timeout), [&] {
-                        return ((!active_) || (!capacity_) ||
-                                (capacity_ && (deque_.size() < capacity_)));
-                    });
+                is_timeout = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
+                    return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
+                });
             } else {
                 producer_.wait(lock, [&] {
-                    return ((!active_) || (!capacity_) ||
-                            (capacity_ && (deque_.size() < capacity_)));
+                    return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
             }
             if ((!active_) || (is_timeout == std::cv_status::timeout)) {
                 return false;
             }
         } else {
-            while (capacity_ && deque_.size() >= capacity_) {
-                deque_.pop_front();
-            }
-        }
-        deque_.push_back(item);
-        consumer_.notify_one();
-        return true;
-    }
-
-    /**
-     * @brief Push item to the back of the queue in a blocking way.
-     *
-     * @param item Input elemet that is going to be pushed.
-     * @return true if the function successfully pushes one element to the back
-     * of the queue,
-     * @return false if it fails.
-     */
-    bool PushBack(T&& item) {
-        std::unique_lock<std::mutex> lock(mutex_);
-        if (push_block_) {
-            producer_.wait(lock, [&] {
-                return !active_ || !capacity_ ||
-                       (capacity_ && (deque_.size() < capacity_));
-            });
-            if (!active_) {
-                return false;
-            }
-        } else {
-            while (capacity_ && deque_.size() >= capacity_) {
+            while (capacity_limit_ && deque_.size() >= capacity_limit_) {
                 deque_.pop_front();
             }
         }
-        deque_.push_back(std::move(item));
+        deque_.push_back(element);
         consumer_.notify_one();
         return true;
     }
 
     /**
-     * @brief Push item to the back of the queue within {timeout} microseconds
-     * in a blocking way.
+     * @brief Push element to the back of the queue within {timeout} microseconds in a blocking way. If the {timeout}
+     * parameter is set to 0, then it will always try to push until success or the queue is closed.
      *
-     * @param item Input elemet that is going to be pushed.
+     * @param element Input element that is going to be pushed.
      * @param timeout in microseconds.
-     * @return true if the function successfully pushes one element to the
-     * back of the queue,
+     * @return true if the function is not always full during {timeout} microseconds and successfully pushes one element
+     * to the back of the queue,
      * @return false if it fails.
      */
-    bool PushBack(T&& item, const uint64_t& timeout) {
+    bool PushBack(T&& element, const uint64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
-            const auto& is_timeout = producer_.wait_for(
-                lock, std::chrono::microseconds(timeout), [&] {
-                    return ((!active_) || (!capacity_) ||
-                            (capacity_ && (deque_.size() < capacity_)));
+            std::cv_status is_timeout = std::cv_status::no_timeout;
+            if (timeout) {
+                is_timeout = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
+                    return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
+            } else {
+                producer_.wait(lock, [&] {
+                    return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
+                });
+            }
             if ((!active_) || (is_timeout == std::cv_status::timeout)) {
                 return false;
             }
         } else {
-            while (capacity_ && deque_.size() >= capacity_) {
+            while (capacity_limit_ && deque_.size() >= capacity_limit_) {
                 deque_.pop_front();
             }
         }
@@ -211,112 +143,68 @@ public:
     }
 
     /**
-     * @brief Push item to the front of the queue in a blocking way.
+     * @brief Push element to the front of the queue within {timeout} microseconds in a blocking way. If the {timeout}
+     * parameter is set to 0, then it will always try to push until success or the queue is closed.
      *
-     * @param item Input elemet that is going to be pushed.
-     * @return true if the function successfully pushes one element to the front
-     * of the queue,
-     * @return false if it fails.
-     */
-    bool PushFront(const T& item) {
-        std::unique_lock<std::mutex> lock(mutex_);
-        if (push_block_) {
-            producer_.wait(lock, [&] {
-                return !active_ || !capacity_ ||
-                       (capacity_ && (deque_.size() < capacity_));
-            });
-            if (!active_) {
-                return false;
-            }
-        } else {
-            while (capacity_ && deque_.size() >= capacity_) {
-                deque_.pop_back();
-            }
-        }
-        deque_.push_front(item);
-        consumer_.notify_one();
-    }
-
-    /**
-     * @brief Push item to the front of the queue within {timeout} microseconds
-     * in a blocking way.
-     *
-     * @param item Input elemet that is going to be pushed.
+     * @param element Input element that is going to be pushed.
      * @param timeout Timeout in microseconds.
-     * @return true if the function successfully pushes one element to the
-     * front of the queue,
+     * @return true if the function is not always full during {timeout} microseconds and successfully pushes one element
+     * to the front of the queue,
      * @return false if it fails.
      */
-    bool PushFront(const T& item, const uint64_t& timeout) {
+    bool PushFront(const T& element, const uint64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
-            const auto& is_timeout = producer_.wait_for(
-                lock, std::chrono::microseconds(timeout), [&] {
-                    return ((!active_) || (!capacity_) ||
-                            (capacity_ && (deque_.size() < capacity_)));
+            std::cv_status is_timeout = std::cv_status::no_timeout;
+            if (timeout) {
+                is_timeout = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
+                    return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
+                });
+            } else {
+                producer_.wait(lock, [&] {
+                    return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
-            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
-                return false;
-            }
-        } else {
-            while (capacity_ && deque_.size() >= capacity_) {
-                deque_.pop_back();
             }
-        }
-        deque_.push_front(item);
-        consumer_.notify_one();
-    }
-
-    /**
-     * @brief Push item to the front of the queue in a blocking way.
-     *
-     * @param item Input elemet that is going to be pushed.
-     * @return true if the function successfully pushes one element to the front
-     * of the queue,
-     * @return false if it fails.
-     */
-    bool PushFront(T&& item) {
-        std::unique_lock<std::mutex> lock(mutex_);
-        if (push_block_) {
-            producer_.wait(lock, [&] {
-                return !active_ || !capacity_ ||
-                       (capacity_ && (deque_.size() < capacity_));
-            });
-            if (!active_) {
+            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
                 return false;
             }
         } else {
-            if (capacity_ && deque_.size() >= capacity_) {
+            while (capacity_limit_ && deque_.size() >= capacity_limit_) {
                 deque_.pop_back();
             }
         }
-        deque_.push_front(std::move(item));
+        deque_.push_front(element);
         consumer_.notify_one();
     }
 
     /**
-     * @brief Push item to the front of the queue within {timeout} microseconds
-     * in a blocking way.
+     * @brief Push element to the front of the queue within {timeout} microseconds in a blocking way. If the {timeout}
+     * parameter is set to 0, then it will always try to push until success or the queue is closed.
      *
-     * @param item Input elemet that is going to be pushed.
+     * @param element Input element that is going to be pushed.
      * @param timeout Timeout in microseconds.
-     * @return true if the function successfully pushes one element to the
-     * front of the queue,
+     * @return true if the function is not always full during {timeout} microseconds and successfully pushes one element
+     * to the front of the queue,
      * @return false if it fails.
      */
-    bool PushFront(T&& item, const uint64_t& timeout) {
+    bool PushFront(T&& item, const uint64_t& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
-            const auto& is_timeout = producer_.wait_for(
-                lock, std::chrono::microseconds(timeout), [&] {
-                    return ((!active_) || (!capacity_) ||
-                            (capacity_ && (deque_.size() < capacity_)));
+            std::cv_status is_timeout = std::cv_status::no_timeout;
+            if (timeout) {
+                is_timeout = producer_.wait_for(lock, std::chrono::microseconds(timeout), [&] {
+                    return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
+                });
+            } else {
+                producer_.wait(lock, [&] {
+                    return ((!active_) || (!capacity_limit_) || (capacity_limit_ && (deque_.size() < capacity_limit_)));
                 });
+            }
             if ((!active_) || (is_timeout == std::cv_status::timeout)) {
                 return false;
             }
         } else {
-            if (capacity_ && deque_.size() >= capacity_) {
+            while (capacity_limit_ && deque_.size() >= capacity_limit_) {
                 deque_.pop_back();
             }
         }
@@ -325,46 +213,28 @@ public:
     }
 
     /**
-     * @brief Pop front element in a blocking way.
+     * @brief Pop the front element of the queue within {timeout} microseconds in a blocking way. If the {timeout}
+     * parameter is set to 0, then it will always try to pop until success or the queue is closed.
      *
-     * @param item output item
-     * @return true if the fuction successfully pops one element from the front
-     * of the queue,
+     * @param element Output element.
+     * @param timeout Timeout in microseconds.
+     * @return true if the function is not always empty during {timeout} microseconds and successfully pops one element
+     * from the front of the queue,
      * @return false if it fails.
      */
-    bool PopFront(T& item) {
+    bool PopFront(T& element, const int& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
-        consumer_.wait(lock, [&] { return !active_ || !deque_.empty(); });
-        if (!active_) {
-            return false;
-        }
-        item = deque_.front();
-        deque_.pop_front();
-        if (push_block_) {
-            producer_.notify_one();
+        std::cv_status is_timeout = std::cv_status::no_timeout;
+        if (timeout) {
+            is_timeout = consumer_.wait_for(lock, std::chrono::microseconds(timeout),
+                                            [&] { return ((!active_) || (!deque_.empty())); });
+        } else {
+            consumer_.wait(lock, [&] { return ((!active_) || (!deque_.empty())); });
         }
-        return true;
-    }
-
-    /**
-     * @brief Pop front element within {timeout} microseconds in a blocking way.
-     *
-     * @param item output item.
-     * @param timeout in microseconds.
-     * @return true if the function is not always empty in {timeout}
-     * microseconds and successfully pops one element from the front of the
-     * queue,
-     * @return false if it fails.
-     */
-    bool PopFront(T& item, const int& timeout) {
-        std::unique_lock<std::mutex> lock(mutex_);
-        const auto& is_timeout =
-            consumer_.wait_for(lock, std::chrono::microseconds(timeout),
-                               [&] { return !active_ || deque_.empty(); });
         if ((!active_) || (is_timeout == std::cv_status::timeout)) {
             return false;
         };
-        item = deque_.front();
+        element = deque_.front();
         deque_.pop_front();
         if (push_block_) {
             producer_.notify_one();
@@ -373,45 +243,28 @@ public:
     }
 
     /**
-     * @brief Pop back element in a blocking way.
+     * @brief Pop the back element of the queue within {timeout} microseconds in a blocking way. If the {timeout}
+     * parameter is set to 0, then it will always try to pop until success or the queue is closed.
      *
-     * @param item output item.
-     * @return true if the function successfully pops one element from the back
-     * of the queue,
+     * @param element Output element.
+     * @param timeout Timeout in microseconds.
+     * @return true if the function is not always empty during {timeout} microseconds and successfully pops one element
+     * from the back of the queue,
      * @return false if it fails.
      */
-    bool PopBack(T& item) {
+    bool PopBack(T& element, const int& timeout = 0) {
         std::unique_lock<std::mutex> lock(mutex_);
-        consumer_.wait(lock, [&] { return !active_ || !deque_.empty(); });
-        if (!active_) {
-            return false;
-        }
-        item = deque_.back();
-        deque_.pop_back();
-        if (push_block_) {
-            producer_.notify_one();
+        std::cv_status is_timeout = std::cv_status::no_timeout;
+        if (timeout) {
+            is_timeout = consumer_.wait_for(lock, std::chrono::microseconds(timeout),
+                                            [&] { return ((!active_) || (!deque_.empty())); });
+        } else {
+            consumer_.wait(lock, [&] { return ((!active_) || (!deque_.empty())); });
         }
-        return true;
-    }
-
-    /**
-     * @brief Pop back element within {timeout} microseconds in a blocking way.
-     *
-     * @param item output item.
-     * @param timeout in microseconds.
-     * @return true if the function is not empty in {timeout} microseconds and
-     * successfully pops one element from the back of the queue,
-     * @return false if it fails.
-     */
-    bool PopBack(T& item, const int& timeout) {
-        std::unique_lock<std::mutex> lock(mutex_);
-        auto is_timeout =
-            consumer_.wait_for(lock, std::chrono::microseconds(timeout),
-                               [&] { return ((!active_) || deque_.empty()); });
         if ((!active_) || (is_timeout == std::cv_status::timeout)) {
             return false;
         };
-        item = deque_.back();
+        element = deque_.back();
         deque_.pop_back();
         if (push_block_) {
             producer_.notify_one();
@@ -426,7 +279,7 @@ public:
 
     size_t Capacity() {
         std::lock_guard<std::mutex> lock(mutex_);
-        return capacity_;
+        return capacity_limit_;
     }
 
     bool Empty() {
@@ -436,7 +289,7 @@ public:
 
     bool Full() {
         std::lock_guard<std::mutex> lock(mutex_);
-        return (deque_.size() >= capacity_);
+        return (deque_.size() >= capacity_limit_);
     }
 
     T Front() {
@@ -457,7 +310,7 @@ public:
 private:
     const bool push_block_;
     bool active_;
-    const size_t capacity_;
+    const size_t capacity_limit_;
     std::mutex mutex_;
     std::condition_variable consumer_;  // guarded by mutex_
     std::condition_variable producer_;  // guarded by mutex_
diff --git a/src/base/copyable.hpp b/src/base/copyable.hpp
index e910a95..f70582f 100644
--- a/src/base/copyable.hpp
+++ b/src/base/copyable.hpp
@@ -4,11 +4,11 @@
  * @brief A copyable base class
  * @version 1.0
  * @date 2024-10-08
- * 
+ *
  * <========================================================================>
- *                   © 2024 FengWenxi. All Rights Reserved.                
+ *                   © 2024 FengWenxi. All Rights Reserved.
  * <========================================================================>
- * 
+ *
  */
 #ifndef CYBERTRON_BASE_COPYABLE_HPP
 #define CYBERTRON_BASE_COPYABLE_HPP
diff --git a/src/base/noncopyable.hpp b/src/base/noncopyable.hpp
index 304f2b1..de50feb 100644
--- a/src/base/noncopyable.hpp
+++ b/src/base/noncopyable.hpp
@@ -4,19 +4,18 @@
  * @brief A noncopyable base class
  * @version 1.0
  * @date 2024-10-08
- * 
+ *
  * <========================================================================>
- *                   © 2024 FengWenxi. All Rights Reserved.                
+ *                   © 2024 FengWenxi. All Rights Reserved.
  * <========================================================================>
- * 
+ *
  */
 #ifndef CYBERTRON_BASE_NONCOPYABLE_HPP
 #define CYBERTRON_BASE_NONCOPYABLE_HPP
 
 namespace cybertron::base {
 /**
- * @brief A tag class emphasises that the objects are noncopyable.
- * Derived class should be noncopyable, too.
+ * @brief A tag class emphasises that the objects are noncopyable. Derived class should be noncopyable, too.
  *
  */
 class Noncopyable {
diff --git a/src/base/singleton.hpp b/src/base/singleton.hpp
index d2a8e5a..1a02fc9 100644
--- a/src/base/singleton.hpp
+++ b/src/base/singleton.hpp
@@ -1,15 +1,14 @@
 /**
  * @file singleton.hpp
  * @author FengWenxi (ThorfromAsgard@outlook.com)
- * @brief A simple implementation of singleton class using template and perfect
- * forwarding
+ * @brief A simple implementation of singleton class using template and perfect forwarding.
  * @version 1.0
  * @date 2024-10-08
- * 
+ *
  * <========================================================================>
- *                   © 2024 FengWenxi. All Rights Reserved.                
+ *                   © 2024 FengWenxi. All Rights Reserved.
  * <========================================================================>
- * 
+ *
  */
 #ifndef CYBERTRON_BASE_SINGLETON_HPP
 #define CYBERTRON_BASE_SINGLETON_HPP
@@ -21,10 +20,9 @@
 namespace cybertron::base {
 template <typename T>
 /**
-	 * @brief A template singleton class, use singleton through function
-	 * "GetInstance"
-	 *
-	 */
+ * @brief A template singleton class, use singleton through function "GetInstance()"
+ *
+ */
 class Singleton : public Noncopyable {
 public:
     Singleton(Singleton&&) = delete;
@@ -33,8 +31,7 @@ public:
 
     template <typename... Args>
     static T& GetInstance(Args&&... args) {
-        std::call_once(init_flag_, Initialize<Args...>,
-                       std::forward<Args>(args)...);
+        std::call_once(init_flag_, Initialize<Args...>, std::forward<Args>(args)...);
         return *instance_;
     }
 
```
