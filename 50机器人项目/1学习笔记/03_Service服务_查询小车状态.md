# 第三课：Service——直接运行，问一次状态、拿一次答复

更新日期：2026-10-13。本版对应你虚拟机里已准备并验证过的 `status_server.py` 和 `status_client.py`。**不用创建包、复制代码或重新构建。**

建议先完成第二课的 Action 终端练习再学本课。本课只用 Ubuntu 终端，不需要 Gazebo、Nav2 导航或 RViz。

## 0．先知道本课问什么

第二课提交的是一个持续执行的任务。本课问一句：“当前车的位置和状态是什么？”服务端给一次答复，客户端打印后退出。

| 程序 | 角色 | 工作 |
|---|---|---|
| `status_server.py` | 服务端 | 保存模拟位置和状态，收到查询后返回 |
| `status_client.py` | 客户端 | 发一次查询，显示答复 |

为了不依赖其他系统，服务端内部用定时器模拟位置：从 0 米以约 0.1 米/秒变化，到 1 米停止。**这不是 Gazebo 的数据，也没有真实速度控制。**

Service 负责传查询和答复。内部模拟位置的更新是另一个工作，不是 Service 自己计算了位置。

## 1．先关闭第二课的练习，再开两个终端

第二课客户端完成后已退出；在运行模拟 Action 服务端的 A 中按 `Ctrl+C` 结束。然后用 A、B 做本课。

**终端 A 输入：**

```bash
bash ~/ros2_learning_ws/run_course.sh 3 server
```

看到 `STATUS_SERVER_READY` 后保持运行。

**终端 B 输入：**

```bash
bash ~/ros2_learning_ws/run_course.sh 3 client
```

快捷脚本自动加载 ROS2 和本课安装目录，并设置 `ROS_DOMAIN_ID=90`。你不需要重复手动配置。

请只启动一个本课服务端。A 在等查询时不持续打印，属于正常状态。

## 2．看懂一次查询的完整过程

B 可能打印：

```text
STATUS_REPLY: x=0.32, state=MOVING
```

A 同时多一行：

```text
REQUEST_RECEIVED: {"x": 0.32, "state": "MOVING"}
```

位置数字取决于服务端已运行多久。如果你隔较久才启动 B，可能第一次就看到 `x=1.0, state=ARRIVED`，也正常。

```text
B 发查询请求
    ↓ /lesson3/get_status
A 收到请求，执行 on_request，读取自己保存的状态
    ↓ 返回答复
B 收到答复，打印，然后退出
```

B 退出不代表 A 也退出。A 还在更新内部位置并等待新的查询。

## 3．再问一次，比较变化

保持 A 运行，在 B 再输入：

```bash
bash ~/ros2_learning_ws/run_course.sh 3 client
```

这是一次新请求，并非第一次查询一直在订阅。位置到 1 米以后，多次查询可能都得到同一个结果。

如果要看早期 MOVING：先在 A 按 `Ctrl+C`，重新运行服务端命令，再立即在 B 查询。重新启动服务端会重新初始化 x=0。

**位置在变，来自 A 的定时器；查询结果出现，来自 B 发请求。** 没有查询，A 不会主动给已经退出的 B 发送状态答复。

## 4．把服务端关闭，体验“没有人回答”

在 A 按 `Ctrl+C` 结束服务端，然后在 B 再运行客户端命令。

约 5 秒后应看到：

```text
SERVER_NOT_READY
```

意思是没有找到对应服务端，不是小车位置算错了。恢复 A 后，再试 B 应能得到答复。

如果关闭 A 后仍能取得答复，可能另一个终端还有同名服务端在运行。不要只根据某个窗口关闭就认定节点全部结束。

## 5．打开已有源码，先看这些关键代码

查看服务端：

```bash
nano /home/sbx/ros2_lesson3_ws/src/lesson_service_demo/lesson_service_demo/status_server.py
```

查看客户端：

```bash
nano /home/sbx/ros2_lesson3_ws/src/lesson_service_demo/lesson_service_demo/status_client.py
```

不修改时 `Ctrl+X` 退出。完整代码在文末供对照，不需要再复制一份。

### ① 服务端登记一个接口和处理函数

```python
self.service = self.create_service(
    Trigger, '/lesson3/get_status', self.on_request)
```

`Trigger` 是服务数据类型；`/lesson3/get_status` 是服务名称；`self.on_request` 是收到查询后要执行的回调。

`create_service` 创建通信接口，**不会立刻执行一次查询处理**。之后收到请求、节点通过 spin 处理事件时，才调用回调。

### ② 回调填写答复

```python
def on_request(self, request, response):
    response.success = True
    response.message = json.dumps({
        'x': round(self.x, 3),
        'state': 'ARRIVED' if self.x >= 1.0 else 'MOVING',
    })
    return response
```

`request` 是收到的请求，`response` 是要填写并返回的答复。

本课使用现成 `std_srvs/srv/Trigger` 类型：请求没有业务字段；答复包含布尔值 `success` 和字符串 `message`。

我们把 x、state 打包成 JSON 字符串放进 `message`。JSON 只是字符串内部的格式，不是 ROS2 服务通信必须使用的东西。正式复杂接口可以定义自己的 `.srv` 字段。

### ③ 客户端创建对应接口

```python
client = node.create_client(Trigger, '/lesson3/get_status')
```

两端要使用相同的服务名称、类型和可互通的通信配置。本课快捷脚本保证同为编号 90。

### ④ 发一次请求

```python
future = client.call_async(Trigger.Request())
rclpy.spin_until_future_complete(node, future, timeout_sec=5.0)
response = future.result()
```

第一行发送请求并取得 Future；第二行处理 ROS2 事件并等答复；第三行取出答复。完整代码在取答复前还检查是否超时。

这里仍有“异步发送”，不是说 Service 天生只能同步阻塞。**Service 和 Action 的区别主要在接口功能，不是一个只能同步、另一个只能异步。**

本课选择在主函数等一次答复，是为了容易理解。等待期间 spin 仍会处理通信；不要随意把同步等待放进某个回调里，导致它占住执行器、其他事件无法处理。

### ⑤ 拆开字符串，打印

```python
data = json.loads(response.message)
```

服务端把字典转成 JSON 字符串，客户端把它转回字典，再读取 `data['x']`、`data['state']`。

### ⑥ 位置为什么能变化

服务端创建了定时器：

```python
self.timer = self.create_timer(0.1, self.update_cart)
```

`update_cart` 根据实际经过时间更新 x，限制最大为 1 米。`rclpy.spin(node)` 同时处理这个定时器和服务请求。

所以一个节点可以同时有定时器、服务、发布器和订阅器。节点不等于“只能完成一种操作”。

## 6．你可以只改一行，亲自验证

先退出服务端 A。在 `status_server.py` 中找到：

```python
self.x = min(1.0, self.x + 0.1 * dt)
```

把这行里的 `0.1 * dt` 改为 `0.2 * dt`，保存。保持定时器周期不变：你改的是模拟运动速度，不是查询频率。

重新运行 A，再在 B 查询。它到 1 米大约由 10 秒缩短为 5 秒，实际观察受调度与启动耗时影响。

使用本课的 symlink 安装时，只改这个 `.py` 内容，保存并重启对应节点即可，不需要重新构建。运行中的旧节点不会自动换成刚保存的代码。

## 7．现在比较 Topic、Service、Action

| 机制 | 举例 | 你目前亲手体验过的内容 |
|---|---|---|
| Topic | 有新位置或状态事件时发布，订阅者处理 | 第一课位置与速度；消息可以周期发布，也可以只在事件发生时发布 |
| Service | 查一次当前状态，得到一次答复 | 本课查询 x、state |
| Action | 提交一个耗时任务，查看反馈和最终结果，可申请取消 | 第二课模拟导航任务 |

Service 的一次处理可能很快，也可能花时间，并非保证“立刻返回”。但普通 Service 没有 Action 那套标准任务进度和取消流程，所以导航这种任务适合 Action。

本课查询返回 `state=ARRIVED`，只是本课自己定义的状态字符串，不是第二课的 Action 最终状态；不能因为拼写相似就把两种接口混为一谈。

## 8．你的原导览项目到底有没有 Service？

需要分清范围：

- **自己写的导览业务节点**：主要通过 Topic 传状态、讲解、问题和指令，通过 Action 下发导航目标。不能把本课查询服务说成原业务已经实现的功能。
- **整套 ROS2/Nav2 系统**：生命周期状态查询、配置和激活等会用到 Service。项目中的生命周期准备工具就使用这类接口。

因此“整套项目完全没有 Service”过于绝对；“我自己写了哪些服务接口”也不能靠框架内部使用来证明。

你做完本课，可以说自己练习实现了 ROS2 服务端和客户端，理解请求、响应、异步调用与回调。若未来把查询服务真正接进导览业务，再描述那项集成。

## 9．本课学完的判断标准

用自己的话回答：

1. A、B 哪个提供服务，哪个发查询？
2. `on_request` 是什么时候被谁调用的？
3. B 没启动时，A 的位置还会不会更新？为什么？
4. B 打印一次就退出，为什么是正常行为？
5. 查当前状态为什么用 Service；去某个展点为什么用 Action？

能回答并完成一次查询、重复查询、关闭服务端的实验，就进入第四课 TF。

## 10．常见卡点

| 现象 | 检查 |
|---|---|
| `run_course.sh` 找不到 | 是否在准备过课程的 Ubuntu 虚拟机；另一台公司服务器不自动拥有这些文件 |
| `SERVER_NOT_READY` | A 是否启动，是否有异常；自定义启动时检查编号和名称 |
| `RESPONSE_TIMEOUT` | 已找到服务但 5 秒内没拿到答复，查看 A 的日志与系统负载 |
| 第一次查询已 ARRIVED | A 已运行约 10 秒以上；重启 A 后尽快查询 |
| 一直相同的 x=1 | 模型已到目标，预期如此 |
| 关闭 A 后仍能查询 | 是否还有另一个同名服务端运行 |

## 11．完整参考代码：不用复制覆盖

以下是本次准备并验证的版本。你做第 6 步修改后，本地源码与这里不同是正常的。

服务端 `status_server.py`：

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

客户端 `status_client.py`：

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

2026-10-13 已验证工作区安装后的服务端、客户端可以通过实际 ROS2 Service 取得状态答复。测试用隔离通信编号，未控制原导览小车。本次教材更新没有改动这些运行程序。

参考：[ROS2 Jazzy 官方 Python 服务教程](https://raw.githubusercontent.com/ros2/ros2_documentation/jazzy/source/Tutorials/Beginner-Client-Libraries/Writing-A-Simple-Py-Service-And-Client.rst)。
