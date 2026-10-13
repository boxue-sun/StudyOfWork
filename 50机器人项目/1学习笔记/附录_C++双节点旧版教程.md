# ROS2 C++ 从零动手教程：自己建文件，让控制循环跑起来

当前入门请使用 [Python 版教程](01_节点与Topic_让控制循环跑起来.md)。本份保留，供以后对照 C++ 写法。

这次由你自己建目录、建文件、复制代码、编译和运行。我之前准备的 `/home/sbx/ros2_learning_ws` 可以先放着。

本课使用新目录 `/home/sbx/ros2_from_zero_ws`。教程不会提前替你创建它。
所有命令都在 **Ubuntu 虚拟机终端** 输入，所有 C++ 代码都粘贴到对应文件。
今天先完成到第 8 步，亲眼看到位置变化。代码解释可以边做边看，不用一口气背下来。

## 0．先知道这次要做什么

我们写两个 C++ 程序，每个程序运行一个 ROS2 节点。

| 程序/节点 | 谁写的逻辑 | 工作 |
|---|---|---|
| `simple_cart` | 你按教程创建 | 接收速度，根据速度更新位置，发布当前位置 |
| `move_to_x` | 你按教程创建 | 接收当前位置，判断方向，发布速度，到目标后发 0 |

这次没有 Gazebo 和 Nav2，用一个位置数字 x 表示“车在什么地方”。它是一个简单一维数学模型，主要让你体验真正的 ROS2 消息通信。

```text
move_to_x 收到位置
      ↓ 判断应该往哪里走
发布速度消息到 /learn_robot/cmd_vel
      ↓
simple_cart 收到速度，计算位置变化
      ↓
发布位置消息到 /learn_robot/odom
      ↓
move_to_x 收到新位置，再判断
```

回到原项目时：Nav2 的控制节点做更复杂的速度计算，Gazebo 的底盘插件执行速度指令。ROS2 负责节点间通信。

## 1．自己建工作区和包目录

打开一个 Ubuntu 终端，按顺序运行下面三条命令：

```bash
mkdir -p ~/ros2_from_zero_ws/src/robot_control_demo/src
cd ~/ros2_from_zero_ws/src/robot_control_demo
pwd
```

`mkdir` 是创建目录，`-p` 表示把中间缺少的目录一起创建；`~` 是你的用户主目录 `/home/sbx`。
`cd` 是进入目录，`pwd` 是显示当前目录。最后应显示：

```text
/home/sbx/ros2_from_zero_ws/src/robot_control_demo
```

理解三个名字：

- **工作区 workspace**：整个工程的容器，这次是 `ros2_from_zero_ws`。
- **包 package**：一组相关程序和配置，这次是 `robot_control_demo`。
- **节点 node**：运行时参与 ROS2 通信的程序单元，这次有两个。

我们接下来手动创建四个文件，最终目录应该是：

```text
ros2_from_zero_ws/
└── src/
    └── robot_control_demo/
        ├── package.xml
        ├── CMakeLists.txt
        └── src/
            ├── simple_cart.cpp
            └── move_to_x.cpp
```

外层 `src` 放包，包里面的 `src` 放 C++ 源码。两个同名目录属于不同层级。

## 2．自己创建 package.xml

确认还在第 1 步的包目录，然后输入：

```bash
nano package.xml
```

这会打开终端文本编辑器，新文件一开始是空白的。
把下面代码完整复制进去，只复制代码块里的内容。
Ubuntu 终端粘贴通常使用 **Ctrl+Shift+V**，也可以用终端右键菜单的粘贴。

```xml
<?xml version="1.0"?>
<package format="3">
  <name>robot_control_demo</name>
  <version>0.1.0</version>
  <description>A small C++ ROS 2 position and velocity learning exercise.</description>
  <maintainer email="student@example.com">ROS learner</maintainer>
  <license>Apache-2.0</license>
  <buildtool_depend>ament_cmake</buildtool_depend>
  <depend>rclcpp</depend>
  <depend>geometry_msgs</depend>
  <depend>nav_msgs</depend>
  <export><build_type>ament_cmake</build_type></export>
</package>
```

保存操作：**Ctrl+O → Enter → Ctrl+X**。
Ctrl+O 保存，Enter 确认文件名，Ctrl+X 退出编辑器；这些是按键，不是输入文字。

这是“包的说明书”，没有机器人控制逻辑。其中：

| 内容 | 含义 |
|---|---|
| `<name>` | 包叫什么，后面运行命令会用到 |
| `ament_cmake` | 这个包采用的 ROS2 C++ 构建方式 |
| `rclcpp` | ROS2 的 C++ 开发库，创建节点、收发消息会用到 |
| `geometry_msgs` | 包含速度等消息定义 |
| `nav_msgs` | 包含里程计等消息定义 |

## 3．自己创建 CMakeLists.txt

输入：

```bash
nano CMakeLists.txt
```

完整复制：

```cmake
cmake_minimum_required(VERSION 3.8)
project(robot_control_demo)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
add_executable(simple_cart src/simple_cart.cpp)
ament_target_dependencies(simple_cart rclcpp geometry_msgs nav_msgs)
add_executable(move_to_x src/move_to_x.cpp)
ament_target_dependencies(move_to_x rclcpp geometry_msgs nav_msgs)
install(TARGETS simple_cart move_to_x DESTINATION lib/${PROJECT_NAME})
ament_package()
```

同样用 Ctrl+O、Enter、Ctrl+X 保存退出。文件名大小写必须和上面一致。

这是“怎么编译”的说明。四处关键内容：

```cmake
find_package(rclcpp REQUIRED)
```

找到 ROS2 C++ 开发库。

```cmake
add_executable(simple_cart src/simple_cart.cpp)
```

把指定的 `.cpp` 编译成叫 `simple_cart` 的可执行程序。

```cmake
ament_target_dependencies(simple_cart rclcpp geometry_msgs nav_msgs)
```

告诉构建工具这个程序需要哪些 ROS2 库和消息包。

```cmake
install(TARGETS simple_cart move_to_x DESTINATION lib/${PROJECT_NAME})
```

构建时把程序安装到 ROS2 能找到的位置。这样以后才能用 `ros2 run` 运行。
`${PROJECT_NAME}` 是 CMake 变量，这里代表包名 `robot_control_demo`。

## 4．自己创建模拟车程序 simple_cart.cpp

输入：

```bash
nano src/simple_cart.cpp
```

把下面完整代码复制进去，保存退出：

```cpp
#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"

using namespace std::chrono_literals;

// A one-dimensional arithmetic model: position += speed * elapsed time.
// This is a learning stand-in for a base driver, not a Gazebo physics model.
class SimpleCart : public rclcpp::Node {
public:
  SimpleCart() : Node("simple_cart") {
    last_tick_ = last_command_ = Clock::now();
    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("/learn_robot/odom", 10);
    cmd_sub_ = create_subscription<geometry_msgs::msg::Twist>(
      "/learn_robot/cmd_vel", 10,
      [this](geometry_msgs::msg::Twist::ConstSharedPtr msg) {
        // Callback: ROS invokes this when a speed message arrives.
        speed_ = std::isfinite(msg->linear.x)
          ? std::clamp(msg->linear.x, -0.3, 0.3) : 0.0;
        last_command_ = Clock::now();
      });
    timer_ = create_wall_timer(50ms, [this]() { tick(); });
    RCLCPP_INFO(get_logger(), "CART_READY: x=0; waiting for velocity messages");
  }

private:
  using Clock = std::chrono::steady_clock;
  void tick() {
    const auto now_steady = Clock::now();
    const double dt = std::chrono::duration<double>(now_steady - last_tick_).count();
    last_tick_ = now_steady;
    // Stop if the controller stops sending commands for half a second.
    if (std::chrono::duration<double>(now_steady - last_command_).count() > 0.5) {
      speed_ = 0.0;
    }
    x_ += speed_ * dt;
    nav_msgs::msg::Odometry msg;
    msg.header.stamp = this->now();
    msg.header.frame_id = "learn_odom";
    msg.child_frame_id = "learn_base";
    msg.pose.pose.position.x = x_;
    msg.pose.pose.orientation.w = 1.0;
    msg.twist.twist.linear.x = speed_;
    odom_pub_->publish(msg);
    if (++ticks_ % 20 == 0) {
      RCLCPP_INFO(get_logger(), "x=%.3f m, v=%.2f m/s", x_, speed_);
    }
  }

  double x_{0.0};
  double speed_{0.0};
  unsigned int ticks_{0};
  Clock::time_point last_tick_, last_command_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_sub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SimpleCart>());
  rclcpp::shutdown();
  return 0;
}
```

### 这份代码干什么

它保存两个主要数据：位置 `x_` 和速度 `speed_`，一开始都是 0。

先看接收速度的部分：

```cpp
cmd_sub_ = create_subscription<geometry_msgs::msg::Twist>(
  "/learn_robot/cmd_vel", 10,
  [this](geometry_msgs::msg::Twist::ConstSharedPtr msg) {
    speed_ = std::isfinite(msg->linear.x)
      ? std::clamp(msg->linear.x, -0.3, 0.3) : 0.0;
    last_command_ = Clock::now();
  });
```

- `create_subscription`：创建订阅者，准备接收消息。
- `Twist`：ROS2 提供的速度消息类型。
- `/learn_robot/cmd_vel`：本练习的速度话题名，两边名字要一致。
- `10`：这里设置消息队列深度；它不是速度，也不是发布频率。
- `[this](...){...}`：C++ lambda 回调，ROS2 收到消息后调用这段代码。
- `msg->linear.x`：从收到的消息中取出前后方向的速度。
- `clamp`：把速度限制在 -0.3 到 0.3 米/秒；`isfinite` 排除无效数值。

**订阅并不是自己一直去查有没有消息，而是先登记回调，收到消息后由 ROS2 调用它。**

再看定时更新：

```cpp
timer_ = create_wall_timer(50ms, [this]() { tick(); });
```

这表示大约每 50 毫秒调用一次 `tick()`。这是实际时间，不依赖原 Gazebo 的仿真时钟。
`tick()` 里面最核心的一行是：

```cpp
x_ += speed_ * dt;
```

含义：新位置 = 原位置 + 速度 × 经过时间。
例如速度 0.2 米/秒，经过 0.05 秒，位置就增加约 0.01 米。
`dt` 使用实际经过的时间，所以没有假设定时器每次都刚好按时执行。

最后它将位置填进消息并发布：

```cpp
msg.pose.pose.position.x = x_;
odom_pub_->publish(msg);
```

`Odometry` 是 ROS2 提供的里程计消息类型，本练习主要使用其中的位置和速度。
真实机器人通常由传感器和底盘驱动提供里程计，本练习用数学计算产生它。

其他细节现在只需知道用途：`header` 描述消息时间和坐标参考，四元数 `w=1` 表示零旋转；本课没有发布 TF。
每秒打印一次是为了方便阅读，但它每 50 毫秒都在更新、发布位置。

## 5．自己创建控制程序 move_to_x.cpp

输入：

```bash
nano src/move_to_x.cpp
```

完整复制下面代码，保存退出：

```cpp
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"

class MoveToX : public rclcpp::Node {
public:
  MoveToX() : Node("move_to_x") {
    target_x_ = declare_parameter<double>("target_x", 1.0);
    speed_ = declare_parameter<double>("speed", 0.2);
    if (!std::isfinite(target_x_) || std::abs(target_x_) > 2.0 ||
        !std::isfinite(speed_) || speed_ <= 0.0 || speed_ > 0.3) {
      throw std::invalid_argument("target_x must be within [-2,2]; speed within (0,0.3]");
    }
    cmd_pub_ = create_publisher<geometry_msgs::msg::Twist>("/learn_robot/cmd_vel", 10);
    odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
      "/learn_robot/odom", 10,
      [this](nav_msgs::msg::Odometry::ConstSharedPtr msg) { on_position(msg); });
    RCLCPP_INFO(get_logger(), "CONTROLLER_READY: target=%.2f; waiting for position", target_x_);
  }

private:
  void on_position(nav_msgs::msg::Odometry::ConstSharedPtr msg) {
    // Step 1: read current position from the message.
    const double x = msg->pose.pose.position.x;
    if (!std::isfinite(x)) {
      cmd_pub_->publish(geometry_msgs::msg::Twist{});
      return;
    }
    // Step 2: decide a velocity. This simple rule is OUR code, not ROS code.
    const double error = target_x_ - x;
    geometry_msgs::msg::Twist cmd;
    if (std::abs(error) > 0.02) {
      cmd.linear.x = std::copysign(speed_, error);
      reported_arrival_ = false;
    } else {
      cmd.linear.x = 0.0;
      if (!reported_arrival_) {
        RCLCPP_INFO(get_logger(), "GOAL_REACHED: x=%.3f, target=%.2f; commanding STOP", x, target_x_);
        reported_arrival_ = true;
      }
    }
    // Step 3: publish the command. SimpleCart receives it in its callback.
    cmd_pub_->publish(cmd);
  }

  double target_x_{1.0};
  double speed_{0.2};
  bool reported_arrival_{false};
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  int result = 0;
  try {
    rclcpp::spin(std::make_shared<MoveToX>());
  } catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    result = 1;
  }
  rclcpp::shutdown();
  return result;
}
```

### 这份代码干什么

这次“怎么控制”的判断就在 `on_position()` 函数里，按三步读。

**第一步，读取位置，算离目标多远：**

```cpp
const double x = msg->pose.pose.position.x;
const double error = target_x_ - x;
```

假设目标是 1，当前位置是 0.3，那么差是 0.7，说明目标在前面。
假设目标是 -0.5，当前位置是 1，那么差是 -1.5，说明目标在后面。

**第二步，选择速度：**

```cpp
if (std::abs(error) > 0.02) {
  cmd.linear.x = std::copysign(speed_, error);
} else {
  cmd.linear.x = 0.0;
}
```

- `abs` 取绝对值，判断距离是否超过 2 厘米。
- `copysign` 保留速度大小，按 `error` 的正负给速度设置正负号。
- 差为正 → 正速度 → 向前走。
- 差为负 → 负速度 → 向后走。
- 足够接近 → 速度为 0 → 停车。

这是我们自己写的简单控制规则。ROS2 没有替我们决定这个规则。

**第三步，发布速度：**

```cpp
cmd_pub_->publish(cmd);
```

模拟车收到消息后，把自己的速度更新为这次命令，然后在定时器中更新位置。

### 回调怎么被调用

构造函数里的这段代码把订阅和处理函数连起来：

```cpp
odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
  "/learn_robot/odom", 10,
  [this](nav_msgs::msg::Odometry::ConstSharedPtr msg) { on_position(msg); });
```

意思是：收到 `/learn_robot/odom` 里的位置消息，就把这条消息交给 `on_position()`。
`this` 让 lambda 可以访问当前节点对象。`ConstSharedPtr` 是消息的只读共享指针，先理解成“收到的消息对象”即可。

### main 函数干什么

两个程序最后都有这些动作：

```cpp
rclcpp::init(argc, argv);
rclcpp::spin(std::make_shared<MoveToX>());
rclcpp::shutdown();
```

- `init`：初始化 ROS2。
- `make_shared<MoveToX>()`：创建控制节点对象。
- `spin`：持续处理节点事件，使收到消息时回调能得到执行。
- `shutdown`：退出时清理 ROS2 运行环境。

构造函数负责准备发布者、订阅者和参数。`spin` 运行期间，位置回调反复执行，控制循环因此持续运行。
类继承 `rclcpp::Node`，就可以使用 ROS2 提供的节点功能。

## 6．检查四个文件，然后编译

在当前包目录运行：

```bash
ls
ls src
```

第一条应该包含 `package.xml`、`CMakeLists.txt`、`src`。
第二条应该包含 `simple_cart.cpp`、`move_to_x.cpp`。

然后运行：

```bash
cd ~/ros2_from_zero_ws
source /opt/ros/jazzy/setup.bash
MAKEFLAGS=-j1 CMAKE_BUILD_PARALLEL_LEVEL=1 colcon build --packages-select robot_control_demo --executor sequential --parallel-workers 1 --cmake-args -DCMAKE_BUILD_TYPE=Release
```

这是把源代码编译、安装成可执行程序。限制构建并行度是为了适应你内存不大的虚拟机。
相同代码在我准备的练习目录里首次构建约 1 分 41 秒，你的实际时间可能不同。

完成时应该出现类似：

```text
Finished <<< robot_control_demo
Summary: 1 package finished
```

工作区会新增：

| 目录 | 用途 |
|---|---|
| `build` | 编译过程的中间文件 |
| `install` | 安装后的程序和环境脚本 |
| `log` | 构建日志 |

**编译完成不等于节点开始运行。** 到这里你只是得到两个可执行程序。

如果报错，保留报错文字。先解决构建问题，再继续下面的运行步骤。

## 7．在终端 A 启动模拟车

使用一个新终端作为 **终端 A**，依次执行：

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros2_from_zero_ws/install/setup.bash
export ROS_DOMAIN_ID=88
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
ros2 run robot_control_demo simple_cart
```

这几条命令的含义：

| 命令 | 用途 |
|---|---|
| source ROS 的 setup | 让这个终端认识 ROS2 的命令和库 |
| source 工作区的 setup | 让这个终端找到你刚编译的包 |
| ROS_DOMAIN_ID=88 | 给练习设置通信组编号，两个练习终端必须相同 |
| FASTDDS_BUILTIN_TRANSPORTS=UDPv4 | 沿用这台虚拟机已经验证的通信设置 |
| ros2 run 包名 程序名 | 真正启动程序 |

先别启动控制程序，观察几秒，应该一直看到：

```text
CART_READY: x=0; waiting for velocity messages
x=0.000 m, v=0.00 m/s
```

**为什么不动？** 模拟车已经在运行、发布位置，但没有收到任何速度命令。
这对应你之前的问题：程序运行、数据在发布、车在移动，是三个需要分开观察的现象。

## 8．在终端 B 启动控制程序

保留终端 A，再打开一个新的 **终端 B**，执行：

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros2_from_zero_ws/install/setup.bash
export ROS_DOMAIN_ID=88
export FASTDDS_BUILTIN_TRANSPORTS=UDPv4
ros2 run robot_control_demo move_to_x
```

两个节点发现对方可能先需要几秒；开始通信后，终端 A 的位置会逐渐增加：

```text
x=0.060 m, v=0.20 m/s
x=0.260 m, v=0.20 m/s
x=0.460 m, v=0.20 m/s
...
x=0.980 m, v=0.00 m/s
```

这是示例，实际打印的数字可能略有区别。
终端 B 会打印类似：

```text
CONTROLLER_READY: target=1.00; waiting for position
GOAL_REACHED: x=0.980, target=1.00; commanding STOP
```

`GOAL_REACHED` 就是到达目标。允许约 2 厘米误差，所以不要求恰好停在 1.000。
今天做到这里，就已经自己完成了：建包 → 写两个节点 → 编译 → 启动 → 收发消息 → 控制停车。

## 9．改启动参数，体验倒着走

在 **终端 B** 按 Ctrl+C，停止控制节点。**终端 A 保持运行**。

然后在终端 B 输入：

```bash
ros2 run robot_control_demo move_to_x --ros-args -p target_x:=-0.5
```

现在目标在后面，速度变为负数，位置应该减小，最终在 -0.5 米附近停车。

这个参数对应代码：

```cpp
target_x_ = declare_parameter<double>("target_x", 1.0);
```

意思是声明一个叫 `target_x` 的参数，默认值为 1.0；启动命令可以提供其他值。
`--ros-args -p target_x:=-0.5` 就是在启动时把它设置成 -0.5。

停掉前一个控制节点后，还可以试：

```bash
ros2 run robot_control_demo move_to_x --ros-args -p target_x:=1.0 -p speed:=0.1
```

速度减半，行驶过程会更慢。目标限制在 [-2,2]，速度限制在 (0,0.3]。
这些值是启动时读取的参数，因此不用重新编译。
一次保持一个控制节点，避免两个程序同时给同一个模拟车发速度。

## 10．亲手改一行控制代码，再编译

先在两个练习终端分别按 Ctrl+C，停止两个节点。

打开代码：

```bash
nano ~/ros2_from_zero_ws/src/robot_control_demo/src/move_to_x.cpp
```

找到：

```cpp
if (std::abs(error) > 0.02) {
```

只把 `0.02` 改成 `0.10`，保存退出。
含义是到目标 10 厘米范围内就停车。默认目标为 1 米，从 0 出发时应该在约 0.9 米附近停车。

重新编译：

```bash
cd ~/ros2_from_zero_ws
source /opt/ros/jazzy/setup.bash
MAKEFLAGS=-j1 CMAKE_BUILD_PARALLEL_LEVEL=1 colcon build --packages-select robot_control_demo --executor sequential --parallel-workers 1 --cmake-args -DCMAKE_BUILD_TYPE=Release
```

重新按第 7、8 步启动两个节点，记录停止位置。
模拟车重新启动后位置从 0 开始，否则你会在上一次的位置上继续实验。
结束后把 `0.10` 改回 `0.02`，再构建，保留修改前后的输出作为自己的练习记录。

**参数改变**是在启动时换数据；**源码改变**是修改程序，需要重新编译并启动新程序。

## 11．停止控制节点，会发生什么

你可以让它向目标行驶，在途中对终端 B 按 Ctrl+C。
约半秒后，终端 A 的速度会变为 0，位置不再变化。

原因在模拟车的代码里：

```cpp
if (std::chrono::duration<double>(now_steady - last_command_).count() > 0.5) {
  speed_ = 0.0;
}
```

它判断超过半秒没有新速度命令，就自行把速度归零。
这是本练习明确实现的命令超时处理；不能直接推断原项目的所有节点也有相同规则。

停止整个练习：对终端 A 和 B 都按 Ctrl+C。
回原导览项目时打开新的终端，避免沿用练习的 ROS_DOMAIN_ID=88 设置。

## 12．遇到问题，按现象查

| 现象 | 先检查什么 |
|---|---|
| `ros2: command not found` | 当前终端是否 source 了 `/opt/ros/jazzy/setup.bash` |
| 找不到 `robot_control_demo` | 是否构建成功，是否 source 了本课工作区的 install/setup.bash |
| 找不到可执行程序 | CMakeLists.txt 是否包含两个 add_executable 和 install，文件是否保存 |
| C++ 编译报错 | 文件是否复制完整、文件名是否正确；把第一段具体报错发给 AI |
| 两个节点都启动，位置一直是 0 | 两个终端是否都设置 ROS_DOMAIN_ID=88，话题名是否一致，等待几秒看通信是否建立 |
| 运行到旧代码 | 停掉旧节点，重新构建；新终端只 source Jazzy 和本课工作区，避免混入此前准备的学习工作区 |
| 修改 target_x 没效果 | 先 Ctrl+C 停止旧控制节点，再运行带参数的新命令 |

复制代码时不要把 Markdown 的三个反引号复制进文件。
本课不依赖 ROS2 CLI 的话题查询命令，核心现象直接看两个节点的输出即可。

## 13．本课你实际接触到了哪些基础

| 基础 | 在哪体现 |
|---|---|
| 工作区、包、构建 | 建目录、package.xml、CMakeLists.txt、colcon |
| ROS2 C++ 节点 | 类继承 rclcpp::Node |
| 发布、订阅、消息类型 | Twist 速度、Odometry 位置 |
| 回调 | 收到位置后执行 on_position |
| 定时器 | 每 50 毫秒更新模拟位置 |
| 参数 | target_x、speed |
| 反馈控制 | 根据当前位置不断调整速度，接近目标后停车 |
| 基础 C++ | 类、成员变量、lambda、shared_ptr、条件判断 |

本课没有实现路径规划、激光避障、AMCL、Nav2 Action 或 Gazebo 物理仿真。
后续再逐层接回原项目，今天先能解释自己创建的这两个节点。

请你用自己的话回答：

1. 控制判断到底在哪个函数里？
2. 两个节点分别发布什么、订阅什么？
3. 为什么只启动模拟车也有位置数据，但车不动？
4. 目标换到后面，代码为什么会发负速度？
5. 代码修改后，为什么需要编译并重启？

回答这些比先背面试答案更能检验你有没有理解。

## 14．验证范围和参考

教程里的四个文件取自已在用户 VM 编译、运行验证过的练习代码，逻辑保持一致。
已验证：无控制节点时静止、正目标前进到达、负目标倒退到达、控制退出后超时停车。
本课的新工作区由用户亲自创建；教程中的手动创建过程、改误差后的实验尚未替用户执行。

参考：[ROS2 官方 Jazzy C++ 发布订阅教程源码](https://raw.githubusercontent.com/ros2/ros2_documentation/jazzy/source/Tutorials/Beginner-Client-Libraries/Writing-A-Simple-Cpp-Publisher-And-Subscriber.rst)。
