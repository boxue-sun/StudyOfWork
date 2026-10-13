# 02 · Action 导航客户端：怎么给 Nav2 一个"去哪儿"的指令

> 这一版只讲**你项目里真实用到的**东西。先给你一句话体感，再贴你自己的代码，然后给一个能跑的最小片段，最后说简历怎么答。
> 对应简历：**第 2 条**（定位与导航：通过 Action 下发导航目标）和**第 3 条**（导航与导览调度：监听结果回调）。

---

## 0. 一句话先记住

**Topic 是"广播"（我喊一句、你们听着）；Action 是"派活"（我让你干件长活，你要告诉我接没接、干到哪了、干完没）。**

你项目里的"导航"就是典型的长活——从 A 点走到 B 点要好几秒甚至几十秒，中途还要知道走没走偏。所以用 Action 派给 Nav2，而不是用 Topic。

---

## 1. 你项目里这段在哪儿、长啥样

打开 `src/tour_guide/tour_guide/tour_node.py`：

- **第 76 行**：建了一个 Action 客户端，连到 Nav2 标准的 `/navigate_to_pose`（话题名是 Nav2 定的，不是你起的）。
  ```python
  self._nav_client = ActionClient(self, NavigateToPose, "/navigate_to_pose")
  ```

- **第 209–222 行 `_send_nav_goal()`**：把"下一个展点"的坐标打包成目标发出去。
  - `wait_for_server(timeout_sec=1.0)`：先看 Nav2 准备好了没。没准备好就**重试**——你现在日志里看到的"Nav2 Action Server 尚未就绪，重试..."就是这里打的。
  - `goal.pose = self._point_pose(self._idx)`：坐标来自 `config/tour_points.yaml`（比如"党史长廊"是 `x:-4.5, y:3.0`）。
  - `send_goal_async(goal, feedback_callback=self._feedback_cb)`：**异步**发出，不等结果，结果在回调里处理（不会卡住主循环）。

- **第 178–207 行** 两个回调（这是面试最爱问的"失败怎么处理"）：
  - `_goal_response_cb`：Nav2 回"我接了"还是"我拒了"。
  - `_goal_result_cb`：活干完了。看结果状态：
    - `SUCCEEDED` → 切到讲解（`GUIDE`）；
    - `CANCELED` → 啥也不做（是你手动 `stop`/`pause` 取消的，合理）；
    - **其他（失败/中止）** → 播报"导航出现异常，我将跳过该展点" → 切到 `NEXT`（去下一个点，**不回退重试**）。

> 关键一句：**导航失败 = 跳过当前展点继续巡游，不是回退重来。** 这句和你简历第 3 条完全一致，照答。

---

## 2. 动手体验：先跑个最小 Action 客户端

你项目要真跑 Nav2 才能发导航目标。想**单独体会 Action 的"派活→接活→给结果"流程**，有两种方式：

**方式 A（推荐，不用装 Nav2）**：用我之前给你做的 `hands_on_demo` 里的 `mini_fsm.py`。它不是 Action，但把"定时器轮询 + 状态切换 + 收到指令就推进"这套思路跑通了，和 tour_node 的状态机是同一套思维。路径：
```
C:/Users/sunboxue/WorkBuddy/ROS项目/_analysis/hands_on_demo/
```
按里面《动手体验指南.md》装好 ROS2 环境后，`ros2 run demo_ros2 mini_fsm` 就能看。

**方式 B（看真实 Action 全貌）**：下面这个**自带的 mock 服务端**，不依赖 Nav2 也能看 Action 三步走。先开一个终端跑服务端（假装自己是 Nav2），再开一个跑客户端。
> ⚠️ 需要 ROS2 + `nav2_msgs` 环境（你的课程/虚拟机里有）。只想懂概念的话看方式 A 就够了。

```python
# mock_nav_server.py —— 假装自己是 Nav2，收到目标 1 秒后说"到了"
import time, rclpy
from rclpy.action import ActionServer
from rclpy.node import Node
from nav2_msgs.action import NavigateToPose
from action_msgs.msg import GoalStatus

class MockNav(Node):
    def __init__(self):
        super().__init__("mock_nav")
        self._as = ActionServer(self, NavigateToPose, "/navigate_to_pose", self._execute)
    def _execute(self, gh):
        self.get_logger().info(f"收到目标 {gh.request.pose.pose.position}")
        time.sleep(1.0)          # 模拟"走过去"要花时间
        gh.succeed()
        r = NavigateToPose.Result()
        r.result = GoalStatus.STATUS_SUCCEEDED
        return r

rclpy.init(); rclpy.spin(MockNav())
```

```python
# mock_nav_client.py —— 派活并等结果
import rclpy
from rclpy.action import ActionClient
from rclpy.node import Node
from nav2_msgs.action import NavigateToPose
from geometry_msgs.msg import PoseStamped

class Client(Node):
    def __init__(self):
        super().__init__("c")
        self._ac = ActionClient(self, NavigateToPose, "/navigate_to_pose")
    def go(self):
        self._ac.wait_for_server()                       # ① 等服务端就绪
        g = NavigateToPose.Goal()
        g.pose = PoseStamped()                           # 目标：一个坐标
        g.pose.header.frame_id = "map"
        g.pose.pose.position.x = -4.5
        g.pose.pose.position.y = 3.0
        send_future = self._ac.send_goal_async(g)        # ② 派活（异步）
        gh = send_future.result().goal_handle
        if not gh.accepted:
            print("被拒绝"); return
        result_future = gh.get_result_async()             # ③ 等结果
        print("结果:", result_future.result().status)

rclpy.init(); Client().go(); rclpy.shutdown()
```
跑起来你会看到：客户端"派活 → 等 1 秒 → 拿到结果"，这就是 Action 的三步。

---

## 3. 原理：Action 到底是什么

一个 Action 由三部分组成：

| 部分 | 作用 | 你项目里的例子 |
|---|---|---|
| **Goal（目标）** | 你要它干啥 | "去 map 坐标系下 (-4.5, 3.0)" |
| **Feedback（反馈）** | 干到哪了（可选、周期发） | Nav2 周期发当前进度（你代码里 `_feedback_cb` 暂时空着，没用） |
| **Result（结果）** | 干完没 | `SUCCEEDED` / `CANCELED` / 其他 |

**为什么导航不用 Topic？** Topic 只能单向广播，没法问"你到没到"。导航是长任务，必须能跟踪状态。
**为什么导航不用 Service？** Service 是一问一答**立刻返回**，适合"查个状态"（比如"现在电量多少"）。导航要好几秒，期间还要进度，Service 会卡住线程。Action 专为"长任务 + 进度"设计。

---

## 4. 对应简历怎么答

- **第 2 条**："通过 ROS2 的 Action（`/navigate_to_pose`）向 Nav2 下发导航目标，目标坐标是 `config/tour_points.yaml` 里预置的展点位姿。"
- **第 3 条**："监听 Action 的结果回调：成功就进讲解，失败就播报'跳过该展点'继续巡游（不是回退重试）。"
- 被追问"Action 和 Service 区别" → "Action 是带进度的长任务（导航用），Service 是一问一答（本项目没用到）。"

---

## 5. 别踩坑（诚实底线）

- 你项目里**只有一个 Action**：`/navigate_to_pose`。别在面试里说"用了多个 Action"或"自己写了 Action 服务端"——服务端是 Nav2 的，你只是客户端。
- 导航目标的坐标系是 **`map`**（见 `_point_pose` 第 125 行 `pose.header.frame_id = "map"`），不是 `odom`。地图坐标和里程计坐标不一样，这点在第 4 篇 TF 里会讲。
- Feedback 回调你现在是空的（`pass`），别吹"做了实时进度显示"。
