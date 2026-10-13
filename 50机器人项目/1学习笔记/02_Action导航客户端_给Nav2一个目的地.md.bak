# ROS2 Python 第二课：自己写程序，让 Nav2 带小车去一个目的地

第一课你写了两个节点，通过 Topic 传位置和速度。这一课你亲手写一个 **导航 Action 客户端**，让原导览项目里的小车去指定位置。

全部命令都在 **Ubuntu 虚拟机** 输入。这次仍然由你自己生成包、建文件、复制代码和运行，我没有提前替你创建练习工作区。

有原项目和图形界面时先做到第 7 步；只有远程终端时，先读第 0、1 步，再直接做文末第 14 步。

先做到第 7 步，看到“目标被接受 → 途中反馈 → 到达”，再回头读代码。不要求你先看懂每一个函数。

## 0．这次到底写什么？

假设你对司机说：“带我去接待台旁边。”司机接单后开车，途中告诉你还有多远，最后告诉你到达了。

这里你写的程序是“乘客”，Nav2 是“司机”，Gazebo 里的小车执行实际运动。

```text
你写的 nav_client.py 节点
    │ 给目的地：map 坐标系里的 x、y、朝向
    ▼
Nav2 接收导航任务
    │ 结合定位、地图、雷达规划路线并计算速度
    ▼
/cmd_vel_nav → 速度平滑器 → /cmd_vel
    ▼
桥接 → Gazebo 差速驱动插件 → 小车运动

Nav2 同时给你的客户端返回：接受/拒绝、途中反馈、最终结果
```

**本课客户端不发布速度。** 第一课的 `move_to_x` 自己决定速度；这次我们把“怎么走”的工作交给现成的 Nav2，只写“去哪里”和“怎么看结果”。

## 1．Topic、Service、Action 先分清这一点

| 机制 | 好理解的例子 | 本项目里的用途 |
|---|---|---|
| Topic 话题 | 广播当前车速，接收者不断听 | 雷达、里程计、速度命令 |
| Service 服务 | 问一次，得到一次答复 | 查询或改变导航节点的生命周期状态 |
| Action 动作 | 提交一个耗时任务，有过程反馈和最终结果，还能请求取消 | 导航到目的地 |

`/navigate_to_pose` 是本课使用的 Action 名称；`NavigateToPose` 是它的数据类型，描述“目标、反馈、结果”分别有哪些字段。

不用自己创建这个类型，`nav2_msgs` 已经提供了。也不用自己写导航服务端，原项目中的 Nav2 已经提供了。

这里说的“客户端”不是浏览器，而是发起请求的程序；“服务端”是接收并执行请求的程序。本课只有你写的一个新节点。

## 2．自动生成一个 Python 包

如果第一课的两个练习节点还在运行，先在它们各自的终端按 `Ctrl+C` 结束练习。打开一个新 Ubuntu 终端，输入：

```bash
source /opt/ros/jazzy/setup.bash
mkdir -p ~/ros2_python_lesson2_ws/src
cd ~/ros2_python_lesson2_ws/src

ros2 pkg create nav_action_demo \
  --build-type ament_python \
  --license Apache-2.0 \
  --dependencies rclpy nav2_msgs action_msgs
```

`package.xml`、`setup.py`、`setup.cfg`、内层 Python 目录、`__init__.py` 等都由命令生成，**不用手写 package.xml**。

| 名称 | 用途 |
|---|---|
| `rclpy` | Python 创建 ROS2 节点、使用 Action 的库 |
| `nav2_msgs` | Nav2 定义的消息和 Action 类型 |
| `action_msgs` | Action 的通用状态，如成功、取消、失败 |
| `nav_action_demo` | 本次练习的包名 |

包只创建一次。如果提示目录已存在，检查已有文件并继续编辑，不要删除重建。

## 3．自己建立客户端文件，复制完整代码

输入：

```bash
cd ~/ros2_python_lesson2_ws/src/nav_action_demo
nano nav_action_demo/nav_client.py
```

注意有两层 `nav_action_demo`：外层是 ROS2 包，内层是 Python 模块目录。`nav_client.py` 放在内层，与自动生成的 `__init__.py` 同级。

复制下面全部代码。Python 缩进也要一起复制。

```python
import math
import sys
import time

import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from action_msgs.msg import GoalStatus
from nav2_msgs.action import NavigateToPose


class NavigationClient(Node):
    def __init__(self):
        super().__init__('learning_navigation_client')
        self.declare_parameter('x', 0.0)
        self.declare_parameter('y', -3.0)
        self.declare_parameter('yaw', 0.0)
        self.declare_parameter('cancel_after', 0.0)
        self.x = self.get_parameter('x').value
        self.y = self.get_parameter('y').value
        self.yaw = self.get_parameter('yaw').value
        self.cancel_after = self.get_parameter('cancel_after').value
        if not all(math.isfinite(v) for v in [self.x, self.y, self.yaw, self.cancel_after]):
            raise ValueError('Goal parameters must be finite numbers')
        if self.cancel_after < 0.0:
            raise ValueError('cancel_after must be >= 0')

        self.nav = ActionClient(self, NavigateToPose, '/navigate_to_pose')
        self.goal_handle = None
        self.cancel_timer = None
        self.last_feedback = 0.0

    def run_navigation(self):
        # 第一步：确认有程序提供导航 Action 服务。
        self.get_logger().info('WAIT_SERVER: waiting for Nav2')
        if not self.nav.wait_for_server(timeout_sec=10.0):
            self.get_logger().error('SERVER_NOT_READY: check Nav2 and ROS_DOMAIN_ID')
            return 1

        # 第二步：填写目的地。这里的位置属于 map 坐标系。
        goal = NavigateToPose.Goal()
        goal.pose.header.frame_id = 'map'
        goal.pose.header.stamp = self.get_clock().now().to_msg()
        goal.pose.pose.position.x = self.x
        goal.pose.pose.position.y = self.y
        goal.pose.pose.orientation.z = math.sin(self.yaw / 2.0)
        goal.pose.pose.orientation.w = math.cos(self.yaw / 2.0)

        # 第三步：发送目标，等待服务端回答是否接受。
        self.get_logger().info(f'SEND_GOAL: x={self.x}, y={self.y}, yaw={self.yaw}')
        send_future = self.nav.send_goal_async(goal, feedback_callback=self.on_feedback)
        rclpy.spin_until_future_complete(self, send_future)
        self.goal_handle = send_future.result()
        if not self.goal_handle.accepted:
            self.get_logger().warn('GOAL_REJECTED: task was not accepted')
            return 1
        self.get_logger().info('GOAL_ACCEPTED: accepted, but NOT arrived yet')

        # 可选实验：任务接受后，等待指定时间，再申请取消。
        if self.cancel_after > 0.0:
            self.cancel_timer = self.create_timer(self.cancel_after, self.cancel_once)

        # 第四步：继续处理反馈，直到收到最终结果。
        result_future = self.goal_handle.get_result_async()
        rclpy.spin_until_future_complete(self, result_future)
        response = result_future.result()
        if self.cancel_timer is not None:
            self.cancel_timer.cancel()

        if response.status == GoalStatus.STATUS_SUCCEEDED:
            self.get_logger().info('SUCCEEDED: robot arrived at the goal')
            return 0
        if response.status == GoalStatus.STATUS_CANCELED:
            self.get_logger().info('CANCELED: navigation task has ended by cancellation')
            return 0
        self.get_logger().error(
            f'FAILED: status={response.status}, '
            f'error_code={response.result.error_code}, error_msg={response.result.error_msg}')
        return 1

    def on_feedback(self, msg):
        # 日志按实际时间限频；不改变服务端的反馈频率。
        now = time.monotonic()
        if now - self.last_feedback >= 1.0:
            self.last_feedback = now
            distance = msg.feedback.distance_remaining
            self.get_logger().info(f'FEEDBACK: remaining={distance:.2f} m')

    def cancel_once(self):
        self.cancel_timer.cancel()
        self.get_logger().info('CANCEL_REQUESTED: asking Nav2 to cancel this goal')
        future = self.goal_handle.cancel_goal_async()
        future.add_done_callback(self.on_cancel_response)

    def on_cancel_response(self, future):
        response = future.result()
        if response.goals_canceling:
            self.get_logger().info('CANCEL_ACCEPTED: wait for final CANCELED result')
        else:
            self.get_logger().warn('CANCEL_NOT_ACCEPTED: goal may already have ended')


def main(args=None):
    rclpy.init(args=args)
    node = NavigationClient()
    try:
        return node.run_navigation()
    except KeyboardInterrupt:
        print('Client interrupted. Closing the client does not confirm the robot stopped.')
        return 130
    finally:
        node.nav.destroy()
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    sys.exit(main())
```

在 nano 中按 `Ctrl+O`，回车保存，再按 `Ctrl+X` 退出。这里代码里的英文日志会在运行时出现，第 7 步有对应中文。

## 4．让 ros2 run 知道从哪里启动

仍在外层包目录，输入：

```bash
nano setup.py
```

将文件内容替换成下面完整代码：

```python
from setuptools import find_packages, setup

package_name = 'nav_action_demo'

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
    description='Learn to send one Nav2 navigation goal',
    license='Apache-2.0',
    entry_points={
        'console_scripts': [
            'go_to_point = nav_action_demo.nav_client:main',
        ],
    },
)
```

保存并退出。最关键的是：

```python
'go_to_point = nav_action_demo.nav_client:main'
```

意思是：以后运行 `go_to_point`，就调用 `nav_action_demo` 目录中 `nav_client.py` 文件里的 `main()`。

你现在应该有这些文件：

```text
ros2_python_lesson2_ws/
└── src/
    └── nav_action_demo/
        ├── package.xml          自动生成，保留
        ├── setup.cfg            自动生成，保留
        ├── setup.py             刚才替换的文件
        ├── resource/            自动生成，保留
        └── nav_action_demo/
            ├── __init__.py      自动生成，保留
            └── nav_client.py    刚才创建的文件
```

## 5．构建一次

```bash
cd ~/ros2_python_lesson2_ws
source /opt/ros/jazzy/setup.bash
colcon build --packages-select nav_action_demo \
  --symlink-install --executor sequential
```

看到 `1 package finished` 就可以继续。

这里构建主要完成 Python 包安装和运行入口登记，没有把 Python 编译成 C++。以后只改 `nav_client.py` 内容，使用本教程的 symlink 安装时，保存后退出旧节点、重新启动即可；修改 `setup.py` 入口或依赖配置，需要重新构建。

## 6．准备原项目，并暂停它自己的导航安排

本课需要原项目中的仿真、桥接和 Nav2 正在运行。

**如果现在 RViz 能看到正在运行的小车，保持项目运行，直接做下面的“停止自动导览”。** 如果原项目已经完全关闭，在另一个 Ubuntu 终端输入：

```bash
cd ~/tour_guide_ws
export ROS_DOMAIN_ID=0
bash run_all.sh --view
```

它会启动完整项目及 RViz，自动导览也会启动。虚拟机启动较慢，等一两分钟，负载较高时更久。不要连续运行这条命令，它会清理并重新启动项目。

如果项目在后台运行，只是没有 RViz 窗口，可以使用：

```bash
cd ~/tour_guide_ws
export ROS_DOMAIN_ID=0
bash view_demo.sh
```

可视化只是让你看见运行情况，不是小车开始运动的开关。

### 停止自动导览，保留仿真和 Nav2

打开一个专门运行练习的新终端，输入：

```bash
source /opt/ros/jazzy/setup.bash
source ~/tour_guide_ws/install/setup.bash
export ROS_DOMAIN_ID=0
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
cd ~/tour_guide_ws
python3 tools/study_tools.py send cmd stop
```

这个 `stop` 是发给项目**导览程序**的：停止它的自动安排，并请求取消它当前的导航。它不会关掉 Gazebo 或 Nav2。

这样做，是因为导览程序也会给 Nav2 下发目标；你练习时让它停在 `IDLE`，避免同时有两个程序安排去哪儿。

等待几秒，观察 RViz 中小车停止，并检查导览日志：

```bash
tail -n 25 /tmp/tour_launch.log
```

应看到近期状态切换到 `IDLE`。如果这条日志路径不存在，查看启动脚本打印的导览日志位置。**发送命令成功不等于执行已经完成；状态进入 IDLE 也不能单独证明导航取消完成，要同时观察小车已停。** 如果没有停，不要急着运行客户端，先检查 stop 是否送到了导览节点。

注意：第一课使用 `ROS_DOMAIN_ID=88`，把练习节点与原项目隔开。本课要连接原项目，必须设为 `0`。这个数字可以理解为通信分组，同组的程序才能发现对方；它不是 IP 地址或节点编号。

## 7．运行你自己写的程序

仍在刚才的练习终端，继续输入：

```bash
source ~/ros2_python_lesson2_ws/install/setup.bash
ros2 run nav_action_demo go_to_point --ros-args \
  -p use_sim_time:=true \
  -p x:=0.0 -p y:=-3.0 -p yaw:=0.0
```

所有参数都按上面浮点格式输入，例如 `0.0`，不要改成整数 `0`。

本项目的 START 接待台坐标是 `(0.0, -3.5)`，这里的目的地 `(0.0, -3.0)` 位于它旁边，已检查它在静态地图的可通行区域。**小车会从当前所在位置出发**；这条命令不会把它瞬间移回 START。

如果刚启动仿真，小车最初出生在 START；如果此前跑过导览，可能已经在别处。要看 RViz 中当前机器人的位置，不能把目的地误当成起点。

`yaw:=0.0` 表示到达后朝向地图的正 x 方向，不一定是屏幕的正上方。`use_sim_time:=true` 表示节点跟随仿真时间。

观察终端和 RViz：

| 日志 | 中文意思 | 此时是否已到达 |
|---|---|---|
| `WAIT_SERVER` | 找 Nav2 提供的导航接口 | 否 |
| `SEND_GOAL` | 正在发送目的地 | 否 |
| `GOAL_ACCEPTED` | Nav2 接受了这个任务 | **还没有** |
| `FEEDBACK: remaining=...` | Nav2 报告沿路径估计还剩多少米 | 行驶过程中 |
| `SUCCEEDED` | Nav2 判定目标达成 | 是，满足其位置/朝向容差 |
| `FAILED` | 已接受的任务最终失败 | 不能算到达 |

这里“到达”不是要求坐标与目标小数点完全相等。反馈距离也不保证每次都变小：调整路线、绕障碍时可能增加。

如果车本来就在目标附近，可能很快结束，未必能看到明显的运动或多条反馈。成功后客户端会退出，这是本课程序正常结束，不是节点崩溃。

RViz 中如果启用了路径显示，还能看到规划路径；没有路径显示时，先观察机器人位置变化即可，不必为了本课增加大量显示项。

## 8．亲手体验一次取消导航

先等上一条命令结束，再发一个更远的目标。Tech 展点在 `(4.5, 3.0)`，输入：

```bash
ros2 run nav_action_demo go_to_point --ros-args \
  -p use_sim_time:=true \
  -p x:=4.5 -p y:=3.0 -p yaw:=3.14159 \
  -p cancel_after:=3.0
```

这次目标接受后，客户端等待 **3 秒仿真时间**，再申请取消。虚拟机仿真比现实慢时，实际等待会超过 3 秒。

你应该看到：

```text
GOAL_ACCEPTED
FEEDBACK: ...
CANCEL_REQUESTED
CANCEL_ACCEPTED
CANCELED
```

三条取消日志意思不同：请求已发送 → 服务端愿意取消 → 任务最终结束为取消。小车随后应停止当前导航运动。若已提前到达，可能得到 `SUCCEEDED`，或取消不被接受；它不一定是程序坏了。

**不要把关闭客户端等同于取消导航。** 对本课客户端按 `Ctrl+C`，只是结束你的程序，已经交给 Nav2 的任务可能继续执行。练习取消请使用上面的参数，并查看最终 `CANCELED` 和实际停车情况。

如果需要直接结束整个项目，在原项目终端使用：

```bash
cd ~/tour_guide_ws
python3 tools/stop_stack.py
```

这会停止仿真和导航，后续练习需要重新启动完整项目。

## 9．回头看代码：只先啃这六处

### ① 创建节点

```python
class NavigationClient(Node):
    def __init__(self):
        super().__init__('learning_navigation_client')
```

这和第一课一样：定义一个类，继承 ROS2 的 `Node`，初始化时给节点取名字。

### ② 创建 Action 客户端

```python
self.nav = ActionClient(self, NavigateToPose, '/navigate_to_pose')
```

三个参数依次是：挂在哪个节点上、使用什么 Action 数据类型、连接哪个 Action 名称。它不是创建一个新的 Nav2，而是创建一个与现有 Nav2 联系的接口对象。

### ③ 填目的地

```python
goal = NavigateToPose.Goal()
goal.pose.header.frame_id = 'map'
goal.pose.pose.position.x = self.x
goal.pose.pose.position.y = self.y
```

`Goal()` 可以先理解为“拿一张空白任务单”。后面给它填位置和朝向。

`map` 指定位置用地图坐标表达。机器人自己局部坐标里的“前方一米”和地图坐标里的“x 等于一米”是两件事，不能混用。本课不用自己计算 TF，但 Nav2 会使用项目已经发布的 TF，把当前机器人位置与地图目标对应起来。

朝向在 ROS 消息里用四元数表示。本课机器人只在平面转动，因此用 `sin(yaw/2)`、`cos(yaw/2)` 填 z、w。先会用：`0.0` 朝正 x，`3.14159` 朝负 x；不要求今天推导四元数。

### ④ 发送目标，等“接受”

```python
send_future = self.nav.send_goal_async(goal, feedback_callback=self.on_feedback)
rclpy.spin_until_future_complete(self, send_future)
self.goal_handle = send_future.result()
```

`async` 表示发出去后，不在发送函数里一直等着任务做完。函数给你一个 `Future`，可以先理解为“以后会填入答复的盒子”。

`spin_until_future_complete` 一边处理这个节点收到的 ROS2 事件，一边等这个盒子有答复。**这里等到的是任务是否接受，不是导航是否到达。**

接受后得到的 `goal_handle` 是这一次任务的操作对象：通过它查询最终结果或请求取消。它不是机器人的位置，也不是速度消息。

### ⑤ 收途中反馈，等“最终结果”

```python
result_future = self.goal_handle.get_result_async()
rclpy.spin_until_future_complete(self, result_future)
```

这次等待的才是任务结束。在等待过程中，ROS2 收到导航反馈，会调用之前登记的 `self.on_feedback`：

```python
def on_feedback(self, msg):
    distance = msg.feedback.distance_remaining
```

完整代码给日志加了一秒限频。它只少打印一些行，没有降低 Nav2 的导航控制频率。

这也回答第一课的“订阅和回调”：你先把处理函数登记给 ROS2，ROS2 在收到对应消息/反馈时调用它。**回调不是你每次手动调用的，也不是传感器本身。** Action 反馈是 Nav2 发来的任务进展；传感器数据仍由 Gazebo、桥接和原导航节点处理。

第一课里 `create_subscription(..., self.on_position, ...)` 登记的是 Topic 回调；本课 `feedback_callback=self.on_feedback` 登记的是 Action 反馈回调。

### ⑥ 请求取消

```python
future = self.goal_handle.cancel_goal_async()
future.add_done_callback(self.on_cancel_response)
```

它针对这一次导航任务申请取消。`add_done_callback` 的意思是：取消申请有答复时，请调用 `on_cancel_response`。

本课只有一个目标，用“发送 → 等接受 → 等结果”的写法方便看懂。没有加入整体任务超时、目标队列或故障重试；这些可以等基础熟悉后再加。

## 10．它与导览项目是什么关系？

原项目的 `tour_node.py` 会读取展点配置，决定下一站，调用导航 Action，到达后切换讲解、问答等状态。

本课将其中最小的一段独立出来：**只给一个坐标，观察任务结果。** 它没有自动讲解、问答或展点循环。停止自动导览后，如果之前的语音已经开始播放，可能还会听到那段语音，但本课客户端没有调用 TTS。

| 谁 | 负责什么 |
|---|---|
| 本课客户端 | 指定目的地，接收任务进展和结果，请求取消 |
| 原导览程序 | 决定展点顺序，安排导航、讲解和问答 |
| Nav2 | 接收导航目标，调用规划、控制和恢复模块 |
| Nav2 控制节点 | 根据路线、当前位姿和障碍信息计算速度 |
| 桥接 | 在 ROS2 与 Gazebo 之间转换消息 |
| Gazebo 差速驱动插件 | 接收速度并驱动仿真机器人 |

**“ROS2 节点控制机器人”这个说法很宽。你现在应该能进一步说清：哪个节点决定目的地，哪个节点计算速度，哪个插件执行速度。**

练习结束，并且当前目标已成功或已取消后，如果想恢复完整导览：

```bash
cd ~/tour_guide_ws
bash replay_tour.sh
```

这个脚本让导览程序从接待台重新安排一轮导览。它不是重建 Gazebo 世界。不要在本课目标还在执行时运行它。

## 11．常见卡点

| 现象 | 先检查什么 |
|---|---|
| `Package nav_action_demo not found` | 是否构建，是否 source 本课的 install/setup.bash |
| `No executable found` | setup.py 的入口是否完整，修改后是否重新构建 |
| `No module named nav2_msgs` | 先 source Jazzy；是否在原项目的 Ubuntu ROS2 环境里运行 |
| `SERVER_NOT_READY` | 原项目/Nav2 是否启动，本终端 ROS_DOMAIN_ID 是否为 0；高负载时服务发现也可能更慢 |
| `GOAL_REJECTED` | 服务已找到但没接受任务，检查 Nav2 是否 active、日志里的拒绝原因 |
| 接受目标但长时间不动 | 看 Nav2 日志、定位/TF、速度桥接是否正常；接受不代表全链路健康 |
| `FAILED` | 查看打印的 error_code、error_msg，并结合原导航日志判断 |
| 取消比 3 秒慢 | use_sim_time 为 true，3 秒按仿真时钟计算 |
| 修改代码后效果没变 | 是否保存、结束旧客户端、重新运行，是否 source 了正确工作区 |

先把具体日志保留下来，不要同时改 Nav2 参数、坐标和桥接配置。

## 12．你做完后回答这五个问题

1. 本课客户端有没有自己发布 `/cmd_vel`？实际速度是谁计算的？
2. `GOAL_ACCEPTED` 和 `SUCCEEDED` 有什么区别？
3. `frame_id='map'` 是在说“用哪个坐标系”还是“用哪个节点”？
4. `on_feedback` 为什么会被调用，调用它的是谁？
5. 关闭客户端，是否等于机器人已经取消导航？应该看什么确认？

你先完成运行，把终端日志和这五个问题的答案发给我，再进入下一课。

## 13．代码验证与范围

2026-10-11，完整客户端代码已在用户 Ubuntu VM 的独立目录 `/home/sbx/ros2_learning_ws/action_validation` 完成 Python 语法检查、ROS2 包生成及构建安装。包构建约 11.9 秒。

使用 `ROS_DOMAIN_ID=89` 的独立测试服务端，按真实 `NavigateToPose` 类型进行了 ROS2 Action 通信验证，四项均通过：

- 接受目标，收到反馈，最终返回成功。
- 服务端拒绝目标，客户端正确识别拒绝并以非零退出码结束。
- 已接受目标被服务端中止，客户端打印失败状态、错误码及错误信息。
- 客户端申请取消，收到取消接受答复，最终收到取消结果。

验证脚本退出码为 0，日志末尾为 `ALL_ACTION_CHECKS_PASSED`；日志保存在上述验证目录的 `verification.log`，测试节点已退出。

这些验证确认客户端的通信和结果处理，**使用的是模拟 Action 服务端，没有驱动真实 Nav2 或 Gazebo 小车**。本次没有给原导览项目下发目标，也没有改变它的运行状态；第 7、8 步的实际导航与停车效果由你按教程观察确认。

教程的 `(0.0, -3.0)` 目标已对照项目静态地图检查，最近占据栅格约在 1.36 米之外；这只说明地图上的位置可通行，不能替代实际导航验证。

代码接口已对照 [Nav2 Jazzy NavigateToPose 定义](https://raw.githubusercontent.com/ros-navigation/navigation2/jazzy/nav2_msgs/action/NavigateToPose.action)；Action 客户端用法参考 [ROS2 Jazzy 官方 Python Action 教程](https://raw.githubusercontent.com/ros2/ros2_documentation/jazzy/source/Tutorials/Intermediate/Writing-an-Action-Server-Client/Py.rst)。

## 14．公司只有远程终端：使用测试服务端练 Action

本节不需要 Gazebo、RViz 或完整 Nav2，但需要 ROS2 Jazzy 和 `nav2_msgs`、`action_msgs` 的接口类型。

先完成第 2～5 步，生成 `nav_action_demo` 包、创建 `nav_client.py`、配置入口并构建。**跳过第 6～8 步的真实项目操作**，改按本节运行。

你自己的客户端代码不变；我们另外写一个模拟导航服务端，按真实 Action 接口回报接受、反馈、结果和取消。它没有地图、规划和小车运动。

自己创建文件：

```bash
nano ~/ros2_python_lesson2_ws/mock_nav_server.py
```

复制保存：

```python
import time

import rclpy
from rclpy.action import ActionServer, CancelResponse, GoalResponse
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.executors import MultiThreadedExecutor
from rclpy.node import Node
from nav2_msgs.action import NavigateToPose


class MockNavigation(Node):
    def __init__(self):
        super().__init__('lesson_action_test_server')
        self.server = ActionServer(
            self, NavigateToPose, '/navigate_to_pose', self.execute,
            callback_group=ReentrantCallbackGroup(),
            goal_callback=self.accept,
            cancel_callback=lambda handle: CancelResponse.ACCEPT)

    def accept(self, request):
        if request.pose.pose.position.x == 99.0:
            return GoalResponse.REJECT
        return GoalResponse.ACCEPT

    def execute(self, handle):
        result = NavigateToPose.Result()
        if handle.request.pose.pose.position.x == -99.0:
            result.error_code = 999
            result.error_msg = 'Intentional test failure'
            handle.abort()
            return result
        for index in range(20):
            if handle.is_cancel_requested:
                handle.canceled()
                return result
            feedback = NavigateToPose.Feedback()
            feedback.distance_remaining = (20 - index) / 10.0
            handle.publish_feedback(feedback)
            time.sleep(0.1)
        handle.succeed()
        return result


def main():
    rclpy.init()
    node = MockNavigation()
    executor = MultiThreadedExecutor(num_threads=3)
    executor.add_node(node)
    try:
        executor.spin()
    except KeyboardInterrupt:
        pass
    finally:
        executor.shutdown()
        node.server.destroy()
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
```

此文件直接用 `python3` 运行，不需要加入 setup.py。

终端 A：

```bash
source /opt/ros/jazzy/setup.bash
export ROS_DOMAIN_ID=89
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
python3 ~/ros2_python_lesson2_ws/mock_nav_server.py
```

终端 B：

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros2_python_lesson2_ws/install/setup.bash
export ROS_DOMAIN_ID=89
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
ros2 run nav_action_demo go_to_point
```

你应看到 `GOAL_ACCEPTED → FEEDBACK → SUCCEEDED`。日志中的“到达”来自模拟服务端成功结果，**不代表实际小车走了路**。

继续在 B 一次运行一条：

```bash
# 正常目标，中途取消；本节默认用实际时钟
ros2 run nav_action_demo go_to_point --ros-args -p cancel_after:=0.3

# 特殊测试值 99.0：模拟服务端拒绝
ros2 run nav_action_demo go_to_point --ros-args -p x:=99.0

# 特殊测试值 -99.0：模拟服务端接受后故意失败
ros2 run nav_action_demo go_to_point --ros-args -p x:=-99.0
```

这些特殊 x 值只是本节测试约定，不是真实 Nav2 的规则。**不要向原项目发送它们。** 本节 `ROS_DOMAIN_ID=89` 与原项目的 `0` 隔离。

模拟服务端的 `goal_callback` 决定接不接单，`execute` 模拟执行并发反馈，`cancel_callback` 接受取消申请，执行函数检查 `is_cancel_requested` 后结束任务。

测试服务端使用多线程执行器和允许重入的回调组，是为了它模拟执行时还能处理取消请求；客户端仍是第二课的简单写法。今天先体验机制，不要求先掌握 ROS2 多线程设计。

结束时关闭 B，A 按 `Ctrl+C`。回家连接真实项目前，重新按第 6、7 步设通信编号为 0、启用仿真时钟。
