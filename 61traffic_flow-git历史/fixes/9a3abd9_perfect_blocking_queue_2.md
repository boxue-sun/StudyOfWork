# 9a3abd9  perfect blocking queue

- 作者：fengwenxi
- 日期：2024-10-16
- 完整 hash：`9a3abd9409b0aabd1df97a8c507b4d683810946f`
- 改动：+230 / -76 行，涉及 1 个文件

涉及文件：
- `src/base/blocking_queue.hpp`

```diff
diff --git a/src/base/blocking_queue.hpp b/src/base/blocking_queue.hpp
index 3a48f69..fb88294 100644
--- a/src/base/blocking_queue.hpp
+++ b/src/base/blocking_queue.hpp
@@ -33,10 +33,14 @@ class BlockingQueue : public Noncopyable {
 public:
     /**
      * @brief Construct a new Blocking Queue object, use it carefully when set
-     * parameter "size" to 0
+     * the parameter "size" to 0.
+     *
+     * @param push_block Whether the push method will work in blocking mode or
+     * not.
+     *
+     * @param max_capacity Again, you should be very aware of what you are doing
+     * when set it to 0, if you are not sure, then you are not aware !!!!!!
      *
-     * @param size Again, you should be very aware of what you are doing when
-     * set it to 0, if you are not sure, then you are not aware !!!!!!
      */
     explicit BlockingQueue(bool push_block = false, size_t max_capacity = 0)
         : push_block_(push_block),
@@ -56,6 +60,11 @@ public:
         }
     }
 
+    /**
+     * @brief Destroy the Blocking Queue object.
+     *
+     * TODO: Do tests to make sure this works fine!
+     */
     ~BlockingQueue() {
         {
             std::lock_guard<std::mutex> lock(mutex_);
@@ -66,6 +75,14 @@ public:
         consumer_.notify_all();
     }
 
+    /**
+     * @brief Push item to the back of the queue in a blocking way.
+     *
+     * @param item Input elemet that is going to be pushed.
+     * @return true if the function successfully pushes one element to the back
+     * of the queue,
+     * @return false if it fails.
+     */
     bool PushBack(const T& item) {
         /**
          * @brief Use unique_lock instead of lock_guard because of it allows
@@ -75,16 +92,44 @@ public:
          */
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
-            // TODO: change to an elegant implementation
-            // producer_.wait(lock, [this] {
-            //     return !active_ || (capacity_ && (deque_.size() <
-            //     capacity_));
-            // });
+            producer_.wait(lock, [&] {
+                return !active_ || !capacity_ ||
+                       (capacity_ && (deque_.size() < capacity_));
+            });
+            if (!active_) {
+                return false;
+            }
+        } else {
             while (capacity_ && deque_.size() >= capacity_) {
-                producer_.wait(lock);
-                if (!active_) {
-                    return false;
-                }
+                deque_.pop_front();
+            }
+        }
+        deque_.push_back(item);
+        consumer_.notify_one();
+        return true;
+    }
+
+    /**
+     * @brief Push item to the back of the queue within {timeout} microseconds
+     * in a blocking way.
+     *
+     * @param item Input element that is going to be pushed.
+     * @param timeout in microseconds.
+     * @return true if the function is not always empty in {timeout}
+     * microseconds and successfully pushes one element to the back of the
+     * queue,
+     * @return false if it fails.
+     */
+    bool PushBack(const T& item, const int& timeout) {
+        std::unique_lock<std::mutex> lock(mutex_);
+        if (push_block_) {
+            const auto& is_timeout = producer_.wait_for(
+                lock, std::chrono::microseconds(timeout), [&] {
+                    return ((!active_) || (!capacity_) ||
+                            (capacity_ && (deque_.size() < capacity_)));
+                });
+            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+                return false;
             }
         } else {
             while (capacity_ && deque_.size() >= capacity_) {
@@ -96,14 +141,54 @@ public:
         return true;
     }
 
+    /**
+     * @brief Push item to the back of the queue in a blocking way.
+     *
+     * @param item Input elemet that is going to be pushed.
+     * @return true if the function successfully pushes one element to the back
+     * of the queue,
+     * @return false if it fails.
+     */
     bool PushBack(T&& item) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
+            producer_.wait(lock, [&] {
+                return !active_ || !capacity_ ||
+                       (capacity_ && (deque_.size() < capacity_));
+            });
+            if (!active_) {
+                return false;
+            }
+        } else {
             while (capacity_ && deque_.size() >= capacity_) {
-                producer_.wait(lock);
-                if (!active_) {
-                    return false;
-                }
+                deque_.pop_front();
+            }
+        }
+        deque_.push_back(std::move(item));
+        consumer_.notify_one();
+        return true;
+    }
+
+    /**
+     * @brief Push item to the back of the queue within {timeout} microseconds
+     * in a blocking way.
+     *
+     * @param item Input elemet that is going to be pushed.
+     * @param timeout in microseconds.
+     * @return true if the function successfully pushes one element to the
+     * back of the queue,
+     * @return false if it fails.
+     */
+    bool PushBack(T&& item, const int& timeout) {
+        std::unique_lock<std::mutex> lock(mutex_);
+        if (push_block_) {
+            const auto& is_timeout = producer_.wait_for(
+                lock, std::chrono::microseconds(timeout), [&] {
+                    return ((!active_) || (!capacity_) ||
+                            (capacity_ && (deque_.size() < capacity_)));
+                });
+            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+                return false;
             }
         } else {
             while (capacity_ && deque_.size() >= capacity_) {
@@ -115,14 +200,53 @@ public:
         return true;
     }
 
+    /**
+     * @brief Push item to the front of the queue in a blocking way.
+     *
+     * @param item Input elemet that is going to be pushed.
+     * @return true if the function successfully pushes one element to the front
+     * of the queue,
+     * @return false if it fails.
+     */
     bool PushFront(const T& item) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
+            producer_.wait(lock, [&] {
+                return !active_ || !capacity_ ||
+                       (capacity_ && (deque_.size() < capacity_));
+            });
+            if (!active_) {
+                return false;
+            }
+        } else {
             while (capacity_ && deque_.size() >= capacity_) {
-                producer_.wait(lock);
-                if (!active_) {
-                    return false;
-                }
+                deque_.pop_back();
+            }
+        }
+        deque_.push_front(item);
+        consumer_.notify_one();
+    }
+
+    /**
+     * @brief Push item to the front of the queue within {timeout} microseconds
+     * in a blocking way.
+     *
+     * @param item Input elemet that is going to be pushed.
+     * @param timeout Timeout in microseconds.
+     * @return true if the function successfully pushes one element to the
+     * front of the queue,
+     * @return false if it fails.
+     */
+    bool PushFront(const T& item, const int& timeout) {
+        std::unique_lock<std::mutex> lock(mutex_);
+        if (push_block_) {
+            const auto& is_timeout = producer_.wait_for(
+                lock, std::chrono::microseconds(timeout), [&] {
+                    return ((!active_) || (!capacity_) ||
+                            (capacity_ && (deque_.size() < capacity_)));
+                });
+            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+                return false;
             }
         } else {
             while (capacity_ && deque_.size() >= capacity_) {
@@ -133,14 +257,23 @@ public:
         consumer_.notify_one();
     }
 
+    /**
+     * @brief Push item to the front of the queue in a blocking way.
+     *
+     * @param item Input elemet that is going to be pushed.
+     * @return true if the function successfully pushes one element to the front
+     * of the queue,
+     * @return false if it fails.
+     */
     bool PushFront(T&& item) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
-            while (capacity_ && deque_.size() >= capacity_) {
-                producer_.wait(lock);
-                if (!active_) {
-                    return false;
-                }
+            producer_.wait(lock, [&] {
+                return !active_ || !capacity_ ||
+                       (capacity_ && (deque_.size() < capacity_));
+            });
+            if (!active_) {
+                return false;
             }
         } else {
             if (capacity_ && deque_.size() >= capacity_) {
@@ -152,19 +285,48 @@ public:
     }
 
     /**
-     * @brief Pop front element in a blocking way
+     * @brief Push item to the front of the queue within {timeout} microseconds
+     * in a blocking way.
      *
-     * @param item output item
-     * @return true if the queue successfully pops one element from front
-     * @return false if the queue fails to pop one element from front
+     * @param item Input elemet that is going to be pushed.
+     * @param timeout Timeout in microseconds.
+     * @return true if the function successfully pushes one element to the
+     * front of the queue,
+     * @return false if it fails.
      */
-    bool PopFront(T& item) {
+    bool PushFront(T&& item, const int& timeout) {
         std::unique_lock<std::mutex> lock(mutex_);
-        while (deque_.empty()) {
-            consumer_.wait(lock);
-            if (!active_) {
+        if (push_block_) {
+            const auto& is_timeout = producer_.wait_for(
+                lock, std::chrono::microseconds(timeout), [&] {
+                    return ((!active_) || (!capacity_) ||
+                            (capacity_ && (deque_.size() < capacity_)));
+                });
+            if ((!active_) || (is_timeout == std::cv_status::timeout)) {
                 return false;
             }
+        } else {
+            if (capacity_ && deque_.size() >= capacity_) {
+                deque_.pop_back();
+            }
+        }
+        deque_.push_front(std::move(item));
+        consumer_.notify_one();
+    }
+
+    /**
+     * @brief Pop front element in a blocking way.
+     *
+     * @param item output item
+     * @return true if the fuction successfully pops one element from the front
+     * of the queue,
+     * @return false if it fails.
+     */
+    bool PopFront(T& item) {
+        std::unique_lock<std::mutex> lock(mutex_);
+        consumer_.wait(lock, [&] { return !active_ || !deque_.empty(); });
+        if (!active_) {
+            return false;
         }
         item = deque_.front();
         deque_.pop_front();
@@ -175,26 +337,23 @@ public:
     }
 
     /**
-     * @brief Pop front element within {timeout} seconds
+     * @brief Pop front element within {timeout} microseconds in a blocking way.
      *
-     * @param item output item
-     * @param timeout in seconds
-     * @return true if the queue is not empty in {timeout} seconds and
-     * successfully pops one element from front
-     * @return false if the queue is always empty during {timeout} seconds and
-     * thus fails to pop one element from front
+     * @param item output item.
+     * @param timeout in microseconds.
+     * @return true if the function is not always empty in {timeout}
+     * microseconds and successfully pops one element from the front of the
+     * queue,
+     * @return false if it fails.
      */
-    bool PopFront(T& item, int timeout) {
+    bool PopFront(T& item, const int& timeout) {
         std::unique_lock<std::mutex> lock(mutex_);
-        while (deque_.empty()) {
-            if (consumer_.wait_for(lock, std::chrono::seconds(timeout)) ==
-                std::cv_status::timeout) {
-                return false;
-            }
-            if (!active_) {
-                return false;
-            }
-        }
+        const auto& is_timeout =
+            consumer_.wait_for(lock, std::chrono::microseconds(timeout),
+                               [&] { return !active_ || deque_.empty(); });
+        if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+            return false;
+        };
         item = deque_.front();
         deque_.pop_front();
         if (push_block_) {
@@ -204,19 +363,18 @@ public:
     }
 
     /**
-     * @brief Pop back element in a blocking way
+     * @brief Pop back element in a blocking way.
      *
-     * @param item output item
-     * @return true if the queue successfully pops one element from back
-     * @return false if the queue fails to pop one element from back
+     * @param item output item.
+     * @return true if the function successfully pops one element from the back
+     * of the queue,
+     * @return false if it fails.
      */
     bool PopBack(T& item) {
         std::unique_lock<std::mutex> lock(mutex_);
-        while (deque_.empty()) {
-            consumer_.wait(lock);
-            if (!active_) {
-                return false;
-            }
+        consumer_.wait(lock, [&] { return !active_ || !deque_.empty(); });
+        if (!active_) {
+            return false;
         }
         item = deque_.back();
         deque_.pop_back();
@@ -227,26 +385,22 @@ public:
     }
 
     /**
-     * @brief Pop back element within {timeout} seconds
+     * @brief Pop back element within {timeout} microseconds in a blocking way.
      *
-     * @param item output item
-     * @param timeout in seconds
-     * @return true if the queue is not empty in {timeout} seconds and
-     * successfully pops one element from back
-     * @return false if the queue is always empty during {timeout} seconds and
-     * thus fails to pop from back
+     * @param item output item.
+     * @param timeout in microseconds.
+     * @return true if the function is not empty in {timeout} microseconds and
+     * successfully pops one element from the back of the queue,
+     * @return false if it fails.
      */
-    bool PopBack(T& item, int timeout) {
+    bool PopBack(T& item, const int& timeout) {
         std::unique_lock<std::mutex> lock(mutex_);
-        while (deque_.empty()) {
-            if (consumer_.wait_for(lock, std::chrono::seconds(timeout)) ==
-                std::cv_status::timeout) {
-                return false;
-            }
-            if (!active_) {
-                return false;
-            }
-        }
+        auto is_timeout =
+            consumer_.wait_for(lock, std::chrono::microseconds(timeout),
+                               [&] { return ((!active_) || deque_.empty()); });
+        if ((!active_) || (is_timeout == std::cv_status::timeout)) {
+            return false;
+        };
         item = deque_.back();
         deque_.pop_back();
         if (push_block_) {
```
