# 第六课：RViz 可视化——自己配置机器人运行的观察窗口

这一课需要能显示图形的 Ubuntu 桌面。公司只有 SSH 终端时先阅读，回家在现有虚拟机动手。不需要重新安装一个 Ubuntu。

## 0．分清“世界”和“观察窗口”

Gazebo 模拟实体和物理运动；RViz 根据 ROS2 的消息与 TF 绘制结果。

RViz 显示的是系统发布的数据，不是 Gazebo 的视频。没有地图消息就画不出地图，没有 TF 就无法把雷达点摆到地图的正确位置。

本课先用第四课的两个坐标轴练基本操作，再配置原导览项目。无需新建 ROS2 包，但你会自己保存 `.rviz` 配置文件。

## 1．先用第四课练坐标轴显示

按第四课启动广播节点，通信编号为 `91`。在 Ubuntu 桌面另开终端：

```bash
source /opt/ros/jazzy/setup.bash
export ROS_DOMAIN_ID=91
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
rviz2
```

在窗口里操作：

1. 左侧 `Global Options → Fixed Frame` 填 `lesson_map`。
2. `Frame Rate` 改成 `10`。
3. 点击左下 `Add`，选择 `TF`。
4. 展开 TF，打开坐标轴和名字显示；找到 `lesson_map`、`lesson_base`。
5. 用鼠标缩放、拖动观察坐标轴的位置和方向。

`Fixed Frame` 可以理解为整个画面的参考坐标系。它决定其他数据最终转换到哪个坐标系显示。

此时退出广播节点不一定让坐标轴立即消失，因为静态 TF 已进入 RViz 的缓存。不能只凭“图还在”认定发布节点仍在运行。

完成后关掉这个 RViz，再进入原项目，避免同时开多个窗口占内存。

## 2．准备原项目

如果原项目已经运行，不重复启动。若完全停止，在另一个终端：

```bash
cd ~/tour_guide_ws
export ROS_DOMAIN_ID=0
bash run_all.sh --view
```

等仿真、定位和导航准备好。它会打开现有配置的 RViz。

为了从空白窗口练习，先关闭已开的 RViz 窗口，保留项目后台节点；再在 Ubuntu 桌面终端输入：

```bash
source /opt/ros/jazzy/setup.bash
source ~/tour_guide_ws/install/setup.bash
export ROS_DOMAIN_ID=0
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
export LP_NUM_THREADS=2
rviz2 --ros-args -p use_sim_time:=true
```

不要把本课独立 RViz 设置成第四课的 `91`；项目消息在 `0`。

## 3．从空白窗口添加显示项

先将 `Global Options` 中：

- `Fixed Frame` 设置为 `map`。
- `Frame Rate` 设置为 `10`。

再依次点击 `Add → By display type` 添加以下项，并展开设置：

| 显示项 | 配置 | 你会看到什么 |
|---|---|---|
| Grid | 默认即可 | 参考网格，不是障碍地图 |
| Map | Topic `/map`；Durability `Transient Local`，Reliability `Reliable` | 展厅占据栅格地图 |
| RobotModel | Description Source `Topic`，Description Topic `/robot_description` | 机器人模型 |
| LaserScan | Topic `/scan`；Reliability `Best Effort` | 雷达测到的点 |
| Path | Topic `/plan` | 当前全局规划路线，通常导航时才出现 |
| TF | 打开名字和坐标轴 | 坐标关系；平时可关闭以减小画面负担 |
| MarkerArray | Topic `/tour_guide/view_labels` | 项目中的展点编号标签，需标签发布节点在运行 |

不同 RViz 版本的属性展开方式略有区别；Topic 一般在显示项下的 `Topic → Value`。RobotModel 的 `/robot_description` 若要接收之前发布的模型描述，也要使用 `Transient Local`。

**QoS 是通信约定。** 地图经常只发布一次，新加入的窗口需要获取保存的最新地图；雷达则连续更新、适合传感器的 Best Effort 方式。不是话题名相同就一定能收到。

不要求全部一次成功。先让 Map 显示，再加机器人，再加雷达和路线，每次只排查一个显示项。

## 4．怎么读画面

- 地图黑色通常代表占据区域，白/浅灰代表空闲，未知区域颜色由显示配置决定。
- 机器人模型或位置标记代表当前估计位姿；不要把展点标签当机器人。
- 雷达点代表当前测量，不是完整世界模型；与墙边大致对齐是一个定位观察线索。
- `/plan` 是规划路线，不是机器人已经走过的轨迹。
- 展点标签是配置好的目的地，不是机器人每次都从那里出生。

如果机器人停着，画面没有明显变化也可以是正常的。静态地图仍显示，不代表导航任务一定正在执行。

## 5．保存你自己的配置文件

先在终端创建目录：

```bash
mkdir -p ~/ros2_lesson6
```

在 RViz 菜单点 `File → Save Config As`，保存为：

```text
/home/sbx/ros2_lesson6/my_tour_view.rviz
```

下次打开这个配置：

```bash
source /opt/ros/jazzy/setup.bash
source ~/tour_guide_ws/install/setup.bash
export ROS_DOMAIN_ID=0
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
export LP_NUM_THREADS=2
rviz2 -d ~/ros2_lesson6/my_tour_view.rviz --ros-args -p use_sim_time:=true
```

这会恢复显示配置，不会启动 Gazebo、Nav2 或导览程序。配置文件里保存的是“怎么显示”，不是导航任务进度。

在 `Views` 面板中可选 `TopDownOrtho` 作为俯视视角；若当前版本没有这个选项，保留 Orbit，把视角调到俯视即可。

## 6．亲手观察一次导航

按第二课先停止自动导览，确认小车停住。运行第二课的客户端，给目的地 `(0.0,-3.0)`。

观察：目标发出后路径出现、机器人沿路线运动、雷达点更新、客户端最后打印 `SUCCEEDED`。若本来已经很接近目标，运动可能很小。

本课不额外用工具栏 `2D Goal Pose` 发目标，避免同时有两个操作来源。以后使用它前，也先停下自动导览。

`2D Pose Estimate` 是给定位模块一个初始位姿估计，不是把 Gazebo 小车瞬间移动到你点击的位置。原项目已经预设初始位姿，正常演示不需要随意点击。

## 7．常见报错怎么读

| 提示/现象 | 意思与检查 |
|---|---|
| Fixed Frame 不存在 | `map` TF 未建立，可能 AMCL 未 active、仿真未起或通信分组错 |
| Map 没收到消息 | Topic、QoS、map_server 是否运行 |
| RobotModel 没模型 | `/robot_description` 的发布节点和 QoS |
| LaserScan 的 TF 报错 | `laser_frame` 到 `map` 的变换是否完整、时钟是否一致 |
| Path 没数据 | 是否真的有导航任务、`/plan` 是否发布；停车时没有新路线未必是故障 |
| 坐标轴特别多 | TF 显示可以暂时关闭，不代表系统错误 |
| 窗口打不开、DISPLAY 为空 | SSH 没有图形显示环境；回桌面完成，不要随便 export 一个 DISPLAY |

先展开对应显示项的 `Status` 看文字，不要只看画面颜色。降低帧率到 10、只保留必要显示项，可以减少虚拟机负载，但不能保证所有机器都不卡。

## 8．结束后的自测

- 关闭 RViz 后，后台机器人是否还会导航？为什么？
- `/plan` 是已走轨迹还是待走路线？
- 雷达有消息却无法显示，为什么可能是 TF 问题？
- 为什么地图使用 Transient Local？
- 保存 `.rviz` 是否等于保存了 Gazebo 世界？

本课涉及鼠标操作和实际显示效果，需要你在桌面上确认；检查范围见附录。参考：[RViz Jazzy 官方用户指南](https://github.com/ros2/rviz/blob/jazzy/rviz2/doc/index.rst)、[ROS2 Jazzy QoS 说明](https://docs.ros.org/en/jazzy/Concepts/Intermediate/About-Quality-of-Service-Settings.html)。
