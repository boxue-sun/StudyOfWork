# ebbd4d7  add fisheye & solve problem with file  path in chinese

- 作者：fengwenxi
- 日期：2025-10-14
- 完整 hash：`ebbd4d7ce2bbe7b3a08e056b977195ba22cdee38`
- 改动：+245 / -332 行，涉及 3 个文件

涉及文件：
- `tool/FusionFlow/draw_roi_page.py`
- `tool/FusionFlow/pixel2wgs84.py`
- `tool/FusionFlow/traffic_flow_entry_page.py`

```diff
diff --git a/tool/FusionFlow/draw_roi_page.py b/tool/FusionFlow/draw_roi_page.py
index 6d03b32..a9f9d51 100644
--- a/tool/FusionFlow/draw_roi_page.py
+++ b/tool/FusionFlow/draw_roi_page.py
@@ -1,17 +1,18 @@
 # -*- coding: utf-8 -*-
 import os
 import time
+import struct
 import glob
 import json
 import cv2
 import numpy as np
 from PyQt6.QtCore import Qt, QSize
-from PyQt6.QtGui import QImage, QPixmap, QAction, QIcon
+from PyQt6.QtGui import QImage, QPixmap, QIcon
 from PyQt6.QtWidgets import (
-    QToolButton, QLabel, QLineEdit, QMainWindow, QWidget, QGridLayout, QPushButton, QVBoxLayout, QHBoxLayout, QTextEdit, QMenu, QFrame, QButtonGroup, QScrollArea
+    QToolButton, QLabel, QLineEdit, QMainWindow, QWidget, QGridLayout, QPushButton, QVBoxLayout, QHBoxLayout, QTextEdit, QMessageBox, QFrame, QButtonGroup, QScrollArea
 )
 
-from pixel2wgs84 import Pixel2WGS84
+from pixel2wgs84 import Pixel2WGS84, FisheyePixel2WGS84
 from util import resource_path
 
 class ImageLabel(QLabel):
@@ -20,7 +21,7 @@ class ImageLabel(QLabel):
     """
     def __init__(self, image_path, parent=None):
         super().__init__(parent)
-        self.image = cv2.imread(image_path)
+        self.image = cv2.imdecode(np.fromfile(image_path, dtype=np.uint8), cv2.IMREAD_COLOR)
         if self.image is None:
             raise FileNotFoundError(f"Cannot load image: {image_path}")
         # Load coordinate conversion data: try .json, then .bin, else error
@@ -28,27 +29,32 @@ class ImageLabel(QLabel):
         json_path = image_path.rsplit('.', 1)[0] + '.json'
         bin_path = image_path.rsplit('.', 1)[0] + '.bin'
         if os.path.exists(json_path):
-            with open(json_path, 'r') as f:
+            with open(json_path, 'r', encoding='utf-8') as f:
                 data = json.load(f)
             pmatrix_data = data["PMatrix"]
             self.coord_convertor = Pixel2WGS84(pmatrix_data)
         elif os.path.exists(bin_path):
-            with open(bin_path, 'rb') as f:
-                pmatrix_data = f.read()
+            try:
+                with open(bin_path, 'rb') as f:
+                    header = f.read(16)
+                    width, height = struct.unpack('dd', header)
+                    width = int(width)
+                    height = int(height)
+                    data = np.frombuffer(f.read(), dtype=np.float64)
+                    matrix = data.reshape(height, width, 2)
+            except Exception as e:
+                raise RuntimeError(f"Failed to read binary matrix file: {str(e)}")
             self.is_fisheye = True
-            # coord_convertor is a placeholder; use pixel2wgs84_fisheye
-            self.coord_convertor = None
+            self.coord_convertor = FisheyePixel2WGS84(matrix, width, height)
+            self.set_fisheye_roi()
+
         else:
-            raise FileNotFoundError(f"Cannot find coordinate file for image: {image_path}")
+            raise FileNotFoundError(f"Cannot find coordinate conversion matrix for image: {image_path}")
         self.height, self.width, _ = self.image.shape
 
-        self.points_group1 = []
-        self.points_group2 = []
         self.current_group = 1  # 1 or 2, indicates which group is active for adding points
         self.setMouseTracking(True)
         self.current_mouse_pos = None  # Current mouse position
-
-        self.is_full_size = False
         self.scale_factor = 0.25  # 25% scale for thumbnail, can be dynamically changed by Ctrl+Wheel
         self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
 
@@ -62,11 +68,6 @@ class ImageLabel(QLabel):
         self.setAlignment(Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignTop)
         self.setCursor(Qt.CursorShape.CrossCursor)
         self.update_display()
-    
-    def pixel2wgs84_fisheye(self, pixel_coord_list):
-        # Placeholder for fisheye camera coordinate conversion
-        # TODO: implement actual fisheye mapping
-        return [(0.0, 0.0) for _ in pixel_coord_list]
 
     def enterEvent(self, event):
         """
@@ -75,6 +76,14 @@ class ImageLabel(QLabel):
         self.setFocus()
         super().enterEvent(event)
 
+    def set_fisheye_roi(self):
+        self.roi_mask = self.coord_convertor.get_roi_mask()
+        img_bgr = self.image.copy()
+        mask_3ch = np.repeat(self.roi_mask[:, :, np.newaxis], 3, axis=2)
+        img_bgr[mask_3ch == 0] = (img_bgr[mask_3ch == 0] * 0.5).astype(np.uint8)
+        self.image = img_bgr
+        return
+
     def convert_to_qimage(self, img_bgr):
         rgb_image = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)
         return QImage(rgb_image.data, img_bgr.shape[1], img_bgr.shape[0], 3 * img_bgr.shape[1], QImage.Format.Format_RGB888)
@@ -87,14 +96,9 @@ class ImageLabel(QLabel):
         2. Draw points and lines of the current lane (highlighted solid lines).
         When only one lane remains, other lanes are not displayed, and the current lane is highlighted.
         """
-        if self.is_full_size:
-            display_img = self.image.copy()
-            disp_w = self.width
-            disp_h = self.height
-        else:
-            disp_w = int(self.width * self.scale_factor)
-            disp_h = int(self.height * self.scale_factor)
-            display_img = cv2.resize(self.image, (disp_w, disp_h), interpolation=cv2.INTER_AREA)
+        disp_w = int(self.width * self.scale_factor)
+        disp_h = int(self.height * self.scale_factor)
+        display_img = cv2.resize(self.image, (disp_w, disp_h), interpolation=cv2.INTER_AREA)
 
         lanes_valid = hasattr(self, 'lanes') and isinstance(self.lanes, list) and len(self.lanes) > 0 \
             and hasattr(self, 'current_lane_idx') and 0 <= self.current_lane_idx < len(self.lanes)
@@ -111,15 +115,15 @@ class ImageLabel(QLabel):
                 color_line2 = (0, 0, 255)
                 # Draw line1 points
                 for pt in lane.get("line1_points", []):
-                    px, py = pt[0], pt[1]
-                    disp_px = int(round(px * disp_w / self.width))
-                    disp_py = int(round(py * disp_h / self.height))
+                    pixel_coord_x, pixel_coord_y = pt[0], pt[1]
+                    disp_px = int(round(pixel_coord_x * disp_w / self.width))
+                    disp_py = int(round(pixel_coord_y * disp_h / self.height))
                     cv2.circle(overlay, (disp_px, disp_py), 5, color_line1, -1)
                 # Draw line2 points
                 for pt in lane.get("line2_points", []):
-                    px, py = pt[0], pt[1]
-                    disp_px = int(round(px * disp_w / self.width))
-                    disp_py = int(round(py * disp_h / self.height))
+                    pixel_coord_x, pixel_coord_y = pt[0], pt[1]
+                    disp_px = int(round(pixel_coord_x * disp_w / self.width))
+                    disp_py = int(round(pixel_coord_y * disp_h / self.height))
                     cv2.circle(overlay, (disp_px, disp_py), 5, color_line2, -1)
                 # Draw line1 polyline
                 line1_points = lane.get("line1_points", [])
@@ -149,11 +153,6 @@ class ImageLabel(QLabel):
             line1_points = lane.get("line1_points", [])
             line2_points = lane.get("line2_points", [])
         else:
-            # fallback legacy
-            for pt in self.points_group1:
-                points_to_draw.append((pt, 1))
-            for pt in self.points_group2:
-                points_to_draw.append((pt, 2))
             draw_lines = False
             line1_points = []
             line2_points = []
@@ -168,25 +167,23 @@ class ImageLabel(QLabel):
         # Draw points with indices
         for idx, (point, group) in enumerate(points_sorted):
             if len(point) == 6:
-                px, py, wx, wy, global_idx, _ = point
+                pixel_coord_x, pixel_coord_y, latitude, longitude, global_idx, _ = point
             elif len(point) == 5:
-                px, py, wx, wy, global_idx = point
+                pixel_coord_x, pixel_coord_y, latitude, longitude, global_idx = point
             else:
-                px, py, wx, wy = point
+                pixel_coord_x, pixel_coord_y, latitude, longitude = point
                 global_idx = None
-            disp_px = int(round(px * disp_w / self.width))
-            disp_py = int(round(py * disp_h / self.height))
+            disp_px = int(round(pixel_coord_x * disp_w / self.width))
+            disp_py = int(round(pixel_coord_y * disp_h / self.height))
             color = (255, 0, 0) if group == 1 else (0, 0, 255)
             cv2.circle(display_img, (disp_px, disp_py), 5, color, -1)
             # Highlight dragging point
             if (self.dragging and self.dragging_point_idx is not None and group == self.dragging_group):
                 if lanes_valid:
                     group_points = lane["line1_points"] if group == 1 else lane["line2_points"]
-                else:
-                    group_points = self.points_group1 if group == 1 else self.points_group2
                 if self.dragging_point_idx < len(group_points):
                     pt_drag = group_points[self.dragging_point_idx]
-                    if pt_drag[:2] == (px, py):
+                    if pt_drag[:2] == (pixel_coord_x, pixel_coord_y):
                         cv2.circle(display_img, (disp_px, disp_py), 10, (0, 255, 255), 2)
             # Use global sequential index, prefix LP/RP
             if global_idx is not None:
@@ -209,26 +206,22 @@ class ImageLabel(QLabel):
         # Display current mouse pixel coordinates
         if self.current_mouse_pos is not None:
             mx, my = self.current_mouse_pos
-            px = mx * self.width / disp_w
-            py = my * self.height / disp_h
-            show_px = round(px)
-            show_py = round(py)
+            pixel_coord_x = mx * self.width / disp_w
+            pixel_coord_y = my * self.height / disp_h
+            show_px = round(pixel_coord_x)
+            show_py = round(pixel_coord_y)
             cv2.putText(display_img, f"({show_px}, {show_py})", (10, disp_h - 10),
                         cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2)
 
         qimage = self.convert_to_qimage(display_img)
         pixmap = QPixmap.fromImage(qimage)
         self.setPixmap(pixmap)
-        if self.is_full_size:
-            self.setFixedSize(self.width, self.height)
-        else:
-            self.setFixedSize(pixmap.size())
+        self.setFixedSize(pixmap.size())
 
     def find_nearest_point(self, mx, my, group=None):
         """
         Find the nearest point to the mouse position (mx, my) within a given group.
-        Prefer searching in self.lanes[current_lane_idx] if available, otherwise fall back to
-        self.points_group1 / self.points_group2 for backward compatibility.
+        Prefer searching in self.lanes[current_lane_idx] if available.
         """
         min_dist = float('inf')
         min_idx = None
@@ -264,24 +257,6 @@ class ImageLabel(QLabel):
                 return min_idx, min_group
             return None, None
 
-        # Fallback to old storage
-        groups = []
-        if group is None:
-            groups = [(self.points_group1, 1), (self.points_group2, 2)]
-        else:
-            groups = [(self.points_group1, 1)] if group == 1 else [(self.points_group2, 2)]
-        for pts, g in groups:
-            for idx, pt in enumerate(pts):
-                pt_x, pt_y = pt[0], pt[1]
-                dist = ((pt_x - px)**2 + (pt_y - py)**2) ** 0.5
-                if dist < min_dist:
-                    min_dist = dist
-                    min_idx = idx
-                    min_group = g
-        if min_dist <= self.drag_threshold:
-            return min_idx, min_group
-        return None, None
-
     def mousePressEvent(self, event):
         self.setFocus()
         pos = event.position().toPoint()
@@ -325,10 +300,11 @@ class ImageLabel(QLabel):
 
             # Add new point to current group if no nearby point found
             pixel_coord = [pixel_x, pixel_y]
-            if getattr(self, "is_fisheye", True):
-                world_coords = self.pixel2wgs84_fisheye([pixel_coord])
-            else:
+            try:
                 world_coords = self.coord_convertor.pixel2wgs84([pixel_coord])
+            except Exception as e:
+                QMessageBox.warning(self, "鱼眼坐标转换失败", f"{str(e)}")
+                return
             lon, lat = world_coords[0]
             creation_order = time.time()
 
@@ -340,8 +316,6 @@ class ImageLabel(QLabel):
                         self.lanes.append({"lane_id": len(self.lanes)+1, "line1_points": [], "line2_points": []})
                 lane = self.lanes[lane_idx]
                 global_idx = None
-                if hasattr(top_window, 'get_next_global_index'):
-                    global_idx = top_window.get_next_global_index()
                 pt_tuple = (pixel_x, pixel_y, lat, lon, global_idx, creation_order)
                 if self.current_group == 1:
                     lane['line1_points'].append(pt_tuple)
@@ -355,15 +329,6 @@ class ImageLabel(QLabel):
                 self.update_display()
                 return
 
-            # Fallback (legacy): append into points_group lists
-            pt_tuple = (pixel_x, pixel_y, lat, lon, None, creation_order)
-            if self.current_group == 1:
-                self.points_group1.append(pt_tuple)
-            else:
-                self.points_group2.append(pt_tuple)
-            if hasattr(top_window, 'reassign_global_indices'):
-                top_window.reassign_global_indices()
-            return
         elif event.button() == Qt.MouseButton.RightButton:
             # Delete last point in current group
             top_window = self.window()
@@ -403,17 +368,6 @@ class ImageLabel(QLabel):
                     if hasattr(top_window, "reassign_global_indices"):
                         top_window.reassign_global_indices()
                 return
-            # Fallback to legacy groups
-            group_points = self.points_group1 if self.dragging_group == 1 else self.points_group2
-            if 0 <= self.dragging_point_idx < len(group_points):
-                pt = group_points[self.dragging_point_idx]
-                new_pt = (px, py) + pt[2:]
-                group_points[self.dragging_point_idx] = new_pt
-                self.update_display()
-                top_window = self.window()
-                if hasattr(top_window, "reassign_global_indices"):
-                    top_window.reassign_global_indices()
-            return
         # Update current mouse position for display
         if 0 <= mx < disp_w and 0 <= my < disp_h:
             self.current_mouse_pos = (mx, my)
@@ -452,11 +406,9 @@ class ImageLabel(QLabel):
         # Switch between groups with 1 and 2
         if event.key() == Qt.Key.Key_1:
             self.current_group = 1
-            print("Switch to group 1")
             self.update_display()
         elif event.key() == Qt.Key.Key_2:
             self.current_group = 2
-            print("Switch to group 2")
             self.update_display()
         else:
             super().keyPressEvent(event)
@@ -513,13 +465,13 @@ class DrawRoiPage(QMainWindow):
         lane1_layout.setSpacing(2)
         self.lane1_button = QPushButton("车道左边界")
         self.lane1_button.setCheckable(True)
-        self.lane1_button.clicked.connect(self.on_set_left_boundary)
+        self.lane1_button.clicked.connect(lambda:self.set_current_boundary(1))
         self.lane1_del_btn = QToolButton()
         self.lane1_del_btn.setIcon(QIcon(resource_path("material/delete.svg")))
         self.lane1_del_btn.setIconSize(QSize(16, 16))
         self.lane1_del_btn.setFixedSize(20, 20)
         self.lane1_del_btn.setStyleSheet("border: none;")
-        self.lane1_del_btn.clicked.connect(self.clear_current_line1)
+        self.lane1_del_btn.clicked.connect(lambda: self.clear_current_line(1))
         lane1_layout.addWidget(self.lane1_button)
         lane1_layout.addWidget(self.lane1_del_btn)
         lane_buttons_layout.addWidget(lane1_widget)
@@ -530,13 +482,13 @@ class DrawRoiPage(QMainWindow):
         lane2_layout.setSpacing(2)
         self.lane2_button = QPushButton("车道右边界")
         self.lane2_button.setCheckable(True)
-        self.lane2_button.clicked.connect(self.on_set_right_boundary)
+        self.lane2_button.clicked.connect(lambda:self.set_current_boundary(2))
         self.lane2_del_btn = QToolButton()
         self.lane2_del_btn.setIcon(QIcon(resource_path("material/delete.svg")))
         self.lane2_del_btn.setIconSize(QSize(16, 16))
         self.lane2_del_btn.setFixedSize(20, 20)
         self.lane2_del_btn.setStyleSheet("border: none;")
-        self.lane2_del_btn.clicked.connect(self.clear_current_line2)
+        self.lane2_del_btn.clicked.connect(lambda: self.clear_current_line(2))
         lane2_layout.addWidget(self.lane2_button)
         lane2_layout.addWidget(self.lane2_del_btn)
         lane_buttons_layout.addWidget(lane2_widget)
@@ -593,7 +545,6 @@ class DrawRoiPage(QMainWindow):
             self.image_files = []
         self.image_labels = []
         self.thumb_size = 600  # thumbnail size for layout
-        self.desired_columns = 2  # number of columns in grid layout
         self.init_image_labels()
 
         # Init lanes
@@ -641,31 +592,46 @@ class DrawRoiPage(QMainWindow):
         self.update_all_points_text()
 
         # Add buttons for lane selection
-        self.lane1_button.clicked.connect(self.on_set_left_boundary)
-        self.lane2_button.clicked.connect(self.on_set_right_boundary)
         self.btn_add_lane.clicked.connect(self.add_new_lane)
 
-        self.all_roi_points = []
-
-    def clear_current_line1(self):
-        if not self.lanes:
-            return
-        lane = self.lanes[self.current_lane_idx]
-        lane["line1_points"].clear()
-        for label in self.image_labels:
-            if hasattr(label, "lanes") and self.current_lane_idx < len(label.lanes):
-                label.lanes[self.current_lane_idx]["line1_points"].clear()
-                label.update_display()
-        self.update_all_points_text()
+        # Return to entry page button
+        self.return_entry_btn = QPushButton("返回进口道选择")
+        main_layout.addWidget(self.return_entry_btn)
+        self.return_entry_btn.setStyleSheet("""
+            QPushButton {
+                background-color: #0078d7;
+                color: white;
+                border-radius: 6px;
+                padding: 8px 16px;
+                font-size: 15px;
+                font-weight: bold;
+            }
+            QPushButton:hover {
+                background-color: #1495ff;
+            }
+            QPushButton:pressed {
+                background-color: #005a9e;
+            }
+        """)
+        self.return_entry_btn.clicked.connect(self.return_to_entry_page)
+
+
+    def return_to_entry_page(self):
+        if hasattr(self, "entry_window_ref") and self.entry_window_ref is not None:
+            self.hide()
+            self.entry_window_ref.show()
+            self.entry_window_ref.raise_()
+            self.entry_window_ref.activateWindow()
 
-    def clear_current_line2(self):
+    def clear_current_line(self, group):
         if not self.lanes:
             return
         lane = self.lanes[self.current_lane_idx]
-        lane["line2_points"].clear()
+        key = "line1_points" if group == 1 else "line2_points"
+        lane[key].clear()
         for label in self.image_labels:
             if hasattr(label, "lanes") and self.current_lane_idx < len(label.lanes):
-                label.lanes[self.current_lane_idx]["line2_points"].clear()
+                label.lanes[self.current_lane_idx][key].clear()
                 label.update_display()
         self.update_all_points_text()
 
@@ -677,7 +643,7 @@ class DrawRoiPage(QMainWindow):
     def reassign_global_indices(self):
         """
         reassign global sequential indices to left/right boundary points across all images for each lane.
-        with the format: (px, py, wx, wy, idx, order)
+        with the format: (px, py, latitude, longitude, idx, order)
         """
         num_lanes = len(getattr(self, "lanes", []))
         if num_lanes == 0:
@@ -685,39 +651,22 @@ class DrawRoiPage(QMainWindow):
 
         # Deal with each lane separately
         for lane_idx in range(num_lanes):
-            # Collect all (label, idx_in_label, pt, order) for line1 and line2
-            left_entries = []
-            right_entries = []
-            for label in getattr(self, "image_labels", []):
-                if not hasattr(label, "lanes") or lane_idx >= len(label.lanes):
-                    continue
-                line1 = label.lanes[lane_idx].get("line1_points", [])
-                for i, pt in enumerate(line1):
-                    order = pt[5] if len(pt) >= 6 else float("inf")
-                    left_entries.append({"label": label, "idx_in_label": i, "pt": pt, "order": order})
-                line2 = label.lanes[lane_idx].get("line2_points", [])
-                for i, pt in enumerate(line2):
-                    order = pt[5] if len(pt) >= 6 else float("inf")
-                    right_entries.append({"label": label, "idx_in_label": i, "pt": pt, "order": order})
-
-            # Reassign global indices according to creation_order
-            left_entries_sorted = sorted(left_entries, key=lambda x: x["order"])
-            for new_idx, entry in enumerate(left_entries_sorted, start=1):
-                label = entry["label"]
-                i = entry["idx_in_label"]
-                pt = entry["pt"]
-                px, py, wx, wy = pt[0], pt[1], pt[2], pt[3]
-                order = entry["order"]
-                label.lanes[lane_idx]["line1_points"][i] = (px, py, wx, wy, new_idx, order)
-
-            right_entries_sorted = sorted(right_entries, key=lambda x: x["order"])
-            for new_idx, entry in enumerate(right_entries_sorted, start=1):
-                label = entry["label"]
-                i = entry["idx_in_label"]
-                pt = entry["pt"]
-                px, py, wx, wy = pt[0], pt[1], pt[2], pt[3]
-                order = entry["order"]
-                label.lanes[lane_idx]["line2_points"][i] = (px, py, wx, wy, new_idx, order)
+            for key in ["line1_points", "line2_points"]:
+                entries = []
+                for label in getattr(self, "image_labels", []):
+                    if not hasattr(label, "lanes") or lane_idx >= len(label.lanes):
+                        continue
+                    for i, pt in enumerate(label.lanes[lane_idx].get(key, [])):
+                        order = pt[5] if len(pt) >= 6 else float("inf")
+                        entries.append({"label": label, "idx_in_label": i, "pt": pt, "order": order})
+                entries_sorted = sorted(entries, key=lambda x: x["order"])
+                for new_idx, entry in enumerate(entries_sorted, start=1):
+                    label = entry["label"]
+                    i = entry["idx_in_label"]
+                    pt = entry["pt"]
+                    pixel_coord_x, pixel_coord_y, latitude, longitude = pt[0], pt[1], pt[2], pt[3]
+                    order = entry["order"]
+                    label.lanes[lane_idx][key][i] = (pixel_coord_x, pixel_coord_y, latitude, longitude, new_idx, order)
 
         # Update displays
         for label in getattr(self, "image_labels", []):
@@ -725,47 +674,24 @@ class DrawRoiPage(QMainWindow):
                 label.update_display()
         self.update_all_points_text()
 
-    def get_next_global_index(self):
-        if not hasattr(self, "global_counter"):
-            self.global_counter = 1
-        idx = self.global_counter
-        self.global_counter += 1
-        return idx
-
     def update_current_lane_field(self, field_name, value):
         """ save the input box value to the current lane's data dictionary """
         if 0 <= self.current_lane_idx < len(self.lanes):
             self.lanes[self.current_lane_idx][field_name] = value
 
-    def on_set_left_boundary(self):
-        """ 
-        Switch all images' current editing line group to lane left boundary.
-        """
+    def set_current_boundary(self, group):
         if not self.lanes:
             print("No lanes to switch")
             return
-        self.ensure_lane_exists()
-        self.current_group = 1
-        for label in self.image_labels:
-            label.current_lane_idx = self.current_lane_idx
-            label.current_line_group = 1
-            label.current_group = 1
-            label.update_display()
-        print(f"Switch to lane {self.current_lane_idx+1} lane left boundary")
-
-    def on_set_right_boundary(self):
-        """ Switch all images' current editing line group to lane right boundary """
-        if not self.lanes:
-            print("No lanes to switch")
-            return
-        self.ensure_lane_exists()
-        self.current_group = 2
+        if self.current_lane_idx >= len(self.lanes):
+            self.add_new_lane()
+        self.current_group = group
         for label in self.image_labels:
             label.current_lane_idx = self.current_lane_idx
-            label.current_line_group = 2
-            label.current_group = 2
+            label.current_line_group = group
+            label.current_group = group
             label.update_display()
-        print(f"Switch to lane {self.current_lane_idx+1} lane right boundary")
+        print(f"Switch to lane {self.current_lane_idx+1} {'left' if group==1 else 'right'} boundary")
 
     def add_new_lane(self):
         """ Add a new lane and switch to it, also inherit the current boundary selection (left/right) and synchronize it to all labels. """
@@ -961,10 +887,6 @@ class DrawRoiPage(QMainWindow):
         if hasattr(self, "update_all_points_text"):
             self.update_all_points_text()
 
-    def ensure_lane_exists(self):
-        if self.current_lane_idx >= len(self.lanes):
-            self.add_new_lane()
-
     def init_image_labels(self):
         # Clear existing labels
         for i in reversed(range(self.grid_layout.count())):
@@ -973,21 +895,17 @@ class DrawRoiPage(QMainWindow):
                 widget.setParent(None)
         self.image_labels = []
         for image_path in self.image_files:
-            try:
-                img_label = ImageLabel(image_path, parent=self)
-                img_label.setStyleSheet("border: none; margin: 0px; padding: 0px;")
-                # Set thumbnail size, will be dynamically adjusted later by update_thumb_size
-                orig_h, orig_w = img_label.image.shape[:2]
-                scale = min(self.thumb_size / orig_w, self.thumb_size / orig_h)
-                img_label.scale_factor = scale
-                # Support multi lanes
-                img_label.lanes = []
-                img_label.current_lane_idx = 0
-                img_label.current_line_group = 1
-                self.image_labels.append(img_label)
-            except Exception as e:
-                msg = QLabel(f"图片加载失败: {image_path}\n{e}")
-                self.image_labels.append(msg)
+            img_label = ImageLabel(image_path, parent=self)
+            img_label.setStyleSheet("border: none; margin: 0px; padding: 0px;")
+            # Set thumbnail size, will be dynamically adjusted later by update_thumb_size
+            orig_h, orig_w = img_label.image.shape[:2]
+            scale = min(self.thumb_size / orig_w, self.thumb_size / orig_h)
+            img_label.scale_factor = scale
+            # Support multi lanes
+            img_label.lanes = []
+            img_label.current_lane_idx = 0
+            img_label.current_line_group = 1
+            self.image_labels.append(img_label)
         self.arrange_grid()
 
     def arrange_grid(self):
@@ -998,7 +916,7 @@ class DrawRoiPage(QMainWindow):
             if widget:
                 widget.setParent(None)
         # Re-add widgets in grid layout
-        n_col = self.desired_columns
+        n_col = 2
         row, col = 0, 0
         for label in self.image_labels:
             self.grid_layout.addWidget(label, row, col)
@@ -1074,86 +992,39 @@ class DrawRoiPage(QMainWindow):
         else:
             removed = label.lanes[lane_idx]["line2_points"].pop(idx_in_label)
 
-        px, py, wx, wy = removed[0], removed[1], removed[2], removed[3]
+        pixel_coord_x, pixel_coord_y, latitude, longitude = removed[0], removed[1], removed[2], removed[3]
         removed_idx = removed[4] if len(removed) >= 5 else None
-        print(f"Removed lane {lane_idx+1} group {group} last indexed point: Pixel ({px}, {py}) → Coord ({wx:.3f}, {wy:.3f}), idx={removed_idx}")
+        print(f"Removed lane {lane_idx+1} group {group} last indexed point: Pixel ({pixel_coord_x}, {pixel_coord_y}) → Coord ({latitude:.3f}, {longitude:.3f}), idx={removed_idx}")
 
         self.reassign_global_indices()
 
     def keyPressEvent(self, event):
         super().keyPressEvent(event)
 
-    def update_all_points_text(self):
-        if not hasattr(self, "points_text1") or not hasattr(self, "points_text2"):
-            return
-
-        lines1 = []
-        lines2 = []
-        latest1 = []
-        latest2 = []
-
-        lane_idx = getattr(self, "current_lane_idx", -1)
-        if lane_idx < 0:
-            # No lanes exist, clear text and return
-            self.points_text1.setPlainText("尚未添加车道左边界点")
-            self.points_text2.setPlainText("尚未添加车道右边界点")
-            self._latest_group_points = {1: [], 2: []}
-            return
-
-        # Collect left boundary points across all images for current lane
-        left_entries = []
+    def collect_points_text(self, group_key, prefix, lane_idx):
+        entries = []
         for label_idx, label in enumerate(self.image_labels):
             if not hasattr(label, "lanes") or lane_idx >= len(label.lanes) or len(label.lanes) == 0:
                 continue
             lane = label.lanes[lane_idx]
-            for idx_in_label, pt in enumerate(lane.get("line1_points", [])):
+            for idx_in_label, pt in enumerate(lane.get(group_key, [])):
                 order = pt[5] if len(pt) >= 6 else float("inf")
-                left_entries.append({
+                entries.append({
                     "label": label,
                     "label_idx": label_idx,
                     "pt": pt,
                     "order": order,
                     "idx_in_label": idx_in_label,
                 })
-        left_entries_sorted = sorted(left_entries, key=lambda x: x["order"])
-        for disp_idx, entry in enumerate(left_entries_sorted, start=1):
+        entries_sorted = sorted(entries, key=lambda x: x["order"])
+        lines = []
+        latest = []
+        for disp_idx, entry in enumerate(entries_sorted, start=1):
             pt = entry["pt"]
-            label_idx = entry["label_idx"]
             px, py = pt[0], pt[1]
-            lines1.append(f"车道{lane_idx+1} LP{disp_idx}: ({px}, {py}) [图片{label_idx+1}]")
-            latest1.append({
-                "label_idx": label_idx,
-                "img_label": entry["label"],
-                "pixel_x": px,
-                "pixel_y": py,
-                "global_idx": disp_idx,
-                "creation_order": entry["order"],
-                "lane_idx": lane_idx,
-                "idx_in_label": entry["idx_in_label"],
-            })
-
-        # Collect right boundary points across all images for current lane
-        right_entries = []
-        for label_idx, label in enumerate(self.image_labels):
-            if not hasattr(label, "lanes") or lane_idx >= len(label.lanes) or len(label.lanes) == 0:
-                continue
-            lane = label.lanes[lane_idx]
-            for idx_in_label, pt in enumerate(lane.get("line2_points", [])):
-                order = pt[5] if len(pt) >= 6 else float("inf")
-                right_entries.append({
-                    "label": label,
-                    "label_idx": label_idx,
-                    "pt": pt,
-                    "order": order,
-                    "idx_in_label": idx_in_label,
-                })
-        right_entries_sorted = sorted(right_entries, key=lambda x: x["order"])
-        for disp_idx, entry in enumerate(right_entries_sorted, start=1):
-            pt = entry["pt"]
             label_idx = entry["label_idx"]
-            px, py = pt[0], pt[1]
-            lines2.append(f"车道{lane_idx+1} RP{disp_idx}: ({px}, {py}) [图片{label_idx+1}]")
-            latest2.append({
+            lines.append(f"车道{lane_idx+1} {prefix}{disp_idx}: ({px}, {py}) [图片{label_idx+1}]")
+            latest.append({
                 "label_idx": label_idx,
                 "img_label": entry["label"],
                 "pixel_x": px,
@@ -1163,12 +1034,25 @@ class DrawRoiPage(QMainWindow):
                 "lane_idx": lane_idx,
                 "idx_in_label": entry["idx_in_label"],
             })
+        return lines, latest
+
+    def update_all_points_text(self):
+        if not hasattr(self, "points_text1") or not hasattr(self, "points_text2"):
+            return
+
+        lane_idx = getattr(self, "current_lane_idx", -1)
+        if lane_idx < 0:
+            # No lanes exist, clear text and return
+            self.points_text1.setPlainText("尚未添加车道左边界点")
+            self.points_text2.setPlainText("尚未添加车道右边界点")
+            self._latest_group_points = {1: [], 2: []}
+            return
+
+        lines1, latest1 = self.collect_points_text("line1_points", "LP", lane_idx)
+        lines2, latest2 = self.collect_points_text("line2_points", "RP", lane_idx)
 
-        # Update text widgets
         self.points_text1.setPlainText("尚未添加车道左边界点" if not lines1 else '\n'.join(lines1))
         self.points_text2.setPlainText("尚未添加车道右边界点" if not lines2 else '\n'.join(lines2))
-
-        # Save latest point info for context menu delete/locate
         self._latest_group_points = {1: latest1, 2: latest2}
 
     def showEvent(self, event):
@@ -1176,29 +1060,6 @@ class DrawRoiPage(QMainWindow):
         self.update_all_points_text()
         return super().showEvent(event)
 
-    def on_text1_context_menu(self, pos):
-        self.show_context_menu_for_group(self.points_text1, group=1, pos=pos)
-
-    def on_text2_context_menu(self, pos):
-        self.show_context_menu_for_group(self.points_text2, group=2, pos=pos)
-
-    def show_context_menu_for_group(self, text_widget, group, pos):
-        # Get the clicked line number
-        cursor = text_widget.cursorForPosition(pos)
-        line_num = cursor.blockNumber()
-        # Get the corresponding point info
-        group_pts = self._latest_group_points.get(group, [])
-        if line_num < 0 or line_num >= len(group_pts):
-            return
-        pt = group_pts[line_num]
-        menu = QMenu(text_widget)
-        action_del = QAction("删除该点", menu)
-        def do_delete():
-            self.delete_point_by_info(pt, group)
-        action_del.triggered.connect(do_delete)
-        menu.addAction(action_del)
-        menu.exec(text_widget.mapToGlobal(pos))
-
     def delete_point_by_info(self, ptinfo, group):
         """ Delete point by info, including updating the text and image labels"""
         img_label = ptinfo["img_label"]
@@ -1212,12 +1073,6 @@ class DrawRoiPage(QMainWindow):
             idx_to_del = next((i for i, pt in enumerate(pts) if (pt[0], pt[1]) == (px, py)), None)
             if idx_to_del is not None:
                 del pts[idx_to_del]
-        else:
-            # Fallback: search all lanes in this image label
-            group_list = img_label.points_group1 if group == 1 else img_label.points_group2
-            idx_to_del = next((i for i, pt in enumerate(group_list) if (pt[0], pt[1]) == (px, py)), None)
-            if idx_to_del is not None:
-                del group_list[idx_to_del]
 
         self.reassign_global_indices()
         # Refresh the latest points info
@@ -1225,10 +1080,3 @@ class DrawRoiPage(QMainWindow):
             if hasattr(label, "update_display"):
                 label.update_display()
         self.update_all_points_text()
-
-    # Bind context menu to points text widgets
-    def bind_context_menu(self):
-        self.points_text1.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
-        self.points_text2.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
-        self.points_text1.customContextMenuRequested.connect(self.on_text1_context_menu)
-        self.points_text2.customContextMenuRequested.connect(self.on_text2_context_menu)
diff --git a/tool/FusionFlow/pixel2wgs84.py b/tool/FusionFlow/pixel2wgs84.py
index fbedeab..a9e6f13 100644
--- a/tool/FusionFlow/pixel2wgs84.py
+++ b/tool/FusionFlow/pixel2wgs84.py
@@ -1,5 +1,6 @@
 # -*- coding: utf-8 -*-
 import math
+import struct
 import numpy as np
 import cv2
 
@@ -284,9 +285,6 @@ class UTM:
         # Convert to degrees
         return (UTM.rad_to_deg(lat), UTM.rad_to_deg(lon))
     
-
-
-
 class Pixel2WGS84:
     def __init__(self, matrix):
         self.zone = matrix[0]
@@ -304,23 +302,85 @@ class Pixel2WGS84:
             wgs84_points.append([lon, lat])
         return wgs84_points
     
+class FisheyePixel2WGS84:
+    def __init__(self, matrix, width, height):
+        self.matrix = matrix
+        self.width = width
+        self.height = height
+        return
+    
+    def pixel2wgs84(self, pixels):
+        pixel_points = np.array(pixels, dtype=np.float64)
+        pixel_points = pixel_points.reshape(-1, 1, 2)
+        for one_point in pixel_points:
+            x, y = int(one_point[0, 0]), int(one_point[0, 1])
+            wgs84_points = []
+            # Validation check & convert
+            if 0 <= x < self.width and 0 <= y < self.height:
+                lat, lon = self.matrix[y, x, 0], self.matrix[y, x, 1]
+                if lat == 361 or lon == 361:
+                    raise ValueError(f"Invalid geographic coordinate at pixel: {x} {y}")
+                wgs84_points.append([lon, lat])
+            else:
+                raise ValueError(f"Image out of range: {x} {y} {self.width}x{self.height}")
+        return wgs84_points
+    
+    def get_roi_mask(self):
+        """
+        Generate a binary mask based on the fisheye mapping matrix.
+        Points where lat or lon == 361 (invalid) are set to 0, others to 1.
+        """
+        lat = self.matrix[:, :, 0]
+        lon = self.matrix[:, :, 1]
+        mask = np.ones((self.height, self.width), dtype=np.uint8)
+        mask[(lat == 361) | (lon == 361)] = 0
+        return mask
+
+def read_fisheye_matrix(bin_file):
+    """
+    Read fisheye to WGS84 mapping matrix from a binary file.
+    Return: width, height, geo_matrix(shape: [height, width, 2])
+    """
+    try:
+        with open(bin_file, 'rb') as f:
+            header = f.read(16)
+            width, height = struct.unpack('dd', header)
+
+            data = np.frombuffer(f.read(), dtype=np.float64)
+            matrix = data.reshape(int(height), int(width), 2)
+
+            return int(width), int(height), matrix
+    except struct.error:
+        raise ValueError("Binary file format error, please check file integrity")
+    except Exception as e:
+        raise RuntimeError(f"Failed to read binary file with error:{str(e)}")
 
 if __name__ == '__main__':
-    matrix = [
-            50,
-            0,
-            -299.2879882973968,
-            -10662.85374635621,
-            458398.00457307196,
-            -2873.0746739852066,
-            -102388.15888196803,
-            4403572.31024974,
-            -0.0006524323536258867,
-            -0.023249806603792653,
-            1.0
-        ]
-    transform_test = Pixel2WGS84(matrix)
-    pixels = [[600.0,500.0],[520,400], [710,300]]
-    points = transform_test.pixel2wgs84(pixels)
-    print(points)
+    # matrix = [
+    #         50,
+    #         0,
+    #         -299.2879882973968,
+    #         -10662.85374635621,
+    #         458398.00457307196,
+    #         -2873.0746739852066,
+    #         -102388.15888196803,
+    #         4403572.31024974,
+    #         -0.0006524323536258867,
+    #         -0.023249806603792653,
+    #         1.0
+    #     ]
+    # transform_test = Pixel2WGS84(matrix)
+    # pixels = [[600.0,500.0],[520,400], [710,300]]
+    # points = transform_test.pixel2wgs84(pixels)
+    # print(points)
     #output format:[[116.51704541630275, 39.78354513978091], [116.51711474955962, 39.78359659059658], [116.51725782153622, 39.78363187654715]]
+    
+    bin_path = "/Users/skywalker/Work/CICTCI/TrafficFlow/traffic_flow/tool/FusionFlow/test_data/SY-241/SY-241-45.bin"
+    width, height, matrix = read_fisheye_matrix(bin_path)
+    print(width, height, matrix.shape)
+    fisheye_calibrator = FisheyePixel2WGS84(matrix, width, height)
+    roi = fisheye_calibrator.get_roi_mask()
+    pixels = [[600.0,500.0],[600.0,500.0], [600.0,500.0]]
+    points = fisheye_calibrator.pixel2wgs84(pixels)
+
+    print(points)
\ No newline at end of file
diff --git a/tool/FusionFlow/traffic_flow_entry_page.py b/tool/FusionFlow/traffic_flow_entry_page.py
index 626a5f6..dd859d7 100644
--- a/tool/FusionFlow/traffic_flow_entry_page.py
+++ b/tool/FusionFlow/traffic_flow_entry_page.py
@@ -66,8 +66,8 @@ class TrafficFlowEntryPage(QMainWindow):
             "lanes": [
                 {
                     "contour": [
-                        [ [wx, wy], [wx, wy], ... ],   # left boundary
-                        [ [wx, wy], [wx, wy], ... ]    # right boundary
+                        [ [longitude, latitude], [longitude, latitude], ... ],   # left boundary
+                        [ [longitude, latitude], [longitude, latitude], ... ]    # right boundary
                     ],
                     "LaneNo": {
                         "inter_id": "...",
@@ -101,13 +101,13 @@ class TrafficFlowEntryPage(QMainWindow):
                         # left boundary points
                         for pt in label.lanes[lane_idx].get("line1_points", []):
                             if len(pt) >= 4:
-                                wx, wy = pt[2], pt[3]
-                                left_points.append([wx, wy])
+                                latitude, longitude = pt[2], pt[3]
+                                left_points.append([latitude, longitude])
                         # right boundary points
                         for pt in label.lanes[lane_idx].get("line2_points", []):
                             if len(pt) >= 4:
-                                wx, wy = pt[2], pt[3]
-                                right_points.append([wx, wy])
+                                latitude, longitude = pt[2], pt[3]
+                                right_points.append([latitude, longitude])
 
                     # use per-lane metadata if available
                     lane_info = {}
@@ -173,7 +173,12 @@ class TrafficFlowEntryPage(QMainWindow):
         vbox.addLayout(main_hbox)
         delete_btn.setVisible(True)
 
-        draw_roi_page = DrawRoiPage(files)
+        try:
+            draw_roi_page = DrawRoiPage(files)
+        except Exception as e:
+            print(f"Error loading image {files}: {e}")
+            QMessageBox.warning(self, "图片加载失败", f"图片加载失败: {files}\n{e}")
+            return
         draw_roi_page.entry_window_ref = self
         # if pre_saved_points is given, set them in draw_roi_page
         if pre_saved_points is not None:
```
