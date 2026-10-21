# 5219596  fix include error

- 作者：lijin1
- 日期：2025-05-28
- 完整 hash：`52195961c3fd387ebf7c60b1edbb455a2dc1acd8`
- 改动：+1 / -0 行，涉及 1 个文件

涉及文件：
- `src/math/util.hpp`

```diff
diff --git a/src/math/util.hpp b/src/math/util.hpp
index de3100c..e2dd998 100644
--- a/src/math/util.hpp
+++ b/src/math/util.hpp
@@ -14,6 +14,7 @@
 #define TRAFFIC_FLOW_MATH_UTIL_HPP
 
 #include <algorithm>
+#include <limits>
 
 namespace fusion_perception::math {
 const double DOUBLE_EPSILON = std::numeric_limits<double>::epsilon();
```
