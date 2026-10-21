# 4f73eaa  fix Fusion Flow file open sequence bug

- 作者：fengwenxi
- 日期：2025-10-21
- 完整 hash：`4f73eaa6576995227b3b55702ec3dd985db94e95`
- 改动：+8 / -1 行，涉及 1 个文件

涉及文件：
- `tool/FusionFlow/draw_roi_page.py`

```diff
diff --git a/tool/FusionFlow/draw_roi_page.py b/tool/FusionFlow/draw_roi_page.py
index 3005fa6..1060303 100644
--- a/tool/FusionFlow/draw_roi_page.py
+++ b/tool/FusionFlow/draw_roi_page.py
@@ -6,6 +6,7 @@ import glob
 import json
 import cv2
 import numpy as np
+import re
 from PyQt6.QtCore import Qt, QSize
 from PyQt6.QtGui import QImage, QPixmap, QIcon
 from PyQt6.QtWidgets import (
@@ -539,7 +540,13 @@ class DrawRoiPage(QMainWindow):
 
         # Glob directory for images
         if isinstance(image_dir_or_files, list):
-            self.image_files = image_dir_or_files
+            # natural sort by basename with numeric awareness, case-insensitive
+            def natural_key(path):
+                name = os.path.basename(path)
+                # split into digit / non-digit parts, convert digit parts to int for numeric ordering
+                return [int(t) if t.isdigit() else t.lower() for t in re.split(r'(\d+)', name)]
+            # sort the incoming list so order is deterministic (and numbers sort naturally)
+            self.image_files = sorted(image_dir_or_files, key=natural_key)
         elif isinstance(image_dir_or_files, str):
             self.image_files = sorted(glob.glob(f"{image_dir_or_files}/*.jpg") + glob.glob(f"{image_dir_or_files}/*.png"))
         else:
```
