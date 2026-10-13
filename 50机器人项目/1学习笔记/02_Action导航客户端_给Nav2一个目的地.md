# 第二课：Action——直接运行，体验“交任务、看进度、拿结果”

更新日期：2026-10-13。本版对应你虚拟机里已经准备、构建并验证过的代码。**不用重新建包、复制代码或构建，先运行再读解释。**

你现在先学这一课。先完成第 1～4 步的终端练习，回答第 7 步的问题；再看第三课 Service。第 8 步连接真实 Nav2 是后续可选体验，不是今天学习的前提。

所有命令在 Ubuntu 虚拟机终端输入。Windows 里的这份 Markdown 只是教材。

## 0．本课做什么，先用一句话理解

你对司机说：“带我去一个地方。”司机告诉你：接不接这一单、途中还剩多远、最后是否到达。途中你还可以申请取消。

在机器人系统里，这种耗时任务可以通过 **Action** 交给执行程序。

| 本课程序 | 角色 | 干什么 |
|---|---|---|
| `nav_client.py` | 客户端，发任务的人 | 提交目的地、处理接受答复、反馈和最终结果，申请取消 |
| `mock_nav_server.py` | 测试服务端，接任务的人 | 模拟执行，发反馈，最后返回成功、失败或取消 |

本课先用模拟服务端。它与客户端之间的 ROS2 Action 通信是真实运行的；小车行驶与剩余距离由测试代码模拟。**不会启动 Gazebo，不需要 RViz，不会让原导览小车动。**

等换成原项目的 Nav2 服务端，同一个客户端就能把目的地交给真实导航系统。

## 1．先打开两个 Ubuntu 终端

把两个窗口分别叫 A、B。这是同一台虚拟机上的两个终端，不是两个系统。

**终端 A 输入一条：**

```bash
bash ~/ros2_learning_ws/run_course.sh 2 server
```

它启动模拟导航服务端，保持运行。服务端启动后不一定持续打印内容；等待别人发目标时，安静是正常的。

**终端 B 输入一条：**

```bash
bash ~/ros2_learning_ws/run_course.sh 2 client
```

快捷脚本已经自动加载 ROS2、加载本课安装目录、设置 `ROS_DOMAIN_ID=89`。你现在不需要自己 source 或输入构建命令。

一次只在 A 启动一个本课服务端。如果以前已在 A 运行着，就保持它运行，不要再开一个重复服务端。

## 2．把终端里的变化看懂

终端 B 会出现类似日志：

```text
WAIT_SERVER: waiting for Nav2
SEND_GOAL: x=0.0, y=-3.0, yaw=0.0
GOAL_ACCEPTED: accepted, but NOT arrived yet
FEEDBACK: remaining=2.00 m
FEEDBACK: remaining=1.00 m
SUCCEEDED: robot arrived at the goal
```

时刻和反馈条数可能不同。日志中使用了 Nav2/robot 的通用文字，但本节接的是模拟服务端，`SUCCEEDED` 不能当作 Gazebo 小车真实到达的证据。

| 日志 | 你应该怎么理解 |
|---|---|
| `WAIT_SERVER` | 找谁来执行任务 |
| `SEND_GOAL` | 把任务发送出去 |
| `GOAL_ACCEPTED` | 对方接单了，任务还没做完 |
| `FEEDBACK` | 对方报告过程信息 |
| `SUCCEEDED` | 对方给出了成功的最终结果 |

**任务被接受与任务完成，是两次不同的答复。** 今天最先记住这点。

B 打印成功后退出是正常的：它只提交一个任务。A 仍在运行，可以接下一个任务。再运行一次 B 的命令，会发起一个新任务。

## 3．亲手体验取消

保持 A 运行，等 B 上一次任务结束，再在 B 输入：

```bash
bash ~/ros2_learning_ws/run_course.sh 2 cancel
```

这次在目标接受约 0.3 秒后申请取消。本节按实际时钟计时。

你应看到：

```text
GOAL_ACCEPTED
FEEDBACK: ...
CANCEL_REQUESTED
CANCEL_ACCEPTED
CANCELED
```

| 日志 | 含义 |
|---|---|
| `CANCEL_REQUESTED` | 客户端发出了申请 |
| `CANCEL_ACCEPTED` | 服务端同意进入取消流程 |
| `CANCELED` | 任务最终以取消结束 |

关闭客户端与取消任务是不同操作。在真实 Nav2 中，客户端退出后，已经被接受的导航目标可能继续执行；不能仅靠 `Ctrl+C` 判断车已停车。

## 4．再体验“拒绝”与“失败”

保持 A 运行，在 B 一次运行一条，等上一条结束再继续。

**拒绝任务：**

```bash
bash ~/ros2_learning_ws/run_course.sh 2 reject
```

应看到 `GOAL_REJECTED`，表示没有接单。

**接受任务后执行失败：**

```bash
bash ~/ros2_learning_ws/run_course.sh 2 fail
```

应看到 `GOAL_ACCEPTED`，随后 `FAILED`，其中错误码为测试服务端设置的 `999`。

两者不同：拒绝是没有开始执行；失败是接受后没有成功完成。

本课测试代码约定 `x=99.0` 拒绝、`x=-99.0` 失败。这是测试约定，不是真实 Nav2 的规则，也不是合法展点。快捷脚本把本课隔离在通信编号 89；不要把这些坐标发给原项目。

## 5．现在打开文件，看六段关键代码

**文件已经有了，用下面命令查看即可，不需要粘贴覆盖：**

```bash
nano /home/sbx/ros2_python_lesson2_ws/src/nav_action_demo/nav_action_demo/nav_client.py
```

不修改时直接 `Ctrl+X` 退出。完整参考代码也放在本课末尾，只用来对照。

### ① 创建节点与客户端

```python
super().__init__('learning_navigation_client')
self.nav = ActionClient(self, NavigateToPose, '/navigate_to_pose')
```

第一行创建 ROS2 节点。第二行给它创建一个 Action 客户端：

- `self`：客户端属于哪个节点。
- `NavigateToPose`：目标、反馈、结果使用哪种数据结构。
- `/navigate_to_pose`：连接哪个 Action 接口。

这是 **Action 名称**，不是单个 Topic。Action 的交互在底层会用到服务和话题；本课通过 `ActionClient` 使用它，不必自己拼底层通信。

这个对象不会自动创建 Nav2，也不会自动让小车动。执行任务的程序必须另外运行，就是终端 A。

### ② 填一张任务单

```python
goal = NavigateToPose.Goal()
goal.pose.header.frame_id = 'map'
goal.pose.pose.position.x = self.x
goal.pose.pose.position.y = self.y
```

先拿一个空的 Goal，再填坐标。`map` 表示位置按地图坐标系表达；x、y 的单位是米。

朝向由 yaw 转成四元数的 z、w 字段。先会用：yaw=0 朝地图正 x，yaw≈3.14159 朝负 x，具体坐标转换第四课再学。

本节模拟服务端并不会依据这些坐标规划路线；换成 Nav2 后才会使用地图、定位和障碍信息。

### ③ 发目标，等“接受答复”

```python
send_future = self.nav.send_goal_async(goal, feedback_callback=self.on_feedback)
rclpy.spin_until_future_complete(self, send_future)
self.goal_handle = send_future.result()
```

`send_goal_async` 发出目标，返回一个 Future。先把 Future 理解为“以后会装入答复的盒子”。

`spin_until_future_complete` 一边处理 ROS2 事件，一边等答复；这里等的是是否接受目标，不是已经到达。

`send_future.result()` 得到这次任务的 `goal_handle`。后面通过它查询最终结果或取消这次任务。

### ④ 收到反馈时调用函数

发送时登记了 `feedback_callback=self.on_feedback`。服务端发来进度后，ROS2 在处理事件时调用 `on_feedback`。

完整回调中的关键读取是：

```python
distance = msg.feedback.distance_remaining
```

**回调是处理反馈的函数，不是采集雷达的传感器。** 这次消息是服务端生成并发来的。真实项目中，Nav2 根据导航执行情况提供反馈，雷达数据来自 Gazebo 传感器与桥接。

本课日志只每秒打印一次左右，不等于服务端只每秒发一次反馈。

### ⑤ 等“最终结果”

```python
result_future = self.goal_handle.get_result_async()
rclpy.spin_until_future_complete(self, result_future)
response = result_future.result()
```

这次 Future 才代表任务完成答复。在等待期间，节点继续处理反馈和取消定时器。

`response.status` 是成功、取消、中止等状态；`response.result` 是 `NavigateToPose` 定义的结果内容，包含错误码和错误信息。两者不要混成一个字段。

### ⑥ 申请取消

```python
future = self.goal_handle.cancel_goal_async()
future.add_done_callback(self.on_cancel_response)
```

向服务端申请取消这次任务；申请有答复时，ROS2 调用 `on_cancel_response`。仍需最终任务结果确认结束。

本课是“发一个目标，等它结束”的入门写法，没有加入完整目标队列、整体任务超时或故障重试。

## 6．服务端为什么会有数据，代码在哪里？

查看现有文件：

```bash
nano /home/sbx/ros2_python_lesson2_ws/mock_nav_server.py
```

| 服务端代码 | 用途 |
|---|---|
| `ActionServer(..., '/navigate_to_pose', ...)` | 提供与客户端对应的 Action 接口 |
| `accept` | 决定接不接目标 |
| `execute` | 模拟任务执行，生成并发送反馈 |
| `publish_feedback` | 将进度消息送给客户端 |
| `handle.succeed()` / `abort()` / `canceled()` | 设置任务最终状态 |

所以你看到的“剩余距离”来自它的循环，不是未启动的 Gazebo。**谁运行、谁发布，决定你会收到什么数据。**

结束本课后，在 A 按 `Ctrl+C`。客户端正常完成后已退出。如果之前把服务端放到后台运行，关闭终端不一定结束它，应检查对应进程；本课直接前台运行即可。

## 7．你回答清楚这些，就进入第三课

1. A 和 B 分别是什么角色？谁生成反馈、谁处理反馈？
2. `GOAL_ACCEPTED` 和 `SUCCEEDED` 有什么区别？
3. 拒绝与接受后失败有什么区别？
4. `on_feedback` 是谁调用的？
5. 当前客户端有没有自己计算并发布速度？

能用自己的话讲出大意就可以，不要求背函数名。**先完成本课终端实验，再打开《03_Service服务_查询小车状态.md》。**

## 8．可选：同一个客户端连接原项目的 Nav2

这一步需要原仿真和 Nav2，不是模拟服务端。没有桌面时可以先跳过。

### 8.1 准备原项目

先关闭本课 A 的模拟服务端。原项目已运行就保持它运行；如果完全关闭，在 Ubuntu 桌面终端输入：

```bash
cd ~/tour_guide_ws
export ROS_DOMAIN_ID=0
bash run_all.sh --view
```

等启动流程完成。只想打开已有系统的 RViz，用 `bash view_demo.sh`，它不会重新启动整个项目。

### 8.2 停下原自动导览，保留 Nav2

在另一个终端输入：

```bash
source /opt/ros/jazzy/setup.bash
source ~/tour_guide_ws/install/setup.bash
export ROS_DOMAIN_ID=0
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
cd ~/tour_guide_ws
python3 tools/study_tools.py send cmd stop
tail -n 25 /tmp/tour_launch.log
```

等导览状态进入 IDLE，并观察小车已停。命令发出、状态变化和导航取消完成不是同一件事，不要只凭“已发布”就立即发另一个目标。

### 8.3 给真实 Nav2 发目标

在刚才的终端继续：

```bash
source ~/ros2_python_lesson2_ws/install/setup.bash
ros2 run nav_action_demo go_to_point --ros-args \
  -p use_sim_time:=true \
  -p x:=0.0 -p y:=-3.0 -p yaw:=0.0
```

不要用快捷脚本的 `2 client` 做这步：它固定在编号 89，连接的是教学测试。真实项目使用编号 0。

目的地 `(0,-3.0)` 在 START 接待台 `(0,-3.5)` 旁边。小车从当前所在位置出发，不会因为发目标就瞬间移回 START。

```text
你的客户端：决定去哪儿
Nav2：结合地图、定位和障碍，规划路线与计算速度
速度平滑器 → 桥接 → Gazebo 差速插件：执行速度
RViz：显示收到的地图、位置、雷达和路线
```

此时 `SUCCEEDED` 表示 Nav2 按位置与朝向容差判断到达，不要求坐标小数点完全相等。

真实模式的取消实验可换远一点的 Tech 目标：

```bash
ros2 run nav_action_demo go_to_point --ros-args \
  -p use_sim_time:=true -p x:=4.5 -p y:=3.0 -p yaw:=3.14159 \
  -p cancel_after:=3.0
```

3 秒按仿真时钟计算，慢速虚拟机实际等待会更久。确认最终 CANCELED，并观察实际运动停止。若提前自然到达，也可能返回 SUCCEEDED。

目标已结束后，在原项目目录用 `bash replay_tour.sh` 恢复一轮导览。想结束整个原项目，用 `python3 tools/stop_stack.py`。

## 9．卡住时先看这里

| 现象 | 先检查 |
|---|---|
| `run_course.sh` 找不到 | 是否在我们准备过的这台 Ubuntu 虚拟机，而不是另一台服务器 |
| `SERVER_NOT_READY` | A 是否运行；快捷版是否同为编号 89；真实 Nav2 版是否为编号 0 |
| 服务端不打印 | 等待任务时可能没有日志，先运行 B |
| B 打印一次成功就退出 | 正常，一次任务结束 |
| 服务端有异常堆栈 | 把 A 的报错文字保留，不要只看 B 的等待日志 |
| 修改文件后还是旧行为 | 是否保存并重启对应进程；symlink 安装不会自动更新运行中的对象 |

本机快捷脚本不等于公司服务器也已经部署。如果换另一台机器，需要把代码和 ROS2 环境准备到那台机器，不能仅拿这条路径直接运行。

## 10．完整客户端参考：不用复制覆盖

下面与虚拟机已准备的客户端一致。先会看第 5 步，再读完整文件。

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

2026-10-13 已对工作区根目录的安装入口验证成功、反馈、取消、拒绝、执行失败；这些实测连接隔离的模拟服务端，没有给原导览小车下发目标。本次教材更新只调整说明与学习顺序，没有改动已验证程序。

接口参考：[Nav2 Jazzy NavigateToPose 定义](https://raw.githubusercontent.com/ros-navigation/navigation2/jazzy/nav2_msgs/action/NavigateToPose.action)。
