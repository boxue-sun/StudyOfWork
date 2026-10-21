# 75f6228  FusionFlow:fix export json bug& init FusionFlowFluent

- 作者：fengwenxi
- 日期：2025-10-23
- 完整 hash：`75f62289eacdc152b0d1f1c26eadd25b2e918e6a`
- 改动：+3897 / -1 行，涉及 8 个文件

涉及文件：
- `tool/FusionFlow/traffic_flow_entry_page.py`
- `tool/FusionFlowFluent/app/common/config.py`
- `tool/FusionFlowFluent/app/resource/i18n/gallery.zh_CN.qm`
- `tool/FusionFlowFluent/app/resource/i18n/gallery.zh_CN.ts`
- `tool/FusionFlowFluent/app/resource/i18n/gallery.zh_HK.qm`
- `tool/FusionFlowFluent/app/resource/i18n/gallery.zh_HK.ts`
- `tool/FusionFlowFluent/app/view/main_window.py`
- `tool/FusionFlowFluent/fusion_flow.py`

```diff
diff --git a/tool/FusionFlow/traffic_flow_entry_page.py b/tool/FusionFlow/traffic_flow_entry_page.py
index 0f68c0b..b95a141 100644
--- a/tool/FusionFlow/traffic_flow_entry_page.py
+++ b/tool/FusionFlow/traffic_flow_entry_page.py
@@ -116,7 +116,7 @@ class TrafficFlowEntryPage(QMainWindow):
                     fdir = entrance_obj["f_dir_8_no"]
                     lane_dict = {
                         "pixel_contour": {"left": left_pixel, "right": right_pixel},
-                        "contour": {"left": left_geo, "right": right_geo},
+                        "contour": [left_geo, right_geo],
                         "LaneNo": {
                             "inter_id": inter_id_value,
                             "upstream_cross_id": entrance_obj["upstream_cross_id"],
@@ -131,6 +131,22 @@ class TrafficFlowEntryPage(QMainWindow):
             entrances.append(entrance_obj)
             entrance_id_counter += 1
 
+        # Check before export: ensure all required fields are filled
+        for lane in lanes_list:
+            lane_info = lane.get("LaneNo", {})
+            inter_id = lane_info.get("inter_id", "")
+            fdir = lane_info.get("f_dir_8_no", "")
+            sdtype = lane_info.get("lane_sdtype_no", "")
+            laneid = lane_info.get("LaneID", "")
+            turndir = lane_info.get("turn_dir_no", "")
+
+            if not inter_id or fdir in ("", -1, None) or sdtype in ("", -1, None) or not laneid or not turndir:
+                QMessageBox.warning(
+                    self,
+                    "字段缺失",
+                    "存在车道信息字段未填写完整(inter_id、f_dir_8_no、lane_sdtype_no、LaneID、turn_dir_no),请检查后再导出。"
+                )
+            
         output = {"lanes": lanes_list, "entrances": entrances}
 
         if not lanes_list:
diff --git a/tool/FusionFlowFluent/app/common/config.py b/tool/FusionFlowFluent/app/common/config.py
new file mode 100644
index 0000000..6765ef2
--- /dev/null
+++ b/tool/FusionFlowFluent/app/common/config.py
@@ -0,0 +1,65 @@
+# coding:utf-8
+import sys
+from enum import Enum
+
+from PySide6.QtCore import QLocale
+from qfluentwidgets import (qconfig, QConfig, ConfigItem, OptionsConfigItem, BoolValidator,
+                            OptionsValidator, RangeConfigItem, RangeValidator,
+                            FolderListValidator, Theme, FolderValidator, ConfigSerializer, __version__)
+
+
+class Language(Enum):
+    """ Language enumeration """
+
+    CHINESE_SIMPLIFIED = QLocale(QLocale.Chinese, QLocale.China)
+    CHINESE_TRADITIONAL = QLocale(QLocale.Chinese, QLocale.HongKong)
+    ENGLISH = QLocale(QLocale.English)
+    AUTO = QLocale()
+
+
+class LanguageSerializer(ConfigSerializer):
+    """ Language serializer """
+
+    def serialize(self, language):
+        return language.value.name() if language != Language.AUTO else "Auto"
+
+    def deserialize(self, value: str):
+        return Language(QLocale(value)) if value != "Auto" else Language.AUTO
+
+
+def is_win11():
+    return sys.platform == 'win32' and sys.getwindowsversion().build >= 22000
+
+
+class Config(QConfig):
+    """ Config of application """
+
+    # folders
+    workspace = ConfigItem(
+        "Folders", "Workspace", [], FolderListValidator())
+
+    # main window
+    mica_enabled = ConfigItem("MainWindow", "MicaEnabled", is_win11(), BoolValidator())
+    dpi_scale = OptionsConfigItem(
+        "MainWindow", "DpiScale", "Auto", OptionsValidator([1, 1.25, 1.5, 1.75, 2, "Auto"]), restart=True)
+    language = OptionsConfigItem(
+        "MainWindow", "Language", Language.AUTO, OptionsValidator(Language), LanguageSerializer(), restart=True)
+
+    # Material
+    blur_radius  = RangeConfigItem("Material", "AcrylicBlurRadius", 15, RangeValidator(0, 40))
+
+    # software update
+    check_update_at_start_up = ConfigItem("Update", "CheckUpdateAtStartUp", False, BoolValidator())
+
+
+YEAR = 2025
+AUTHOR = "Feng Wenxi"
+VERSION = __version__
+OFFICIAL_URL = "https://www.cictci.com"
+REPO_URL = "http://10.255.33.42/fengwenxi/traffic_flow"
+# ZH_SUPPORT_URL = "https://qfluentwidgets.com/zh/price/"
+
+
+cfg = Config()
+cfg.themeMode.value = Theme.AUTO
+qconfig.load('app/config/config.json', cfg)
\ No newline at end of file
diff --git a/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_CN.qm b/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_CN.qm
new file mode 100644
index 0000000..dd79e99
Binary files /dev/null and b/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_CN.qm differ
diff --git a/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_CN.ts b/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_CN.ts
new file mode 100644
index 0000000..c8528af
--- /dev/null
+++ b/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_CN.ts
@@ -0,0 +1,1811 @@
+<?xml version="1.0" encoding="utf-8"?>
+<!DOCTYPE TS>
+<TS version="2.1" language="zh_CN" sourcelanguage="en_US">
+<context>
+    <name>BasicInputInterface</name>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="29"/>
+        <source>A simple button with text content</source>
+        <translation>带有文本的简单按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="39"/>
+        <source>A button with graphical content</source>
+        <translation>带有图标的按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="186"/>
+        <source>A hyperlink button that navigates to a URI</source>
+        <translation>导航到一个超链接的按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="88"/>
+        <source>A 2-state CheckBox</source>
+        <translation>双态复选框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="97"/>
+        <source>A 3-state CheckBox</source>
+        <translation>三态复选框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="108"/>
+        <source>A ComboBox with items</source>
+        <translation>下拉框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="209"/>
+        <source>A group of RadioButton controls in a button group</source>
+        <translation>单选按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="220"/>
+        <source>A simple horizontal slider</source>
+        <translation>水平滑动条</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="271"/>
+        <source>A simple switch button</source>
+        <translation>开关按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="310"/>
+        <source>Off</source>
+        <translation>关</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="308"/>
+        <source>On</source>
+        <translation>开</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="29"/>
+        <source>Standard push button</source>
+        <translation>标准按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="46"/>
+        <source>Accent style button</source>
+        <translation>主题色按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="88"/>
+        <source>Two-state CheckBox</source>
+        <translation>双态复选框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="95"/>
+        <source>Three-state CheckBox</source>
+        <translation>三态复选框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="314"/>
+        <source>Star Platinum</source>
+        <translation>白金之星</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="314"/>
+        <source>Crazy Diamond</source>
+        <translation>疯狂钻石</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="200"/>
+        <source>Soft and Wet</source>
+        <translation>软又湿</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="314"/>
+        <source>Gold Experience</source>
+        <translation>黄金体验</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="314"/>
+        <source>Sticky Fingers</source>
+        <translation>钢链手指</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="252"/>
+        <source>Choose your stand</source>
+        <translation>选择你的替身</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="124"/>
+        <source>An editable ComboBox</source>
+        <translation>可编辑的下拉框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="132"/>
+        <source>Send</source>
+        <translation>发送</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="133"/>
+        <source>Save</source>
+        <translation>保存</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="168"/>
+        <source>Email</source>
+        <translation>邮件</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="136"/>
+        <source>A push button with drop down menu</source>
+        <translation>带下拉菜单的按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="144"/>
+        <source>A tool button with drop down menu</source>
+        <translation>带下拉菜单的工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="292"/>
+        <source>Start practicing</source>
+        <translation>开始练习</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="229"/>
+        <source>A split push button with drop down menu</source>
+        <translation>带下拉菜单的拆分按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="236"/>
+        <source>Sing</source>
+        <translation>唱</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="236"/>
+        <source>Jump</source>
+        <translation>跳</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="236"/>
+        <source>Rap</source>
+        <translation>Rap</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="236"/>
+        <source>Music</source>
+        <translation>Music</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="245"/>
+        <source>A split tool button with drop down menu</source>
+        <translation>带下拉菜单的工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="46"/>
+        <source>Accent style applied to push button</source>
+        <translation>主题色按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="53"/>
+        <source>Accent style applied to tool button</source>
+        <translation>主题色工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="153"/>
+        <source>A primary color push button with drop down menu</source>
+        <translation>带下拉菜单的主题色按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="161"/>
+        <source>A primary color tool button with drop down menu</source>
+        <translation>带下拉菜单的主题色工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="254"/>
+        <source>A primary color split push button with drop down menu</source>
+        <translation>带下拉菜单的主题色拆分按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="262"/>
+        <source>A primary color split tool button with drop down menu</source>
+        <translation>带下拉菜单的主题色拆分工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="74"/>
+        <source>A transparent push button</source>
+        <translation>透明按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="81"/>
+        <source>A transparent tool button</source>
+        <translation>透明工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="170"/>
+        <source>A transparent push button with drop down menu</source>
+        <translation>带下拉菜单的透明按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="179"/>
+        <source>A transparent tool button with drop down menu</source>
+        <translation>带下拉菜单的透明工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="278"/>
+        <source>A simple toggle push button</source>
+        <translation>状态开关按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="285"/>
+        <source>A simple toggle tool button</source>
+        <translation>状态开关工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="292"/>
+        <source>A transparent toggle push button</source>
+        <translation>透明的状态开关按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="299"/>
+        <source>A transparent toggle tool button</source>
+        <translation>透明的状态开关工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="74"/>
+        <source>Transparent push button</source>
+        <translation>透明按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="60"/>
+        <source>Pill push button</source>
+        <translation>椭圆按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="60"/>
+        <source>Tag</source>
+        <translation>标签</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="67"/>
+        <source>Pill tool button</source>
+        <translation>椭圆工具按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="186"/>
+        <source>QFluentWidgets official website</source>
+        <translation></translation>
+    </message>
+</context>
+<context>
+    <name>CustomMessageBox</name>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="182"/>
+        <source>Open URL</source>
+        <translation>打开 URL</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="185"/>
+        <source>Enter the URL of a file, stream, or playlist</source>
+        <translation>输入文件、流或者播放列表的 URL</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="193"/>
+        <source>Open</source>
+        <translation>打开</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="194"/>
+        <source>Cancel</source>
+        <translation>取消</translation>
+    </message>
+</context>
+<context>
+    <name>DateTimeInterface</name>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="37"/>
+        <source>A simple DatePicker</source>
+        <translation>日期选择器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="37"/>
+        <source>A fast CalendarPicker</source>
+        <translation>快速日历选择器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="43"/>
+        <source>A DatePicker in another format</source>
+        <translation>另一种格式的日期选择器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="50"/>
+        <source>A simple TimePicker</source>
+        <translation>时间选择器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="57"/>
+        <source>A TimePicker using a 24-hour clock</source>
+        <translation>24 小时制的时间选择器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="64"/>
+        <source>A TimePicker with seconds column</source>
+        <translation>显示秒的时间选择器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="22"/>
+        <source>A simple CalendarPicker</source>
+        <translation>日历选择器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="30"/>
+        <source>A CalendarPicker in another format</source>
+        <translation>自定义格式的日历选择器</translation>
+    </message>
+</context>
+<context>
+    <name>DialogInterface</name>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="47"/>
+        <source>Show dialog</source>
+        <translation>显示对话框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="25"/>
+        <source>A frameless message box</source>
+        <translation>无边框对话框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="33"/>
+        <source>A message box with mask</source>
+        <translation>带遮罩的对话框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="49"/>
+        <source>A color dialog</source>
+        <translation>颜色对话框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="88"/>
+        <source>This is a frameless message dialog</source>
+        <translation>这是一个无边框对话框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="99"/>
+        <source>If the content of the message box is veeeeeeeeeeeeeeeeeeeeeeeeeery long, it will automatically wrap like this.</source>
+        <translation>一生消えない傷でいいな，絆創膏の様にいつも包んでよ。貴方のそばでわがまま言いたいな，一分一秒刻み貴方を知り，あたしをあげる~</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="113"/>
+        <source>Choose color</source>
+        <translation>选择颜色</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="98"/>
+        <source>This is a message dialog with mask</source>
+        <translation>这是一个带遮罩的对话框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="79"/>
+        <source>Show teaching tip</source>
+        <translation>显示气泡弹窗</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="73"/>
+        <source>A teaching tip</source>
+        <translation>气泡弹窗</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="118"/>
+        <source>With respect, let&apos;s advance towards a new stage of the spin.</source>
+        <translation>表达敬意吧，表达出敬意，然后迈向回旋的另一个全新阶段！</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="131"/>
+        <source>The shortest shortcut is to take a detour.</source>
+        <translation>最短的捷径就是绕远路，绕远路才是我的最短捷径。</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="81"/>
+        <source>A teaching tip with image and button</source>
+        <translation>带图片和按钮的气泡弹窗</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="63"/>
+        <source>Show flyout</source>
+        <translation>显示浮出控件</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="57"/>
+        <source>A simple flyout</source>
+        <translation>简单的浮出控件</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="65"/>
+        <source>A flyout with image and button</source>
+        <translation>带图片和按钮的浮出控件</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="148"/>
+        <source>Believe in the spin, just keep believing!</source>
+        <translation>相信回旋吧，只管相信就是了！</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="157"/>
+        <source>Julius·Zeppeli</source>
+        <translation>杰洛·齐贝林</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="41"/>
+        <source>A custom message box</source>
+        <translation>自定义对话框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="157"/>
+        <source>Where the tennis ball will land when it touches the net, no one can predict.
+If that moment comes, I hope the &apos;goddess&apos; exists.
+In that case, I would accept it no matter which side the ball falls on.</source>
+        <translation>触网而起的网球会落到哪一侧，谁也无法知晓。
+如果那种时刻到来，我希望「女神」是存在的。
+这样的话，不管网球落到哪一边，我都会坦然接受的吧。</translation>
+    </message>
+</context>
+<context>
+    <name>ExampleCard</name>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="110"/>
+        <source>Source code</source>
+        <translation>源代码</translation>
+    </message>
+</context>
+<context>
+    <name>IconCardView</name>
+    <message>
+        <location filename="../../view/icon_interface.py" line="124"/>
+        <source>Fluent Icons Library</source>
+        <translation>流畅图标库</translation>
+    </message>
+</context>
+<context>
+    <name>IconInfoPanel</name>
+    <message>
+        <location filename="../../view/icon_interface.py" line="82"/>
+        <source>Icon name</source>
+        <translation>图标名字</translation>
+    </message>
+    <message>
+        <location filename="../../view/icon_interface.py" line="84"/>
+        <source>Enum member</source>
+        <translation>枚举成员</translation>
+    </message>
+</context>
+<context>
+    <name>LayoutInterface</name>
+    <message>
+        <location filename="../../view/layout_interface.py" line="21"/>
+        <source>Flow layout without animation</source>
+        <translation>不带动画效果的流式布局</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="28"/>
+        <source>Flow layout with animation</source>
+        <translation>带有动画效果的流式布局</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="37"/>
+        <source>Star Platinum</source>
+        <translation>白金之星</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="37"/>
+        <source>Hierophant Green</source>
+        <translation>法皇之绿</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="38"/>
+        <source>Silver Chariot</source>
+        <translation>银色战车</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="38"/>
+        <source>Crazy diamond</source>
+        <translation>疯狂钻石</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="39"/>
+        <source>Heaven&apos;s Door</source>
+        <translation>黑蚊子多</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="39"/>
+        <source>Killer Queen</source>
+        <translation>杀手皇后💀</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="40"/>
+        <source>Gold Experience</source>
+        <translation>黄金体验</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="40"/>
+        <source>Sticky Fingers</source>
+        <translation>钢链手指</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="41"/>
+        <source>Sex Pistols</source>
+        <translation>性感手枪</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="41"/>
+        <source>Dirty Deeds Done Dirt Cheap</source>
+        <translation>D4C</translation>
+    </message>
+</context>
+<context>
+    <name>LineEdit</name>
+    <message>
+        <location filename="../../view/icon_interface.py" line="22"/>
+        <source>Search icons</source>
+        <translation>搜索图标</translation>
+    </message>
+</context>
+<context>
+    <name>ListFrame</name>
+    <message>
+        <location filename="../../view/view_interface.py" line="90"/>
+        <source>Star Platinum</source>
+        <translation>白金之星</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="90"/>
+        <source>Hierophant Green</source>
+        <translation>法皇之绿</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="91"/>
+        <source>Made in Haven</source>
+        <translation>天堂制造</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="91"/>
+        <source>King Crimson</source>
+        <translation>绯红之王</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="92"/>
+        <source>Silver Chariot</source>
+        <translation>银色战车</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="92"/>
+        <source>Crazy diamond</source>
+        <translation>疯狂钻石</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="93"/>
+        <source>Metallica</source>
+        <translation>金属制品</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="93"/>
+        <source>Another One Bites The Dust</source>
+        <translation>败者食尘</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="94"/>
+        <source>Heaven&apos;s Door</source>
+        <translation>黑蚊子多</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="95"/>
+        <source>The Grateful Dead</source>
+        <translation>壮烈成仁</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="95"/>
+        <source>Stone Free</source>
+        <translation>石之自由</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="96"/>
+        <source>The World</source>
+        <translation>砸瓦鲁多</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="96"/>
+        <source>Sticky Fingers</source>
+        <translation>钢链手指</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="97"/>
+        <source>Ozone Baby</source>
+        <translation>臭氧宝宝</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="97"/>
+        <source>Love Love Deluxe</source>
+        <translation>华丽挚爱</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="98"/>
+        <source>Hermit Purple</source>
+        <translation>隐者之紫</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="98"/>
+        <source>Gold Experience</source>
+        <translation>黄金体验</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="99"/>
+        <source>King Nothing</source>
+        <translation>虚无之王</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="99"/>
+        <source>Paper Moon King</source>
+        <translation>纸月之王</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="100"/>
+        <source>Scary Monster</source>
+        <translation>骇人恶兽</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="100"/>
+        <source>Mandom</source>
+        <translation>男子领域</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="101"/>
+        <source>20th Century Boy</source>
+        <translation>20世纪男孩</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="101"/>
+        <source>Tusk Act 4</source>
+        <translation>牙 Act 4</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="102"/>
+        <source>Ball Breaker</source>
+        <translation>铁球破坏者</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="102"/>
+        <source>Sex Pistols</source>
+        <translation>性感手枪</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="103"/>
+        <source>D4C • Love Train</source>
+        <translation>D4C • 爱之列车</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="103"/>
+        <source>Born This Way</source>
+        <translation>天生完美</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="104"/>
+        <source>SOFT &amp; WET</source>
+        <translation>软又湿</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="104"/>
+        <source>Paisley Park</source>
+        <translation>佩斯利公园</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="105"/>
+        <source>Wonder of U</source>
+        <translation>奇迹于你</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="105"/>
+        <source>Walking Heart</source>
+        <translation>行走的心</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="106"/>
+        <source>Cream Starter</source>
+        <translation>护霜旅行者</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="106"/>
+        <source>November Rain</source>
+        <translation>十一月雨</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="107"/>
+        <source>Smooth Operators</source>
+        <translation>调情圣手</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="107"/>
+        <source>The Matte Kudasai</source>
+        <translation>片刻静候</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="94"/>
+        <source>Killer Queen</source>
+        <translation>杀手皇后💀</translation>
+    </message>
+</context>
+<context>
+    <name>MainWindow</name>
+    <message>
+        <location filename="../../view/main_window.py" line="71"/>
+        <source>Home</source>
+        <translation>主页</translation>
+    </message>
+    <message>
+        <location filename="../../view/main_window.py" line="95"/>
+        <source>Settings</source>
+        <translation>设置</translation>
+    </message>
+</context>
+<context>
+    <name>MaterialInterface</name>
+    <message>
+        <location filename="../../view/material_interface.py" line="30"/>
+        <source>Acrylic label</source>
+        <translation>亚克力标签</translation>
+    </message>
+</context>
+<context>
+    <name>MenuInterface</name>
+    <message>
+        <location filename="../../view/menu_interface.py" line="51"/>
+        <source>Rounded corners menu</source>
+        <translation>圆角菜单</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="70"/>
+        <source>Show menu</source>
+        <translation>显示菜单</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="114"/>
+        <source>Copy</source>
+        <translation>复制</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="115"/>
+        <source>Cut</source>
+        <translation>剪切</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="120"/>
+        <source>Video</source>
+        <translation>视频</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="120"/>
+        <source>Music</source>
+        <translation>音乐</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="127"/>
+        <source>Paste</source>
+        <translation>粘贴</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="127"/>
+        <source>Undo</source>
+        <translation>撤回</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="134"/>
+        <source>Select all</source>
+        <translation>全选</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="219"/>
+        <source>Settings</source>
+        <translation>设置</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="139"/>
+        <source>Help</source>
+        <translation>帮助</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="139"/>
+        <source>Feedback</source>
+        <translation>反馈</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="118"/>
+        <source>Add to</source>
+        <translation>添加到</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="81"/>
+        <source>Command bar</source>
+        <translation>命令栏</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="94"/>
+        <source>Click the image to open a command bar flyout 👇️🥵</source>
+        <translation>点击图片来弹出命令栏 👇️🥵</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="103"/>
+        <source>Command bar flyout</source>
+        <translation>浮出命令栏</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="184"/>
+        <source>Add</source>
+        <translation>添加</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="184"/>
+        <source>Rotate</source>
+        <translation>旋转</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="184"/>
+        <source>Zoom in</source>
+        <translation>放大</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="184"/>
+        <source>Zoom out</source>
+        <translation>缩小</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="191"/>
+        <source>Edit</source>
+        <translation>编辑</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="191"/>
+        <source>Info</source>
+        <translation>信息</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="216"/>
+        <source>Delete</source>
+        <translation>删除</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="213"/>
+        <source>Share</source>
+        <translation>分享</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="27"/>
+        <source>Create Date</source>
+        <translation>创建日期</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="28"/>
+        <source>Shooting Date</source>
+        <translation>拍摄日期</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="30"/>
+        <source>Name</source>
+        <translation>名称</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="37"/>
+        <source>Ascending</source>
+        <translation>升序</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="38"/>
+        <source>Descending</source>
+        <translation>降序</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="214"/>
+        <source>Save</source>
+        <translation>保存</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="215"/>
+        <source>Add to favorate</source>
+        <translation>添加到收藏夹</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="218"/>
+        <source>Print</source>
+        <translation>打印</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="227"/>
+        <source>Save image</source>
+        <translation>保存图像</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="199"/>
+        <source>Sort</source>
+        <translation>排序</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="29"/>
+        <source>Modified time</source>
+        <translation>修改日期</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="74"/>
+        <source>Checkable menu</source>
+        <translation>可选中菜单</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="62"/>
+        <source>Rounded corners menu with custom widget</source>
+        <translation>自定义组件菜单</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="153"/>
+        <source>Shoko</source>
+        <translation>硝子酱</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="157"/>
+        <source>Manage account profile</source>
+        <translation>管理账户和设置</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="157"/>
+        <source>Payment method</source>
+        <translation>支付方式</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="157"/>
+        <source>Redemption code and gift card</source>
+        <translation>兑换代码和礼品卡</translation>
+    </message>
+</context>
+<context>
+    <name>NavigationViewInterface</name>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="42"/>
+        <source>A basic pivot</source>
+        <translation>顶部导航栏</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="48"/>
+        <source>A segmented control</source>
+        <translation>分段导航栏</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="60"/>
+        <source>A tab bar</source>
+        <translation>标签栏</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="28"/>
+        <source>Home</source>
+        <translation>主页</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="28"/>
+        <source>Documents</source>
+        <translation>文档</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="28"/>
+        <source>Study</source>
+        <translation>学习资料</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="29"/>
+        <source>Janpanese Sensei</source>
+        <translation>霓虹国老师</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="29"/>
+        <source>Action Film</source>
+        <translation>动作类型电影</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="29"/>
+        <source>G Cup</source>
+        <translation>G Cup</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="30"/>
+        <source>Mikami Yua</source>
+        <translation>三上老师</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="30"/>
+        <source>Folder1</source>
+        <translation>文件夹 1</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="30"/>
+        <source>Folder2</source>
+        <translation>文件夹 2</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="35"/>
+        <source>Breadcrumb bar</source>
+        <translation>面包屑导航栏</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="54"/>
+        <source>Another segmented control</source>
+        <translation>另一种分段导航栏</translation>
+    </message>
+</context>
+<context>
+    <name>PivotInterface</name>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="95"/>
+        <source>Song</source>
+        <translation>歌曲</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="96"/>
+        <source>Album</source>
+        <translation>专辑</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="97"/>
+        <source>Artist</source>
+        <translation>歌手</translation>
+    </message>
+</context>
+<context>
+    <name>ProgressWidget</name>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="292"/>
+        <source>Progress</source>
+        <translation>进度</translation>
+    </message>
+</context>
+<context>
+    <name>ScrollInterface</name>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="34"/>
+        <source>Smooth scroll area</source>
+        <translation>平滑滚动区域</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="40"/>
+        <source>Chitanda Eru is too hot 🥵</source>
+        <translation>回眸一笑百媚生，六宫粉黛无颜色 🥵</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="51"/>
+        <source>Smooth scroll area implemented by animation</source>
+        <translation>使用动画实现的平滑滚动区域</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="57"/>
+        <source>Chitanda Eru is so hot 🥵🥵</source>
+        <translation>春寒赐浴华清池，温泉水滑洗凝脂 🥵🥵</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="68"/>
+        <source>Single direction scroll scroll area</source>
+        <translation>单方向平滑滚动区域</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="74"/>
+        <source>Chitanda Eru is so hot 🥵🥵🥵</source>
+        <translation>春宵苦短日高起，从此君王不早朝🥵🥵🥵</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="82"/>
+        <source>Pips pager</source>
+        <translation>圆点分页组件</translation>
+    </message>
+</context>
+<context>
+    <name>SettingInterface</name>
+    <message>
+        <location filename="../../view/setting_interface.py" line="27"/>
+        <source>Settings</source>
+        <translation>设置</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="30"/>
+        <source>Music on this PC</source>
+        <translation>此PC上的音乐</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="32"/>
+        <source>Local music library</source>
+        <translation>本地音乐库</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="206"/>
+        <source>Choose folder</source>
+        <translation>选择文件夹</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="39"/>
+        <source>Download directory</source>
+        <translation>下载目录</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="48"/>
+        <source>Personalization</source>
+        <translation>个性化</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="57"/>
+        <source>Application theme</source>
+        <translation>应用主题</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="57"/>
+        <source>Change the appearance of your application</source>
+        <translation>调整你的应用的外观</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="57"/>
+        <source>Light</source>
+        <translation>浅色</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="57"/>
+        <source>Dark</source>
+        <translation>深色</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="86"/>
+        <source>Use system setting</source>
+        <translation>跟随系统设置</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="68"/>
+        <source>Theme color</source>
+        <translation>主题色</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="68"/>
+        <source>Change the theme color of you application</source>
+        <translation>调整你的应用的主题色</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="75"/>
+        <source>Interface zoom</source>
+        <translation>界面缩放</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="75"/>
+        <source>Change the size of widgets and fonts</source>
+        <translation>调整小部件和字体的大小</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="86"/>
+        <source>Language</source>
+        <translation>语言</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="86"/>
+        <source>Set your preferred language for UI</source>
+        <translation>选择界面所使用的语言</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="96"/>
+        <source>Material</source>
+        <translation>材料</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="98"/>
+        <source>Acrylic blur radius</source>
+        <translation>亚克力磨砂半径</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="98"/>
+        <source>The greater the radius, the more blurred the image</source>
+        <translation>磨砂半径越大，图像越模糊</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="107"/>
+        <source>Software update</source>
+        <translation>软件更新</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="109"/>
+        <source>Check for updates when the application starts</source>
+        <translation>在应用程序启动时检查更新</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="109"/>
+        <source>The new version will be more stable and have more features</source>
+        <translation>新版本将更加稳定并拥有更多功能（建议启用此选项）</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="135"/>
+        <source>About</source>
+        <translation>关于</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="119"/>
+        <source>Open help page</source>
+        <translation>打开帮助页面</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="119"/>
+        <source>Help</source>
+        <translation>帮助</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="119"/>
+        <source>Discover new features and learn useful tips about PyQt-Fluent-Widgets</source>
+        <translation>发现新功能并了解有关 PyQt-Fluent-Widgets 的使用技巧</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="128"/>
+        <source>Provide feedback</source>
+        <translation>提供反馈</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="128"/>
+        <source>Help us improve PyQt-Fluent-Widgets by providing feedback</source>
+        <translation>通过提供反馈帮助我们改进 PyQt-Fluent-Widgets</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="135"/>
+        <source>Check update</source>
+        <translation>检查更新</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="135"/>
+        <source>Copyright</source>
+        <translation>版权所有</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="135"/>
+        <source>Version</source>
+        <translation>当前版本</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="197"/>
+        <source>Configuration takes effect after restart</source>
+        <translation>配置在重启软件后生效</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="197"/>
+        <source>Updated successfully</source>
+        <translation>更新成功</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="50"/>
+        <source>Mica effect</source>
+        <translation>云母效果</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="50"/>
+        <source>Apply semi transparent to windows and surfaces</source>
+        <translation>窗口和表面显示半透明</translation>
+    </message>
+</context>
+<context>
+    <name>StatusInfoInterface</name>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="29"/>
+        <source>State tool tip</source>
+        <translation>进度提示条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="49"/>
+        <source>Label with a ToolTip</source>
+        <translation>多看一眼就会爆炸</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="52"/>
+        <source>A label with a ToolTip</source>
+        <translation>带有工具提示的标签</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="202"/>
+        <source>Show StateToolTip</source>
+        <translation>显示进度提示条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="36"/>
+        <source>Button with a simple ToolTip</source>
+        <translation>带有工具提示的按钮</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="38"/>
+        <source>Simple ToolTip</source>
+        <translation>只因在人群之中多看了你一眼~</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="200"/>
+        <source>The model training is complete!</source>
+        <translation>模型训练完成啦！</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="206"/>
+        <source>Training model</source>
+        <translation>正在训练模型</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="206"/>
+        <source>Please wait patiently</source>
+        <translation>心急吃不了热豆腐，请耐心等待哦~</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="208"/>
+        <source>Hide StateToolTip</source>
+        <translation>隐藏进度提示条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="76"/>
+        <source>Success</source>
+        <translation>成功</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="86"/>
+        <source>A closable InfoBar</source>
+        <translation>可关闭的消息条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="94"/>
+        <source>Warning</source>
+        <translation>警告</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="104"/>
+        <source>A closable InfoBar with long message</source>
+        <translation>可关闭的长消息条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="111"/>
+        <source>GitHub</source>
+        <translation>GitHub</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="111"/>
+        <source>When you look long into an abyss, the abyss looks into you.</source>
+        <translation>当你长时间凝视深渊时，深渊也在凝视你。</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="123"/>
+        <source>An InfoBar with custom icon, background color and widget.</source>
+        <translation>自定义图标、背景和小部件的消息条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="154"/>
+        <source>InfoBar with different pop-up locations</source>
+        <translation>不同弹出位置的消息条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="247"/>
+        <source>No Internet</source>
+        <translation>无网络连接</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="225"/>
+        <source>Lesson 4</source>
+        <translation>Lesson 4</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="236"/>
+        <source>Lesson 5</source>
+        <translation>Lesson 5</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="93"/>
+        <source>My name is kira yoshikake, 33 years old. Living in the villa area northeast of duwangting, unmarried. I work in Guiyou chain store. Every day I have to work overtime until 8 p.m. to go home. I don&apos;t smoke. The wine is only for a taste. Sleep at 11 p.m. for 8 hours a day. Before I go to bed, I must drink a cup of warm milk, then do 20 minutes of soft exercise, get on the bed, and immediately fall asleep. Never leave fatigue and stress until the next day. Doctors say I&apos;m normal.</source>
+        <translation>我的名字是吉良吉影，年龄33岁，家住杜王町东北部别墅区，未婚。我在“龟友百货连锁公司”上班，每天最晚也是八点前回家，不吸烟，酒也是浅尝辄止，晚上十一点上床，保证八个小时的充足睡眠，睡前喝一杯热牛奶，再做二十分钟伸展运动暖身，然后再睡觉，基本可以熟睡到天亮。像婴儿一样不留下疲劳与压力，迎来第二天的早晨，健康检查结果也显示我很健康。我的意思是我是一个随时都想追求平静生活的人，不拘泥于胜负与烦恼，不树立令我夜不能寐的敌人，这就是我对于这个社会的生活态度，我也清楚这就是我的幸福。</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="213"/>
+        <source>Lesson 3</source>
+        <translation>Lesson 3</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="213"/>
+        <source>Believe in the spin, just keep believing!</source>
+        <translation>相信回旋吧，只管相信就是了！</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="225"/>
+        <source>With respect, let&apos;s advance towards a new stage of the spin.</source>
+        <translation>表达敬意吧，表达出敬意，然后迈向回旋的另一个全新阶段！</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="247"/>
+        <source>An error message which won&apos;t disappear automatically.</source>
+        <translation>一条不会自动消失的消息。</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="76"/>
+        <source>The Anthem of man is the Anthem of courage.</source>
+        <translation>人类的赞歌就是勇气的赞歌！</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="121"/>
+        <source>Action</source>
+        <translation></translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="132"/>
+        <source>Top right</source>
+        <translation>右上角</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="133"/>
+        <source>Top</source>
+        <translation>顶部居中</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="134"/>
+        <source>Top left</source>
+        <translation>左上角</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="135"/>
+        <source>Bottom right</source>
+        <translation>右下角</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="136"/>
+        <source>Bottom</source>
+        <translation>底部居中</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="137"/>
+        <source>Bottom left</source>
+        <translation>左下角</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="258"/>
+        <source>Lesson 1</source>
+        <translation>Lesson 1</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="258"/>
+        <source>Don&apos;t have any strange expectations of me.</source>
+        <translation>别对我抱有什么奇怪的期待</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="269"/>
+        <source>Lesson 2</source>
+        <translation>Lesson 2</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="269"/>
+        <source>Don&apos;t let your muscles notice.</source>
+        <translation>不要让肌肉察觉</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="163"/>
+        <source>An indeterminate progress bar</source>
+        <translation>不确定进度条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="173"/>
+        <source>An determinate progress bar</source>
+        <translation>确定的进度条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="192"/>
+        <source>An determinate progress ring</source>
+        <translation>确定的圆形进度条</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="236"/>
+        <source>The shortest shortcut is to take a detour.</source>
+        <translation>最短的捷径就是绕远路，绕远路才是我的最短捷径。</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="182"/>
+        <source>An indeterminate progress ring</source>
+        <translation>不确定进度环</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="69"/>
+        <source>InfoBadge in different styles</source>
+        <translation>不同样式的徽章</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="39"/>
+        <source>A button with a simple ToolTip</source>
+        <translation>带工具提示的按钮</translation>
+    </message>
+</context>
+<context>
+    <name>TabInterface</name>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="148"/>
+        <source>IsTabMovable</source>
+        <translation>启用标签拖拽</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="149"/>
+        <source>IsTabScrollable</source>
+        <translation>启用标签滚动</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="153"/>
+        <source>TabCloseButtonDisplayMode</source>
+        <translation>关闭按钮显示模式</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="175"/>
+        <source>Always</source>
+        <translation>始终显示</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="176"/>
+        <source>OnHover</source>
+        <translation>进入时显示</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="177"/>
+        <source>Never</source>
+        <translation>从不显示</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="180"/>
+        <source>Song</source>
+        <translation>歌曲</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="182"/>
+        <source>Album</source>
+        <translation>专辑</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="184"/>
+        <source>Artist</source>
+        <translation>歌手</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="150"/>
+        <source>IsTabShadowEnabled</source>
+        <translation>启用标签阴影</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="151"/>
+        <source>TabMaximumWidth</source>
+        <translation>标签最大宽度</translation>
+    </message>
+</context>
+<context>
+    <name>TableFrame</name>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Title</source>
+        <translation>标题</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Artist</source>
+        <translation>歌手</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Album</source>
+        <translation>专辑</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Year</source>
+        <translation>年份</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Duration</source>
+        <translation>时长</translation>
+    </message>
+</context>
+<context>
+    <name>TextInterface</name>
+    <message>
+        <location filename="../../view/text_interface.py" line="25"/>
+        <source>ko no dio da！</source>
+        <translation></translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="27"/>
+        <source>A LineEdit with a clear button</source>
+        <translation>带清空按钮的输入框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="87"/>
+        <source>A DoubleSpinBox with a spin button</source>
+        <translation>浮点数旋转框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="94"/>
+        <source>A DateEdit with a spin button</source>
+        <translation>日期编辑框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="101"/>
+        <source>A TimeEdit with a spin button</source>
+        <translation>时间编辑框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="108"/>
+        <source>A DateTimeEdit with a spin button</source>
+        <translation>日期时间编辑框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="80"/>
+        <source>A SpinBox with a spin button</source>
+        <translation>旋转框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="119"/>
+        <source>A simple TextEdit</source>
+        <translation>富文本框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="63"/>
+        <source>A autosuggest line edit</source>
+        <translation>带补全功能的输入框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="35"/>
+        <source>Type a stand name</source>
+        <translation>输入替身名称</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="72"/>
+        <source>Enter your password</source>
+        <translation>请输入密码</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="73"/>
+        <source>A password line edit</source>
+        <translation>密码输入框</translation>
+    </message>
+</context>
+<context>
+    <name>ToolBar</name>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="42"/>
+        <source>Documentation</source>
+        <translation>在线文档</translation>
+    </message>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="44"/>
+        <source>Source</source>
+        <translation>源代码</translation>
+    </message>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="81"/>
+        <source>Toggle theme</source>
+        <translation>切换主题</translation>
+    </message>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="83"/>
+        <source>Send feedback</source>
+        <translation>提供反馈</translation>
+    </message>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="82"/>
+        <source>Support me</source>
+        <translation>支持作者🥰</translation>
+    </message>
+</context>
+<context>
+    <name>Translator</name>
+    <message>
+        <location filename="../../common/translator.py" line="19"/>
+        <source>Basic input</source>
+        <translation>基本输入</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="16"/>
+        <source>Material</source>
+        <translation>材料</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="20"/>
+        <source>Status &amp; info</source>
+        <translation>状态和信息</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="15"/>
+        <source>Scrolling</source>
+        <translation>滚动</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="13"/>
+        <source>Layout</source>
+        <translation>布局</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="9"/>
+        <source>Text</source>
+        <translation>文本</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="12"/>
+        <source>Icons</source>
+        <translation>图标</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="10"/>
+        <source>View</source>
+        <translation>视图</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="17"/>
+        <source>Date &amp; time</source>
+        <translation>日期和时间</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="18"/>
+        <source>Navigation</source>
+        <translation>导航</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="18"/>
+        <source>Price</source>
+        <translation>价格方案</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="14"/>
+        <source>Dialogs &amp; flyouts</source>
+        <translation>对话框和弹出窗口</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="11"/>
+        <source>Menus &amp; toolbars</source>
+        <translation>菜单和工具栏</translation>
+    </message>
+</context>
+<context>
+    <name>TreeFrame</name>
+    <message>
+        <location filename="../../view/view_interface.py" line="122"/>
+        <source>JoJo 1 - Phantom Blood</source>
+        <translation>JoJo 1 - 幻影之血</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="123"/>
+        <source>Jonathan Joestar</source>
+        <translation>乔纳森·乔斯达</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="123"/>
+        <source>Dio Brando</source>
+        <translation>迪奥·布兰度</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="123"/>
+        <source>Will A. Zeppeli</source>
+        <translation>威廉·A·齐贝林</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="130"/>
+        <source>JoJo 3 - Stardust Crusaders</source>
+        <translation>JoJo3 - 星尘斗士</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="131"/>
+        <source>Jotaro Kujo</source>
+        <translation></translation>
+    </message>
+</context>
+<context>
+    <name>ViewInterface</name>
+    <message>
+        <location filename="../../view/view_interface.py" line="40"/>
+        <source>A simple TreeView</source>
+        <translation>简单的树状组件</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="47"/>
+        <source>A TreeView with Multi-selection enabled</source>
+        <translation>启用复选框的树状组件</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="32"/>
+        <source>A simple TableView</source>
+        <translation>简单的表格组件</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="25"/>
+        <source>A simple ListView</source>
+        <translation>简单的列表组件</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="61"/>
+        <source>Flip view</source>
+        <translation>翻转视图</translation>
+    </message>
+</context>
+</TS>
diff --git a/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_HK.qm b/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_HK.qm
new file mode 100644
index 0000000..2a9b01f
Binary files /dev/null and b/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_HK.qm differ
diff --git a/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_HK.ts b/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_HK.ts
new file mode 100644
index 0000000..e88de0d
--- /dev/null
+++ b/tool/FusionFlowFluent/app/resource/i18n/gallery.zh_HK.ts
@@ -0,0 +1,1811 @@
+<?xml version="1.0" encoding="utf-8"?>
+<!DOCTYPE TS>
+<TS version="2.1" language="zh_CN" sourcelanguage="en_US">
+<context>
+    <name>BasicInputInterface</name>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="29"/>
+        <source>A simple button with text content</source>
+        <translation>帶有文本的簡單按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="39"/>
+        <source>A button with graphical content</source>
+        <translation>帶有圖標的按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="186"/>
+        <source>A hyperlink button that navigates to a URI</source>
+        <translation>導航到一個超鏈接的按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="88"/>
+        <source>A 2-state CheckBox</source>
+        <translation>雙態複選框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="97"/>
+        <source>A 3-state CheckBox</source>
+        <translation>三態複選框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="108"/>
+        <source>A ComboBox with items</source>
+        <translation>下拉框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="209"/>
+        <source>A group of RadioButton controls in a button group</source>
+        <translation>單選按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="220"/>
+        <source>A simple horizontal slider</source>
+        <translation>水平滑動條</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="271"/>
+        <source>A simple switch button</source>
+        <translation>開關按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="310"/>
+        <source>Off</source>
+        <translation>關</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="308"/>
+        <source>On</source>
+        <translation>開</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="29"/>
+        <source>Standard push button</source>
+        <translation>標準按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="46"/>
+        <source>Accent style button</source>
+        <translation>主題色按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="88"/>
+        <source>Two-state CheckBox</source>
+        <translation>雙態複選框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="95"/>
+        <source>Three-state CheckBox</source>
+        <translation>三態複選框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="314"/>
+        <source>Star Platinum</source>
+        <translation>白金之星</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="314"/>
+        <source>Crazy Diamond</source>
+        <translation>瘋狂鑽石</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="200"/>
+        <source>Soft and Wet</source>
+        <translation>軟又溼</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="314"/>
+        <source>Gold Experience</source>
+        <translation>黃金體驗</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="314"/>
+        <source>Sticky Fingers</source>
+        <translation>鋼鏈手指</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="252"/>
+        <source>Choose your stand</source>
+        <translation>選擇你的替身</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="124"/>
+        <source>An editable ComboBox</source>
+        <translation>可編輯的下拉框</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="132"/>
+        <source>Send</source>
+        <translation>髮送</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="133"/>
+        <source>Save</source>
+        <translation>保存</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="168"/>
+        <source>Email</source>
+        <translation>郵件</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="136"/>
+        <source>A push button with drop down menu</source>
+        <translation>帶下拉菜單的按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="144"/>
+        <source>A tool button with drop down menu</source>
+        <translation>帶下拉菜單的工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="292"/>
+        <source>Start practicing</source>
+        <translation>開始練習</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="229"/>
+        <source>A split push button with drop down menu</source>
+        <translation>帶下拉菜單的拆分按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="236"/>
+        <source>Sing</source>
+        <translation>唱</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="236"/>
+        <source>Jump</source>
+        <translation>跳</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="236"/>
+        <source>Rap</source>
+        <translation>Rap</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="236"/>
+        <source>Music</source>
+        <translation>Music</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="245"/>
+        <source>A split tool button with drop down menu</source>
+        <translation>帶下拉菜單的工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="46"/>
+        <source>Accent style applied to push button</source>
+        <translation>主題色按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="53"/>
+        <source>Accent style applied to tool button</source>
+        <translation>主題色工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="153"/>
+        <source>A primary color push button with drop down menu</source>
+        <translation>帶下拉菜單的主題色按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="161"/>
+        <source>A primary color tool button with drop down menu</source>
+        <translation>帶下拉菜單的主題色工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="254"/>
+        <source>A primary color split push button with drop down menu</source>
+        <translation>帶下拉菜單的主題色拆分按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="262"/>
+        <source>A primary color split tool button with drop down menu</source>
+        <translation>帶下拉菜單的主題色拆分工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="74"/>
+        <source>A transparent push button</source>
+        <translation>透明按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="81"/>
+        <source>A transparent tool button</source>
+        <translation>透明工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="170"/>
+        <source>A transparent push button with drop down menu</source>
+        <translation>帶下拉菜單的透明按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="179"/>
+        <source>A transparent tool button with drop down menu</source>
+        <translation>帶下拉菜單的透明工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="278"/>
+        <source>A simple toggle push button</source>
+        <translation>狀態開關按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="285"/>
+        <source>A simple toggle tool button</source>
+        <translation>狀態開關工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="292"/>
+        <source>A transparent toggle push button</source>
+        <translation>透明的狀態開關按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="299"/>
+        <source>A transparent toggle tool button</source>
+        <translation>透明的狀態開關工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="74"/>
+        <source>Transparent push button</source>
+        <translation>透明按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="60"/>
+        <source>Pill push button</source>
+        <translation>橢圓按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="60"/>
+        <source>Tag</source>
+        <translation>標籤</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="67"/>
+        <source>Pill tool button</source>
+        <translation>橢圓工具按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/basic_input_interface.py" line="186"/>
+        <source>QFluentWidgets official website</source>
+        <translation></translation>
+    </message>
+</context>
+<context>
+    <name>CustomMessageBox</name>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="182"/>
+        <source>Open URL</source>
+        <translation>打開 URL</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="185"/>
+        <source>Enter the URL of a file, stream, or playlist</source>
+        <translation>輸入文件、流或者播放列表的 URL</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="193"/>
+        <source>Open</source>
+        <translation>打開</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="194"/>
+        <source>Cancel</source>
+        <translation>取消</translation>
+    </message>
+</context>
+<context>
+    <name>DateTimeInterface</name>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="37"/>
+        <source>A simple DatePicker</source>
+        <translation>日期選擇器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="43"/>
+        <source>A DatePicker in another format</source>
+        <translation>另一種格式的日期選擇器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="50"/>
+        <source>A simple TimePicker</source>
+        <translation>時間選擇器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="57"/>
+        <source>A TimePicker using a 24-hour clock</source>
+        <translation>24 小時製的時間選擇器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="64"/>
+        <source>A TimePicker with seconds column</source>
+        <translation>顯示秒的時間選擇器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="22"/>
+        <source>A simple CalendarPicker</source>
+        <translation>日曆選擇器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="22"/>
+        <source>A fast CalendarPicker</source>
+        <translation>快速日曆選擇器</translation>
+    </message>
+    <message>
+        <location filename="../../view/date_time_interface.py" line="30"/>
+        <source>A CalendarPicker in another format</source>
+        <translation>自定義格式的日曆選擇器</translation>
+    </message>
+</context>
+<context>
+    <name>DialogInterface</name>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="47"/>
+        <source>Show dialog</source>
+        <translation>顯示對話框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="25"/>
+        <source>A frameless message box</source>
+        <translation>無邊框對話框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="33"/>
+        <source>A message box with mask</source>
+        <translation>帶遮罩的對話框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="49"/>
+        <source>A color dialog</source>
+        <translation>顏色對話框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="88"/>
+        <source>This is a frameless message dialog</source>
+        <translation>這是一個無邊框對話框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="99"/>
+        <source>If the content of the message box is veeeeeeeeeeeeeeeeeeeeeeeeeery long, it will automatically wrap like this.</source>
+        <translation>一生消えない傷でいいな，絆創膏の様にいつも包んでよ。貴方のそばでわがまま言いたいな，一分一秒刻み貴方を知り，あたしをあげる~</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="113"/>
+        <source>Choose color</source>
+        <translation>選擇顏色</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="98"/>
+        <source>This is a message dialog with mask</source>
+        <translation>這是一個帶遮罩的對話框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="79"/>
+        <source>Show teaching tip</source>
+        <translation>顯示氣泡彈窗</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="73"/>
+        <source>A teaching tip</source>
+        <translation>氣泡彈窗</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="118"/>
+        <source>With respect, let&apos;s advance towards a new stage of the spin.</source>
+        <translation>表達敬意吧，表達出敬意，然後邁向回旋的另一個全新階段！</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="131"/>
+        <source>The shortest shortcut is to take a detour.</source>
+        <translation>最短的捷徑就是繞遠路，繞遠路才是我的最短捷徑。</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="81"/>
+        <source>A teaching tip with image and button</source>
+        <translation>帶圖片和按鈕的氣泡彈窗</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="63"/>
+        <source>Show flyout</source>
+        <translation>顯示浮出控件</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="57"/>
+        <source>A simple flyout</source>
+        <translation>簡單的浮出控件</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="65"/>
+        <source>A flyout with image and button</source>
+        <translation>帶圖片和按鈕的浮出控件</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="148"/>
+        <source>Believe in the spin, just keep believing!</source>
+        <translation>相信回旋吧，隻管相信就是了！</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="157"/>
+        <source>Julius·Zeppeli</source>
+        <translation>傑洛·齊貝林</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="41"/>
+        <source>A custom message box</source>
+        <translation>自定義對話框</translation>
+    </message>
+    <message>
+        <location filename="../../view/dialog_interface.py" line="157"/>
+        <source>Where the tennis ball will land when it touches the net, no one can predict.
+If that moment comes, I hope the &apos;goddess&apos; exists.
+In that case, I would accept it no matter which side the ball falls on.</source>
+        <translation>觸網而起的網球會落到哪一側，誰也無法知曉。
+如果那種時刻到來，我希望「女神」是存在的。
+這樣的話，不管網球落到哪一邊，我都會坦然接受的吧。</translation>
+    </message>
+</context>
+<context>
+    <name>ExampleCard</name>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="110"/>
+        <source>Source code</source>
+        <translation>源代碼</translation>
+    </message>
+</context>
+<context>
+    <name>IconCardView</name>
+    <message>
+        <location filename="../../view/icon_interface.py" line="124"/>
+        <source>Fluent Icons Library</source>
+        <translation>流暢圖標庫</translation>
+    </message>
+</context>
+<context>
+    <name>IconInfoPanel</name>
+    <message>
+        <location filename="../../view/icon_interface.py" line="82"/>
+        <source>Icon name</source>
+        <translation>圖標名字</translation>
+    </message>
+    <message>
+        <location filename="../../view/icon_interface.py" line="84"/>
+        <source>Enum member</source>
+        <translation>枚舉成員</translation>
+    </message>
+</context>
+<context>
+    <name>LayoutInterface</name>
+    <message>
+        <location filename="../../view/layout_interface.py" line="21"/>
+        <source>Flow layout without animation</source>
+        <translation>不帶動畫效果的流式佈局</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="28"/>
+        <source>Flow layout with animation</source>
+        <translation>帶有動畫效果的流式佈局</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="37"/>
+        <source>Star Platinum</source>
+        <translation>白金之星</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="37"/>
+        <source>Hierophant Green</source>
+        <translation>法皇之綠</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="38"/>
+        <source>Silver Chariot</source>
+        <translation>銀色戰車</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="38"/>
+        <source>Crazy diamond</source>
+        <translation>瘋狂鑽石</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="39"/>
+        <source>Heaven&apos;s Door</source>
+        <translation>黑蚊子多</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="39"/>
+        <source>Killer Queen</source>
+        <translation>殺手皇後💀</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="40"/>
+        <source>Gold Experience</source>
+        <translation>黃金體驗</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="40"/>
+        <source>Sticky Fingers</source>
+        <translation>鋼鏈手指</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="41"/>
+        <source>Sex Pistols</source>
+        <translation>性感手槍</translation>
+    </message>
+    <message>
+        <location filename="../../view/layout_interface.py" line="41"/>
+        <source>Dirty Deeds Done Dirt Cheap</source>
+        <translation>D4C</translation>
+    </message>
+</context>
+<context>
+    <name>LineEdit</name>
+    <message>
+        <location filename="../../view/icon_interface.py" line="22"/>
+        <source>Search icons</source>
+        <translation>蒐索圖標</translation>
+    </message>
+</context>
+<context>
+    <name>ListFrame</name>
+    <message>
+        <location filename="../../view/view_interface.py" line="90"/>
+        <source>Star Platinum</source>
+        <translation>白金之星</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="90"/>
+        <source>Hierophant Green</source>
+        <translation>法皇之綠</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="91"/>
+        <source>Made in Haven</source>
+        <translation>天堂製造</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="91"/>
+        <source>King Crimson</source>
+        <translation>緋紅之王</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="92"/>
+        <source>Silver Chariot</source>
+        <translation>銀色戰車</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="92"/>
+        <source>Crazy diamond</source>
+        <translation>瘋狂鑽石</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="93"/>
+        <source>Metallica</source>
+        <translation>金屬製品</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="93"/>
+        <source>Another One Bites The Dust</source>
+        <translation>敗者食塵</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="94"/>
+        <source>Heaven&apos;s Door</source>
+        <translation>黑蚊子多</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="95"/>
+        <source>The Grateful Dead</source>
+        <translation>壯烈成仁</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="95"/>
+        <source>Stone Free</source>
+        <translation>石之自由</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="96"/>
+        <source>The World</source>
+        <translation>砸瓦魯多</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="96"/>
+        <source>Sticky Fingers</source>
+        <translation>鋼鏈手指</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="97"/>
+        <source>Ozone Baby</source>
+        <translation>臭氧寶寶</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="97"/>
+        <source>Love Love Deluxe</source>
+        <translation>華麗摯愛</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="98"/>
+        <source>Hermit Purple</source>
+        <translation>隱者之紫</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="98"/>
+        <source>Gold Experience</source>
+        <translation>黃金體驗</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="99"/>
+        <source>King Nothing</source>
+        <translation>虛無之王</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="99"/>
+        <source>Paper Moon King</source>
+        <translation>紙月之王</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="100"/>
+        <source>Scary Monster</source>
+        <translation>駭人噁獸</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="100"/>
+        <source>Mandom</source>
+        <translation>男子領域</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="101"/>
+        <source>20th Century Boy</source>
+        <translation>20世紀男孩</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="101"/>
+        <source>Tusk Act 4</source>
+        <translation>牙 Act 4</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="102"/>
+        <source>Ball Breaker</source>
+        <translation>鐵球破壞者</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="102"/>
+        <source>Sex Pistols</source>
+        <translation>性感手槍</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="103"/>
+        <source>D4C • Love Train</source>
+        <translation>D4C • 愛之列車</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="103"/>
+        <source>Born This Way</source>
+        <translation>天生完美</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="104"/>
+        <source>SOFT &amp; WET</source>
+        <translation>軟又溼</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="104"/>
+        <source>Paisley Park</source>
+        <translation>佩斯利公園</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="105"/>
+        <source>Wonder of U</source>
+        <translation>奇跡於你</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="105"/>
+        <source>Walking Heart</source>
+        <translation>行走的心</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="106"/>
+        <source>Cream Starter</source>
+        <translation>護霜旅行者</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="106"/>
+        <source>November Rain</source>
+        <translation>十一月雨</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="107"/>
+        <source>Smooth Operators</source>
+        <translation>調情聖手</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="107"/>
+        <source>The Matte Kudasai</source>
+        <translation>片刻靜候</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="94"/>
+        <source>Killer Queen</source>
+        <translation>殺手皇後💀</translation>
+    </message>
+</context>
+<context>
+    <name>MainWindow</name>
+    <message>
+        <location filename="../../view/main_window.py" line="71"/>
+        <source>Home</source>
+        <translation>主頁</translation>
+    </message>
+    <message>
+        <location filename="../../view/main_window.py" line="95"/>
+        <source>Settings</source>
+        <translation>設置</translation>
+    </message>
+</context>
+<context>
+    <name>MaterialInterface</name>
+    <message>
+        <location filename="../../view/material_interface.py" line="30"/>
+        <source>Acrylic label</source>
+        <translation>亞克力標籤</translation>
+    </message>
+</context>
+<context>
+    <name>MenuInterface</name>
+    <message>
+        <location filename="../../view/menu_interface.py" line="51"/>
+        <source>Rounded corners menu</source>
+        <translation>圓角菜單</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="70"/>
+        <source>Show menu</source>
+        <translation>顯示菜單</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="114"/>
+        <source>Copy</source>
+        <translation>複製</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="115"/>
+        <source>Cut</source>
+        <translation>剪切</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="120"/>
+        <source>Video</source>
+        <translation>視頻</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="120"/>
+        <source>Music</source>
+        <translation>音樂</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="127"/>
+        <source>Paste</source>
+        <translation>粘貼</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="127"/>
+        <source>Undo</source>
+        <translation>撤回</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="134"/>
+        <source>Select all</source>
+        <translation>全選</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="219"/>
+        <source>Settings</source>
+        <translation>設置</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="139"/>
+        <source>Help</source>
+        <translation>幫助</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="139"/>
+        <source>Feedback</source>
+        <translation>反饋</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="118"/>
+        <source>Add to</source>
+        <translation>添加到</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="81"/>
+        <source>Command bar</source>
+        <translation>命令欄</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="94"/>
+        <source>Click the image to open a command bar flyout 👇️🥵</source>
+        <translation>點擊圖片來彈出命令欄 👇️🥵</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="103"/>
+        <source>Command bar flyout</source>
+        <translation>浮出命令欄</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="184"/>
+        <source>Add</source>
+        <translation>添加</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="184"/>
+        <source>Rotate</source>
+        <translation>旋轉</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="184"/>
+        <source>Zoom in</source>
+        <translation>放大</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="184"/>
+        <source>Zoom out</source>
+        <translation>縮小</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="191"/>
+        <source>Edit</source>
+        <translation>編輯</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="191"/>
+        <source>Info</source>
+        <translation>信息</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="216"/>
+        <source>Delete</source>
+        <translation>刪除</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="213"/>
+        <source>Share</source>
+        <translation>分享</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="27"/>
+        <source>Create Date</source>
+        <translation>創建日期</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="28"/>
+        <source>Shooting Date</source>
+        <translation>拍攝日期</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="30"/>
+        <source>Name</source>
+        <translation>名稱</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="37"/>
+        <source>Ascending</source>
+        <translation>昇序</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="38"/>
+        <source>Descending</source>
+        <translation>降序</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="214"/>
+        <source>Save</source>
+        <translation>保存</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="215"/>
+        <source>Add to favorate</source>
+        <translation>添加到收藏夾</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="218"/>
+        <source>Print</source>
+        <translation>打印</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="227"/>
+        <source>Save image</source>
+        <translation>保存圖像</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="199"/>
+        <source>Sort</source>
+        <translation>排序</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="29"/>
+        <source>Modified time</source>
+        <translation>修改日期</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="74"/>
+        <source>Checkable menu</source>
+        <translation>可選中菜單</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="62"/>
+        <source>Rounded corners menu with custom widget</source>
+        <translation>自定義組件菜單</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="153"/>
+        <source>Shoko</source>
+        <translation>硝子醬</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="157"/>
+        <source>Manage account profile</source>
+        <translation>管理賬戶和設置</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="157"/>
+        <source>Payment method</source>
+        <translation>支付方式</translation>
+    </message>
+    <message>
+        <location filename="../../view/menu_interface.py" line="157"/>
+        <source>Redemption code and gift card</source>
+        <translation>兌換代碼和禮品卡</translation>
+    </message>
+</context>
+<context>
+    <name>NavigationViewInterface</name>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="42"/>
+        <source>A basic pivot</source>
+        <translation>頂部導航欄</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="48"/>
+        <source>A segmented control</source>
+        <translation>分段導航欄</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="60"/>
+        <source>A tab bar</source>
+        <translation>標籤欄</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="28"/>
+        <source>Home</source>
+        <translation>主頁</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="28"/>
+        <source>Documents</source>
+        <translation>文檔</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="28"/>
+        <source>Study</source>
+        <translation>學習資料</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="29"/>
+        <source>Janpanese Sensei</source>
+        <translation>霓虹國老師</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="29"/>
+        <source>Action Film</source>
+        <translation>動作類型電影</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="29"/>
+        <source>G Cup</source>
+        <translation>G Cup</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="30"/>
+        <source>Mikami Yua</source>
+        <translation>三上老師</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="30"/>
+        <source>Folder1</source>
+        <translation>文件夾 1</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="30"/>
+        <source>Folder2</source>
+        <translation>文件夾 2</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="35"/>
+        <source>Breadcrumb bar</source>
+        <translation>麵包屑導航欄</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="54"/>
+        <source>Another segmented control</source>
+        <translation>另一種分段導航欄</translation>
+    </message>
+</context>
+<context>
+    <name>PivotInterface</name>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="95"/>
+        <source>Song</source>
+        <translation>歌曲</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="96"/>
+        <source>Album</source>
+        <translation>專輯</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="97"/>
+        <source>Artist</source>
+        <translation>歌手</translation>
+    </message>
+</context>
+<context>
+    <name>ProgressWidget</name>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="292"/>
+        <source>Progress</source>
+        <translation>進度</translation>
+    </message>
+</context>
+<context>
+    <name>ScrollInterface</name>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="34"/>
+        <source>Smooth scroll area</source>
+        <translation>平滑滾動區域</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="40"/>
+        <source>Chitanda Eru is too hot 🥵</source>
+        <translation>回眸一笑百媚生，六宮粉黛無顏色 🥵</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="51"/>
+        <source>Smooth scroll area implemented by animation</source>
+        <translation>使用動畫實現的平滑滾動區域</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="57"/>
+        <source>Chitanda Eru is so hot 🥵🥵</source>
+        <translation>春寒賜浴華清池，溫泉水滑洗凝脂 🥵🥵</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="68"/>
+        <source>Single direction scroll scroll area</source>
+        <translation>單方向平滑滾動區域</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="74"/>
+        <source>Chitanda Eru is so hot 🥵🥵🥵</source>
+        <translation>春宵苦短日高起，從此君王不早朝🥵🥵🥵</translation>
+    </message>
+    <message>
+        <location filename="../../view/scroll_interface.py" line="82"/>
+        <source>Pips pager</source>
+        <translation>圓點分頁組件</translation>
+    </message>
+</context>
+<context>
+    <name>SettingInterface</name>
+    <message>
+        <location filename="../../view/setting_interface.py" line="27"/>
+        <source>Settings</source>
+        <translation>設置</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="30"/>
+        <source>Music on this PC</source>
+        <translation>此PC上的音樂</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="32"/>
+        <source>Local music library</source>
+        <translation>本地音樂庫</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="206"/>
+        <source>Choose folder</source>
+        <translation>選擇文件夾</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="39"/>
+        <source>Download directory</source>
+        <translation>下載目錄</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="48"/>
+        <source>Personalization</source>
+        <translation>個性化</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="57"/>
+        <source>Application theme</source>
+        <translation>應用主題</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="57"/>
+        <source>Change the appearance of your application</source>
+        <translation>調整你的應用的外觀</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="57"/>
+        <source>Light</source>
+        <translation>淺色</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="57"/>
+        <source>Dark</source>
+        <translation>深色</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="86"/>
+        <source>Use system setting</source>
+        <translation>跟隨繫統設置</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="68"/>
+        <source>Theme color</source>
+        <translation>主題色</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="68"/>
+        <source>Change the theme color of you application</source>
+        <translation>調整你的應用的主題色</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="75"/>
+        <source>Interface zoom</source>
+        <translation>界麵縮放</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="75"/>
+        <source>Change the size of widgets and fonts</source>
+        <translation>調整小部件和字體的大小</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="86"/>
+        <source>Language</source>
+        <translation>語言</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="86"/>
+        <source>Set your preferred language for UI</source>
+        <translation>選擇界麵所使用的語言</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="96"/>
+        <source>Material</source>
+        <translation>材料</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="98"/>
+        <source>Acrylic blur radius</source>
+        <translation>亞克力磨砂半徑</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="98"/>
+        <source>The greater the radius, the more blurred the image</source>
+        <translation>磨砂半徑越大，圖像越模糊</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="107"/>
+        <source>Software update</source>
+        <translation>軟件更新</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="109"/>
+        <source>Check for updates when the application starts</source>
+        <translation>在應用程序啟動時檢查更新</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="109"/>
+        <source>The new version will be more stable and have more features</source>
+        <translation>新版本將更加穩定並擁有更多功能（建議啟用此選項）</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="135"/>
+        <source>About</source>
+        <translation>關於</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="119"/>
+        <source>Open help page</source>
+        <translation>打開幫助頁麵</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="119"/>
+        <source>Help</source>
+        <translation>幫助</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="119"/>
+        <source>Discover new features and learn useful tips about PyQt-Fluent-Widgets</source>
+        <translation>髮現新功能並了解有關 PyQt-Fluent-Widgets 的使用技巧</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="128"/>
+        <source>Provide feedback</source>
+        <translation>提供反饋</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="128"/>
+        <source>Help us improve PyQt-Fluent-Widgets by providing feedback</source>
+        <translation>通過提供反饋幫助我們改進 PyQt-Fluent-Widgets</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="135"/>
+        <source>Check update</source>
+        <translation>檢查更新</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="135"/>
+        <source>Copyright</source>
+        <translation>版權所有</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="135"/>
+        <source>Version</source>
+        <translation>當前版本</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="197"/>
+        <source>Configuration takes effect after restart</source>
+        <translation>配置在重啟軟件後生效</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="197"/>
+        <source>Updated successfully</source>
+        <translation>更新成功</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="50"/>
+        <source>Mica effect</source>
+        <translation>雲母效果</translation>
+    </message>
+    <message>
+        <location filename="../../view/setting_interface.py" line="50"/>
+        <source>Apply semi transparent to windows and surfaces</source>
+        <translation>窗口和表麵顯示半透明</translation>
+    </message>
+</context>
+<context>
+    <name>StatusInfoInterface</name>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="29"/>
+        <source>State tool tip</source>
+        <translation>進度提示條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="49"/>
+        <source>Label with a ToolTip</source>
+        <translation>多看一眼就會爆炸</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="52"/>
+        <source>A label with a ToolTip</source>
+        <translation>帶有工具提示的標籤</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="202"/>
+        <source>Show StateToolTip</source>
+        <translation>顯示進度提示條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="36"/>
+        <source>Button with a simple ToolTip</source>
+        <translation>帶有工具提示的按鈕</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="38"/>
+        <source>Simple ToolTip</source>
+        <translation>隻因在人群之中多看了你一眼~</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="200"/>
+        <source>The model training is complete!</source>
+        <translation>模型訓練完成啦！</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="206"/>
+        <source>Training model</source>
+        <translation>正在訓練模型</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="206"/>
+        <source>Please wait patiently</source>
+        <translation>心急吃不了熱荳腐，請耐心等待哦~</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="208"/>
+        <source>Hide StateToolTip</source>
+        <translation>隱藏進度提示條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="76"/>
+        <source>Success</source>
+        <translation>成功</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="86"/>
+        <source>A closable InfoBar</source>
+        <translation>可關閉的消息條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="94"/>
+        <source>Warning</source>
+        <translation>警告</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="104"/>
+        <source>A closable InfoBar with long message</source>
+        <translation>可關閉的長消息條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="111"/>
+        <source>GitHub</source>
+        <translation>GitHub</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="111"/>
+        <source>When you look long into an abyss, the abyss looks into you.</source>
+        <translation>當你長時間凝視深淵時，深淵也在凝視你。</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="123"/>
+        <source>An InfoBar with custom icon, background color and widget.</source>
+        <translation>自定義圖標、背景和小部件的消息條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="154"/>
+        <source>InfoBar with different pop-up locations</source>
+        <translation>不同彈出位置的消息條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="247"/>
+        <source>No Internet</source>
+        <translation>無網絡連接</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="225"/>
+        <source>Lesson 4</source>
+        <translation>Lesson 4</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="236"/>
+        <source>Lesson 5</source>
+        <translation>Lesson 5</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="93"/>
+        <source>My name is kira yoshikake, 33 years old. Living in the villa area northeast of duwangting, unmarried. I work in Guiyou chain store. Every day I have to work overtime until 8 p.m. to go home. I don&apos;t smoke. The wine is only for a taste. Sleep at 11 p.m. for 8 hours a day. Before I go to bed, I must drink a cup of warm milk, then do 20 minutes of soft exercise, get on the bed, and immediately fall asleep. Never leave fatigue and stress until the next day. Doctors say I&apos;m normal.</source>
+        <translation>我的名字是吉良吉影，年齡33歲，家住杜王町東北部別墅區，未婚。我在“龜友百貨連鎖公司”上班，每天最晚也是八點前回家，不吸菸，酒也是淺嚐輒止，晚上十一點上床，保証八個小時的充足睡眠，睡前喝一盃熱牛奶，再做二十分鐘伸展運動暖身，然後再睡覺，基本可以熟睡到天亮。像嬰兒一樣不留下疲勞與壓力，迎來第二天的早晨，健康檢查結果也顯示我很健康。我的意思是我是一個隨時都想追求平靜生活的人，不拘泥於勝負與煩惱，不樹立令我夜不能寐的敵人，這就是我對於這個社會的生活態度，我也清楚這就是我的幸福。</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="213"/>
+        <source>Lesson 3</source>
+        <translation>Lesson 3</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="213"/>
+        <source>Believe in the spin, just keep believing!</source>
+        <translation>相信回旋吧，隻管相信就是了！</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="225"/>
+        <source>With respect, let&apos;s advance towards a new stage of the spin.</source>
+        <translation>表達敬意吧，表達出敬意，然後邁向回旋的另一個全新階段！</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="247"/>
+        <source>An error message which won&apos;t disappear automatically.</source>
+        <translation>一條不會自動消失的消息。</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="76"/>
+        <source>The Anthem of man is the Anthem of courage.</source>
+        <translation>人類的讚歌就是勇氣的讚歌！</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="121"/>
+        <source>Action</source>
+        <translation></translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="132"/>
+        <source>Top right</source>
+        <translation>右上角</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="133"/>
+        <source>Top</source>
+        <translation>頂部居中</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="134"/>
+        <source>Top left</source>
+        <translation>左上角</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="135"/>
+        <source>Bottom right</source>
+        <translation>右下角</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="136"/>
+        <source>Bottom</source>
+        <translation>底部居中</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="137"/>
+        <source>Bottom left</source>
+        <translation>左下角</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="258"/>
+        <source>Lesson 1</source>
+        <translation>Lesson 1</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="258"/>
+        <source>Don&apos;t have any strange expectations of me.</source>
+        <translation>別對我抱有什麼奇怪的期待</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="269"/>
+        <source>Lesson 2</source>
+        <translation>Lesson 2</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="269"/>
+        <source>Don&apos;t let your muscles notice.</source>
+        <translation>不要讓肌肉察覺</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="163"/>
+        <source>An indeterminate progress bar</source>
+        <translation>不確定進度條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="173"/>
+        <source>An determinate progress bar</source>
+        <translation>確定的進度條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="192"/>
+        <source>An determinate progress ring</source>
+        <translation>確定的圓形進度條</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="236"/>
+        <source>The shortest shortcut is to take a detour.</source>
+        <translation>最短的捷徑就是繞遠路，繞遠路才是我的最短捷徑。</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="182"/>
+        <source>An indeterminate progress ring</source>
+        <translation>不確定進度環</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="69"/>
+        <source>InfoBadge in different styles</source>
+        <translation>不同樣式的徽章</translation>
+    </message>
+    <message>
+        <location filename="../../view/status_info_interface.py" line="39"/>
+        <source>A button with a simple ToolTip</source>
+        <translation>帶工具提示的按鈕</translation>
+    </message>
+</context>
+<context>
+    <name>TabInterface</name>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="148"/>
+        <source>IsTabMovable</source>
+        <translation>啟用標籤拖拽</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="149"/>
+        <source>IsTabScrollable</source>
+        <translation>啟用標籤滾動</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="153"/>
+        <source>TabCloseButtonDisplayMode</source>
+        <translation>關閉按鈕顯示模式</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="175"/>
+        <source>Always</source>
+        <translation>始終顯示</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="176"/>
+        <source>OnHover</source>
+        <translation>進入時顯示</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="177"/>
+        <source>Never</source>
+        <translation>從不顯示</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="180"/>
+        <source>Song</source>
+        <translation>歌曲</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="182"/>
+        <source>Album</source>
+        <translation>專輯</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="184"/>
+        <source>Artist</source>
+        <translation>歌手</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="150"/>
+        <source>IsTabShadowEnabled</source>
+        <translation>啟用標籤陰影</translation>
+    </message>
+    <message>
+        <location filename="../../view/navigation_view_interface.py" line="151"/>
+        <source>TabMaximumWidth</source>
+        <translation>標籤最大寬度</translation>
+    </message>
+</context>
+<context>
+    <name>TableFrame</name>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Title</source>
+        <translation>標題</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Artist</source>
+        <translation>歌手</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Album</source>
+        <translation>專輯</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Year</source>
+        <translation>年份</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="164"/>
+        <source>Duration</source>
+        <translation>時長</translation>
+    </message>
+</context>
+<context>
+    <name>TextInterface</name>
+    <message>
+        <location filename="../../view/text_interface.py" line="25"/>
+        <source>ko no dio da！</source>
+        <translation></translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="27"/>
+        <source>A LineEdit with a clear button</source>
+        <translation>帶清空按鈕的輸入框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="87"/>
+        <source>A DoubleSpinBox with a spin button</source>
+        <translation>浮點數旋轉框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="94"/>
+        <source>A DateEdit with a spin button</source>
+        <translation>日期編輯框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="101"/>
+        <source>A TimeEdit with a spin button</source>
+        <translation>時間編輯框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="108"/>
+        <source>A DateTimeEdit with a spin button</source>
+        <translation>日期時間編輯框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="80"/>
+        <source>A SpinBox with a spin button</source>
+        <translation>旋轉框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="119"/>
+        <source>A simple TextEdit</source>
+        <translation>富文本框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="63"/>
+        <source>A autosuggest line edit</source>
+        <translation>帶補全功能的輸入框</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="35"/>
+        <source>Type a stand name</source>
+        <translation>輸入替身名稱</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="72"/>
+        <source>Enter you password</source>
+        <translation>請輸入密碼</translation>
+    </message>
+    <message>
+        <location filename="../../view/text_interface.py" line="73"/>
+        <source>A password line edit</source>
+        <translation>密碼輸入框</translation>
+    </message>
+</context>
+<context>
+    <name>ToolBar</name>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="42"/>
+        <source>Documentation</source>
+        <translation>在線文檔</translation>
+    </message>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="44"/>
+        <source>Source</source>
+        <translation>源代碼</translation>
+    </message>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="81"/>
+        <source>Toggle theme</source>
+        <translation>切換主題</translation>
+    </message>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="83"/>
+        <source>Send feedback</source>
+        <translation>提供反饋</translation>
+    </message>
+    <message>
+        <location filename="../../view/gallery_interface.py" line="82"/>
+        <source>Support me</source>
+        <translation>支持作者🥰</translation>
+    </message>
+</context>
+<context>
+    <name>Translator</name>
+    <message>
+        <location filename="../../common/translator.py" line="19"/>
+        <source>Basic input</source>
+        <translation>基本輸入</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="16"/>
+        <source>Material</source>
+        <translation>材料</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="20"/>
+        <source>Status &amp; info</source>
+        <translation>狀態和信息</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="15"/>
+        <source>Scrolling</source>
+        <translation>滾動</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="13"/>
+        <source>Layout</source>
+        <translation>佈局</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="9"/>
+        <source>Text</source>
+        <translation>文本</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="12"/>
+        <source>Icons</source>
+        <translation>圖標</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="10"/>
+        <source>View</source>
+        <translation>視圖</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="17"/>
+        <source>Date &amp; time</source>
+        <translation>日期和時間</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="18"/>
+        <source>Navigation</source>
+        <translation>導航</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="18"/>
+        <source>Price</source>
+        <translation>價格方案</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="14"/>
+        <source>Dialogs &amp; flyouts</source>
+        <translation>對話框和彈出窗口</translation>
+    </message>
+    <message>
+        <location filename="../../common/translator.py" line="11"/>
+        <source>Menus &amp; toolbars</source>
+        <translation>菜單和工具欄</translation>
+    </message>
+</context>
+<context>
+    <name>TreeFrame</name>
+    <message>
+        <location filename="../../view/view_interface.py" line="122"/>
+        <source>JoJo 1 - Phantom Blood</source>
+        <translation>JoJo 1 - 幻影之血</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="123"/>
+        <source>Jonathan Joestar</source>
+        <translation>喬納森·喬斯達</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="123"/>
+        <source>Dio Brando</source>
+        <translation>迪奧·佈蘭度</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="123"/>
+        <source>Will A. Zeppeli</source>
+        <translation>威廉·A·齊貝林</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="130"/>
+        <source>JoJo 3 - Stardust Crusaders</source>
+        <translation>JoJo3 - 星塵鬥士</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="131"/>
+        <source>Jotaro Kujo</source>
+        <translation></translation>
+    </message>
+</context>
+<context>
+    <name>ViewInterface</name>
+    <message>
+        <location filename="../../view/view_interface.py" line="40"/>
+        <source>A simple TreeView</source>
+        <translation>簡單的樹狀組件</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="47"/>
+        <source>A TreeView with Multi-selection enabled</source>
+        <translation>啟用複選框的樹狀組件</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="32"/>
+        <source>A simple TableView</source>
+        <translation>簡單的表格組件</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="25"/>
+        <source>A simple ListView</source>
+        <translation>簡單的列表組件</translation>
+    </message>
+    <message>
+        <location filename="../../view/view_interface.py" line="61"/>
+        <source>Flip view</source>
+        <translation>翻轉視圖</translation>
+    </message>
+</context>
+</TS>
diff --git a/tool/FusionFlowFluent/app/view/main_window.py b/tool/FusionFlowFluent/app/view/main_window.py
new file mode 100644
index 0000000..acd8959
--- /dev/null
+++ b/tool/FusionFlowFluent/app/view/main_window.py
@@ -0,0 +1,157 @@
+# coding: utf-8
+from typing import List
+from PySide6.QtCore import Qt, Signal, QEasingCurve, QUrl, QSize, QTimer
+from PySide6.QtGui import QIcon, QDesktopServices, QColor
+from PySide6.QtWidgets import QApplication, QHBoxLayout, QFrame, QWidget
+
+from qfluentwidgets import (NavigationAvatarWidget, NavigationItemPosition, MessageBox, FluentWindow,
+                            SplashScreen, SystemThemeListener, isDarkTheme)
+from qfluentwidgets import FluentIcon as FIF
+
+from .gallery_interface import GalleryInterface
+from .home_interface import HomeInterface
+from .basic_input_interface import BasicInputInterface
+from .date_time_interface import DateTimeInterface
+from .dialog_interface import DialogInterface
+from .layout_interface import LayoutInterface
+from .icon_interface import IconInterface
+from .material_interface import MaterialInterface
+from .menu_interface import MenuInterface
+from .navigation_view_interface import NavigationViewInterface
+from .scroll_interface import ScrollInterface
+from .status_info_interface import StatusInfoInterface
+from .setting_interface import SettingInterface
+from .text_interface import TextInterface
+from .view_interface import ViewInterface
+from ..common.config import ZH_SUPPORT_URL, EN_SUPPORT_URL, cfg
+from ..common.icon import Icon
+from ..common.signal_bus import signalBus
+from ..common.translator import Translator
+from ..common import resource
+
+
+class MainWindow(FluentWindow):
+
+    def __init__(self):
+        super().__init__()
+        self.initWindow()
+
+        # create system theme listener
+        self.theme_listener = SystemThemeListener(self)
+
+        # create sub interface
+        self.home_interface = HomeInterface(self)
+        self.icon_interface = IconInterface(self)
+        self.basicInputInterface = BasicInputInterface(self)
+        self.dateTimeInterface = DateTimeInterface(self)
+        self.dialogInterface = DialogInterface(self)
+        self.layoutInterface = LayoutInterface(self)
+        self.menuInterface = MenuInterface(self)
+        self.materialInterface = MaterialInterface(self)
+        self.navigationViewInterface = NavigationViewInterface(self)
+        self.scrollInterface = ScrollInterface(self)
+        self.statusInfoInterface = StatusInfoInterface(self)
+        self.settingInterface = SettingInterface(self)
+        self.textInterface = TextInterface(self)
+        self.viewInterface = ViewInterface(self)
+
+        # enable acrylic effect
+        self.navigationInterface.setAcrylicEnabled(True)
+
+        self.connectSignalToSlot()
+
+        # add items to navigation interface
+        self.initNavigation()
+        self.splashScreen.finish()
+
+        # start theme listener
+        self.theme_listener.start()
+
+    def connectSignalToSlot(self):
+        signalBus.micaEnableChanged.connect(self.setMicaEffectEnabled)
+        signalBus.switchToSampleCard.connect(self.switchToSample)
+        signalBus.supportSignal.connect(self.onSupport)
+
+    def initNavigation(self):
+        # add navigation items
+        t = Translator()
+        self.addSubInterface(self.home_interface, FIF.HOME, self.tr('Home'))
+        self.addSubInterface(self.icon_interface, Icon.EMOJI_TAB_SYMBOLS, t.icons)
+        self.navigationInterface.addSeparator()
+
+        pos = NavigationItemPosition.SCROLL
+        self.addSubInterface(self.basicInputInterface, FIF.CHECKBOX,t.basicInput, pos)
+        self.addSubInterface(self.dateTimeInterface, FIF.DATE_TIME, t.dateTime, pos)
+        self.addSubInterface(self.dialogInterface, FIF.MESSAGE, t.dialogs, pos)
+        self.addSubInterface(self.layoutInterface, FIF.LAYOUT, t.layout, pos)
+        self.addSubInterface(self.materialInterface, FIF.PALETTE, t.material, pos)
+        self.addSubInterface(self.menuInterface, Icon.MENU, t.menus, pos)
+        self.addSubInterface(self.navigationViewInterface, FIF.MENU, t.navigation, pos)
+        self.addSubInterface(self.scrollInterface, FIF.SCROLL, t.scroll, pos)
+        self.addSubInterface(self.statusInfoInterface, FIF.CHAT, t.statusInfo, pos)
+        self.addSubInterface(self.textInterface, Icon.TEXT, t.text, pos)
+        self.addSubInterface(self.viewInterface, Icon.GRID, t.view, pos)
+
+        # add custom widget to bottom
+        self.navigationInterface.addItem(
+            routeKey='price',
+            icon=Icon.PRICE,
+            text=t.price,
+            onClick=self.onSupport,
+            selectable=False,
+            tooltip=t.price,
+            position=NavigationItemPosition.BOTTOM
+        )
+        self.addSubInterface(
+            self.settingInterface, FIF.SETTING, self.tr('Settings'), NavigationItemPosition.BOTTOM)
+
+    def initWindow(self):
+        self.resize(960, 780)
+        self.setMinimumWidth(760)
+        self.setWindowIcon(QIcon(':/gallery/images/logo.png'))
+        self.setWindowTitle('PyQt-Fluent-Widgets')
+
+        self.setMicaEffectEnabled(cfg.get(cfg.mica_enabled))
+
+        # create splash screen
+        self.splashScreen = SplashScreen(self.windowIcon(), self)
+        self.splashScreen.setIconSize(QSize(106, 106))
+        self.splashScreen.raise_()
+
+        desktop = QApplication.screens()[0].availableGeometry()
+        w, h = desktop.width(), desktop.height()
+        self.move(w//2 - self.width()//2, h//2 - self.height()//2)
+        self.show()
+        QApplication.processEvents()
+
+    def onSupport(self):
+        language = cfg.get(cfg.language).value
+        if language.name() == "zh_CN":
+            QDesktopServices.openUrl(QUrl(ZH_SUPPORT_URL))
+        else:
+            QDesktopServices.openUrl(QUrl(EN_SUPPORT_URL))
+
+    def resizeEvent(self, e):
+        super().resizeEvent(e)
+        if hasattr(self, 'splashScreen'):
+            self.splashScreen.resize(self.size())
+
+    def closeEvent(self, e):
+        self.theme_listener.terminate()
+        self.theme_listener.deleteLater()
+        super().closeEvent(e)
+
+    def _onThemeChangedFinished(self):
+        super()._onThemeChangedFinished()
+
+        # retry
+        if self.isMicaEffectEnabled():
+            QTimer.singleShot(100, lambda: self.windowEffect.setMicaEffect(self.winId(), isDarkTheme()))
+
+    def switchToSample(self, routeKey, index):
+        """ switch to sample """
+        interfaces = self.findChildren(GalleryInterface)
+        for w in interfaces:
+            if w.objectName() == routeKey:
+                self.stackedWidget.setCurrentWidget(w, False)
+                w.scrollToCard(index)
diff --git a/tool/FusionFlowFluent/fusion_flow.py b/tool/FusionFlowFluent/fusion_flow.py
new file mode 100644
index 0000000..1498db0
--- /dev/null
+++ b/tool/FusionFlowFluent/fusion_flow.py
@@ -0,0 +1,36 @@
+# coding:utf-8
+import os
+import sys
+
+from PySide6.QtCore import Qt, QTranslator
+from PySide6.QtGui import QFont
+from PySide6.QtWidgets import QApplication
+from qfluentwidgets import FluentTranslator
+
+from app.common.config import cfg
+from app.view.main_window import MainWindow
+
+if __name__ == "__main__":
+    # enable dpi scale
+    if cfg.get(cfg.dpi_scale) != "Auto":
+        os.environ["QT_ENABLE_HIGHDPI_SCALING"] = "0"
+        os.environ["QT_SCALE_FACTOR"] = str(cfg.get(cfg.dpi_scale))
+
+    # create application
+    app = QApplication(sys.argv)
+    app.setAttribute(Qt.AA_DontCreateNativeWidgetSiblings)
+
+    # internationalization
+    locale = cfg.get(cfg.language).value
+    translator = FluentTranslator(locale)
+    gallery_translator = QTranslator()
+    gallery_translator.load(locale, "gallery", ".", ":/gallery/i18n")
+
+    app.installTranslator(translator)
+    app.installTranslator(gallery_translator)
+
+    # create main window
+    main_window = MainWindow()
+    main_window.show()
+
+    sys.exit(app.exec())
\ No newline at end of file
```
