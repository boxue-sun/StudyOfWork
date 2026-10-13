# 06 · RViz 可视化：把机器人"内部数据"看个明白

> 对应简历：**第 4 条**（建模与仿真：RViz 可视化）。
> 这一篇讲清楚：RViz 不是"仿真器"，而是"观察窗口"——它把 ROS 话题里的数据画成你能看的图。你项目里 RViz 配置已经写好了，直接开就行。

---

## 0. 一句话先记住

**Gazebo 让机器人"跑"，RViz 让你"看"。** RViz 本身不产生数据，它只是把 `/scan`、`/map`、`/tf` 这些话题画成激光点、地图、坐标系箭头。

---

## 1. 你项目里 RViz 怎么开

两个启动文件各带一个 RViz 配置（都是 `.rviz` 文件，里面记着要显示哪些项）：

- `ros2 launch diff_drive_robot robot.launch.py` → 自动开 `rviz/bot.rviz`（看机器人模型 + 激光 + 坐标系）。
- `ros2 launch diff_drive_robot navigation.launch.py` → 带 `--rviz true` 才开 `rviz/nav2.rviz`（额外看规划路径）。**默认是关的**（`declare_rviz` 默认 `false`），因为虚拟机开 RViz 挺吃资源，跑导航时通常先关掉保流畅。

所以你不用从零配 RViz——配置已经随 launch 带好了。这篇的价值是：**知道每个显示项对应什么，出问题能看懂。**

---

## 2. 你项目该看哪几项（对照着勾）

| RViz 显示项 | 看的是哪个话题 | 它告诉你什么 |
|---|---|---|
| **Map** | `/map` | 预置的展厅地图（导航的"底图"） |
| **RobotModel** | 读 URDF | 机器人长啥样、各零件位置 |
| **LaserScan** | `/scan` | 激光雷达看到的障碍（一圈点） |
| **Path（全局）** | `/plan` | Nav2 规划的全局路线（从你到展点） |
| **Path（局部）** | `/local_plan` | 小车当前这一小段怎么走 |
| **TF** | `/tf` | 坐标系树（map/odom/base_link…见 04 篇） |
| **PoseArray / Marker** | 你的展点标记 | 展点在地图上的位置（tour_points 可视化） |
| **Camera**（可选） | `/camera/image_raw` | 摄像头画面（迎宾视觉检测时看） |

---

## 3. 一个容易踩的坑：QoS（数据有效期）

地图 `/map` 和激光 `/scan` 用的是 **transient local（留存式）** QoS——意思是"晚来的订阅者也能拿到之前发过的数据"。
如果你的 RViz 比地图节点**晚启动**，普通 QoS 会啥也收不到（错过的就没了），而 transient local 会把历史地图补发给你。
所以你项目里 map_server / 激光的 QoS 是特意这么配的（看 `nav2_params.yaml` 里 `map_subscribe_transient_local: True`）。你只要知道"为什么有时 RViz 里地图是空的"——多半是 QoS 或启动顺序问题，不是代码错。

---

## 4. 动手体验

```bash
# 终端1：起机器人 + RViz（bot.rviz 会自己弹出来）
ros2 launch diff_drive_robot robot.launch.py
# 终端2：起导航（想要看规划路径时加 --rviz true）
ros2 launch diff_drive_robot navigation.launch.py --rviz true
```
然后看 RViz 里：机器人模型站起来了、激光一圈点扫出来、发导航后 `/plan` 画出一条线。这就是"数据被看见"的全过程。

---

## 5. 对应简历怎么答

被问"RViz 干嘛的" →
> "RViz 是 ROS 的可视化工具，我把地图、激光、规划路径、坐标系树和展点标记显示出来，用来观察调试导航和导览流程。它不是仿真器，Gazebo 才是跑物理的，RViz 只负责'看'。"

---

## 6. 别踩坑（诚实底线）

- ❌ 别把 RViz 和 Gazebo 搞混：Gazebo 跑物理（车真在动），RViz 只是显示（你看的窗口）。
- ✅ 你能"配置/使用 RViz 观察调试"是真实的，但 RViz 不是你写的工具，照实说。
