# 第三课：Service 服务——问小车一次，它回答一次

本课只需要 Ubuntu 的 ROS2 Jazzy 和终端，不需要 Gazebo、RViz 或原导览项目。你自己生成包、建文件、复制代码。

## 0．先知道要看到什么

我们写两个 Python 节点：

| 文件 | 用途 |
|---|---|
| `status_server.py` | 保存一辆简化模拟车的状态，接收查询并返回当前位置 |
| `status_client.py` | 发一次查询，显示答复，然后退出 |

为了把注意力放在服务通信上，小车位置由服务端内部的定时器模拟：从 0 米以约 0.1 米/秒增长，到 1 米停住。不是读取 Gazebo，也没有向真实小车发速度。

```text
客户端：请求“当前状态是什么？”
             ↓ /lesson3/get_status
服务端：读取自己保存的 x 和 state
             ↓ 返回一次答复
客户端：打印结果，结束
```

与 Topic 不同，服务端不会主动不断把查询结果推给客户端。内部定时器持续更新位置，与服务请求是否发生是两件事。

## 1．创建工作区和包

在 Ubuntu 新终端中输入：

```bash
source /opt/ros/jazzy/setup.bash
mkdir -p ~/ros2_lesson3_ws/src
cd ~/ros2_lesson3_ws/src
ros2 pkg create lesson_service_demo --build-type ament_python \
  --license Apache-2.0 --dependencies rclpy std_srvs
cd lesson_service_demo
```

`std_srvs` 提供现成的服务类型。我们用 `Trigger`：请求没有业务字段，答复有 `success` 布尔值和 `message` 字符串。它适合本课简单查询；正式复杂接口通常会自定义有明确字段的 `.srv`。

包已存在就不要重复创建；保留自动生成的 `package.xml`、`setup.cfg`、`resource` 和 `__init__.py`。

## 2．创建服务端

```bash
nano lesson_service_demo/status_server.py
```

复制完整代码：

```python
import json
import time

import rclpy
from rclpy.node import Node
from std_srvs.srv import Trigger


class StatusServer(Node):
    def __init__(self):
        super().__init__('lesson_status_server')
        # 为了不依赖 Gazebo，本课在节点里模拟一个从 0 到 1 米的小车。
        self.x = 0.0
        self.last_time = time.monotonic()
        self.timer = self.create_timer(0.1, self.update_cart)
        self.service = self.create_service(
            Trigger, '/lesson3/get_status', self.on_request)
        self.get_logger().info('STATUS_SERVER_READY')

    def update_cart(self):
        now = time.monotonic()
        dt = now - self.last_time
        self.last_time = now
        self.x = min(1.0, self.x + 0.1 * dt)

    def on_request(self, request, response):
        response.success = True
        response.message = json.dumps({
            'x': round(self.x, 3),
            'state': 'ARRIVED' if self.x >= 1.0 else 'MOVING',
        })
        self.get_logger().info('REQUEST_RECEIVED: ' + response.message)
        return response


def main(args=None):
    rclpy.init(args=args)
    node = StatusServer()
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

按 `Ctrl+O`、回车保存，`Ctrl+X` 退出。

## 3．创建客户端

```bash
nano lesson_service_demo/status_client.py
```

复制：

```python
import json
import sys

import rclpy
from rclpy.node import Node
from std_srvs.srv import Trigger


def main(args=None):
    rclpy.init(args=args)
    node = Node('lesson_status_client')
    client = node.create_client(Trigger, '/lesson3/get_status')
    try:
        if not client.wait_for_service(timeout_sec=5.0):
            node.get_logger().error('SERVER_NOT_READY')
            return 1
        future = client.call_async(Trigger.Request())
        rclpy.spin_until_future_complete(node, future, timeout_sec=5.0)
        if not future.done():
            node.get_logger().error('RESPONSE_TIMEOUT')
            return 1
        response = future.result()
        if not response.success:
            node.get_logger().error(response.message)
            return 1
        data = json.loads(response.message)
        node.get_logger().info(f"STATUS_REPLY: x={data['x']}, state={data['state']}")
        return 0
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    sys.exit(main())
```

保存退出。

## 4．配置运行入口并构建

```bash
nano setup.py
```

把内容替换为：

```python
from setuptools import find_packages, setup

package_name = 'lesson_service_demo'

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
            'status_server = lesson_service_demo.status_server:main',
            'status_client = lesson_service_demo.status_client:main',
        ],
    },
)
```

保存后执行：

```bash
cd ~/ros2_lesson3_ws
colcon build --packages-select lesson_service_demo \
  --symlink-install --executor sequential
```

## 5．在两个终端运行

终端 A 每次先输入：

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros2_lesson3_ws/install/setup.bash
export ROS_DOMAIN_ID=90
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
ros2 run lesson_service_demo status_server
```

出现 `STATUS_SERVER_READY` 后，保持运行。

终端 B 输入：

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros2_lesson3_ws/install/setup.bash
export ROS_DOMAIN_ID=90
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
ros2 run lesson_service_demo status_client
```

你会看到类似：

```text
STATUS_REPLY: x=0.32, state=MOVING
```

实际数字取决于服务端已运行多久。过几秒，再在 B 运行最后那一条命令，可能变为 `x=1.0, state=ARRIVED`。A 中每次会多一条 `REQUEST_RECEIVED`。

客户端打印一次就退出，是正常行为。服务端继续运行，内部模拟的位置继续更新。

## 6．代码先看这五处

1. `create_service(Trigger, '/lesson3/get_status', self.on_request)`：提供一个服务，登记处理请求的函数。
2. `on_request(self, request, response)`：ROS2 收到请求时调用它；给 `response` 填字段并返回。
3. `create_client(Trigger, '/lesson3/get_status')`：创建对应客户端。两端名称、类型和通信分组都要一致。
4. `call_async(Trigger.Request())`：发一个请求，取得以后会放入答复的 Future。
5. `spin_until_future_complete(...)`：处理通信事件并等答复；本课最多等 5 秒。

`json.dumps` 把 Python 字典变成字符串，放进 `message`；客户端 `json.loads` 再变回字典。JSON 只是本课字符串里的数据格式，ROS2 服务机制本身不要求用 JSON。

`create_timer(0.1, self.update_cart)` 让 ROS2 约每 0.1 秒调用一次更新函数；`spin` 处理定时器和请求。不是代码停在 `spin` 处就不工作了。

## 7．亲手改一次、制造一次失败

先在 A 按 `Ctrl+C`，修改服务端中的 `0.1 * dt` 为 `0.2 * dt`，保存、重新启动 A。你应看到模拟车更快到 1 米。使用 symlink 安装时只改 `.py` 内容不用再次构建，但旧进程必须重启。

再关闭 A，在 B 运行客户端。约 5 秒后出现 `SERVER_NOT_READY`：客户端没有找到服务端。然后恢复 A，再试一次。

若 A、B 都启动却找不到服务，检查是否同为 `ROS_DOMAIN_ID=90`。查询失败与“小车到没到”是不同问题。

本课查询超时只是停止等待，不代表服务端会自动撤销已收到的工作；这也是耗时任务通常选择 Action 的原因之一。

## 8．对应原项目，回答问题

原项目里 Nav2 生命周期查询和状态转换使用 Service。去展点要耗时、需要反馈和取消，所以使用 Action；雷达、位置和速度持续传输，所以使用 Topic。

- 为什么服务端不需要知道客户端什么时候启动？
- 两个节点都有 `spin` 或等待 Future 的过程，它们在处理什么？
- 本课没有 Gazebo，位置为什么仍然会变化？
- 服务端的回调是谁调用的？它应该返回什么？
- 想显示实时位置，反复调用服务与订阅位置 Topic 有什么区别？

验证范围见目录中的《附录_教程检查与运行验证记录》。参考：[ROS2 Jazzy 官方 Python 服务教程](https://raw.githubusercontent.com/ros2/ros2_documentation/jazzy/source/Tutorials/Beginner-Client-Libraries/Writing-A-Simple-Py-Service-And-Client.rst)。
