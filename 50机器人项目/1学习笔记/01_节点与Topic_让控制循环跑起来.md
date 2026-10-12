# ROS2 Python 从零动手教程：自己建文件，让控制循环跑起来

从这一份开始学。之前的 C++ 教程保留作以后对照，不需要先完成它。
本课由你自己创建工作区、生成包、写文件、运行程序。所有节点的完整代码都在下面，直接复制即可。

所有命令在 **Ubuntu 虚拟机终端** 输入。代码粘贴进指定的 `.py` 文件。
使用新目录 `/home/sbx/ros2_python_from_zero_ws`，我不会提前在这个目录替你建好文件。
今天先做到第 7 步，看到位置变化、到目标后停车，再回头读关键代码。

## 0．先知道我们要实现什么

这次只写两个很小的 Python 程序，每个程序运行一个 ROS2 节点。

| 程序 | 干什么 |
|---|---|
| `simple_cart.py` | 接收速度，计算位置变化，发布当前位置 |
| `move_to_x.py` | 接收位置，判断前进、后退或停车，发布速度 |

用一个数字 x 表示小车位置，在终端观察变化。这是简单的一维数学模型，没有 Gazebo、雷达或路径规划。

```text
控制节点收到位置
       ↓
判断该前进、后退还是停车
       ↓ 发布速度消息
模拟车收到速度，更新位置
       ↓ 发布新位置消息
控制节点再判断
```

你会亲手看到：只有模拟车时位置不变；启动控制节点后开始前进，到目标附近后停下。

## 1．创建工作区，用命令自动生成包

打开一个新的 Ubuntu 终端，依次执行：

```bash
source /opt/ros/jazzy/setup.bash
mkdir -p ~/ros2_python_from_zero_ws/src
cd ~/ros2_python_from_zero_ws/src
```

说明：`source` 让终端认识 ROS2；`mkdir -p` 创建目录；`cd` 进入目录；`~` 是 `/home/sbx`。

再执行下面这整条命令：

```bash
ros2 pkg create robot_python_demo \
  --build-type ament_python \
  --license Apache-2.0 \
  --dependencies rclpy geometry_msgs std_msgs
```

这是一个命令，行尾的反斜杠表示下一行仍属于它。也可以合成一行输入。
包只需要创建一次；如果提示同名包目录已存在，检查已有文件，继续编辑即可。

| 参数 | 含义 |
|---|---|
| `robot_python_demo` | 包名，后面运行命令会用到 |
| `ament_python` | 使用 ROS2 的 Python 包构建方式 |
| `rclpy` | ROS2 的 Python 开发库，用来创建节点、收发消息 |
| `geometry_msgs` | 速度等消息类型所在的包 |
| `std_msgs` | 简单数字、字符串等消息类型所在的包 |

进入刚生成的包：

```bash
cd ~/ros2_python_from_zero_ws/src/robot_python_demo
ls
```

框架已经包含这些内容，具体还可能有 LICENSE、test 等文件：

```text
robot_python_demo/            ← 外层：ROS2 包目录
├── package.xml              ← 包名、依赖等说明，命令自动生成
├── setup.py                 ← Python 包的安装与运行入口配置
├── setup.cfg                ← 告诉安装工具程序应放到哪里
├── resource/
│   └── robot_python_demo    ← 包注册标记
└── robot_python_demo/       ← 内层：放 Python 代码
    └── __init__.py          ← Python 包标记，命令自动生成
```

本课工作区是 `ros2_python_from_zero_ws`，ROS2 包是 `robot_python_demo`。
内层同名目录是 Python 包目录。接下来把两个程序放在内层目录里。
`package.xml` 已声明依赖，不用你手写；本课也没有 CMakeLists.txt。

## 2．创建模拟车文件 simple_cart.py

在终端输入：

```bash
nano ~/ros2_python_from_zero_ws/src/robot_python_demo/robot_python_demo/simple_cart.py
```

`nano` 是终端文本编辑器，打开一个新文件后一般是空白的。
复制下面的完整代码，使用终端右键“粘贴”或 Ctrl+Shift+V 粘贴进去。
只复制代码块里的内容，不要复制三个反引号。

```python
import math
import time

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import Float64


class SimpleCart(Node):
    def __init__(self):
        super().__init__('simple_cart')
        self.x = 0.0
        self.speed = 0.0
        self.last_update = time.monotonic()
        self.last_command = self.last_update
        self.count = 0

        self.position_pub = self.create_publisher(
            Float64, '/learn_python/position', 10)
        self.command_sub = self.create_subscription(
            Twist, '/learn_python/cmd_vel', self.on_command, 10)
        self.timer = self.create_timer(0.1, self.on_timer)
        self.get_logger().info('CART_READY: starting at x=0')

    def on_command(self, msg):
        # 收到速度消息后，保存前后方向的速度。
        speed = msg.linear.x
        self.speed = max(-0.3, min(0.3, speed)) if math.isfinite(speed) else 0.0
        self.last_command = time.monotonic()

    def on_timer(self):
        current_time = time.monotonic()
        dt = current_time - self.last_update
        self.last_update = current_time

        # 控制程序超过半秒没发命令，模拟车就停止。
        if current_time - self.last_command > 0.5:
            self.speed = 0.0

        # 模拟位置变化：位移 = 速度 × 时间。
        self.x = self.x + self.speed * dt
        msg = Float64()
        msg.data = self.x
        self.position_pub.publish(msg)

        self.count += 1
        if self.count % 10 == 0:
            self.get_logger().info(f'x={self.x:.3f} m, speed={self.speed:.2f} m/s')


def main(args=None):
    rclpy.init(args=args)
    node = SimpleCart()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
```

保存退出：**Ctrl+O → Enter → Ctrl+X**。Ctrl+O 保存，Enter 确认文件名，Ctrl+X 退出。
Python 用缩进表示代码层级。保留代码中的空格，先不要随意调整缩进。

### 这份代码干什么

先记住两个变量：

```python
self.x = 0.0
self.speed = 0.0
```

`self.x` 是这个模拟车保存的当前位置，`self.speed` 是它保存的当前速度。开始时都为 0。
`self` 指当前对象；写成 `self.x` 后，不同函数可以读取和更新同一份位置。

接收速度的设置是：

```python
self.command_sub = self.create_subscription(
    Twist, '/learn_python/cmd_vel', self.on_command, 10)
```

可以从左到右读成：
“创建一个订阅者，接收 `/learn_python/cmd_vel` 中的 Twist 消息，收到后执行 `on_command`。”

| 内容 | 含义 |
|---|---|
| `create_subscription` | 创建订阅者，登记如何接收消息 |
| `Twist` | ROS2 提供的速度消息类型 |
| `/learn_python/cmd_vel` | 本练习的速度话题名，发送和接收两边必须一致 |
| `self.on_command` | 消息到来后要执行的函数，也叫回调 |
| `10` | 消息队列深度，不是速度或频率 |

`self.on_command` 后面没有括号：这里是在登记函数；收到消息时，ROS2 才调用它。
回调里的 `msg.linear.x` 是收到的前后方向速度，正数向前，负数向后，0 表示停下。

然后是定时器：

```python
self.timer = self.create_timer(0.1, self.on_timer)
```

意思是大约每 0.1 秒调用一次 `on_timer`。本课没有开启仿真时钟，按实际时间运行。
里面的核心计算只有这一行：

```python
self.x = self.x + self.speed * dt
```

新位置 = 原位置 + 速度 × 经过时间。例如速度 0.2 米/秒，经过 0.1 秒，位置增加约 0.02 米。
`time.monotonic()` 用来测经过了多少实际时间，所以不会假设定时器每次都刚好按时执行。

最后，把新位置装进消息并发布：

```python
msg = Float64()
msg.data = self.x
self.position_pub.publish(msg)
```

`Float64` 是只有一个浮点数字段的 ROS2 消息类型。这里用 `data` 保存当前位置。
更新和发布大约每 0.1 秒执行一次；为了容易阅读，终端位置日志约每秒显示一次。
限速和命令超时处理先知道用途就够了，核心是“收速度 → 更新位置 → 发位置”。

## 3．创建控制文件 move_to_x.py

输入：

```bash
nano ~/ros2_python_from_zero_ws/src/robot_python_demo/robot_python_demo/move_to_x.py
```

复制完整代码，按相同方式保存退出：

```python
import math

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import Float64


class MoveToX(Node):
    def __init__(self):
        super().__init__('move_to_x')
        self.declare_parameter('target_x', 1.0)
        self.declare_parameter('speed', 0.2)
        self.target_x = self.get_parameter('target_x').value
        self.speed = self.get_parameter('speed').value
        self.reached = False

        if not math.isfinite(self.target_x) or abs(self.target_x) > 2.0:
            raise ValueError('target_x must be within [-2, 2]')
        if not math.isfinite(self.speed) or not 0.0 < self.speed <= 0.3:
            raise ValueError('speed must be within (0, 0.3]')

        self.command_pub = self.create_publisher(
            Twist, '/learn_python/cmd_vel', 10)
        self.position_sub = self.create_subscription(
            Float64, '/learn_python/position', self.on_position, 10)
        self.get_logger().info(f'CONTROLLER_READY: target={self.target_x}')

    def on_position(self, msg):
        # 第一步：读取当前位置，计算与目标的差。
        x = msg.data
        cmd = Twist()
        if not math.isfinite(x):
            self.command_pub.publish(cmd)
            return
        error = self.target_x - x

        # 第二步：目标在前方就前进，在后方就后退，接近目标就停止。
        if abs(error) > 0.02:
            cmd.linear.x = self.speed if error > 0 else -self.speed
            self.reached = False
        else:
            cmd.linear.x = 0.0
            if not self.reached:
                self.get_logger().info(f'GOAL_REACHED: x={x:.3f}; STOP')
                self.reached = True

        # 第三步：发布速度消息，交给模拟车执行。
        self.command_pub.publish(cmd)


def main(args=None):
    rclpy.init(args=args)
    node = MoveToX()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
```

### 控制逻辑就在 on_position 里

收到位置消息后，它做三步。

**第一步，读取位置，计算目标与当前位置的差：**

```python
x = msg.data
error = self.target_x - x
```

目标是 1，当前位置是 0.3，差就是 0.7，说明目标在前面。
目标是 -0.5，当前位置是 1，差就是 -1.5，说明目标在后面。

**第二步，判断该发什么速度：**

```python
if abs(error) > 0.02:
    cmd.linear.x = self.speed if error > 0 else -self.speed
else:
    cmd.linear.x = 0.0
```

`abs` 取绝对值，判断离目标是否还超过 2 厘米。
中间那句 Python 条件表达式可以读成：“差为正就用正速度，否则用负速度。”
接近目标时，把速度设置成 0。

**第三步，发布速度：**

```python
self.command_pub.publish(cmd)
```

ROS2 负责把消息送给订阅这个话题的模拟车，模拟车收到后更新自己的速度。
“什么时候走、往哪边走、什么时候停”的规则，是我们在 Python 里写的条件判断。

### 回调是怎么接起来的

```python
self.position_sub = self.create_subscription(
    Float64, '/learn_python/position', self.on_position, 10)
```

这句把位置话题和 `on_position` 连起来：收到位置消息，就调用它处理。
代码里没有你手写的 `while True` 去反复收消息，ROS2 会处理消息到达事件。

### 两个程序里的 main 是什么

```python
rclpy.init(args=args)
node = MoveToX()
rclpy.spin(node)
```

`init` 初始化 ROS2；`MoveToX()` 创建节点对象；`spin` 持续处理节点的消息和定时器等事件。
`__init__` 是对象创建时执行的初始化函数，主要准备变量、发布者、订阅者和参数。
`super().__init__('move_to_x')` 调用 ROS2 Node 的初始化，并设置节点名。

程序最后的 `if __name__ == '__main__'` 支持直接运行这个 Python 文件。
我们下面用 `ros2 run` 启动，安装入口会调用同一个 `main` 函数。
`try/finally` 那部分用于 Ctrl+C 退出时清理节点，先不用背。

## 4．配置 setup.py，让 ros2 run 找到两个程序

打开自动生成的文件：

```bash
nano ~/ros2_python_from_zero_ws/src/robot_python_demo/setup.py
```

这个文件已经有内容。先清空旧内容，再粘贴下面完整内容。
在 nano 里可以从文件开头反复按 Ctrl+K 剪切当前行，直到空白，再粘贴。

```python
from setuptools import find_packages, setup

package_name = 'robot_python_demo'

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
    description='A small ROS2 Python control learning demo',
    license='Apache-2.0',
    entry_points={
        'console_scripts': [
            'simple_cart = robot_python_demo.simple_cart:main',
            'move_to_x = robot_python_demo.move_to_x:main',
        ],
    },
)
```

保存退出。最重要的两行是：

```python
'simple_cart = robot_python_demo.simple_cart:main',
'move_to_x = robot_python_demo.move_to_x:main',
```

第一行的含义是：运行 `simple_cart` 这个命令时，调用 Python 包 `robot_python_demo` 里面 `simple_cart.py` 的 `main` 函数。
第二行同理。左侧是运行命令名，右侧是 Python 模块和函数。
`setup.py` 负责安装与注册入口，本身不包含小车控制逻辑。

本课只需要新建两个 `.py`，再修改这个 `setup.py`。
框架生成的 `__init__.py`、`setup.cfg`、resource 标记文件保留即可。

## 5．检查文件，构建安装

执行：

```bash
ls ~/ros2_python_from_zero_ws/src/robot_python_demo/robot_python_demo
```

应该能看到：

```text
__init__.py  simple_cart.py  move_to_x.py
```

然后执行：

```bash
cd ~/ros2_python_from_zero_ws
source /opt/ros/jazzy/setup.bash
colcon build --packages-select robot_python_demo --symlink-install --executor sequential
```

Python 代码不需要像 C++ 那样编译成机器码。这里用 colcon 完成包的安装和入口注册。
成功时会显示类似：

```text
Finished <<< robot_python_demo
Summary: 1 package finished
```

工作区会出现 `build`、`install`、`log`，分别用于构建过程、安装结果和日志。
到这里还没有启动任何小车节点。构建成功后再继续下一步。

## 6．在终端 A 启动模拟车

开一个新终端作为 **终端 A**，按顺序执行：

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros2_python_from_zero_ws/install/setup.bash
export ROS_DOMAIN_ID=88
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
ros2 run robot_python_demo simple_cart
```

说明：

- 第一个 source 让终端认识 ROS2，第二个让它找到你刚安装的包。
- ROS_DOMAIN_ID 是通信组编号，两个练习终端必须相同。
- UDPv4 设置沿用这台虚拟机验证过的通信方式。
- `ros2 run 包名 程序名` 才真正启动节点。

先观察几秒。应该看到：

```text
CART_READY: starting at x=0
x=0.000 m, speed=0.00 m/s
```

模拟车已经在运行，并且持续发布位置消息，但是没人发速度，所以位置不变。

## 7．在终端 B 启动控制节点

保留终端 A，再开一个新终端作为 **终端 B**：

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros2_python_from_zero_ws/install/setup.bash
export ROS_DOMAIN_ID=88
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
ros2 run robot_python_demo move_to_x
```

两个节点发现对方可能先需要几秒，之后终端 A 的位置会增加，约行驶 5 秒后停下。例如：

```text
x=0.180 m, speed=0.20 m/s
x=0.380 m, speed=0.20 m/s
...
x=0.980 m, speed=0.00 m/s
```

数字只是示例，你的具体日志可能不同。终端 B 会打印：

```text
CONTROLLER_READY: target=1.0
GOAL_REACHED: x=0.980; STOP
```

默认目标是 1 米，允许约 2 厘米误差，不要求精确停在 1.000。
你看到的是终端中的位置数字，原导览项目的 RViz 小车不会因此移动。

**今天先做到这里。** 你已经自己体验了建包、写节点、注册入口、收发消息和控制停车。

## 8．改参数，体验倒着走

终端 B 按 Ctrl+C 停止控制节点，终端 A 保持运行。
在终端 B 执行：

```bash
ros2 run robot_python_demo move_to_x --ros-args -p target_x:=-0.5
```

目标在当前位置后面，所以会发布负速度，位置逐渐减小，最终在 -0.5 米附近停下。
这条参数对应初始化代码：

```python
self.declare_parameter('target_x', 1.0)
self.target_x = self.get_parameter('target_x').value
```

第一句声明参数和默认值，第二句读取启动时的值。
命令中的 `--ros-args -p target_x:=-0.5` 提供了新的目标，不需要修改源码。

先停掉旧控制节点，再试改变速度：

```bash
ros2 run robot_python_demo move_to_x --ros-args -p target_x:=1.0 -p speed:=0.1
```

速度变为原来的一半。目标范围为 [-2,2]，速度范围为 (0,0.3]。
参数按默认浮点类型设置，请使用 `1.0` 这样的写法。
一次保持一个控制节点，避免同时有两个程序发布速度。

## 9．改一行 Python 控制代码

对两个终端分别按 Ctrl+C，停止两个节点。
编辑：

```bash
nano ~/ros2_python_from_zero_ws/src/robot_python_demo/robot_python_demo/move_to_x.py
```

把：

```python
if abs(error) > 0.02:
```

改为：

```python
if abs(error) > 0.10:
```

保存，然后按第 6、7 步重新启动两个节点。从 0 出发、目标为 1 米时，预计在约 0.9 米附近停车。
这里是 Python 源码，又使用了 `--symlink-install`，只修改这个已有 `.py` 文件时，通常无需重新构建，但要重启节点才能加载新代码。
修改 `setup.py` 入口、依赖或者包结构时，需要重新运行 colcon build 并 source 环境。
实验结束把 0.10 改回 0.02，保存，并在下一次运行时重启节点。

保留修改前后的输出，亲手验证“我改的判断条件如何影响停车位置”。

## 10．控制节点退出后，为什么模拟车也停了

让它朝目标行驶，途中对终端 B 按 Ctrl+C。
过约半秒，终端 A 的速度会归零，位置不再变化。

原因是模拟车里明确写了：

```python
if current_time - self.last_command > 0.5:
    self.speed = 0.0
```

每次收到速度命令，都记录收到的时间；超过半秒没收到新命令，就停车。
这是本练习的命令超时规则，不能直接认为所有机器人程序都会自动这样处理。

结束整个练习：两个终端分别 Ctrl+C。
回原导览项目时开新的终端，避免沿用练习的 ROS_DOMAIN_ID=88 设置。

## 11．碰到问题怎么查

| 现象 | 检查 |
|---|---|
| ros2 命令找不到 | 当前终端有没有 source Jazzy |
| 包找不到 | colcon 是否成功，是否 source 本课工作区的 install/setup.bash |
| 程序找不到 | setup.py 的两个入口是否完整，修改后有没有构建 |
| ModuleNotFoundError | 文件是否在内层 robot_python_demo，__init__.py 是否保留，文件名是否正确 |
| IndentationError / TabError | Python 缩进是否复制正确，是否混入 Tab |
| 两个程序启动但位置不变 | 两个终端的 ROS_DOMAIN_ID 是否都为 88，话题名是否相同，等几秒观察是否发现对方 |
| 改了源码仍是旧效果 | 是否保存、是否退出旧节点并重新启动，是否 source 了正确工作区 |
| 参数类型报错 | 使用浮点写法，例如 target_x:=1.0 |

先把具体报错文字发给 AI。不要为了消除报错同时修改多处。
核心观察直接看两个终端，不依赖原虚拟机曾出现问题的 ROS2 话题查询命令。

## 12．把练习对应到原项目

| 练习 | 原导览项目 |
|---|---|
| 一个 Float64 位置消息 | /odom、激光和 AMCL 等提供的信息 |
| move_to_x 的方向和停车判断 | Nav2 控制节点结合路线、障碍物计算速度 |
| /learn_python/cmd_vel | /cmd_vel_nav → 平滑器 → /cmd_vel |
| simple_cart 的数学位置更新 | 桥接 → Gazebo 差速插件和物理仿真 |

本课模拟位置用来简化学习，不是读取真实传感器；它没有实现地图、避障、Nav2 导航 Action 或导览状态机。
下一课再给 Nav2 发一个目的地，学习“目标接受”和“任务完成”的区别。

第二课已生成：[自己写 Python 程序，给 Nav2 一个目的地](02_Action导航客户端_给Nav2一个目的地.md)。

现在先用自己的话回答：

1. 哪段代码真正决定发什么速度？
2. 两个节点分别发布什么、订阅什么？
3. 为什么只有模拟车时有数据但不动？
4. 订阅和回调有什么关系？
5. 为什么修改 Python 源码后要重启运行中的节点？

## 13．代码验证记录

2026-10-11，教程中的完整代码已在用户 VM 的独立验证目录 `/home/sbx/ros2_learning_ws/python_validation` 完成语法检查、包生成、构建安装与真实 ROS2 消息交互验证。
本次构建约 11.6 秒。验证结果：

- 只有模拟车时，位置保持 0。
- 默认目标为 1 米，前进到约 1.000 米并停车。
- 目标改为 -0.5 米，倒退到约 -0.480 米并停车。
- 行驶中退出控制节点，模拟车因命令超时停车，后续位置不再变化。
- symlink 安装实际加载源码目录里的 Python 文件。

验证脚本退出码为 0，测试节点已退出。日志在验证目录的 `verification.log`。
教程留给你操作的工作区由你亲自创建；本次未替你执行修改停车误差的练习，也没有向原导览小车发命令。

参考：[ROS2 官方 Jazzy Python 发布订阅教程源码](https://raw.githubusercontent.com/ros2/ros2_documentation/jazzy/source/Tutorials/Beginner-Client-Libraries/Writing-A-Simple-Py-Publisher-And-Subscriber.rst)。
