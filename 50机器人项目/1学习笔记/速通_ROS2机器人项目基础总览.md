<style>
body{font-size:18px;line-height:1.7;}
h1{font-size:30px;}
h2{font-size:24px;border-bottom:2px solid #4a90d9;padding-bottom:6px;}
h3{font-size:20px;color:#2c5f8a;}
code{background:#f4f4f4;padding:2px 6px;border-radius:4px;font-size:16px;}
pre{background:#1e1e1e;color:#d4d4d4;padding:14px;border-radius:8px;font-size:15px;overflow-x:auto;}
table{border-collapse:collapse;width:100%;margin:14px 0;}
th,td{border:1px solid #ccc;padding:8px 10px;text-align:left;}
th{background:#eaf2fb;}
blockquote{background:#fff8e1;border-left:4px solid #ffb300;padding:10px 14px;margin:12px 0;}
.tip{background:#e8f5e9;border-left:4px solid #43a047;padding:10px 14px;margin:12px 0;}
.warn{background:#ffebee;border-left:4px solid #e53935;padding:10px 14px;margin:12px 0;}
svg{display:block;margin:16px auto;background:#fafafa;border:1px solid #ddd;border-radius:8px;max-width:100%;height:auto;}
</style>

# ROS2 机器人项目 · 基础速通（一张图看懂全流程）

> 这份文档把你 `50机器人项目/1学习笔记/` 里的 10 份教程 + `40ROS学习笔记` 的答疑，浓缩成**一条主线 + 几张图**，让你快速建立整体认知。每个小节都标了"源自哪份文档"，想深挖回原文档翻即可。
>
> **用法**：先看懂第 0 节全景图（最重要），再按需看分节。不用死记，理解"谁跟谁说话"就够。

## 整合来源一览

| 本节 | 主题 | 源自原文档 |
|---|---|---|
| 0 | 全景：整套怎么跑起来 | 00 + 08 + 附录设计说明 |
| 1 | ROS2 核心：节点/包/工作空间/source | 00、40答疑 2.1/2.2/2.3 |
| 2 | 通信三件套：Topic / Service / Action | 01、03、02、40答疑 2.5 |
| 3 | 坐标系与 TF | 04 |
| 4 | 仿真与桥接：Gazebo + ros_gz_bridge | 05 |
| 5 | Nav2 导航原理 | 07 |
| 6 | RViz 可视化 | 06 |
| 7 | 项目业务：导览状态机 | 08 |
| 8 | 常用命令速查 | 答疑 + 各文档 |
| 9 | 两个工作空间长啥样 | 项目结构 |

---

## 0. 全景：这套东西到底怎么跑起来的？

**一句话本质**：你在终端敲一条启动命令 → 虚拟机里开一个"仿真世界(Gazebo)"，里面有一辆小车和一个展厅 → 一个"导航司机(Nav2)"负责开车 → 一个"导览大脑(tour_guide)"负责决定去哪站、到站讲什么 → 你用 RViz 在屏幕上"看"这一切。

<div class="tip">你的代码（tour_guide 状态机 + lesson2 的导航客户端）只是"下指令、看结果"的人，真正让车动的是 Nav2 + Gazebo。</div>

<svg viewBox="0 0 680 340" xmlns="http://www.w3.org/2000/svg" font-family="sans-serif" font-size="14">
  <rect x="30" y="25" width="180" height="44" rx="6" fill="#e3f2fd" stroke="#1565c0" stroke-width="2"/>
  <text x="120" y="52" text-anchor="middle" fill="#0d47a1">终端：敲启动命令</text>
  <rect x="30" y="105" width="250" height="180" rx="8" fill="#f1f8e9" stroke="#2e7d32" stroke-width="2"/>
  <text x="45" y="128" fill="#1b5e20" font-weight="bold">tour_guide_ws（项目）</text>
  <rect x="45" y="140" width="220" height="52" rx="6" fill="#fff" stroke="#2e7d32"/>
  <text x="155" y="162" text-anchor="middle">diff_drive_robot</text>
  <text x="155" y="182" text-anchor="middle" font-size="12" fill="#555">仿真底盘 + 展厅(Gazebo 世界)</text>
  <rect x="45" y="208" width="220" height="56" rx="6" fill="#fff" stroke="#2e7d32"/>
  <text x="155" y="230" text-anchor="middle">tour_guide</text>
  <text x="155" y="250" text-anchor="middle" font-size="12" fill="#555">导览状态机(去哪站/讲什么)</text>
  <rect x="370" y="110" width="170" height="52" rx="6" fill="#fff3e0" stroke="#ef6c00" stroke-width="2"/>
  <text x="455" y="140" text-anchor="middle" fill="#e65100">Nav2 导航司机</text>
  <rect x="370" y="188" width="170" height="52" rx="6" fill="#fce4ec" stroke="#c2185b" stroke-width="2"/>
  <text x="455" y="218" text-anchor="middle" fill="#880e4f">Gazebo 仿真世界</text>
  <rect x="370" y="266" width="170" height="46" rx="6" fill="#ede7f6" stroke="#5e35b1" stroke-width="2"/>
  <text x="455" y="294" text-anchor="middle" fill="#311b92">RViz 观察窗口</text>
  <line x1="120" y1="69" x2="120" y2="105" stroke="#2e7d32" stroke-width="2" marker-end="url(#m0a)"/>
  <line x1="280" y1="238" x2="368" y2="136" stroke="#ef6c00" stroke-width="2" marker-end="url(#m0b)"/>
  <line x1="455" y1="162" x2="455" y2="188" stroke="#c2185b" stroke-width="2" marker-end="url(#m0c)"/>
  <line x1="455" y1="240" x2="455" y2="266" stroke="#5e35b1" stroke-width="2" marker-end="url(#m0d)"/>
  <text x="300" y="178" fill="#ef6c00" font-size="12">派目标(Action)</text>
  <text x="465" y="178" fill="#c2185b" font-size="12">/cmd_vel</text>
  <text x="465" y="262" fill="#5e35b1" font-size="12">显示</text>
  <defs>
    <marker id="m0a" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#2e7d32"/></marker>
    <marker id="m0b" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#ef6c00"/></marker>
    <marker id="m0c" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#c2185b"/></marker>
    <marker id="m0d" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#5e35b1"/></marker>
  </defs>
</svg>

**关键点**：
- 你从没直接命令"车轮转"。你只说"去 A 展点"，剩下的路线、速度全是 Nav2 算。
- Gazebo 是"物理世界"（车会不会撞墙、走多快，由它算）；RViz 只是"监控屏幕"，你关掉 RViz 车照样跑。
- 所有模块都是独立的"节点"，靠"话题/服务/动作"互相通信（第 2 节）。

---

## 1. ROS2 核心概念：节点 / 包 / 工作空间 / source

**源自 00 环境说明 + 40 答疑 2.1/2.2/2.3**

**大白话套餐**：
- **节点(node)**：一个正在跑的"小程序"，干一件具体的事（发速度、算定位、播语音……）。你的 `go_to_point` 跑起来就是一个节点。
- **包(package)**：把相关节点、配置、启动文件打包的"文件夹"。`nav_action_demo`、`tour_guide` 都是包。
- **工作空间(workspace)**：放很多包的"大文件夹"，比如 `tour_guide_ws`、`ros2_python_lesson2_ws`。里面 `src/` 是源码，`install/` 是编译结果。
- **source**：把 ROS2 或你自己编译的包"摆到当前终端"，让你能用 `ros2` 命令、能 `import rclpy`。**每个新终端第一句都得 source 系统那个**；自己工作空间的 `install/setup.bash` 必须 `colcon build` 之后才存在。
- **ROS_DOMAIN_ID**：频道号，同号节点才能互相通信（默认 0）。
- **FASTDDS_BUILTIN_TRANSPORTS=UDPv4**：强制走 UDP 网络（虚拟机里共享内存不靠谱，关掉它才稳）。

<svg viewBox="0 0 680 240" xmlns="http://www.w3.org/2000/svg" font-family="sans-serif" font-size="14">
  <rect x="30" y="30" width="160" height="56" rx="6" fill="#e3f2fd" stroke="#1565c0" stroke-width="2"/>
  <text x="110" y="56" text-anchor="middle" fill="#0d47a1">你的节点</text>
  <text x="110" y="76" text-anchor="middle" font-size="12" fill="#555">go_to_point</text>
  <rect x="30" y="150" width="160" height="56" rx="6" fill="#fff3e0" stroke="#ef6c00" stroke-width="2"/>
  <text x="110" y="180" text-anchor="middle" fill="#e65100">Nav2 节点</text>
  <text x="110" y="200" text-anchor="middle" font-size="12" fill="#555">导航服务端</text>
  <rect x="480" y="30" width="170" height="56" rx="6" fill="#fce4ec" stroke="#c2185b" stroke-width="2"/>
  <text x="565" y="56" text-anchor="middle" fill="#880e4f">Gazebo 桥</text>
  <text x="565" y="76" text-anchor="middle" font-size="12" fill="#555">ros_gz_bridge</text>
  <rect x="480" y="150" width="170" height="56" rx="6" fill="#ede7f6" stroke="#5e35b1" stroke-width="2"/>
  <text x="565" y="180" text-anchor="middle" fill="#311b92">激光节点</text>
  <text x="565" y="200" text-anchor="middle" font-size="12" fill="#555">发布 /scan</text>
  <circle cx="310" cy="58" r="30" fill="#fff" stroke="#1565c0" stroke-width="2"/>
  <text x="310" y="56" text-anchor="middle" font-size="11">/cmd_vel</text>
  <text x="310" y="72" text-anchor="middle" font-size="10" fill="#555">话题</text>
  <circle cx="310" cy="178" r="30" fill="#fff" stroke="#5e35b1" stroke-width="2"/>
  <text x="310" y="176" text-anchor="middle" font-size="11">/scan</text>
  <text x="310" y="192" text-anchor="middle" font-size="10" fill="#555">话题</text>
  <line x1="190" y1="58" x2="280" y2="58" stroke="#1565c0" stroke-width="2" marker-end="url(#m1)"/>
  <line x1="340" y1="58" x2="480" y2="58" stroke="#1565c0" stroke-width="2" marker-end="url(#m1)"/>
  <line x1="190" y1="178" x2="280" y2="178" stroke="#5e35b1" stroke-width="2" marker-end="url(#m1)"/>
  <line x1="340" y1="178" x2="480" y2="178" stroke="#5e35b1" stroke-width="2" marker-end="url(#m1)"/>
  <text x="195" y="222" font-size="13" fill="#333">节点之间靠“话题”相连：一方发布、一方订阅，互相不认识也能通信。</text>
  <defs><marker id="m1" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#444"/></marker></defs>
</svg>

<div class="warn">易错：节点 ≠ 包。包是文件夹，节点是里面跑起来的程序。一个包可以有多个节点，一个节点也可以独立存在。</div>

---

## 2. 通信三件套：Topic / Service / Action

**源自 01(节点Topic) / 03(Service) / 02(Action) + 40 答疑 2.5**

这是 ROS2 里模块之间"说话"的三种方式，必须分清：

| 方式 | 类比 | 特点 | 本课例子 |
|---|---|---|---|
| **Topic** | 广播电台 | 一方一直播，多方一直听；无应答 | `/cmd_vel` 速度指令、`/scan` 激光 |
| **Service** | 问前台 | 一问一答，问完结束 | 查询小车状态（03） |
| **Action** | 派外卖 | 派活 + 途中进度 + 最终结果 + 可取消 | 让 Nav2 去某地（02） |

<svg viewBox="0 0 680 270" xmlns="http://www.w3.org/2000/svg" font-family="sans-serif" font-size="13">
  <!-- Topic -->
  <rect x="20" y="20" width="190" height="220" rx="8" fill="#e3f2fd" stroke="#1565c0" stroke-width="2"/>
  <text x="115" y="46" text-anchor="middle" font-weight="bold" fill="#0d47a1">Topic 广播</text>
  <rect x="45" y="62" width="140" height="34" rx="5" fill="#fff" stroke="#1565c0"/>
  <text x="115" y="84" text-anchor="middle">发布者</text>
  <circle cx="115" cy="135" r="22" fill="#fff" stroke="#1565c0" stroke-width="2"/>
  <text x="115" y="139" text-anchor="middle" font-size="11">话题</text>
  <rect x="45" y="184" width="140" height="34" rx="5" fill="#fff" stroke="#1565c0"/>
  <text x="115" y="206" text-anchor="middle">订阅者</text>
  <line x1="115" y1="96" x2="115" y2="113" stroke="#1565c0" stroke-width="2" marker-end="url(#m2)"/>
  <line x1="115" y1="157" x2="115" y2="184" stroke="#1565c0" stroke-width="2" marker-end="url(#m2)"/>
  <text x="115" y="232" text-anchor="middle" font-size="11" fill="#555">一直播 / 一直听</text>
  <!-- Service -->
  <rect x="245" y="20" width="190" height="220" rx="8" fill="#fff3e0" stroke="#ef6c00" stroke-width="2"/>
  <text x="340" y="46" text-anchor="middle" font-weight="bold" fill="#e65100">Service 问答</text>
  <rect x="270" y="120" width="140" height="34" rx="5" fill="#fff" stroke="#ef6c00"/>
  <text x="340" y="142" text-anchor="middle">客户端</text>
  <rect x="270" y="184" width="140" height="34" rx="5" fill="#fff" stroke="#ef6c00"/>
  <text x="340" y="206" text-anchor="middle">服务端</text>
  <line x1="340" y1="154" x2="340" y2="184" stroke="#ef6c00" stroke-width="2" marker-end="url(#m2)"/>
  <text x="340" y="120" text-anchor="middle" font-size="11" fill="#555">问</text>
  <text x="340" y="232" text-anchor="middle" font-size="11" fill="#555">一问一答，完即结束</text>
  <!-- Action -->
  <rect x="470" y="20" width="190" height="220" rx="8" fill="#f1f8e9" stroke="#2e7d32" stroke-width="2"/>
  <text x="565" y="46" text-anchor="middle" font-weight="bold" fill="#1b5e20">Action 派活</text>
  <rect x="495" y="62" width="140" height="32" rx="5" fill="#fff" stroke="#2e7d32"/>
  <text x="565" y="83" text-anchor="middle">客户端(派活)</text>
  <rect x="495" y="200" width="140" height="32" rx="5" fill="#fff" stroke="#2e7d32"/>
  <text x="565" y="221" text-anchor="middle">服务端(干活)</text>
  <line x1="565" y1="94" x2="565" y2="200" stroke="#2e7d32" stroke-width="2" marker-end="url(#m2)"/>
  <path d="M535,180 q-30,0 -30,-30 q0,-30 30,-30" fill="none" stroke="#2e7d32" stroke-width="1.5" stroke-dasharray="4"/>
  <text x="565" y="150" text-anchor="middle" font-size="11" fill="#555">目标→进度→结果</text>
  <text x="565" y="240" text-anchor="middle" font-size="11" fill="#555">可中途取消</text>
  <defs><marker id="m2" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#444"/></marker></defs>
</svg>

**为什么导航用 Action 不用 Topic？**
因为"去某地"是**有头有尾的活**：要等它到、中途想看进度、还可能取消。Topic 只会一直播报、不管"完成没"；Service 一问一答太短、撑不住长时间任务。**Action = 目标(去哪) + 反馈(还有几米) + 结果(到了/失败) + 可取消**，正好匹配导航。

<div class="tip">你写的 `go_to_point` 是 **Action 客户端**（派活+盯进度）；Nav2 是 **Action 服务端**（接单+干活）。客户端不发布速度、不碰 Gazebo——它只定目标。</div>

---

## 3. 坐标系与 TF：机器人在哪、朝哪

**源自 04_TF坐标变换**

机器人世界里"位置"必须说清楚"相对于谁"。常见坐标系（像俄罗斯套娃，一层套一层）：

```
map(地图,固定不动)
 └─ odom(里程计,随轮子累计,会飘)
     └─ base_link(车体中心)
         └─ laser / camera(装在车上的传感器)
```

- **map**：整张地图的固定参考，导航的目标坐标(x,y,yaw)都在这个系里。
- **odom**：车自己用轮子转数估出来的位置，会随时间"飘"，AMCL 靠地图把它校正回 map。
- **base_link**：车体本身；传感器都挂在它下面。

<svg viewBox="0 0 680 260" xmlns="http://www.w3.org/2000/svg" font-family="sans-serif" font-size="13">
  <!-- 树 -->
  <rect x="30" y="30" width="150" height="40" rx="6" fill="#e3f2fd" stroke="#1565c0" stroke-width="2"/>
  <text x="105" y="55" text-anchor="middle" fill="#0d47a1">map 地图(固定)</text>
  <rect x="40" y="100" width="150" height="40" rx="6" fill="#fff3e0" stroke="#ef6c00" stroke-width="2"/>
  <text x="115" y="125" text-anchor="middle" fill="#e65100">odom 里程计</text>
  <rect x="50" y="170" width="150" height="40" rx="6" fill="#f1f8e9" stroke="#2e7d32" stroke-width="2"/>
  <text x="125" y="195" text-anchor="middle" fill="#1b5e20">base_link 车体</text>
  <rect x="60" y="230" width="150" height="26" rx="5" fill="#fff" stroke="#888"/>
  <text x="135" y="248" text-anchor="middle" font-size="11">laser / camera</text>
  <line x1="105" y1="70" x2="115" y2="100" stroke="#444" stroke-width="2" marker-end="url(#m3)"/>
  <line x1="115" y1="140" x2="125" y2="170" stroke="#444" stroke-width="2" marker-end="url(#m3)"/>
  <line x1="125" y1="210" x2="135" y2="230" stroke="#444" stroke-width="2" marker-end="url(#m3)"/>
  <text x="215" y="180" font-size="12" fill="#555">一层套一层，位置都说“相对于谁”</text>
  <!-- 朝向小图 -->
  <rect x="430" y="40" width="210" height="170" rx="8" fill="#fff" stroke="#aaa"/>
  <text x="535" y="62" text-anchor="middle" font-weight="bold" fill="#333">yaw 车头朝向</text>
  <line x1="535" y1="150" x2="600" y2="150" stroke="#c2185b" stroke-width="3" marker-end="url(#m3r)"/>
  <text x="600" y="142" text-anchor="middle" font-size="11" fill="#c2185b">+x</text>
  <rect x="520" y="135" width="20" height="30" rx="3" fill="#2e7d32"/>
  <text x="535" y="110" text-anchor="middle" font-size="12" fill="#2e7d32">yaw=0 朝右</text>
  <line x1="535" y1="195" x2="470" y2="195" stroke="#1565c0" stroke-width="3" marker-end="url(#m3b)"/>
  <text x="470" y="187" text-anchor="middle" font-size="11" fill="#1565c0">-x</text>
  <text x="535" y="220" text-anchor="middle" font-size="12" fill="#1565c0">yaw=π 掉头朝左</text>
  <defs>
    <marker id="m3" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#444"/></marker>
    <marker id="m3r" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#c2185b"/></marker>
    <marker id="m3b" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#1565c0"/></marker>
  </defs>
</svg>

**yaw（朝向）**：机器人只在地面转，用一个数表示"车头朝向"。
- `yaw=0` → 车头朝 map 的 +x 方向（图里向右）
- `yaw=π(3.14)` → 掉头朝 -x（向左）
- 不用推导四元数，先记住这两个结论就够用。

---

## 4. 仿真与桥接：Gazebo + ros_gz_bridge

**源自 05_Gazebo与桥接**

- **Gazebo**：一个"物理仿真软件"，在里面造场地(tour_hall)、造小车(差速驱动)，车会不会撞、走多快都由它按物理规律算。
- **ros_gz_bridge（桥）**：ROS2 和 Gazebo 是两套系统，说话语言不同。桥负责**双向翻译**——把 ROS 的 `/cmd_vel` 速度翻成 Gazebo 能懂的指令让车动；把 Gazebo 里的激光、里程计翻成 ROS 话题让导航用。

<svg viewBox="0 0 680 230" xmlns="http://www.w3.org/2000/svg" font-family="sans-serif" font-size="13">
  <rect x="30" y="60" width="200" height="110" rx="8" fill="#e3f2fd" stroke="#1565c0" stroke-width="2"/>
  <text x="130" y="86" text-anchor="middle" font-weight="bold" fill="#0d47a1">ROS2 世界</text>
  <text x="130" y="112" text-anchor="middle">/cmd_vel 速度</text>
  <text x="130" y="134" text-anchor="middle">/scan 激光</text>
  <text x="130" y="156" text-anchor="middle">/odom 里程计</text>
  <rect x="440" y="60" width="200" height="110" rx="8" fill="#fce4ec" stroke="#c2185b" stroke-width="2"/>
  <text x="540" y="86" text-anchor="middle" font-weight="bold" fill="#880e4f">Gazebo 世界</text>
  <text x="540" y="112" text-anchor="middle">小车物理模型</text>
  <text x="540" y="134" text-anchor="middle">展厅 / 障碍物</text>
  <text x="540" y="156" text-anchor="middle">真实运动结果</text>
  <rect x="270" y="95" width="130" height="50" rx="6" fill="#fff3e0" stroke="#ef6c00" stroke-width="2"/>
  <text x="335" y="118" text-anchor="middle" fill="#e65100" font-weight="bold">ros_gz_bridge</text>
  <text x="335" y="136" text-anchor="middle" font-size="11">翻译官</text>
  <line x1="230" y1="115" x2="270" y2="115" stroke="#c2185b" stroke-width="2" marker-end="url(#m4a)"/>
  <line x1="410" y1="115" x2="440" y2="115" stroke="#1565c0" stroke-width="2" marker-end="url(#m4b)"/>
  <text x="250" y="108" font-size="11" fill="#c2185b">速度指令→车动</text>
  <text x="420" y="108" font-size="11" fill="#1565c0">传感器←回传</text>
  <text x="335" y="180" text-anchor="middle" font-size="12" fill="#333">桥一断，ROS 和 Gazebo 就“失联”</text>
  <defs>
    <marker id="m4a" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#c2185b"/></marker>
    <marker id="m4b" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#1565c0"/></marker>
  </defs>
</svg>

<div class="tip">记住链路：你的节点 → Nav2 算速度 → `/cmd_vel` → 桥 → Gazebo 小车动 → 桥把传感器回传 → Nav2 定位/避障。桥一断，车和 ROS 就"失联"。</div>

---

## 5. Nav2 导航原理：司机怎么开车

**源自 07_Nav2原理**

Nav2 就是"自动驾驶司机"，内部是一条流水线：

```
地图(map_server) + 定位(AMCL) → 规划(planner_server 算路径) → 控制(controller_server/DWB 算速度) → /cmd_vel → 车
                                                                  ↑________ 途中不断修正 ________↓ 实际位置/障碍
```

- **map_server**：提供地图（哪能走哪是墙）。
- **AMCL**：根据激光扫描 + 地图，算"车现在在 map 里的精确位置"。
- **planner（规划器）**：从当前位置到目标，算出一条路径（NavFn/Dijkstra）。
- **controller（控制器，DWB）**：沿着路径，实时算出"这一刻该发多少速度"，发到 `/cmd_vel`。它最贴近"司机打方向盘"。
- **代价地图(costmap)**：把地图 + 实时障碍合成一张"危险分布图"，规划和控制都看它。

<svg viewBox="0 0 680 280" xmlns="http://www.w3.org/2000/svg" font-family="sans-serif" font-size="13">
  <rect x="20" y="40" width="120" height="50" rx="6" fill="#e3f2fd" stroke="#1565c0" stroke-width="2"/>
  <text x="80" y="62" text-anchor="middle" fill="#0d47a1">map_server</text>
  <text x="80" y="80" text-anchor="middle" font-size="10" fill="#555">地图</text>
  <rect x="160" y="40" width="120" height="50" rx="6" fill="#fff3e0" stroke="#ef6c00" stroke-width="2"/>
  <text x="220" y="62" text-anchor="middle" fill="#e65100">AMCL</text>
  <text x="220" y="80" text-anchor="middle" font-size="10" fill="#555">定位</text>
  <rect x="300" y="40" width="120" height="50" rx="6" fill="#f1f8e9" stroke="#2e7d32" stroke-width="2"/>
  <text x="360" y="62" text-anchor="middle" fill="#1b5e20">planner</text>
  <text x="360" y="80" text-anchor="middle" font-size="10" fill="#555">规划路径</text>
  <rect x="440" y="40" width="120" height="50" rx="6" fill="#fce4ec" stroke="#c2185b" stroke-width="2"/>
  <text x="500" y="62" text-anchor="middle" fill="#880e4f">controller</text>
  <text x="500" y="80" text-anchor="middle" font-size="10" fill="#555">算速度(DWB)</text>
  <rect x="590" y="40" width="70" height="50" rx="6" fill="#ede7f6" stroke="#5e35b1" stroke-width="2"/>
  <text x="625" y="62" text-anchor="middle" fill="#311b92">小车</text>
  <line x1="140" y1="65" x2="160" y2="65" stroke="#444" stroke-width="2" marker-end="url(#m5)"/>
  <line x1="280" y1="65" x2="300" y2="65" stroke="#444" stroke-width="2" marker-end="url(#m5)"/>
  <line x1="420" y1="65" x2="440" y2="65" stroke="#444" stroke-width="2" marker-end="url(#m5)"/>
  <line x1="560" y1="65" x2="590" y2="65" stroke="#444" stroke-width="2" marker-end="url(#m5)"/>
  <text x="625" y="115" text-anchor="middle" font-size="11" fill="#5e35b1">/cmd_vel</text>
  <rect x="300" y="150" width="360" height="50" rx="8" fill="#fff" stroke="#888"/>
  <text x="480" y="172" text-anchor="middle" font-size="12" fill="#333">代价地图(costmap)：地图 + 实时障碍 → 规划/控制都看它</text>
  <text x="480" y="192" text-anchor="middle" font-size="11" fill="#555">遇到墙/人会自动绕开</text>
  <path d="M625,90 C660,120 660,150 620,155" fill="none" stroke="#5e35b1" stroke-width="1.5" stroke-dasharray="4" marker-end="url(#m5b)"/>
  <text x="640" y="130" font-size="10" fill="#5e35b1">实际位置</text>
  <defs>
    <marker id="m5" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#444"/></marker>
    <marker id="m5b" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#5e35b1"/></marker>
  </defs>
</svg>

<div class="warn">Nav2 要能跑，前提：有地图、有定位(AMCL 初始化成功)、有目标(Action 发过来)。缺一不可，否则车不动或乱走。</div>

---

## 6. RViz 可视化：只是个"观察窗口"

**源自 06_RViz**

RViz 是一个 3D 显示器：把地图、车模型、激光、路径、摄像头画面叠在一起给你看。**它只"看"，不改任何东西**——你关掉 RViz，导航和小车照常运行。

常用看：地图(`Map`)、机器人模型(`RobotModel`)、激光(`LaserScan`)、规划路径(`Path`)、摄像头(`Image`)。

---

## 7. 项目业务：导览状态机

**源自 08_导览状态机**

真正的"项目大脑"是 `tour_guide` 包里的状态机（`tour_node.py`）。它把"导航 + 讲解 + 迎宾"串成一段自动流程：

```
[起点待命] → 检测到访客(迎宾语音) → 导航去A展点 → 到达后讲解A
          → 导航去B展点 → 到达后讲解B → 回到起点 → 待命(循环)
```

<svg viewBox="0 0 680 200" xmlns="http://www.w3.org/2000/svg" font-family="sans-serif" font-size="13">
  <rect x="20" y="75" width="90" height="44" rx="22" fill="#e3f2fd" stroke="#1565c0" stroke-width="2"/>
  <text x="65" y="102" text-anchor="middle" fill="#0d47a1">起点待命</text>
  <rect x="130" y="75" width="90" height="44" rx="8" fill="#fff3e0" stroke="#ef6c00" stroke-width="2"/>
  <text x="175" y="102" text-anchor="middle" fill="#e65100">迎宾</text>
  <rect x="240" y="75" width="90" height="44" rx="8" fill="#f1f8e9" stroke="#2e7d32" stroke-width="2"/>
  <text x="285" y="102" text-anchor="middle" fill="#1b5e20">导航去A</text>
  <rect x="350" y="75" width="90" height="44" rx="8" fill="#fce4ec" stroke="#c2185b" stroke-width="2"/>
  <text x="395" y="102" text-anchor="middle" fill="#880e4f">讲解A</text>
  <rect x="460" y="75" width="90" height="44" rx="8" fill="#f1f8e9" stroke="#2e7d32" stroke-width="2"/>
  <text x="505" y="102" text-anchor="middle" fill="#1b5e20">导航去B</text>
  <rect x="570" y="75" width="90" height="44" rx="8" fill="#fce4ec" stroke="#c2185b" stroke-width="2"/>
  <text x="615" y="102" text-anchor="middle" fill="#880e4f">讲解B</text>
  <line x1="110" y1="97" x2="128" y2="97" stroke="#444" stroke-width="2" marker-end="url(#m6)"/>
  <line x1="220" y1="97" x2="238" y2="97" stroke="#444" stroke-width="2" marker-end="url(#m6)"/>
  <line x1="330" y1="97" x2="348" y2="97" stroke="#444" stroke-width="2" marker-end="url(#m6)"/>
  <line x1="440" y1="97" x2="458" y2="97" stroke="#444" stroke-width="2" marker-end="url(#m6)"/>
  <line x1="550" y1="97" x2="568" y2="97" stroke="#444" stroke-width="2" marker-end="url(#m6)"/>
  <path d="M615,119 C615,160 65,160 65,120" fill="none" stroke="#888" stroke-width="1.5" stroke-dasharray="4" marker-end="url(#m6g)"/>
  <text x="340" y="155" text-anchor="middle" font-size="12" fill="#555">讲解完 → 回起点 → 待命（循环接待下一位访客）</text>
  <text x="340" y="40" text-anchor="middle" font-size="13" fill="#333">每个“导航去X”底层 = Action 派目标 + Nav2 开车（见第 2、5 节）</text>
  <defs>
    <marker id="m6" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#444"/></marker>
    <marker id="m6g" markerWidth="9" markerHeight="9" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#888"/></marker>
  </defs>
</svg>

- **visitor_detector**：看有没有人靠近（迎宾触发）。
- **tts_node**：把讲解文字念出来（语音）。
- **qa_node / voice_input_sim**：模拟游客提问、回答问题。
- 每个"导航去X"底层还是第 2、5 节的 Action + Nav2。

---

## 8. 常用命令速查

```bash
# ① 每个新终端先来这两句（系统基础）
source /opt/ros/jazzy/setup.bash
export ROS_DOMAIN_ID=0
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4

# ② 进自己的工作空间，编译，再 source 工作空间（让自己写的包可用）
cd ~/tour_guide_ws
colcon build --symlink-install
source install/setup.bash

# ③ 看看现在有哪些节点在跑
ros2 node list
# 看看某个话题在发什么
ros2 topic echo /cmd_vel
# 看看 Action 列表（导航目标就在这）
ros2 action list
# 看看服务
ros2 service list

# ④ 派小车去一个地方（你写的客户端）
ros2 run nav_action_demo go_to_point --ros-args \
  -p use_sim_time:=true -p x:=0.0 -p y:=-3.0 -p yaw:=0.0
```

<div class="tip">`use_sim_time:=true`：让所有节点跟着 Gazebo 的仿真时钟走，否则时间对不上会出 bug。这些参数代码里本来就有默认值(0,-3,0)，不加 -p 也去同一地方。</div>

---

## 9. 两个工作空间长啥样

**`tour_guide_ws`（项目本体）**
- `src/diff_drive_robot/`：仿真底盘 + 场地。里面有 `launch/`(启动)、`config/`(参数)、`urdf/`(小车模型)、`worlds/`(展厅)、`maps/`(地图)、`rviz/`(配置)。
- `src/tour_guide/`：导览业务。`tour_node.py`(状态机)、`visitor_detector.py`(迎宾)、`tts_node.py`(语音)、`qa_node.py`(问答)、`voice_input_sim.py`(模拟提问)。

**`ros2_python_lesson2_ws`（你的练习）**
- `src/nav_action_demo/`：第二课写的**导航客户端** `nav_client.py`，入口名 `go_to_point`。它就是拿来体验"用 Action 给 Nav2 派目的地"的练手项目。

---

> **一句话收尾**：ROS2 = 一堆节点用 Topic/Service/Action 互相通信；你写的是"下指令的人"，Nav2 是"开车的司机"，Gazebo 是"真实马路"，RViz 是"监控屏"，TF 是"大家都认的坐标系"。看懂这张关系网，基础就过关了。
