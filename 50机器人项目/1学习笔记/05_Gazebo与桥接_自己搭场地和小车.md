# 05 · Gazebo 与桥接：在没有真车的情况下把机器人"跑"起来

> 对应简历：**第 4 条**（建模与仿真：Gazebo Harmonic 仿真 + ros_gz_bridge 桥接激光/里程计 + Xvfb headless）。
> 这一篇讲清楚：你的车和世界是"虚拟"的，但数据是真的——靠 `ros_gz_bridge` 这座桥，把仿真数据变成你 Python 节点能订阅的 ROS 话题。

---

## 0. 一句话先记住

**Gazebo 是个"虚拟物理实验室"，把机器人模型和展厅放进去跑；`ros_gz_bridge` 是翻译官，把 Gazebo 说的话变成 ROS 话题，你的节点才能听懂。**

---

## 1. 你项目里这块在哪儿、长啥样

两个文件最关键：

**① 世界文件 `diff_drive_robot/worlds/tour_hall.sdf`**
这就是你的"展厅"——纯几何体（墙、地面、展位），不需要外部 3D 模型。你机器人在里面巡游。`robot.launch.py` 默认就加载它：
```python
world_path = os.path.join(pkg, 'worlds', 'tour_hall.sdf')   # robot.launch.py 第 24–26 行
```

**② 启动文件 `diff_drive_robot/launch/robot.launch.py`** 干四件事：
1. 起 Gazebo 物理服务（`gz_sim.launch.py`，参数 `-r -s` 表示跑服务不卡 GUI）；
2. 按需起 Gazebo 图形界面（`headless:=true` 时关掉，虚拟机更流畅）；
3. 把机器人生成进世界（`ros_gz_sim create`，初始位姿默认 `x:0, y:-3.5`，正好在"接待台"）；
4. 起 **`ros_gz_bridge`**，加载桥接配置。

---

## 2. 桥接配置：Gazebo 和 ROS 怎么"对话"

打开 `diff_drive_robot/config/gz_bridge.yaml`，每一行就是一座"单向桥"：

| ROS 话题 | Gazebo 话题 | 方向 | 是什么 |
|---|---|---|---|
| `/clock` | `clock` | GZ→ROS | 仿真时钟 |
| `/tf` | `tf` | GZ→ROS | 坐标系（odom→base_link 由 Gazebo 发） |
| `/joint_states` | `joint_states` | GZ→ROS | 轮子转角 |
| `/odom` | `odom` | GZ→ROS | 里程计（车走了多远） |
| `/cmd_vel` | `cmd_vel` | **ROS→GZ** | ⚠️ 反向：你发速度指令，Gazebo 让车轮转 |
| `/imu` | `imu` | GZ→ROS | 惯性数据 |
| `/scan` | `scan` | GZ→ROS | **激光雷达**（导航最关键的输入） |
| `/camera/image_raw` | `camera/image` | GZ→ROS | 摄像头图（迎宾视觉检测用） |

记住一个关键点：**绝大多数桥是 GZ→ROS（仿真产生数据给你）**，只有 `/cmd_vel` 是 **ROS→GZ（你下指令驱动车）**。这就是"你写 Python 节点发 `/cmd_vel`，虚拟车就动"的原理。

---

## 3. 动手体验：发一条速度让车动

在 ROS2 + Gazebo 都起好的环境里，直接发速度指令（不用写代码）：
```bash
# 先 ros2 launch diff_drive_robot robot.launch.py 把世界和桥起好
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist \
  "{linear: {x: 0.1}, angular: {z: 0.0}}" --rate 10
```
车就会以 0.1 m/s 往前挪。这就是 `cmd_vel` 这座反向桥在起作用。原笔记里 `drive_once.py` 干的也是这事，只是写成了 Python 脚本。

---

## 4. Xvfb 是什么（简历提到的 headless）

Gazebo 默认要一个"显示器"才能跑图形。虚拟机/服务器常常没屏幕，就用 **Xvfb** 造一个"虚拟显示器"，让 Gazebo 无界面（headless）也能跑。你启动时常加 `headless:=true` 就是这个意思——省资源、不弹窗。

---

## 5. 对应简历怎么答

被问"Gazebo / ros_gz_bridge / Xvfb 干嘛的" →
> "我在 Gazebo Harmonic 里建了机器人模型和展厅世界，用 ros_gz_bridge 把仿真里的激光、里程计等数据桥接成 ROS 话题，让我的 Python 节点能订阅；用 Xvfb 在无界面服务器上 headless 跑仿真。仿真环境是我搭起来并跑通的，但 Gazebo 本身不是我写的。"

---

## 6. 别踩坑（诚实底线）

- ❌ 别吹"我写了仿真器/物理引擎"——Gazebo 是现成的，你只是**搭模型 + 配桥接 + 跑通**。
- ❌ 别吹"我建了精细 3D 展厅"——是 `tour_hall.sdf` 纯几何体（墙+地面），够导航用，不是美术级模型。
- ✅ 重点讲清楚：**桥接让仿真数据变成 ROS 话题，所以你的真实节点代码一套没改就能在仿真里跑。** 这是你项目"仿真驱动"的核心价值。
