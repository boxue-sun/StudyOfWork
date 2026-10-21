# 6b7c6af  perfect BlockingQueue

- 作者：fengwenxi
- 日期：2024-10-15
- 完整 hash：`6b7c6aff66229cef021ba04059783363c1ec7132`
- 改动：+92 / -71 行，涉及 1 个文件

涉及文件：
- `src/base/blocking_queue.hpp`

```diff
diff --git a/src/base/blocking_queue.hpp b/src/base/blocking_queue.hpp
index 82dd00a..3a48f69 100644
--- a/src/base/blocking_queue.hpp
+++ b/src/base/blocking_queue.hpp
@@ -19,6 +19,7 @@
 #include <deque>
 #include <iostream>
 #include <condition_variable>
+#include <cassert>
 
 namespace cybertron::base {
 template <typename T>
@@ -27,6 +28,7 @@ template <typename T>
  * blocked even though the queue has reached its max capacity.
  *
  */
+// TODO: add timeout for push method
 class BlockingQueue : public Noncopyable {
 public:
     /**
@@ -36,14 +38,16 @@ public:
      * @param size Again, you should be very aware of what you are doing when
      * set it to 0, if you are not sure, then you are not aware !!!!!!
      */
-    explicit BlockingQueue(bool push_block = false, size_t size = 0)
+    explicit BlockingQueue(bool push_block = false, size_t max_capacity = 0)
         : push_block_(push_block),
-          capacity_(size),
+          capacity_(max_capacity),
           mutex_(),
           consumer_(mutex_),
           deque_(),
           active_(true) {
-        if (size == 0) {
+        assert(capacity_ >= 0);
+        std::cout << "Blocking Queue capacity: " << capacity_ << std::endl;
+        if (capacity_ == 0) {
             // TODO: Find another way to warning!
             std::cout
                 << "Warning! Blocking queue parameter size is set to 0, may "
@@ -62,7 +66,7 @@ public:
         consumer_.notify_all();
     }
 
-    void PushBack(const T& item) {
+    bool PushBack(const T& item) {
         /**
          * @brief Use unique_lock instead of lock_guard because of it allows
          * unlock&relock temporarily while lock_guard only allows unlock after
@@ -71,69 +75,80 @@ public:
          */
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
+            // TODO: change to an elegant implementation
+            // producer_.wait(lock, [this] {
+            //     return !active_ || (capacity_ && (deque_.size() <
+            //     capacity_));
+            // });
             while (capacity_ && deque_.size() >= capacity_) {
                 producer_.wait(lock);
+                if (!active_) {
+                    return false;
+                }
             }
         } else {
-            if (capacity_ && deque_.size() >= capacity_) {
+            while (capacity_ && deque_.size() >= capacity_) {
                 deque_.pop_front();
             }
         }
-        if (active_) {
-            deque_.push_back(item);
-            consumer_.notify_one();
-        }
+        deque_.push_back(item);
+        consumer_.notify_one();
+        return true;
     }
 
-    void PushBack(T&& item) {
+    bool PushBack(T&& item) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
             while (capacity_ && deque_.size() >= capacity_) {
                 producer_.wait(lock);
+                if (!active_) {
+                    return false;
+                }
             }
         } else {
-            if (capacity_ && deque_.size() >= capacity_) {
+            while (capacity_ && deque_.size() >= capacity_) {
                 deque_.pop_front();
             }
         }
-        if (active_) {
-            deque_.push_back(std::move(item));
-            consumer_.notify_one();
-        }
+        deque_.push_back(std::move(item));
+        consumer_.notify_one();
+        return true;
     }
 
-    void PushFront(const T& item) {
+    bool PushFront(const T& item) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
             while (capacity_ && deque_.size() >= capacity_) {
                 producer_.wait(lock);
+                if (!active_) {
+                    return false;
+                }
             }
         } else {
-            if (capacity_ && deque_.size() >= capacity_) {
+            while (capacity_ && deque_.size() >= capacity_) {
                 deque_.pop_back();
             }
         }
-        if (active_) {
-            deque_.push_front(item);
-            consumer_.notify_one();
-        }
+        deque_.push_front(item);
+        consumer_.notify_one();
     }
 
-    void PushFront(T&& item) {
+    bool PushFront(T&& item) {
         std::unique_lock<std::mutex> lock(mutex_);
         if (push_block_) {
             while (capacity_ && deque_.size() >= capacity_) {
                 producer_.wait(lock);
+                if (!active_) {
+                    return false;
+                }
             }
         } else {
             if (capacity_ && deque_.size() >= capacity_) {
                 deque_.pop_back();
             }
         }
-        if (active_) {
-            deque_.push_front(std::move(item));
-            consumer_.notify_one();
-        }
+        deque_.push_front(std::move(item));
+        consumer_.notify_one();
     }
 
     /**
@@ -147,17 +162,16 @@ public:
         std::unique_lock<std::mutex> lock(mutex_);
         while (deque_.empty()) {
             consumer_.wait(lock);
-        }
-        if (active_) {
-            item = deque_.front();
-            deque_.pop_front();
-            if (push_block_) {
-                producer_.notify_one();
+            if (!active_) {
+                return false;
             }
-            return true;
-        } else {
-            return false;
         }
+        item = deque_.front();
+        deque_.pop_front();
+        if (push_block_) {
+            producer_.notify_one();
+        }
+        return true;
     }
 
     /**
@@ -177,17 +191,16 @@ public:
                 std::cv_status::timeout) {
                 return false;
             }
-        }
-        if (active_) {
-            item = deque_.front();
-            deque_.pop_front();
-            if (push_block_) {
-                producer_.notify_one();
+            if (!active_) {
+                return false;
             }
-            return true;
-        } else {
-            return false;
         }
+        item = deque_.front();
+        deque_.pop_front();
+        if (push_block_) {
+            producer_.notify_one();
+        }
+        return true;
     }
 
     /**
@@ -201,17 +214,16 @@ public:
         std::unique_lock<std::mutex> lock(mutex_);
         while (deque_.empty()) {
             consumer_.wait(lock);
-        }
-        if (active_) {
-            item = deque_.back();
-            deque_.pop_back();
-            if (push_block_) {
-                producer_.notify_one();
+            if (!active_) {
+                return false;
             }
-            return true;
-        } else {
-            return false;
         }
+        item = deque_.back();
+        deque_.pop_back();
+        if (push_block_) {
+            producer_.notify_one();
+        }
+        return true;
     }
 
     /**
@@ -231,32 +243,26 @@ public:
                 std::cv_status::timeout) {
                 return false;
             }
-        }
-        if (active_) {
-            item = deque_.back();
-            deque_.pop_back();
-            if (push_block_) {
-                producer_.notify_one();
+            if (!active_) {
+                return false;
             }
-            return true;
-        } else {
-            return false;
         }
+        item = deque_.back();
+        deque_.pop_back();
+        if (push_block_) {
+            producer_.notify_one();
+        }
+        return true;
     }
 
-    T Front() {
-        std::lock_guard<std::mutex> lock(mutex_);
-        return deque_.front();
-    }
-
-    T Back() {
+    size_t Size() const {
         std::lock_guard<std::mutex> lock(mutex_);
-        return deque_.back();
+        return deque_.size();
     }
 
-    size_t Size() const {
+    size_t Capacity() {
         std::lock_guard<std::mutex> lock(mutex_);
-        return deque_.size();
+        return capacity_;
     }
 
     bool Empty() {
@@ -269,6 +275,21 @@ public:
         return (deque_.size() >= capacity_);
     }
 
+    T Front() {
+        std::lock_guard<std::mutex> lock(mutex_);
+        return deque_.front();
+    }
+
+    T Back() {
+        std::lock_guard<std::mutex> lock(mutex_);
+        return deque_.back();
+    }
+
+    void Clear() {
+        std::lock_guard<std::mutex> lock(mutex_);
+        deque_.clear();
+    }
+
 private:
     const bool push_block_;
     bool active_;
```
