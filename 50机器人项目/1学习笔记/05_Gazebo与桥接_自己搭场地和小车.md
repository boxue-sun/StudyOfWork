# 第五课：Gazebo 与桥接——自己搭一个小场地，让仿真小车动

这一课不直接启动你的大项目。我们自己写一个 SDF 文件，建立地板、展台和两轮小车，再用 ROS2 发速度，观察里程计变化。

需要 Ubuntu 24.04、ROS2 Jazzy、Gazebo Harmonic 和 `ros_gz_bridge`。终端练习使用没有渲染传感器的服务端仿真，不要求桌面；看 3D 窗口是可选步骤。

## 0．先知道三份代码各管什么

| 文件 | 干什么 |
|---|---|
| `mini_hall.sdf` | 描述场地、小车几何、碰撞、质量、车轮关节和差速插件 |
| `bridge.yaml` | 告诉桥接哪些消息从 ROS2 到 Gazebo、哪些反过来 |
| `drive_once.py` | 等通信准备好，发速度行驶 3 秒，再发零速度 |

`SDF` 是仿真描述格式，不是 Python 程序。Gazebo 读取它创建实体；你不用自己写物理引擎。`visual` 决定看起来是什么形状，`collision` 决定碰撞形状，`inertial` 提供质量和惯性。

本课先不加雷达和摄像头，也不接 Nav2。速度通信跑通后，再理解原项目多出的传感器和导航。

## 1．检查现有环境

```bash
source /opt/ros/jazzy/setup.bash
command -v gz
ros2 pkg prefix ros_gz_bridge
gz sim --versions
```

Gazebo Harmonic 对应 Gazebo Sim 8。若缺少软件，先按《00_先看这里》确认系统和安装条件；不要在未知版本的服务器直接复制其他发行版的安装命令。

## 2．创建目录和 Python 包

```bash
mkdir -p ~/ros2_lesson5_ws/src
cd ~/ros2_lesson5_ws/src
ros2 pkg create lesson_gazebo_demo --build-type ament_python \
  --license Apache-2.0 --dependencies rclpy geometry_msgs nav_msgs
cd ~/ros2_lesson5_ws
nano mini_hall.sdf
```

复制整个 SDF 文件：

```xml
<?xml version="1.0"?>
<sdf version="1.9">
  <world name="lesson_hall">
    <physics name="basic" type="ignored">
      <max_step_size>0.005</max_step_size>
      <real_time_factor>1.0</real_time_factor>
    </physics>
    <plugin filename="gz-sim-physics-system" name="gz::sim::systems::Physics"/>
    <plugin filename="gz-sim-user-commands-system" name="gz::sim::systems::UserCommands"/>
    <plugin filename="gz-sim-scene-broadcaster-system" name="gz::sim::systems::SceneBroadcaster"/>
    <light name="sun" type="directional">
      <pose>0 0 5 0 0 0</pose>
      <diffuse>0.8 0.8 0.8 1</diffuse>
      <direction>-0.3 0.2 -1</direction>
    </light>
    <model name="floor">
      <static>true</static>
      <pose>0 0 -0.05 0 0 0</pose>
      <link name="floor_link">
        <collision name="collision"><geometry><box><size>8 6 0.1</size></box></geometry></collision>
        <visual name="visual"><geometry><box><size>8 6 0.1</size></box></geometry>
          <material><diffuse>0.7 0.7 0.7 1</diffuse></material>
        </visual>
      </link>
    </model>
    <model name="exhibit">
      <static>true</static>
      <pose>2 1 0.4 0 0 0</pose>
      <link name="exhibit_link">
        <collision name="collision"><geometry><box><size>0.6 0.6 0.8</size></box></geometry></collision>
        <visual name="visual"><geometry><box><size>0.6 0.6 0.8</size></box></geometry>
          <material><diffuse>0.3 0.6 0.3 1</diffuse></material>
        </visual>
      </link>
    </model>
    <model name="lesson_car">
      <pose>0 0 0 0 0 0</pose>
      <link name="base_link">
        <pose>0 0 0.18 0 0 0</pose>
        <inertial><mass>5</mass><inertia><ixx>0.03</ixx><iyy>0.08</iyy><izz>0.1</izz></inertia></inertial>
        <collision name="body_collision"><geometry><box><size>0.4 0.24 0.1</size></box></geometry></collision>
        <visual name="body_visual"><geometry><box><size>0.4 0.24 0.1</size></box></geometry>
          <material><diffuse>0.2 0.3 0.9 1</diffuse></material>
        </visual>
      </link>
      <link name="left_wheel">
        <pose>0 0.16 0.1 0 0 0</pose>
        <inertial><mass>0.5</mass><inertia><ixx>0.003</ixx><iyy>0.003</iyy><izz>0.003</izz></inertia></inertial>
        <collision name="wheel_collision"><pose>0 0 0 1.5707963 0 0</pose>
          <geometry><cylinder><radius>0.1</radius><length>0.04</length></cylinder></geometry>
        </collision>
        <visual name="wheel_visual"><pose>0 0 0 1.5707963 0 0</pose>
          <geometry><cylinder><radius>0.1</radius><length>0.04</length></cylinder></geometry>
          <material><diffuse>0.1 0.1 0.1 1</diffuse></material>
        </visual>
      </link>
      <link name="right_wheel">
        <pose>0 -0.16 0.1 0 0 0</pose>
        <inertial><mass>0.5</mass><inertia><ixx>0.003</ixx><iyy>0.003</iyy><izz>0.003</izz></inertia></inertial>
        <collision name="wheel_collision"><pose>0 0 0 1.5707963 0 0</pose>
          <geometry><cylinder><radius>0.1</radius><length>0.04</length></cylinder></geometry>
        </collision>
        <visual name="wheel_visual"><pose>0 0 0 1.5707963 0 0</pose>
          <geometry><cylinder><radius>0.1</radius><length>0.04</length></cylinder></geometry>
          <material><diffuse>0.1 0.1 0.1 1</diffuse></material>
        </visual>
      </link>
      <link name="caster">
        <pose>-0.16 0 0.045 0 0 0</pose>
        <inertial><mass>0.1</mass><inertia><ixx>0.0001</ixx><iyy>0.0001</iyy><izz>0.0001</izz></inertia></inertial>
        <collision name="caster_collision"><geometry><sphere><radius>0.045</radius></sphere></geometry>
          <surface><friction><ode><mu>0.01</mu><mu2>0.01</mu2></ode></friction></surface>
        </collision>
        <visual name="caster_visual"><geometry><sphere><radius>0.045</radius></sphere></geometry></visual>
      </link>
      <joint name="left_joint" type="revolute">
        <parent>base_link</parent><child>left_wheel</child><axis><xyz>0 1 0</xyz></axis>
      </joint>
      <joint name="right_joint" type="revolute">
        <parent>base_link</parent><child>right_wheel</child><axis><xyz>0 1 0</xyz></axis>
      </joint>
      <joint name="caster_joint" type="fixed"><parent>base_link</parent><child>caster</child></joint>
      <plugin filename="gz-sim-diff-drive-system" name="gz::sim::systems::DiffDrive">
        <left_joint>left_joint</left_joint><right_joint>right_joint</right_joint>
        <wheel_separation>0.32</wheel_separation><wheel_radius>0.1</wheel_radius>
        <topic>/lesson5/cmd_vel</topic>
        <odom_topic>/lesson5/odom</odom_topic><odom_publish_frequency>10</odom_publish_frequency>
        <tf_topic>/lesson5/tf</tf_topic><frame_id>lesson5_odom</frame_id><child_frame_id>lesson5_base</child_frame_id>
      </plugin>
    </model>
  </world>
</sdf>
```

保存退出。`mini_hall.sdf` 放在工作区根目录；Python 文件放在包的内层目录。不要把 XML 粘贴进 Python 文件。

## 3．创建桥接配置

```bash
nano ~/ros2_lesson5_ws/bridge.yaml
```

复制：

```yaml
- ros_topic_name: /lesson5/cmd_vel
  gz_topic_name: /lesson5/cmd_vel
  ros_type_name: geometry_msgs/msg/Twist
  gz_type_name: gz.msgs.Twist
  direction: ROS_TO_GZ
  lazy: false
- ros_topic_name: /lesson5/odom
  gz_topic_name: /lesson5/odom
  ros_type_name: nav_msgs/msg/Odometry
  gz_type_name: gz.msgs.Odometry
  direction: GZ_TO_ROS
  lazy: false
- ros_topic_name: /tf
  gz_topic_name: /lesson5/tf
  ros_type_name: tf2_msgs/msg/TFMessage
  gz_type_name: gz.msgs.Pose_V
  direction: GZ_TO_ROS
  lazy: false
```

YAML 的缩进必须保留。

| 配置 | 意思 |
|---|---|
| `ros_topic_name` / `gz_topic_name` | 两侧各叫什么名字 |
| `ros_type_name` / `gz_type_name` | 两侧使用的数据类型 |
| `ROS_TO_GZ` | 速度从你的 ROS2 程序送到仿真 |
| `GZ_TO_ROS` | 里程计和 TF 从仿真送到 ROS2 |
| `lazy: false` | 持续建立连接，不等出现订阅者才桥接 |

**Gazebo Transport 和 ROS2 的通信系统不同。** Gazebo 有数据不代表 ROS2 自动就能收到，要有桥接和正确的名称、类型、方向。

## 4．创建速度程序

```bash
nano ~/ros2_lesson5_ws/src/lesson_gazebo_demo/lesson_gazebo_demo/drive_once.py
```

复制保存：

```python
import time

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from rclpy.qos import qos_profile_sensor_data


class DriveOnce(Node):
    def __init__(self):
        super().__init__('lesson5_drive_once')
        self.publisher = self.create_publisher(Twist, '/lesson5/cmd_vel', 10)
        self.subscription = self.create_subscription(
            Odometry, '/lesson5/odom', self.on_odom, qos_profile_sensor_data)
        self.x = None
        self.y = None
        self.next_log = 0.0

    def on_odom(self, msg):
        self.x = msg.pose.pose.position.x
        self.y = msg.pose.pose.position.y

    def run_drive(self):
        deadline = time.monotonic() + 15.0
        while rclpy.ok() and (self.publisher.get_subscription_count() == 0 or self.x is None):
            if time.monotonic() >= deadline:
                self.get_logger().error('NOT_READY: check simulation and bridge')
                return 1
            rclpy.spin_once(self, timeout_sec=0.1)
        self.get_logger().info(f'DRIVE_START: x={self.x:.3f}, y={self.y:.3f}')
        started = time.monotonic()
        while rclpy.ok() and time.monotonic() - started < 3.0:
            command = Twist()
            command.linear.x = 0.15
            self.publisher.publish(command)
            rclpy.spin_once(self, timeout_sec=0.1)
            if time.monotonic() >= self.next_log:
                self.next_log = time.monotonic() + 1.0
                self.get_logger().info(f'DRIVING: x={self.x:.3f}, y={self.y:.3f}')
        self.stop()
        deadline = time.monotonic() + 0.5
        while rclpy.ok() and time.monotonic() < deadline:
            rclpy.spin_once(self, timeout_sec=0.05)
        self.get_logger().info(f'STOP_SENT: x={self.x:.3f}, y={self.y:.3f}')
        return 0

    def stop(self):
        if rclpy.ok():
            for _ in range(5):
                self.publisher.publish(Twist())
                rclpy.spin_once(self, timeout_sec=0.05)


def main(args=None):
    rclpy.init(args=args)
    node = DriveOnce()
    try:
        return node.run_drive()
    except KeyboardInterrupt:
        return 130
    finally:
        if rclpy.ok():
            node.stop()
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    import sys
    sys.exit(main())
```

再编辑外层 `setup.py`：

```bash
nano ~/ros2_lesson5_ws/src/lesson_gazebo_demo/setup.py
```

替换为：

```python
from setuptools import find_packages, setup

package_name = 'lesson_gazebo_demo'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='sbx',
    maintainer_email='student@example.com',
    description='Hands-on ROS2 learning demo',
    license='Apache-2.0',
    entry_points={
        'console_scripts': [
            'drive_once = lesson_gazebo_demo.drive_once:main',
        ],
    },
)
```

构建：

```bash
cd ~/ros2_lesson5_ws
source /opt/ros/jazzy/setup.bash
colcon build --packages-select lesson_gazebo_demo \
  --symlink-install --executor sequential
```

## 5．终端 A：先启动 Gazebo 服务端

每个用于本课的终端都要设置相同的 `GZ_PARTITION` 和 `ROS_DOMAIN_ID`。

```bash
source /opt/ros/jazzy/setup.bash
export ROS_DOMAIN_ID=92
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
export GZ_PARTITION=sbx_lesson5
cd ~/ros2_lesson5_ws
gz sim -s -r mini_hall.sdf
```

`-s` 只启动仿真服务端；`-r` 让仿真开始运行，不停在暂停状态。保持 A 运行。

此时场地和小车已存在，差速插件在发布里程计，但没有速度命令，小车不会主动前进。

## 6．终端 B：启动桥接

```bash
source /opt/ros/jazzy/setup.bash
export ROS_DOMAIN_ID=92
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
export GZ_PARTITION=sbx_lesson5
ros2 run ros_gz_bridge parameter_bridge --ros-args \
  -p config_file:=$HOME/ros2_lesson5_ws/bridge.yaml
```

保持 B 运行。你会看到创建桥接的日志。

如果 A、B 的 `GZ_PARTITION` 不同，Gazebo 侧发现不了对方；如果 ROS2 两端的 `ROS_DOMAIN_ID` 不同，ROS2 侧发现不了对方。两个分组管的是不同通信系统。

## 7．终端 C：启动你写的速度节点

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros2_lesson5_ws/install/setup.bash
export ROS_DOMAIN_ID=92
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
export GZ_PARTITION=sbx_lesson5
ros2 run lesson_gazebo_demo drive_once
```

出现 `DRIVE_START`，随后位置变化，最后出现 `STOP_SENT`。程序正常情况下发零速度后退出。

这里 3 秒按实际时间计算。仿真运行得慢时，车走的距离会比 `0.15 × 3 = 0.45 米` 小；不要仅根据这一个数字判断程序是否正确。

再次运行命令，会从当前位置再走一段。本课没有定位、路径规划和避障，展台只是场景里的实体。

**Gazebo 差速插件可能继续保持最后一条速度。** 本程序正常退出会发送零速度，强制结束进程则不保证停车。要直接结束本课仿真，在 A 按 `Ctrl+C`；随后结束 B。它们与原项目使用不同分组。

## 8．桌面可选：看 3D 小车

不要再启动第二个服务端。如果 A 已在运行，在 Ubuntu 桌面的终端 D 输入：

```bash
source /opt/ros/jazzy/setup.bash
export GZ_PARTITION=sbx_lesson5
gz sim -g
```

`-g` 只打开 GUI 客户端，连接现有服务端。应该能看到灰色地板、绿色展台和蓝色小车。再运行 C 的命令看它运动。

虚拟机如果 GUI 打不开或明显卡顿，关闭这个 GUI，保留终端里程计体验。原项目的大场景已有低负载 RViz 方案，不需要强求两个 3D 窗口同时运行。

## 9．先理解 SDF 中五个地方

- `<world>`：场景容器，内部放物理配置、地板、展台、小车。
- `<model>`：一个模型；`static=true` 的地板、展台固定不动。
- `<pose>`：位置和姿态，通常写成 `x y z roll pitch yaw`。
- `<link>` 与 `<joint>`：车体、轮子是 link；连接和可运动方向由 joint 指定。
- `DiffDrive` 插件：接收线速度与角速度，换算成左右轮运动，并发布里程计。

`wheel_radius=0.1`、`wheel_separation=0.32` 要与模型尺寸一致。平面差速车的理想轮速关系是：

```text
左轮角速度 = (前进速度 - 转向角速度 × 轮距 / 2) / 轮半径
右轮角速度 = (前进速度 + 转向角速度 × 轮距 / 2) / 轮半径
```

你填这些参数，插件完成换算。本课没有自行实现驱动算法。

## 10．改场地、体验转弯

先关闭 C 和 A。把展台的 `<pose>2 1 0.4 ...</pose>` 改成 `<pose>2 -1 0.4 ...</pose>`，保存、重启 A。B 可保持运行，等待重新发现。

再将 Python 速度程序里的：

```python
command.linear.x = 0.15
```

改为：

```python
command.linear.x = 0.15
command.angular.z = 0.3
```

保存、重新运行 C。你应看到 x、y 都变化，小车走弧线。这次是你自己写的速度节点决定速度，Nav2 没参与。

改变 SDF 后需要重启仿真，已有实体不会仅因磁盘文件改变就更新。改 Python 源码要重启对应进程。

## 11．对应原项目与排错

原项目的 `tour_hall.sdf` 定义展厅，URDF 定义机器人结构，`robot.gazebo` 添加仿真插件和传感器。`robot.launch.py` 启动世界、生成机器人、启动桥接等。启动脚本加载已有模型，不是临时绘制地图或执行 SLAM。

本课 `/lesson5/odom` 是车轮运动推算的里程计，不是 AMCL 的地图定位；原项目还需要雷达和 AMCL。碰撞实体不等于 Nav2 已实现避障。

| 卡点 | 先查 |
|---|---|
| `NOT_READY` | A 是否运行、B 是否创建桥接，两组通信编号是否一致 |
| XML/SDF 报错 | 是否完整复制，标签是否闭合，是否用了 Harmonic |
| 有里程计却不动 | 速度是否被桥接，DiffDrive 关节名称与模型是否一致 |
| 位置变化但没窗口 | 服务端模式本来没有窗口，按第 8 步连接 GUI |
| GUI 黑屏或崩溃 | 虚拟显示/显卡问题与 Topic 通信分开排查，先完成终端练习 |

回答：场地是谁创建的？桥接是否计算速度？只有 Gazebo 为什么没有自动导览？`-s` 与 `-g` 各启动什么？

验证范围见附录。参考：[Gazebo Harmonic 差速驱动说明](https://gazebosim.org/docs/harmonic/moving_robot/)、[ROS2 与 Gazebo 桥接说明](https://gazebosim.org/docs/harmonic/ros2_integration/)。
