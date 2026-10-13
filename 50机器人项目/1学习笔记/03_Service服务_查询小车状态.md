# 03 · Service 服务：一问一答（但你的项目里没用它）

> 重要提醒放最前面：**你的导览项目里没有用 Service。** 你简历第 1 条写的是"Topic 与 Action"，这是真实的。
> 那为什么还单开一篇讲 Service？因为面试常问"你为什么不用 Service"，你得**懂区别、能讲清楚**，但不能说"我用了"。
> 这篇是"补充知识 + 防追问"，不是你项目的代码。

---

## 0. 一句话先记住

**Topic = 广播（一直播）；Action = 派长活（带进度）；Service = 问路（问一句、答一句、完事）。**

三者分工不同，你项目只用了前两种。

---

## 1. 三种通信方式对比（面试直接背这张表）

| 方式 | 模式 | 适合场景 | 你项目用了吗 |
|---|---|---|---|
| **Topic** | 发布/订阅，异步广播 | 状态、语音词、迎宾事件、指令——一直流的数据 | ✅ 用了（最多） |
| **Action** | 目标/反馈/结果，长任务 | 导航（要好几秒，要跟踪状态） | ✅ 用了（仅 `/navigate_to_pose`） |
| **Service** | 请求/响应，一问一答 | 查状态、取一次数据（立刻返回） | ❌ **没用** |

你项目里节点间全是"事件广播 + 一个长任务（导航）"，**没有"同步请求-响应"的需求**，所以 Service 用不上。这就是面试被问"为什么不用 Service"的标准答法。

---

## 2. Service 长啥样（教学示例，不是你项目代码）

Service 是"客户端问一句，服务端答一句"，**立刻返回**。比如"查小车现在在哪"：

```python
# status_server.py —— 假装是辆车，被人问就答位置（教学示例，非你项目）
import rclpy
from rclpy.node import Node
from example_interfaces.srv import AddTwoInts   # 借用一个现成 srv 演示

class StatusServer(Node):
    def __init__(self):
        super().__init__("status_server")
        self._srv = self.create_service(AddTwoInts, "/get_status", self._cb)
    def _cb(self, req, resp):
        # 真实项目里这里会回车的位置；这里只演示"一问一答"
        resp.sum = req.a + req.b
        return resp

rclpy.init(); rclpy.spin(StatusServer())
```

```python
# status_client.py —— 问一句
import rclpy
from rclpy.node import Node
from example_interfaces.srv import AddTwoInts

class C(Node):
    def __init__(self):
        super().__init__("c")
        self._cli = self.create_client(AddTwoInts, "/get_status")
    def ask(self):
        self._cli.wait_for_service()
        req = AddTwoInts.Request(); req.a, req.b = 1, 2
        resp = self._cli.call(req)          # 同步调用，拿到就返回
        print("答:", resp.sum)

rclpy.init(); C().ask(); rclpy.shutdown()
```

注意 `call()` 是**阻塞**的——问完一直等到答回来才往下走。这正是它不适合导航的原因：导航要好几秒，你不能卡在 `call()` 里。

---

## 3. 对应简历怎么答

被问"Service 你用了吗 / 为什么不用" →
> "我项目里节点间是事件广播（Topic）加一个长任务（Action 导航），没有同步请求-响应的需求，所以只用了 Topic 和 Action，没用 Service。Service 适合一问一答立刻返回的场景，比如查状态。"

这句话**完全真实**，照答即可。

---

## 4. 别踩坑（诚实底线）

- ❌ 别说"我项目用了 Service 查询小车状态"——你没用。
- ❌ 别把 Topic 和 Service 搞混：Topic 是"一直发"，Service 是"问了才答"。
- ✅ 知道区别、能讲清"为什么不用"就够了。你简历第 1 条写的"Topic 与 Action"是准确的，守住它。
