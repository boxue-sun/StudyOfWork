# 2026-8-3

一、整体概述
airos-edge 是 智路OS（ZhiluOS） 的路侧操作系统部分，源自百度 Apollo 生态,是全球首个开源开放的 智能网联路侧单元操作系统 。

二、啥叫路侧？

三、大致流程：
感知（眼睛）：路口的摄像头、雷达把原始数据（视频流、点云）发给 MEC。
计算（大脑）：MEC 利用强大的 GPU 算力，瞬间把这些原始数据“翻译”成有用的信息（比如：前方50米有辆大货车、当前是红灯）。
通信（嘴巴）：RSU（路侧通信单元）把 MEC 算好的“精简信息”，通过无线电广播给附近的 OBU（汽车）。

为什么必须这样？
因为雷达和摄像头的数据量太大了（每秒几 GB），如果让 RSU 直接发给 OBU，带宽根本撑不住，而且汽车也处理不过来。所以必须先让 MEC 在本地算好，RSU 只负责当个“传话筒”，把算好的结果（可能只有几 KB）发给汽车。

四、什么叫rtsp流？
    RTSP（Real-Time Streaming Protocol，实时流协议）是用于控制实时音视频流传输的网络协议，你可以把它理解成“视频直播的遥控器”。它的核心作用不是直接传输视频数据，而是通过指令控制媒体流的播放、暂停、快进等操作，而真正的视频数据传输通常由另一个协议（如 RTP/RTCP）负责。

五、SSH远程控制
    在终端或者vscode中输入 ssh sunboxue@172.20.65.6 然后输入密码进行连接

# 2026-8-4

一、smb协议：就是同一个局域网下 然后进行共享文件夹的方式
    方法就是先打开smb权限，然后在文件管理器中 输入\\＋局域网，比如\\10.255.20.21/gsafety/中转站/张恩伟/交接 然后输入用户名＋密码 ，用户名前面要加ghrdc\sunboxue。

二、现在mentor给我了这几个任务吧 有点懵逼
首先说我们是搞应用层的 然后说让我以后可能实现对MEC的负载率自身状态的获取 然后同时设计接口能够适配各厂家的MEC
1.连接Jetson这个板子  然后让我可以远程控制这个板子了。
2.交接文件 也就是这个文件夹项目。 "C:\Users\sunboxue\Desktop\交接"
3.git文件  在这个电脑上的  "C:\Users\sunboxue\Downloads\airos2.0-optimization-base" 这个路径的这个

目前给我的小任务是
在这个板子上写一个小demo cpp语言 然后获取一下这个gpu的负载率什么的
请你帮我梳理一下他的用意 和 我为了能有实习的产出的东西 我该怎么做

三、
这个板子的jtop指令出现的结果：
![1785821778441](image/智路OS学习记录1/1785821778441.png)

四、NVML是啥？
    NVML（NVIDIA Management Library）是 NVIDIA 官方提供的 GPU 监控接口，其头文件和动态库通常随 CUDA Toolkit 或 NVIDIA 驱动一起安装。
五、遇到的问题 运行cpp文件 需要安装包 安装包的下载网络不好
    解决：首先 备份当前源 .bak是备份文件 sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
    然后修改源 sudo sed -i 's/mirror.iscas.ac.cn/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list

六、内核接口方式读取数据：
    Linux 内核接口（sysfs） 读取方式
    /sys/class/thermal/thermal_zoneX/temp 是 Linux 内核提供的标准虚拟文件，属于 sysfs（系统文件系统） 的一部分。

/sys/class/thermal/：这是内核中“热管理”子系统的类目录。

thermal_zoneX：代表一个“热区域”（X 是编号，如 0、1、2）。每个热区域对应一个温度传感器（CPU、GPU、DDR 或主板上的某个点）。

temp：这个文件里只有一个数字，就是该传感器当前的温度值。

七、Chat

思考过程

## MonitorService Demo 跑通笔记

### 【前提】

必须在 Linux 环境 运行（WSL2/远程Linux服务器/Jetson设备），Windows无法直接编译运行。

### 步骤1：环境准备（Linux）

安装编译工具：

### 步骤2：编译

进入项目根目录，执行：

✅ 编译完成后， build/ 下会出现 device_probe_demo 可执行文件。

### 步骤3：修改配置

编辑 config/config.json ，核心配置项：

```
{
  "device_probe": {
    "platform": "jetson",      // 【必填】Jetson设备用"jetson"，普通Linux用"linux"
    "gpu_enable": true,        // 【必填】开启GPU温度采集
    "interval_sec": 1,         // 采集间隔（秒），按需调整
    "docker_check": false,     // 无Docker设为false
    "cameras": [],             // 无设备留空
    "radars": [],
    "lidars": [],
    "rsus": []
  }
}
```

### 步骤4：运行

```
cd build
./device_probe_demo ../config/config.json
```

### 步骤5：停止

按 Ctrl + C 终止进程。

# 20260805

一、jeston探测类的构造
二、CPU占用率的计算
    我总结一下这个CPU占用率的计算：就是有一个路径中存放着cpu的各个占用时间
    然后把这些加起来 计算cpu总时长
    然后再更新历史快照 用当前的减去历史的 就是这段时间的数据 包括cpu的跑的总时间和空闲时间 然后占用率就是1-空闲率
    Linux CPU使用率计算笔记：
    CPU使用率通过读取/proc/stat文件的第一行数据计算得出，该文件记录了系统启动以来CPU在各个状态下的累计时间片（单位jiffies），包括user（用户态）、nice（低优先级用户态）、system（内核态）、idle（空闲）、iowait（等待I/O）、irq（硬中断）和softirq（软中断）七个核心字段。
    计算的核心思想是“差值法”——在函数内使用static变量保存上一次采样的总时间（user+nice+system+idle+iowait+irq+softirq）和空闲时间（idle），每次调用时读取当前值并计算两次采样之间的增量，得到这段时间内CPU的总运行时长和空闲时长，然后用公式占用率=(1 - 空闲增量/总增量)×100%得出百分比。
    首次调用因无历史数据会返回0，属于正常现象；该计算方式存在两个注意点：static变量在多线程环境下需要加锁保护，以及iowait在传统监控中被计入总时间而非空闲时间（视为忙碌状态的一部分）。这种“两次采样算差值”的方法不仅是CPU监控的标准范式，也适用于Linux下的网络流量（/proc/net/dev）和磁盘I/O（/proc/diskstats）等所有基于累加计数器的监控场景。

三、CPU温度的计算
    直接一个路径中存的是温度 /sys/class/thermal/thermal_zone数字/temp
    然后直接返回

四、内存占用率
    路径：/proc/meminfo
    如图所示
    ![1785899315054](image/智路OS学习记录1/1785899315054.png)
    先读第一个存到key中 然后读mem的总值和空闲值 读到这俩就不读了 提前退出循环
    然后计算：总-空闲 /总 就是内存的占用率

五、该板子GPU的路径：
    频率：/sys/class/devfreq/17000000.ga10b/cur_freq
    没找到的原因：一直找的是ga1 结果名字是ga10

六、怎么学习这种板子呢?

七、用CUDA API得到的44.22%是什么数据？
    cudaMemGetInfo() 返回的是 CUDA 驱动管理的显存池使用率 ，包含：
    - CUDA 驱动自身开销 ：CUDA 驱动程序运行时占用的内存
    - 内存池 (Memory Pool) ：CUDA 预留的内存池，用于快速分配
    - 上下文数据 ：CUDA context 的管理开销
    -  应用分配 ：CUDA 应用实际分配的内存
    - 系统开销 ：CUDA 运行时的其他开销

    不包含的部分
    - CPU 内存 ：系统 RAM 的使用情况
    - GPU 渲存 ：显示相关的显存（除非 CUDA 使用）
    - 其他硬件 ：VPU、ISP 等硬件的内存

# 2026 08 06

一、之前的gpu一直温度是负的 还有负载率也是0
    是因为gpu没有真的跑动 所以才是这样的 需要程序来运行gpu

二、手敲增加获取GPU频率的功能
    已完成

三、学一下cmake

四、学一下demo怎么写的

五、获取gpu内存
    因为gpu cpu内存是一起的 所以一般都是显示的是二者一起的
    但是jtop就能够显示gpu的共享内存
    所以就想用jtop同样的方式来获取：sudo cat /sys/kernel/debug/nvmap/iovmm/maps 里面直接有总共的total
    直接读取 但是需要权限
    如果读不到就降级用cuda api读取 没有那么精准 读取的是
        进程实际使用的 GPU 内存（~443 MB）：代码、AI模型、视频缓冲区等真正占用的逻辑内存。

    预分配的内存块（~2-5 GB）：CUDA为提高分配速度预先申请的大块内存池。

    缓存和碎片（~1-3 GB）：未释放的小块内存、内存对齐及分配产生的碎片浪费。

    驱动内部使用（~1-2 GB）：CUDA驱动自身维护的上下文、数据结构等。

    其他开销（~0.5 GB）：cublas、cudnn 等加速库的预留空间。

六、proc是啥？
  是Linux内核暴露给用户空间的接口 可以用来查数据 虚拟文件系统
  /proc/ → 通用系统信息
/sys/ → 设备驱动、硬件相关信息（你读的 nvmap 就在这里）

# 20260807

## 一、CMake知识点

## 1.注释用#

## 2.第一行指定cmake最低版本

```
cmake_minium_required（VERSION 3.10）
```

## 3.第二行必须是项目名称 和版本 project()

如：

```
project(CalculatorDemo
    VERSION 1.0.0
    DESCRIPTION "一个简单的计算器演示项目"
    LANGUAGES CXX
)
```

## 4.指定c++标准   `CMAKE_CXX_STANDARD`：指定 C++ 标准（11/14/17/20）

```
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## 5.添加可执行文件

add_executable(calculator
    src/main.cpp
    src/calculator.cpp
)

## 6.包含目录 **告诉编译器"去哪个文件夹找头文件（.h/.hpp）CMAKE_CURRENT_SOURCE_DIR是当前cmake所在路径**

target_include_directories(calculator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

## 7.输出目录设置

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

## 8.打印信息

`STATUS` 是 `message()` 命令的 **日志级别** ，控制信息在终端显示的方式和颜色。

常用日志级别

| 级别               | 作用         | 显示效果                           | 适用场景                              |
| ------------------ | ------------ | ---------------------------------- | ------------------------------------- |
| `STATUS`         | 普通状态信息 | 白色/正常颜色（前面有`--` 前缀） | **最常用** ，显示进度、发现啥了 |
| `WARNING`        | 警告信息     | 红色/黄色                          | 有问题但不致命                        |
| `AUTHOR_WARNING` | 开发者警告   | 红色                               | 告诉开发者注意                        |
| `SEND_ERROR`     | 发送错误     | 红色                               | CMake 继续但标记错误                  |
| `FATAL_ERROR`    | 致命错误     | 红色                               | **停止执行** ，CMake 退出       |
| （无）             | 普通输出     | 无特殊格式                         | 调试用                                |

`PROJECT_NAME` 是 CMake  **内置变量** ，自动存储 `project()` 命令设置的项目名。

打印效果

**cmake**

```
message(STATUS "项目名称: ${PROJECT_NAME}")
```

输出：

**text**

```
-- 项目名称: MyApp
```

message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "项目版本: ${PROJECT_VERSION}")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")

# 二、工厂模式

类比说法：

想象你去**麦当劳**点餐：

* 你不用自己学怎么做汉堡、炸薯条。
* 你只需要对柜台服务员（ **工厂** ）说：“我要一个巨无霸”（ **传入参数** ）。
* 服务员转身去后厨（ **创建逻辑** ），把做好的巨无霸端给你（ **返回产品** ）。

**工厂模式**就是程序里的“麦当劳服务员”：**专门负责帮你创建对象，把复杂的创建过程隐藏起来。**

如下 就是进行了一个判断 传参是哪个平台 就返回哪个平台的ptr

如果没有工厂模式的话 你就需要在main中要哪个平台 每次都要创建 有了的话直接调用工厂模式即可

```c++
#include "platform_probe_factory.h"

#include <iostream>

#include "jetson_probe.h"
#include "linux_probe.h"

namespace fusion_perception::monitor_service {

std::unique_ptr<PlatformProbe> PlatformProbeFactory::Create(const std::string& platform) {
    std::cout << "Create platform: " << platform << std::endl;
    if (platform == "jetson") {
        return std::make_unique<JetsonProbe>();
    }
    if (platform == "linux") {
        return std::make_unique<LinuxProbe>();
    }
    std::cout << "Unknown platform: " << platform << ", fallback to linux" << std::endl;
    return std::make_unique<LinuxProbe>();
}

}  // namespace fusion_perception::monitor_service
```

# 三、docker

你可以把Docker想象成一个 **“安装了完整操作系统和所有开发工具的便携式电脑”** 。但它不是硬件，它是一个 **“软件环境”** 。

具体分三样东西：

* **镜像（Image）** ：相当于一个 **“安装光盘”** 或者 **“系统备份文件”** 。它里面打包好了Ubuntu系统、编译器、各种依赖库，所有东西都齐全。这是只读的，不能改。
* **容器（Container）** ：相当于你用那张“安装光盘” **装好并正在运行的“虚拟电脑”** 。镜像是一个死文件，而容器是活着的、正在运行的环境。你敲命令、跑代码，都是在容器这个“运行环境”里进行的。
* **宿主机（Host）** ：就是你现在坐着的这台真实的物理电脑/服务器。

# 四、git指令

    git branch 查看当前代码在哪个分支

git status **让你随时查看“当前代码文件夹里，到底发生了哪些变化”**

# 五、find / -name "*sunboxue*" 2>/dev/null 模糊搜素

# 六、scp sunboxue@172.20.65.6:/data/sunboxue/monitorservice3.tar.gz C:\Users\sunboxue\Desktop\ 实现linux下载到winodws

下次来了就是在docker中获取路径 。建立新的docker容器 防污染

# 2026-8-10

一、遇到的问题：

    docker exec -it airos2.0-shunyi-3.4_dev_zew /bin/bash

    docker中运行带路径的命令时候 会显示没有。解决方法：挂载。`-v "/sys/kernel/debug:/sys/kernel/debug:ro"`。其中ro是给他一个只读权限

    docker run -p 80:80 是绑定宿主和容器的端口 先外后内

    代码正常运行 但不知道是哪里的错误 可以加入fprintf看调试信息。

    在宿主中执行的cmake 保存的是以前的代码的路径 所以在docker中再make 找不到路径。需要重新cmake，camke的时候后面要加上cuda的路径。
camke .. -DCUDA_TOOLKIT_ROOT_DIR="/usr/local/cuda-11.4/targets/sbsa-linux/lib/libcudart.so"
为啥在宿主电脑中 直接cmake就行？因为他自动去找标准的目录位置中找cuda  CUDA 通常装在这里 ——`/usr/local/cuda`。而docker中不是存在这里。

改代码后需要重新cmake吗？

1. 黄金法则（面试必背）
   cmake：负责“搭架子”。它根据 CMakeLists.txt 生成 Makefile，决定哪些文件需要编译、链接什么库、用什么编译选项。

make：负责“搬砖”。它根据 Makefile 里的指令，去编译具体的 .cpp 文件。

结论：

只要没动 CMakeLists.txt，没动库路径（比如 CUDA 地址），没增删源文件（.cpp/.h），就绝对不要重跑 cmake，直接 make 即可。

2. 什么时候必须重跑 cmake ..？（红线）
   场景	举例	需要 cmake？
   增删源码文件	新建了一个 new_feature.cpp，或在 CMakeLists 里删了某个文件	✅ 必须（Makefile 里没记录新文件名）
   修改编译选项	把 -std=c++11 改成 -std=c++17，或添加 -g 调试选项	✅ 必须
   修改依赖库路径	换了一个 CUDA 路径，或新增了 -lopencv	✅ 必须（这就是你之前指定 CUDA 地址的原因）
   修改了 CMakeLists.txt	改了 install 目录或链接了第三方包	✅ 必须
   清空了 build 目录	执行了 rm -rf *（就像你刚才做的那样）	✅ 必须（因为 Makefile 都没了）
3. 什么时候只需要 make？（绿灯）
   场景	举例	需要 cmake？
   修改了 .cpp 或 .h 文件内容	修了个分号、加了个打印语句、修了一个算法逻辑	❌ 不需要（Makefile 里已经记录了这个文件，make 根据时间戳发现变了，会自动重编它）
   只改注释	改了一下变量命名的解释	❌ 不需要
4. 为什么 make 能感应到你改了 .cpp？（时间戳原理）
   当你执行 vi jetson_probe.cpp 并保存时，这个文件的修改时间（Timestamp）变了。

make 运行时会检查：目标文件（.o）和源文件（.cpp）的修改时间。

发现源文件（.cpp）比目标文件（.o）更新，make 就知道“源码变了，得重新编译这个文件”。

它完全不需要 cmake 参与，因为编译这个文件的指令早就在 Makefile 里写好了。

5. 给你的实战建议（防止犯错）
   日常开发：改完 .cpp 直接 make -j$(nproc)。如果编译报错，改完再次 make。千万不要每次改个打印就重跑 cmake，那会浪费时间。

什么时候主动重跑 cmake：

你执行了 rm -rf * 把 Makefile 删了。
你拉取了同事的新代码，发现新增了文件。
你要切换 Debug/Release 模式（cmake -DCMAKE_BUILD_TYPE=Debug ..）。

二：激活虚拟环境 在虚拟环境运行性能监控平台

![1786352472240](image/智路OS学习记录1/1786352472240.png)

1. 进入项目目录

cd /work/sunboxue/airos2.0-shunyi-3.4/cyber_performance

2. 激活虚拟环境

source .venv/bin/activate

3. 运行程序（后台方式）

python3 cyber_performance_standalone.py > /tmp/performance_monitor.log 2>&1 &

4. 查看启动日志，确认运行成功

sleep 3 && tail -20 /tmp/performance_monitor.log

http://<你的IP>:5000 http://localhost:5000

kill $(pgrep -f cyber_performance_standalone.py)是停止程序

# 2026-8-11

1.顺义项目里的JSON是干嘛的？

    性能上报给运维系统 让后台监控人员看到

***完整的数据流向（串起来理解）***

整个流程是这样的：

1. **数据采集** ：MEC上的监控程序（Agent）每10分钟（默认周期）读取一次本机的CPU、GPU、内存、磁盘、网络状态。
2. **封装JSON** ：按照表格23定义的格式（包含 `load`、`temp`、`uti` 等固定字段），把数据打包成这个JSON结构。
3. **MQTT上报** ：MEC通过MQTT协议，把JSON消息发到平台指定的Topic（`rscu/xxx/running-info/up`）。
4. **平台解析** ：云控平台的后端服务订阅了这个Topic，收到消息后解析JSON，把数据存入数据库。
5. **可视化展示** ：运维人员在大屏或告警界面上看到MEC的实时负载曲线、温度趋势，如果温度过高或磁盘满，系统自动触发告警通知。

2.middleware/protocol/om_common/performence_utils.cc中的修改：
	增加getGPuinfo

middleware/protocol/om_common/performence_utils.h的修改：

    增加getGPuinfo

## Git 基本流程

### 1. 初始化仓库

```bash
# 在项目目录下初始化 Git 仓库
git init

# 或者从远程仓库克隆
git clone <远程仓库地址>
git clone -b +分支名字 +仓库地址
```

### 2. 日常开发流程（核心三步曲）

```
编写代码 → git add → git commit → git push
```

| 步骤               | 命令                         | 说明                        |
| ------------------ | ---------------------------- | --------------------------- |
| **查看状态** | `git status`               | 看哪些文件改了              |
| **暂存文件** | `git add .`                | 把所有改动放入暂存区        |
| **提交**     | `git commit -m "描述信息"` | 保存一个版本快照            |
| **推送**     | `git push`                 | 上传到远程仓库（如 GitHub） |

### 3. 三个重要区域

```
工作区（你写代码的地方）
   │
   │  git add
   ▼
暂存区（准备好要提交的内容）
   │
   │  git commit
   ▼
本地仓库（本地的版本历史）
   │
   │  git push
   ▼
远程仓库（服务器上的仓库）
```

### 4. 常用命令速查

```bash
git pull --rebase 变基合并 把开始节点放到另一个提交的版本后

git pull origin + 分支
git checkout + -b 新分支名字
git checkout + 其他分支 切换分支

当要有新的要pull的时候 先保存当前的
git stash 
然后
git pull
然后
git stash pop

git status          # 查看当前状态
git add .           # 暂存所有改动  git restore --staged .myfusion/  可以用来撤销指定的add
git commit -m "说明" # 提交
git log             # 查看提交历史
git diff            # 查看具体改了什么

git pull            # 拉取远程最新代码
git push            # 推送代码到远程

git branch dev      # 创建新分支
git checkout dev    # 切换到 dev 分支
git merge dev       # 合并 dev 分支到当前分支
```

### 5. 分支（Branch）

分支是 Git 最强大的功能，可以让你在不影响主代码的情况下开发新功能：

```
main（主分支，稳定代码）
  └── dev（开发分支）
       └── feature-gpu（功能分支）
```

### 6. 一个典型的完整例子

```bash
# 第一次使用
cd /work/sunboxue/airos2.0-shunyi-3.4
git init
git remote add origin <远程仓库地址>

# 每天的开发流程
git pull                          # 1. 先拉取最新代码
# ... 写代码 ...
git status                        # 2. 看看改了啥
git add .                         # 3. 暂存改动
git commit -m "添加GPU温度监控"     # 4. 提交
git push                          # 5. 推送到远程
```

### 7. 出错了怎么办？

```bash
git checkout -- 文件名    # 撤销工作区的修改
git reset HEAD 文件名     # 取消暂存
git reset --hard HEAD    # 危险！回退到上次提交（丢失所有未提交改动）
```

git clone超时问题：

    原因没有代理权限，波哥用他目录下的一个脚本 直接给我开启的权限。

   但是克隆的时候 还是卡在一个环节不动：需要输入一下用户名和密码 AD域的

    git clone -b optimization http://你的用户名:你的Token@10.255.10.29/cv2x_application/airos2.0.git

  输入之后 记得清一下历史 history -c 避免密码暴露

# 2026-8-14

## 1、文档3（V3.3）→ 文档4（V3.4）的改动

根据版本历史中 V3.4 的记录，以及逐项对比两个文档的具体内容，V3.4 相比 V3.3 有以下改动：

### 一、授时数据上报（7.3.7.1）— 单时差改双时差

|              | V3.3                           | V3.4                                                  |
| ------------ | ------------------------------ | ----------------------------------------------------- |
| 授时时差字段 | `timeDifference`（一个字段） | 拆成`ptpDifference` + `gpsDifference`（两个字段） |
| 上报说明     | 只报一个总时差                 | 每次同时上报设备与 PTP、GPS 两个授时源的时差          |

### 二、设备注册新增字段

| 设备类型              | 新增字段        | 说明                       |
| --------------------- | --------------- | -------------------------- |
| 摄像机（7.1.2.1.2.1） | `apiDataVer`  | 设备与测试服务系统接口版本 |
|                       | `hardwareVer` | 设备的硬件版本             |
|                       | `softwareVer` | 设备的软件版本             |
| 雷达（7.2.2.2.2）     | `apiDataVer`  | 同上                       |
| MEC（7.3.2.2.2）      | `apiDataVer`  | 同上                       |

### 三、新增"取消升级"接口

V3.3 没有取消升级功能，V3.4 为**三类设备**都新增了取消升级：

| 设备                         | 新增内容                                              |
| ---------------------------- | ----------------------------------------------------- |
| **摄像机** （7.1.6.3） | 新增表 8_2 取消升级消息集 + 表 8_3 取消升级状态反馈   |
| **雷达**               | 新增取消升级消息集 + 状态反馈                         |
| **MEC** （7.3.6.2）    | 新增表 26_3 取消升级消息集 + 表 26_4 取消升级状态反馈 |

升级状态反馈也新增了 `failed：升级失败` 状态。

### 四、新增"MEC 自检结果上报"（7.3.11）

V3.3 **没有**这个接口，V3.4  **新增** ：

| 字段                 | 类型    | 说明                                                          |
| -------------------- | ------- | ------------------------------------------------------------- |
| `faultType`        | Integer | 故障类型（1~12，如信号机原始数据、感知数据、事件、v2x信控等） |
| `faultCategory`    | Integer | **故障类别** ：0=数据异常，1=链路异常                   |
| `faultStatus`      | Integer | 0=故障消失，1=故障产生                                        |
| `faultStartTime`   | Long    | 故障开始时间                                                  |
| `faultStopTime`    | Long    | 故障结束时间                                                  |
| `faultDescription` | String  | 故障描述                                                      |

对应新增 Topic：`rscu/{rscuEsn}/detection/data-status/up`

### 五、信号灯检测（7.3.9）改动

|          | V3.3                     | V3.4                                                                                     |
| -------- | ------------------------ | ---------------------------------------------------------------------------------------- |
| 上报时机 | **定频上报** ，1Hz | **触发上报** ，异常开始时发一次，异常结束时发一次                                  |
| 故障类型 | 7种（0~7）               | 增加到 10种，新增 8=全红、9=全绿、10=黄闪                                                |
| 新增字段 | 无                       | `faultStatus`（故障状态）、`faultStartTime`、`faultStopTime`、`faultDescription` |

### 六、新增"上报路口车辆情况"

V3.3  **没有** ，V3.4 为摄像机新增了此接口：

| 字段                | 类型    | 说明                           |
| ------------------- | ------- | ------------------------------ |
| `allVehicles`     | Integer | 上报时间内全部车辆通过数量     |
| `totalVehicles`   | Integer | 上报时间内自动驾驶车辆通过数量 |
| `illegalVehicles` | Integer | 上报时间内自动驾驶违法车辆数量 |

上报周期：1 分钟。

### 七、数据量统计接口调整

| 操作                     | 说明                                             |
| ------------------------ | ------------------------------------------------ |
| **删除** 原 7.3.10 | "统计向运维系统外系统发送数据量"接口及对应 Topic |
| **新增**           | "MEC 统计向运维系统外发送数据量" Topic           |

### 八、新增 Topic 定义

V3.4 在 Topic 表中新增了：

| Topic                                       | 说明               |
| ------------------------------------------- | ------------------ |
| `rscu/{rscuEsn}/upgrade/cancel`           | 远程升级取消       |
| `rscu/{rscuEsn}/upgrade/cancel/ack`       | 取消远程升级反馈   |
| `rscu/{rscuEsn}/detection/data-status/up` | MEC 自检状态上报   |
| `rscu/{rscuEsn}/signal/data-status/up`    | 信号机数据状态上报 |
| 摄像机"上报路口车辆情况" Topic              | 路口车辆统计上报   |

### 九、附录

V3.3  **没有附录** ，V3.4  **新增附录** ，包含：

* 1.1 设备名称定义（8位命名规则）
* 1.2 目标类型定义（person/bicycle/car 等 256 种）
* 1.3 精度等级（位置/速度/航向/加速度 4 类）
* 1.4 渠道来源
* 1.5 数据来源/设备类别
* 1.6 事件类别（倒车/逆行/闯红灯等 32 种）
* 1.7 目标状态（静止/运动）

### 总结一句话

V3.4 相比 V3.3 的核心改动就是： **授时拆双时差、注册加版本号、新增取消升级、新增 MEC 自检、信号灯检测改为触发上报并增加故障类型、新增路口车辆统计、新增附录** 。

2、3.4代码未实现的：

    （1）路口车辆情况上报 — 只有 Topic 壳子，没有数据模型

    ![1786695884289](image/智路OS学习记录1/1786695884289.png)

 **状态** ：只搭了个 Topic 的壳，里面跑的数据内容还没写。

    （2）摄像机/雷达注册接口缺少版本字段

![1786695872770](image/智路OS学习记录1/1786695872770.png)

 **对比** ：MEC 注册（`data_basic_info.h`）已经加了 `apiDataVer`，但摄像机和雷达的注册结构体没加。

    （3）MEC 统计向运维系统外发送数据量 — 完全没有

![1786695863535](image/智路OS学习记录1/1786695863535.png)

  3、 3.3和3.4的代码的差异

    （1）新增 cyber_performence 是python的web性能监控工具

    （2）新增OS	OS是编译打包后的运行包 是部署产物目录  跑过后就会有 3.3没跑

    （3） 新增procotol是独立协议目录  运行是存储数据的地方 是运行后读或写的数据  跑过后就会有 3.3没跑

    （4） param存放设备参数和配置

    （5）**Cyber RT** 是百度 Apollo 自动驾驶开源的 **运行时框架** ，专门给自动驾驶/车路协同场景用的。

    （6）V3.4-auth空  是认证升级的预留位置**认证模块的作用** ：给每台设备生成唯一的"硬件指纹"，然后基于指纹生成许可证（License），确保软件只能在授权的设备上运行。

    （7）`sound_player/`是移除的

    （8） 扩展了Topic配置  Topic是MQTT通信的消息频道名  设备通过MQTT和平台通信 每种数据走不同的频道

    （9）om模块小修小改

下面都无改动

| 新增功能                                                | 代码变化                           |
| ------------------------------------------------------- | ---------------------------------- |
| 单时差改成 双时差授时（ptpDifference + gpsDifference） | 修改了授时上报的数据结构和处理逻辑 |
| MEC 自检结果上报（含 faultCategory）                    | 新增了自检结果的处理和上报代码     |
| 设备注册新增版本字段                                    | 修改了注册消息的组装逻辑           |
| 新增 Topic 的订阅和发布                                 | 新增了消息处理函数                 |

1）

V3.3 的问题：
  平台收到 timeDifference = 123
  → 不知道是 PTP 不准还是 GPS 不准
  → 没法判断该信哪个来源

V3.4 的改进：
  平台收到 ptpDifference = 100, gpsDifference = 23
  → GPS 偏差小（23μs），PTP 偏差大（100μs）
  → 优先信任 GPS 时间
  → 可以分别监控两个授时源的健康状态

MEC 设备定期做自我诊断，检查各个子系统是否正常

faultCategory = 故障分类

V3.4 新增了 `faultCategory` 字段，用来告诉平台 **这个故障属于哪一类** ：

| faultCategory 值 | 含义               | 例子                                   |
| ---------------- | ------------------ | -------------------------------------- |
| 0                | **数据异常** | 传感器数据丢失、数据格式错误、数据超时 |
| 1                | **链路异常** | 网络断开、MQTT 连接失败、设备通信中断  |

V3.3 vs V3.4

 **V3.3** ：自检结果只报"正常/异常"，不分类

```text
自检结果：异常← 平台只知道"出问题了"，但不知道是什么类型的问题
```

 **V3.4** ：自检结果加上故障分类

```text
自检结果：异常faultCategory：1（链路异常）← 平台知道是"网络/通信层面的问题"，可以针对性处理
```

4.动态库和静态库的区别理解
	动态库.so是在编译时让可执行文件记住了动态库的名字 等可执行文件运行时就去找对应的.so文件 加载到内存

    静态库.a 是在编译时候把所有代码拷贝到了可执行文件 不用找了

5、bazel：谷歌开源编译构建工具 ，适用于大型项目，C++/Python/Java/Proto 全都行

    核心内容：1 WORKSPACE  根目录标记 告诉哪里是根目录

![1786698420827](image/智路OS学习记录1/1786698420827.png)

    2 BUILD 每个目录的编译说明书

    3 Target 编译目标

    格式：`//包路径:目标名`

![1786698641344](image/智路OS学习记录1/1786698641344.png)

项目结构
/work/sunboxue/bazel_demo/
├── WORKSPACE          ← 告诉 Bazel 这是项目根目录
├── BUILD              ← 根目录的构建规则（空的也行）
├── hello/
│   ├── BUILD          ← hello 模块的构建规则
│   ├── hello.cc       ← 源文件
│   └── hello.h        ← 头文件
└── main/
    ├── BUILD          ← main 模块的构建规则
    └── main.cc        ← 主程序，依赖 hello 模块
文件内容

1. WORKSPACE（项目根标记）
   /work/sunboxue/bazel_demo/WORKSPACE

# 空文件就行，有它就代表这是项目根目录

2. hello/hello.h（头文件）
   #ifndef HELLO_H
   #define HELLO_H

#include <string></string>

std::string getGreeting(const std::string& name);

#endif
3. hello/hello.cc（实现）
#include "hello.h"

std::string getGreeting(const std::string& name) {
    return "Hello " + name + ", welcome to Bazel!";
}
4. hello/BUILD（hello 模块的构建规则）
cc_library(
    name = "hello",
    srcs = ["hello.cc"],
    hdrs = ["hello.h"],
    visibility = ["//visibility:public"],
)
5. main/main.cc（主程序）
#include <iostream></iostream>
#include "hello/hello.h"

int main() {
    std::cout << getGreeting("Sunboxue") << std::endl;
    return 0;
}
6. main/BUILD（main 模块的构建规则）
cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = ["//hello"],
)
操作命令
在 Docker 容器里执行：

# 1. 创建目录

mkdir -p /work/sunboxue/bazel_demo/hello
mkdir -p /work/sunboxue/bazel_demo/main

# 2. 把上面的文件都创建好（用 vim 或 echo）

# 3. 进入项目目录

cd /work/sunboxue/bazel_demo

# 4. 编译

bazel build //main:main

# 5. 运行

bazel-bin/main/main
你应该看到
Hello Sunboxue, welcome to Bazel!
这个 demo 教你什么
//main:main 依赖 //hello
        ↓
Bazel 自动先编译 hello 库，再编译 main 程序
        ↓
就像你项目里：
//middleware/protocol/om_common:libom_common.so
  依赖 //base, @cuda, @protobuf ...
  Bazel 自动按依赖顺序编译
核心就 3 个东西：
cc_library — 编译成库（.so / .a）
cc_binary — 编译成可执行文件
deps — 声明依赖，Bazel 自动处理

# 2026-8-17

一、代码对比的进一步：

1. 3.4新增了cyber_performence 。**运维调试工具** ，在设备运行时可以通过 Web 页面实时查看系统资源占用。

这是一个运维调试工具，基于 Python Flask 做的 Web 页面。设备跑起来以后，运维人员可以通过浏览器实时看到 CPU、GPU、内存、磁盘的占用情况。它只在开发调试阶段用，不影响核心业务。

2. 3.4新增了os目录 这是编译打包后的运行包。
   这个目录是编译打包后的运行包。里面包含 DAG 配置文件、编译好的 .so 动态库、启动脚本等等。相当于把编译产物集中放在一个地方，部署的时候直接把这个目录拷到设备上就能用。
3. 新增protocol 运行时数据存储的目录  这是程序运行时的数据读写目录  是`middleware/protocol/`编译运行的产物
   里面存的是各模块的运行配置文件，就是 .flag 文件，还有设备状态数据库 device-status.db，是一个 SQLite 数据库，以及一些 Python 脚本比如 PTP 授时监控、雷达点云录制这些。
4. 新增param这是设备参数配置目录 存各类传感器的配置参数  3.3把这些参数散落在`common_config`中
5. V3.4 把 Cyber RT 的完整运行时 **直接放在项目里** ，确保部署时版本一致。V3.3 依赖外部环境安装的 Cyber RT。
6. 新增 3.4 auth 空目录 预留认证升级的位置
7. 移除了sound_player模块 语音播报功能移除
8. middleware/protocol/om_common/configer_topic_om_mec.h中引入 `#ifdef OM_SELF_CHECK` 条件编译
   V3.4 引入了**自检模式**的概念。通过一个宏开关 `OM_SELF_CHECK`，可以切换所有 MQTT Topic 的默认状态。定义了这个宏即自检模式下大部分 Topic 关闭（`false`），功能开关全关，设备不对外通信，专门用于内部硬件自测。只保留自检相关的通道，这样可以 **隔离测试环境** ，避免正常业务数据干扰调试。
   如果未定义 就是正常工作。
9. middleware/protocol/om/configer_om.h
   3.4代码中增加了条件编译 当**#**ifdef**OM_SELF_CHECK** 进入自检模式才开调试 否则不开
   V3.3 调试打印永远开着，生产环境也会输出大量日志。V3.4 改为 **只在自检模式下开调试** ，生产环境默认关闭， **减少日志量，提升性能** 。
10. proto预编译产物 middleware/protocol/proto/ 新增`.pb.cc` / `.pb.h`预编译产物

    二、未实现的功能：
    1.gpu告警功能
    2.视频 图片本地暂存与FTP检索服务

    文档要求 MEC 按 10 分钟一段存视频、5 秒一张存图片到本地硬盘，然后提供 FTP 服务让云平台来拉取。目前 rtsp_tool 模块只做了 RTSP 直播流推送，没有录像落盘的功能，也没有 FTP 服务端。

    三、std标准库

    std::getline(file,line);

    std::istringstream(line) 字符串数据流

    std::ifstring(path) 路径文件数据流

    std::this_thread::sleep_for(std::chrono::seconds(1))；//睡1秒  std::chrono 是 C++11 标准库中专门用来处理时间和时长的命名空间，位于<chrono></chrono>头文件<chrono></chrono>chrono

# 2026-8-19

	一、完成了项目的功能集成 并且编译正常

	怎么编译的呢：

		首先进入容器中，执行`bash build_airos.sh build`编译安装到目录下 	用于日常开发验证能否通过

		然后bash build_airos.sh release  是编译 安装 收集依赖库  生成版本号

### `build` = 把菜炒出来

```text
源码 (.cc/.h)  →  Bazel 编译  →  .so 库文件  →  拷贝到 /home/airos/os/
```

* 只做一件事： **把代码编译成库文件** ，放到 `os/lib/` 里
* **不收集**第三方依赖库（`/home/airos/os/3rd/` 目录不存在）
* 适合：你改了一行代码，想快速看看能不能编译通过

release = 把菜炒出来 + 把调料餐具都配齐 + 打包好可以上桌
build 的全部步骤

+ 收集所有第三方 .so 依赖库 → /home/airos/os/3rd/
+ 生成版本号文件 → /home/airos/os/.airos_version
  在 build 基础上多做了两件事：
  把项目依赖的所有第三方库（CUDA、protobuf、cyber-rt 等）收集到 os/3rd/
  生成一个版本号文件
  适合：准备一个完整的、可以跑起来的版本

### 为什么需要 release？

你刚才遇到的那个问题就是最好例子：

```text
libom.so 依赖 libairos_base.so → not found
```

* `build` 只生成了 `libom.so`，但它依赖的其他库（`libairos_base.so` 等）没有收集过来
* `release` 会把所有依赖库都收集到 `os/3rd/`，这样程序才能跑起来

**简单说：build 出来的东西跑不起来，release 出来的才能跑。**

二、MQTT

MQTT 是一种 **轻量级的消息协议** ，就像微信聊天一样：

* **Publisher（发布者）** ：发消息的人
* **Subscriber（订阅者）** ：收消息的人
* **Broker（服务器）** ：中转站，负责转发消息

```text
发布者  →  消息  →  Broker（服务器）  →  消息  →  订阅者
```

![1787132985940](image/智路OS学习记录1/1787132985940.png)

**你跑的几条命令：**

| 命令                                      | 作用                     | 相当于                          |
| ----------------------------------------- | ------------------------ | ------------------------------- |
| `mosquitto -d`                          | 启动一个 MQTT 服务器     | 开了个假的"云平台"              |
| `mosquitto_sub -t "rscu/#" -v`          | 订阅所有 rscu 开头的消息 | 假装你是云平台，等着收消息      |
| `mainboard<span> </span>-d dag/om.dag` | 启动 OM 服务             | 你的代码开始采集 GPU 数据并发布 |

### 连接的是谁和谁

```text
OM 模块 (发布者)  ←→  Mosquitto (Broker)  ←→  mosquitto_sub (订阅者)     │                      │                        │  127.0.0.1:1883        127.0.0.1:1883            127.0.0.1:1883     └──────────────────────┴────────────────────────┘                    都是 localhost，同一台机器
```

**三个角色都在你的容器里：**

* OM 模块：采集数据并发布
* Mosquitto：中转消息
* mosquitto_sub：接收并显示消息

生产环境中，Mosquitto 和订阅者（云平台）在远程服务器上，今天你把它们都换成本地的了。

没链接成功 ＋是否有前端

# 2026-8-20

一、遇到的问题：

	输入mainboard后只在终端上输出日志 在订阅方并没有出现 怀疑是不是MQTT没有接通呢。

输入 mainboard 后，终端有日志输出（CPU/GPU 性能数据采集），但 `mosquitto_sub` 订阅端收不到任何消息。

**问题 1：Mosquitto Broker 没启动**

* 原因：容器内 mosquitto 进程根本没运行
* 解决：`mosquitto -d` 启动后台 Broker
* 结果：CCIndex 模块的本地消息能收到了

**问题 2：OM 性能数据发不到本地 Broker**

* 原因：OM 模块的 `publishPerformenceData()` 调用的是 `mqttPushMsg2BrokerCloud()`，强制走 **云端连接** （`172.20.28.101:1883`），而 Docker 容器无法访问该地址，连接持续失败重连
* 解决：修改 `/home/airos/common_config/work_param_config.flag` 第 24 行：

  ```text
  原值: "U_maintenanceCloudUrl": "172.20.28.101:1883"改为: "U_maintenanceCloudUrl": "tcp://127.0.0.1:1883"
  ```
* 结果：cloud MQTT 连上了本地 Broker，数据开始发布

**问题 3：GPU 数据采集失败（`get gpu-info failure!`）**

* 原因：GPU 空闲时温度传感器返回 -256℃（无效值），`getGpuInfo()` 函数判断 `temp >= 0` 才返回 true，所以整体返回失败
* 解决：在宿主机跑 GPU 负载，让 GPU 温度升高，传感器读数恢复正常（~40℃）
* 结果：`get gpu-info failure!` 消失，GPU 数据正常采集

二、tail的用法

`tail -f` 是 Linux/Unix 系统里一个**非常常用**的命令，专门用来 **实时监控日志文件** 。

可以把它拆开理解：

* **`tail`** ：默认显示文件的**末尾 10 行**内容（比如查看日志的最后几行）。
* **`-f`** ：是 **follow（跟踪）** 的缩写。

合在一起，`tail -f 文件名` 的意思是：

> **先显示该文件最后几行内容，然后进入“追踪”模式——只要文件有新内容写入（比如程序产生了新的日志），就会自动把新内容实时打印在屏幕上。**

三、部署工具

	要求：

1. **看文档** → 了解部署工具和工参表是怎么配合的（全链路）
2. **看 Excel** → 了解现在有哪些字段、格式是什么样的
3. **评估改动量** → 如果加一个新字段，从代码到 Excel 到部署，要改哪些地方、多大工作量

工参表：把所有路口的设备ip、证书路径啥的存在表里 是唯一的数据源

部署工具：负责看懂蓝图读excel  、 生成建材 配置文件 、 进场施工即ssh连设备 最后交工验收(重启服务、查版本)

全链路：

	1.准备蓝图

		现场实施人员或运维工程师，按照甲方给的设备清单，把几十甚至上百个路口的参数，填到统一的Excel工参表里（包括MEC工参、传感工参、信控工参等多个Sheet页）。这张表就是后续所有自动化操作的唯一依据。

	2.部署工具加载工参表：

		打开部署工具，点击“选择工参表”，选中填好的Excel文件。
👉  **关键配合点** ：工具立刻开始解析Excel，它会参照表里的“工参表参数说明”页，把每一列数据映射到代码里的字段，然后在工具所在的本地电脑上，自动生成每个MEC设备专属的配置文件 —— `work_param_config.flag`（这个文件相当于给每台MEC单独下发的“施工清单”）。

> 本地生成路径示例：`./local/路口编号/设备类型_MEC-IP/work_param_config.flag`

	3.确定施工范围 - 选择路口和设备

	4.施工队进场施工-工参部署

		把生成好的配置文件以及证书文件 批量上传到mec设备指定目录里

	5.验收和后期维护

* **工参比对** ：工具可以重新连上设备，把设备里的实际配置和本地生成的配置做对比，防止漏传或传错。
* **重启服务** ：配置传完后，工具可以一键重启MEC上的智路OS服务，让新配置立即生效。
* **版本升级** ：如果后续软件有更新，同样通过这个工具，选中路口，远程批量上传新版本包并升级。

数据流：

	excel工参表->` deployTool读取/映射 → 生成JSON配置文件 → C++项目读取使用`

# 2026-8-26

手搓迷你版

1.文件共享锁 即读锁 符合规范 因为读取性能都是读的是linux内核文件夹里的。

	读锁 flock:用读锁（`LOCK_SH`）是因为： **我们只是“读取”数据，多个监控进程可以同时读，互不影响；但我们用这把锁挡住“写操作”，保证在读数据的那一刻，没有进程（懂规矩的进程）跑过来改文件，从而保证我们读到的快照是完整一致的。** （在 `/proc` 下虽然没人真写，但这是为了遵循项目原代码的优秀工程规范）。

在 Linux 系统编程中，给文件加读锁的标准写法就是：

**cpp**

```
flock(fd, LOCK_SH);   // 加读锁（共享锁）
```

对应的，解锁就是：

**cpp**

```
flock(fd, LOCK_UN);   // 解锁
```

`flock` 是 Linux 内核提供的一个 **系统调用（System Call）** ，它的全称是  **File Lock（文件锁）** 。

* **它的本职工作** ：给一个**打开的文件描述符（fd）** 附加一把锁。
* **它的底层原理** ：它不负责拷贝数据，也不负责读写数据。它只是在操作系统内核的“文件表”里，针对你这个 `fd` 做个标记——标记上“我正在读（共享锁）”或“我正在写（排他锁）”。当别的进程也想操作这个文件时，内核会检查这个标记，决定是“放行”还是“堵住”。
* **重要特征（面试高频）** ：`flock` 产生的锁是 **建议性锁（Advisory Lock）** ，不是强制性锁。也就是说，它只能约束“懂规矩、主动调用 `flock` 检查”的进程。如果某个进程是个“愣头青”，直接调用 `read()` 或 `write()` 而不加锁检查，内核**不会**拦截它（在普通文件中可能会写乱，但在 `/proc` 中无所谓，因为没人写它）。

然后为了安全 要禁止拷贝构造和赋值拷贝

	原因：**禁止拷贝构造和拷贝赋值，最根本的原因是“防止两个对象争抢同一把锁”，从而彻底破坏 RAII 的“构造即加锁、析构即解锁”的确定性承诺。**

假设我们不小心允许了拷贝，看这 4 步会发生什么：

1. **加锁时刻** ：`g1` 构造，`flock(fd, LOCK_SH)` 成功。内核锁状态 =  **已加锁** 。`g1` 状态 = 持有。
2. **拷贝时刻** ：`g2 = g1`。内核锁状态 =  **已加锁** （没有变化，还是同一把）。`g1` 和 `g2` 都认为自己在持有。
3. **g2 析构（灾难开始）** ：`~g2()` 调用 `flock(fd, LOCK_UN)`。内核收到指令， **释放锁** 。此时内核锁状态 =  **已解锁** 。但注意：**`g1` 对象还活着，并且它的 `locked_` 依然等于 `true`！**
4. **g1 析构（彻底崩盘）** ：`~g1()` 调用 `flock(fd, LOCK_UN)`。内核再次收到解锁指令，但此时锁已经是释放状态了。虽然 Linux 的 `flock` 对重复解锁通常返回成功，但 **逻辑前提已经彻底崩塌** ——`g1` 认为自己活着期间锁一直存在，但实际上在 `g2` 析构时锁就丢了。如果在这期间有其他进程趁虚而入写文件，`g1` 毫不知情。
5. 拆开看：const + FileLockGuard + &
   FileLockGuard：类型名，表示参数必须是 FileLockGuard 类型的对象。

&：表示这是一个引用（Reference），不是按值传递。也就是说，形参 g1 不会拷贝出一个新对象，而是直接绑定到实参（被拷贝的那个对象）身上，像起了一个别名。

const：表示这个引用是“常引用”，承诺只读，不会修改被引用的对象。

2. 为什么一定要加 &（引用）？（这是关键！）
   如果拷贝构造函数的参数不加 &，写成 FileLockGuard(const FileLockGuard g)（按值传递），会发生什么？

按值传递意味着：你需要把 g1 复制一份作为参数传给构造函数。

但“复制一份”本身就要调用拷贝构造函数！

结果就是：为了调用拷贝构造函数，你先调用了拷贝构造函数 → 无限递归，编译直接报错或栈溢出。

所以 C++ 规定：拷贝构造函数的参数必须是引用类型（通常是 const &），就是为了切断这个死循环。

3. 为什么还要加 const？
   因为拷贝构造函数的语义是“从一个已有对象创建新对象”，我们只读取被拷贝的对象（g1）的成员（比如读它的 fd_ 和 locked_），绝不修改它。

加上 const 后：

编译器帮你把关：如果你在拷贝构造函数里意外写了 g1.fd_ = 999，编译报错。

允许绑定到临时对象（比如 FileLockGuard(FileLockGuard(3)) 这种写法也能编译）。

4. 完整的拷贝构造函数签名长什么样？
   标准写法就是：

cpp
FileLockGuard(const FileLockGuard& other);
other 就是“被拷贝的那个对象”的别名（常引用）。

在函数体里，你可以用 other.fd_ 和 other.locked_ 去初始化新对象。

但是！ 你现在写的代码里，这个函数被 = delete 删掉了，所以函数体都不存在。我们写这个声明只是为了明确告诉编译器：我不允许这种拷贝行为。

2.快照函数是干嘛的。

	**快照模式就是为了把占用文件的时间压缩到极致，虽然它不一定直接挡住别人的读，但它保住了文件描述符不被浪费、保住了写入者不被饿死、保住了采集周期的精准。这是防御性编程的经典做法**

“快照函数”这个名字起得非常形象，我一句话给你说透：

“快照函数”就是在“某一毫秒”，把文件里的所有内容一口气拽出来，存到内存里（std::string），当成一张静态照片保存。
之后程序只分析这张“照片”，不再碰硬盘上的原文件了。

为了让你彻底理解它为什么叫“快照”，而不是普通的“读文件”，我把它拆成 “灵魂三问”：

1. 普通读文件和快照读文件，有什么本质区别？
   普通读文件（边读边解析）：打开文件，读一行，解析一行，再读下一行……整个过程文件描述符一直开着，锁一直占着。如果解析逻辑复杂（比如算一堆公式），可能把锁占用好几十毫秒。

快照读文件（本函数的做法）：打开文件，用最快的速度（while 循环 read）把所有字节塞进内存，然后立刻关闭文件、释放锁。之后再慢慢解析内存里的字符串。

	**普通读法** ：锁的持有时间 = **读取时间 + 解析时间**
 **快照读法** ：锁的持有时间 =  **纯粹的读取时间** （微秒级）

	也就是快照它只读取 而普通读法是读取并解析。

2026-8-27 Traffic_flow项目

我要做红绿灯的接入：

![1787811999870](image/智路OS学习记录1/1787811999870.png)

信号机(硬件) → [二进制] → TLP模块(翻译) → [JSON] → DDS → 你的traffic_flow模块
                                                              │
                                              ┌───────────────┤
                                              │  你要做的：     │
                                              │  1. 订阅接收    │
                                              │  2. 解析JSON   │
                                              │  3. 缓存状态    │
                                              │  4. 检测边界    │
                                              ───────────────┘
                                                              │
                                                              ▼
                                                    detect()主循环(你哥管)
                                                    根据相位边界触发统计

首先改一下订阅配置 把信号机订阅上 告诉DDS我需要信号机的

## 任务 1：服务配置 JSON 新增 subscriber

### 先理解这个文件是干嘛的

这个 JSON 文件就是告诉 DDS 中间件：**我要听哪些频道、我要往哪些频道发消息。**

现在 `subscribers` 里只有一条：

```json
"subscribers":[{"id":0,                          ← 编号0，代码里对应 PartId::RSM_JSON_SUB"name":"DataFusionSubscriber:0",  ← 名字，随便起，日志里用"service":"FusionSubscriber",     ← 固定写法，表示这是个订阅者"topic":"RSMJsonTopic:5",         ← 订阅频道5（RSM感知数据）"domain_id":16,                   ← DDS域ID，同一域内的才能通信        ...}]
```

**你要做的就是在 `subscribers` 数组里再加一条，订阅信号灯频道。**

---

### 怎么确定新条目的值？

| 字段          | 填什么                       | 为什么                                                                |
| ------------- | ---------------------------- | --------------------------------------------------------------------- |
| `id`        | `3`                        | 0 被 RSM 用了，1 和 2 没定义，用 3（跟后面 PartId 枚举对应）          |
| `name`      | `"SignalJsonSubscriber:0"` | 起个有意义的名字，日志好排查                                          |
| `service`   | `"FusionSubscriber"`       | 跟现有的一样，固定写法                                                |
| `topic`     | `"SignalJsonTopic:19"`     | **19 是 `TopicType_SignalJson` 在 FusionPlugin.h 里的枚举值** |
| `domain_id` | `16`                       | **必须跟现有的一样** ，否则收不到消息                           |
| 其他字段      | 跟现有 RSM 的一样就行        | transport、segment_size 等照抄                                        |

---

### 具体改法

两个文件都要改：`traffic_flow_service_5s.json` 和 `traffic_flow_service_5m.json`

改法完全一样，就是把 `subscribers` 数组从 1 条变成 2 条

## 任务 2：PartId 枚举新增 SIGNAL_JSON_SUB

### 先理解 PartId 是干嘛的

`PartId` 就是 **消息来源的编号** 。DDS 收到消息后回调 `on_data_available(id, message)`，这个 `id` 就是配置文件里 subscriber 的 `id`。代码里用枚举把数字变成有意义的名字。

现在的对应关系：

| 配置文件 subscriber id | PartId 枚举值                 | 含义         |
| ---------------------- | ----------------------------- | ------------ |
| 0                      | `RSM_JSON_SUB = 0`          | 感知数据     |
| 1                      | `TRAFFIC_FLOW_JSON_PUB = 1` | 流量结果发布 |
| 3                      | **← 你要加的**         | 信号灯数据   |

**你配置文件里新 subscriber 的 `id` 写了 3，这里枚举也要加一个 `= 3` 的值，两边要对上。**

---

### 具体改法

文件：`src/plugin/traffic_flow_service_impl.h`

```cpp
// ... existing code ...enumclassPartId:uint8_t{    RSM_JSON_SUB =0,    TRAFFIC_FLOW_JSON_PUB =1,    SIGNAL_JSON_SUB =3,    QUALITY_JSON_PUB =253,    PART_ID_STATE_JSON_PUB =254,    PART_ID_LOG_JSON_PUB =255};// ... existing code ...
```

就加了一行 `SIGNAL_JSON_SUB<span> </span>= 3,`，放在`TRAFFIC_FLOW_JSON_PUB`下面就行`

好，任务 3 要改 3 个地方，都在同一个文件里。

---

## 任务 3：on_data_available() 新增 case 分支

### 先理解要改哪几个地方

这个文件里有 3 个函数都用 `switch/case` 或 `if/else` 判断消息来源：

| 函数                    | 作用                     | 要不要改                   |
| ----------------------- | ------------------------ | -------------------------- |
| `on_data_available()` | 收到消息后处理           | **必须改**，核心逻辑 |
| `on_discovery()`      | DDS 发现新参与者时打日志 | 建议改，方便排查           |
| `on_matched()`        | DDS 匹配到参与者时打日志 | 建议改，方便排查           |

---

### 第一处：on_data_available()（核心）

文件：`src/plugin/traffic_flow_service_impl.cpp`

在现有的 `case PartId::RSM_JSON_SUB` 后面加一个新的 case：

```cpp|CODE_EDIT_BLOCK|/work/sunboxue/LiuLiang/traffic_flow/src/plugin/traffic_flow_service_impl.cpp
// ... existing code ...
    switch (message_type) {
        case PartId::RSM_JSON_SUB: {
            std::string contents = std::string(static_cast<char*>(fusion_message->body), fusion_message->body_size);
            if (s_fs_api_->TrafficFlowService_Log) {
                s_fs_api_->TrafficFlowService_Log(LogLevel_Info, "Received one RSMJson.");
            }

            traffic_flow::TrafficFlowFactory& traffic_flow_factory = traffic_flow::TrafficFlowFactory::get_instance();
            bool ret = traffic_flow_factory.update_new_frame(contents);
            break;
        }
        case PartId::SIGNAL_JSON_SUB: {
            std::string signal_json = std::string(
                static_cast<char*>(fusion_message->body), fusion_message->body_size);
            if (s_fs_api_->TrafficFlowService_Log) {
                s_fs_api_->TrafficFlowService_Log(LogLevel_Info, "Received one SignalJson.");
            }
            // TODO: 解析 signal_json 并更新 SignalPhaseManager（任务4、5完成后再实现）
            break;
        }
        default: {
// ... existing code ...
```

**解释：**

- 把 `fusion_message->body`（原始字节）转成 `std::string`（JSON 字符串）
- 打个日志，方便你调试时知道收到消息了
- **现在先留个 TODO**，等任务 4、5 写好了 SignalPhaseManager 再补上调用

---

### 第二处：on_discovery()（日志）

同一个文件，找到 `on_discovery` 函数里的 if-else 判断：

```cpp|CODE_EDIT_BLOCK|/work/sunboxue/LiuLiang/traffic_flow/src/plugin/traffic_flow_service_impl.cpp
// ... existing code ...
    std::stringstream ss;
    if (id == static_cast<uint8_t>(PartId::RSM_JSON_SUB)) {
        ss << "[RSMJsonSubscriber]";
    } else if (id == static_cast<uint8_t>(PartId::SIGNAL_JSON_SUB)) {
        ss << "[SignalJsonSubscriber]";
    } else if (id == static_cast<uint8_t>(PartId::TRAFFIC_FLOW_JSON_PUB)) {
        ss << "[TrafficFlowJsonPublisher]";
    } else {
        return;
    }
// ... existing code ...
```

**解释：** DDS 发现新的信号灯数据发布者时，日志里会显示 `[SignalJsonSubscriber] Discovered: xxx`，方便你确认连接是否正常。

---

### 第三处：on_matched()（日志）

同一个文件，找到 `on_matched` 函数：

```cpp|CODE_EDIT_BLOCK|/work/sunboxue/LiuLiang/traffic_flow/src/plugin/traffic_flow_service_impl.cpp
// ... existing code ...
    std::stringstream ss;
    if (id == static_cast<uint8_t>(PartId::RSM_JSON_SUB)) {
        ss << "[RSMJsonSubscriber] Matched: " << current_count << std::endl;
    } else if (id == static_cast<uint8_t>(PartId::SIGNAL_JSON_SUB)) {
        ss << "[SignalJsonSubscriber] Matched: " << current_count << std::endl;
    } else if (id == static_cast<uint8_t>(PartId::TRAFFIC_FLOW_JSON_PUB)) {
        ss << "[TrafficFlowJsonPublisher] Matched: " << current_count << std::endl;
    } else {
        ss << "No Matched: " << id << std::endl;
        return;
    }
// ... existing code ...
```

---

### 改完效果

三个地方改完后，当信号灯数据从 DDS 过来时：

1. **启动时** → 日志打印 `[SignalJsonSubscriber] Discovered: xxx`（on_discovery）
2. **连接上后** → 日志打印 `[SignalJsonSubscriber] Matched: 1`（on_matched）
3. **收到数据时** → 日志打印 `Received one SignalJson.`（on_data_available）

这样你跑起来就能看到信号灯数据有没有正常收到。

---

改完这 3 处，任务 1-3（第一部分"加字段"）就全部完成了。改好了告诉我，我们开始任务 4（写解析函数）。

---

## 任务 4：TLP JSON 解析 → SignalPhaseData

### 先搞清楚要干嘛

DDS 给你的原始数据是一个 JSON 字符串，长这样：

```json
{
  "msg_type": 1743,
  "timeStamp": 1724745600000,
  "intersections": [{
    "phases": [
      { "phaseId": 2, "phaseStates": [{ "light": 6, "timing": { "counting": { "likelyEndTime": { "timeMark": 150 } } } }] },
      { "phaseId": 10, "phaseStates": [{ "light": 3, "timing": { "counting": { "likelyEndTime": { "timeMark": 200 } } } }] },
      { "phaseId": 18, "phaseStates": [{ "light": 3, "timing": { "counting": { "likelyEndTime": { "timeMark": 200 } } } }] },
      { "phaseId": 26, "phaseStates": [{ "light": 6, "timing": { "counting": { "likelyEndTime": { "timeMark": 150 } } } }] }
    ]
  }]
}
```

**你要写一个函数，把这个 JSON 字符串变成程序能用的结构化数据。**

---

### 第一步：定义数据结构

新建文件 `src/common/signal_phase_data.h`：

```cpp
#ifndef TRAFFIC_FLOW_SIGNAL_PHASE_DATA_H
#define TRAFFIC_FLOW_SIGNAL_PHASE_DATA_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "common/config.h"

namespace fusion_perception::traffic_flow {

enum class SignalLightState : uint8_t {
    UNAVAILABLE = 0,
    DARK = 1,
    FLASHING_RED = 2,
    RED = 3,
    FLASHING_GREEN = 4,
    PERMISSIVE_GREEN = 5,
    PROTECTED_GREEN = 6,
    YELLOW = 7,
    FLASHING_YELLOW = 8
};

struct SignalPhaseData {
    DirectionCode direction;
    SignalLightState light;
    int32_t time_remaining;    // 剩余时间，单位0.1秒
    uint64_t timestamp;
};

inline DirectionCode phase_id_to_direction(int32_t phase_id) {
    switch (phase_id) {
        case 2:  return DirectionCode::NORTH;
        case 10: return DirectionCode::EAST;
        case 18: return DirectionCode::SOUTH;
        case 26: return DirectionCode::WEST;
        default: return DirectionCode::DEFAULT;
    }
}

inline SignalLightState int_to_light_state(int light) {
    if (light >= 0 && light <= 8) {
        return static_cast<SignalLightState>(light);
    }
    return SignalLightState::UNAVAILABLE;
}

std::vector<SignalPhaseData> parse_signal_json(const std::string& json_str);

}  // namespace fusion_perception::traffic_flow

#endif
```

**解释每个部分是干嘛的：**

| 代码                        | 干嘛的                                                                    |
| --------------------------- | ------------------------------------------------------------------------- |
| `SignalLightState` 枚举   | 把 TLP 的 light 数字（0-8）变成有意义的名字（RED=3, GREEN=6...）          |
| `SignalPhaseData` 结构体  | 存一个方向的灯态信息：哪个方向、什么灯、剩几秒、时间戳                    |
| `phase_id_to_direction()` | **核心映射**：phaseId=2→北直行, 10→东直行, 18→南直行, 26→西直行 |
| `int_to_light_state()`    | 把 JSON 里的 light 数字转成枚举                                           |
| `parse_signal_json()`     | 主解析函数，声明在下面实现                                                |

---

### 第二步：实现解析函数

新建文件 `src/common/signal_phase_data.cpp`：

```cpp
#include "signal_phase_data.h"
#include <iostream>

namespace fusion_perception::traffic_flow {

std::vector<SignalPhaseData> parse_signal_json(const std::string& json_str) {
    std::vector<SignalPhaseData> result;

    if (json_str.empty()) {
        std::cerr << "parse_signal_json: empty input" << std::endl;
        return result;
    }

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(json_str);
    } catch (const std::exception& e) {
        std::cerr << "parse_signal_json: JSON parse failed: " << e.what() << std::endl;
        return result;
    }

    if (!j.contains("intersections") || !j["intersections"].is_array() || j["intersections"].empty()) {
        std::cerr << "parse_signal_json: no intersections" << std::endl;
        return result;
    }

    auto& phases = j["intersections"][0]["phases"];
    if (!phases.is_array()) {
        std::cerr << "parse_signal_json: no phases" << std::endl;
        return result;
    }

    uint64_t timestamp = j.contains("timeStamp") ? j["timeStamp"].get<uint64_t>() : 0;

    for (auto& phase : phases) {
        int32_t phase_id = phase.value("phaseId", 0);
        DirectionCode dir = phase_id_to_direction(phase_id);
        if (dir == DirectionCode::DEFAULT) {
            continue;
        }

        auto& phase_states = phase["phaseStates"];
        if (!phase_states.is_array() || phase_states.empty()) {
            continue;
        }

        auto& ps = phase_states[0];
        int light_val = ps.value("light", 0);

        int32_t time_remaining = 0;
        if (ps.contains("timing") && ps["timing"].contains("counting")) {
            auto& counting = ps["timing"]["counting"];
            if (counting.contains("likelyEndTime") && counting["likelyEndTime"].contains("timeMark")) {
                time_remaining = counting["likelyEndTime"]["timeMark"].get<int32_t>();
            }
        }

        SignalPhaseData spd;
        spd.direction = dir;
        spd.light = int_to_light_state(light_val);
        spd.time_remaining = time_remaining;
        spd.timestamp = timestamp;
        result.push_back(spd);
    }

    return result;
}

}  // namespace fusion_perception::traffic_flow
```

**逐段解释：**

```
1. 先检查输入是不是空 → 空就直接返回
2. 把 JSON 字符串解析成 json 对象 → 解析失败就返回
3. 找到 intersections[0].phases → 找不到就返回
4. 遍历每个 phase：
   a. 拿 phaseId → 转成方向（北/南/东/西）
   b. 如果不是直行（DEFAULT）→ 跳过，我们只关心直行
   c. 拿 light 值 → 转成灯态枚举
   d. 拿 timing.counting.likelyEndTime.timeMark → 这就是剩余时间
   e. 组装成 SignalPhaseData → 放进结果数组
5. 返回结果数组
```

---

### 第三步：注册到 CMakeLists.txt

新文件要让编译器知道，需要加到 CMakeLists.txt 里。让我看看现有的：

好消息，CMakeLists.txt 用了 `GLOB_RECURSE`，**自动扫描目录下所有文件**，所以新文件不用手动加。只需要把头文件加到安装列表里：

```cmake|CODE_EDIT_BLOCK|/work/sunboxue/LiuLiang/traffic_flow/src/common/CMakeLists.txt
// ... existing code ...
SET(TRAFFIC_FLOW_HEADERS "")
LIST(APPEND TRAFFIC_FLOW_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/perception_data.h)
LIST(APPEND TRAFFIC_FLOW_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/signal_phase_data.h)

INSTALL(FILES ${TRAFFIC_FLOW_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
```

---

### 总结一下任务 4 你做了什么

| 文件                                 | 干嘛的                                 |
| ------------------------------------ | -------------------------------------- |
| `src/common/signal_phase_data.h`   | 定义数据结构 + 映射函数 + 解析函数声明 |
| `src/common/signal_phase_data.cpp` | 实现 JSON 解析函数                     |
| `src/common/CMakeLists.txt`        | 加一行让头文件被安装                   |

**核心就一件事：把 JSON 字符串变成 `vector<SignalPhaseData>`。**

比如输入那个 JSON，解析后得到：

```
result[0]: direction=NORTH, light=PROTECTED_GREEN(6), time_remaining=150(15秒)
result[1]: direction=EAST,  light=RED(3),           time_remaining=200(20秒)
result[2]: direction=SOUTH, light=RED(3),           time_remaining=200(20秒)
result[3]: direction=WEST,  light=PROTECTED_GREEN(6), time_remaining=150(15秒)
```

这样后续 SignalPhaseManager 拿到这个数组，就知道每个方向当前是什么灯了。

---

### 怎么验证任务 4 对不对？

你可以写个简单的 main 函数测试：

```cpp
#include "signal_phase_data.h"
#include <iostream>

int main() {
    std::string test_json = R"({
        "msg_type": 1743,
        "timeStamp": 1724745600000,
        "intersections": [{
            "phases": [
                {"phaseId": 2, "phaseStates": [{"light": 6, "timing": {"counting": {"likelyEndTime": {"timeMark": 150}}}}]},
                {"phaseId": 10, "phaseStates": [{"light": 3, "timing": {"counting": {"likelyEndTime": {"timeMark": 200}}}}]},
                {"phaseId": 18, "phaseStates": [{"light": 3, "timing": {"counting": {"likelyEndTime": {"timeMark": 200}}}}]},
                {"phaseId": 26, "phaseStates": [{"light": 6, "timing": {"counting": {"likelyEndTime": {"timeMark": 150}}}}]}
            ]
        }]
    })";

    auto result = fusion_perception::traffic_flow::parse_signal_json(test_json);
    std::cout << "Parsed " << result.size() << " phases" << std::endl;
    for (auto& spd : result) {
        std::cout << "direction=" << static_cast<int>(spd.direction)
                  << " light=" << static_cast<int>(spd.light)
                  << " time=" << spd.time_remaining << std::endl;
    }
    return 0;
}
```

预期输出：

```
Parsed 4 phases
direction=1 light=6 time=150    ← NORTH, 绿灯, 15秒
direction=2 light=3 time=200    ← EAST, 红灯, 20秒
direction=3 light=3 time=200    ← SOUTH, 红灯, 20秒
direction=4 light=6 time=150    ← WEST, 绿灯, 15秒
```

---

文件创建好了告诉我，我们继续任务 5（SignalPhaseManager）。

好，任务 5 是核心部分。先看看现有代码里有没有类似的单例模式可以参考。

好，项目里有现成的 `Singleton` 模板类，直接继承就行。

---

## 任务 5：SignalPhaseManager 实现

### 先理解这个类要干嘛

就三件事：

```
1. update()  ← DDS回调线程调用，收到JSON后把灯态存起来
2. check_phase_boundary()  ← detect()主线程调用，问"某方向完成一个周期了吗？"
3. 用 mutex 保证两个线程同时访问不出问题
```

---

### 头文件

新建 `src/common/signal_phase_manager.h`：

```cpp
#ifndef TRAFFIC_FLOW_SIGNAL_PHASE_MANAGER_H
#define TRAFFIC_FLOW_SIGNAL_PHASE_MANAGER_H

#include <mutex>
#include <unordered_map>

#include "base/Singleton.h"
#include "common/config.h"
#include "common/signal_phase_data.h"

namespace fusion_perception::traffic_flow {

class SignalPhaseManager final : public cybertron::base::Singleton<SignalPhaseManager> {
    friend class cybertron::base::Singleton<SignalPhaseManager>;

public:
    void update(const std::vector<SignalPhaseData>& phases);

    bool check_phase_boundary(DirectionCode direction);

    SignalLightState get_light_state(DirectionCode direction);

private:
    SignalPhaseManager() = default;
    virtual ~SignalPhaseManager() = default;

    std::mutex _mtx;
    std::unordered_map<DirectionCode, SignalLightState> _current_states;
    std::unordered_map<DirectionCode, SignalLightState> _previous_states;
    std::unordered_map<DirectionCode, bool> _boundary_triggered;
};

}  // namespace fusion_perception::traffic_flow

#endif
```

**逐行解释：**

| 代码                            | 干嘛的                                        |
| ------------------------------- | --------------------------------------------- |
| `friend class Singleton<...>` | 让单例模板能调用私有构造函数                  |
| `update()`                    | 接收解析好的灯态数组，更新内部状态            |
| `check_phase_boundary()`      | 检测某方向是否触发了相位边界，返回 true/false |
| `get_light_state()`           | 查询某方向当前灯态（调试用）                  |
| `_current_states`             | 存当前4个方向的灯态                           |
| `_previous_states`            | 存上一次的灯态（用来对比变化）                |
| `_boundary_triggered`         | 标记某方向是否完成了绿→红→绿周期            |

---

### 实现文件

新建 `src/common/signal_phase_manager.cpp`：

```cpp
#include "signal_phase_manager.h"
#include <iostream>

namespace fusion_perception::traffic_flow {

void SignalPhaseManager::update(const std::vector<SignalPhaseData>& phases) {
    std::lock_guard<std::mutex> lock(_mtx);

    for (const auto& phase : phases) {
        auto dir = phase.direction;
        if (dir == DirectionCode::DEFAULT) {
            continue;
        }

        _previous_states[dir] = _current_states[dir];
        _current_states[dir] = phase.light;

        if (_previous_states[dir] == SignalLightState::PROTECTED_GREEN &&
            phase.light == SignalLightState::RED) {
            _boundary_triggered[dir] = true;
#ifdef DEBUG
            std::cout << "[SignalPhaseManager] " << direction_code_to_string(dir)
                      << " phase boundary detected: green -> red" << std::endl;
#endif
        }
    }
}

bool SignalPhaseManager::check_phase_boundary(DirectionCode direction) {
    std::lock_guard<std::mutex> lock(_mtx);

    auto it = _boundary_triggered.find(direction);
    if (it != _boundary_triggered.end() && it->second) {
        it->second = false;
        return true;
    }
    return false;
}

SignalLightState SignalPhaseManager::get_light_state(DirectionCode direction) {
    std::lock_guard<std::mutex> lock(_mtx);

    auto it = _current_states.find(direction);
    if (it != _current_states.end()) {
        return it->second;
    }
    return SignalLightState::UNAVAILABLE;
}

}  // namespace fusion_perception::traffic_flow
```

**核心逻辑就一段，我给你拆明白：**

```
update() 被调用时（DDS收到新信号灯JSON）：

  对每个方向的灯态数据：
  │
  ├─ 1. 把"当前"保存到"上一次"
  │     _previous_states[NORTH] = _current_states[NORTH]  // 比如之前是绿灯
  │
  ├─ 2. 更新"当前"为新的灯态
  │     _current_states[NORTH] = 新收到的灯态              // 现在变成红灯了
  │
  └─ 3. 判断：上一次是绿灯 且 现在是红灯？
        ├─ 是 → _boundary_triggered[NORTH] = true  ✅ "北边一个周期结束了！"
        └─ 否 → 什么都不做，等下次
```

```
check_phase_boundary() 被调用时（detect()每帧来问）：

  看 _boundary_triggered[NORTH] 是不是 true
  ├─ true  → 重置为false，返回true  ✅ "是的，该统计了"
  └─ false → 返回false             ❌ "还没，继续等"
```

---

### 然后把任务 3 的 TODO 补上

之前 `on_data_available()` 里留了个 TODO，现在 SignalPhaseManager 写好了，可以补上了：

```cpp|CODE_EDIT_BLOCK|/work/sunboxue/LiuLiang/traffic_flow/src/plugin/traffic_flow_service_impl.cpp
// ... existing code ...
#include "common/config.h"
#include "common/signal_phase_data.h"
#include "common/signal_phase_manager.h"
#include "log.h"
// ... existing code ...
```

```cpp|CODE_EDIT_BLOCK|/work/sunboxue/LiuLiang/traffic_flow/src/plugin/traffic_flow_service_impl.cpp
// ... existing code ...
        case PartId::SIGNAL_JSON_SUB: {
            std::string signal_json = std::string(
                static_cast<char*>(fusion_message->body), fusion_message->body_size);
            if (s_fs_api_->TrafficFlowService_Log) {
                s_fs_api_->TrafficFlowService_Log(LogLevel_Info, "Received one SignalJson.");
            }
            auto phases = traffic_flow::parse_signal_json(signal_json);
            traffic_flow::SignalPhaseManager::get_instance().update(phases);
            break;
        }
// ... existing code ...
```

**现在数据流就通了：**

```
DDS收到信号灯JSON
  → on_data_available(SIGNAL_JSON_SUB)
    → parse_signal_json() 解析成 vector<SignalPhaseData>
      → SignalPhaseManager::update() 缓存状态 + 检测边界
```

---

### 任务 5 改完的文件清单

| 文件                                         | 操作                                |
| -------------------------------------------- | ----------------------------------- |
| `src/common/signal_phase_manager.h`        | **新建**                      |
| `src/common/signal_phase_manager.cpp`      | **新建**                      |
| `src/plugin/traffic_flow_service_impl.cpp` | 加两个 include + 补全 case 里的调用 |

CMakeLists.txt 不用改，`GLOB_RECURSE` 会自动扫到新文件。

改好了告诉我，继续任务 6（最后一个）。

好，最后一个任务。先再看看 detect() 的完整逻辑，确定
好，任务 6 是最后一个。先理解 detect() 现在在干嘛，再看加什么。

---

## 任务 6：detect() 主循环集成

### 先看懂现有 detect() 的结构

```
detect() while 循环:
  │
  ├─ 1. 从队列取一帧感知数据
  │
  ├─ 2. 对所有线圈的 detector 调用 update()（记录车辆事件）
  │
  ├─ 3. 固定周期组（5秒）：
  │     时间到了？→ 对所有线圈 calculate_once() → 发布结果
  │
  └─ 4. ← 你要在这里加：信号灯周期组 ×4（北/南/东/西）
        某方向相位边界触发了？→ 对该方向线圈 calculate_once() → 发布结果
```

**固定周期组**是不管红绿灯，每5秒统计一次。
**信号灯周期组**是跟着红绿灯走，一个绿→红→绿周期统计一次。
两组**互不干扰，各自触发**。

---

### 第一步：加 include

```cpp|CODE_EDIT_BLOCK|/work/sunboxue/LiuLiang/traffic_flow/src/traffic_flow/traffic_flow_factory.cpp
// ... existing code ...
#include "common/perception_data.h"
#include "common/signal_phase_data.h"
#include "common/signal_phase_manager.h"

namespace fusion_perception::traffic_flow {
// ... existing code ...
```

---

### 第二步：在 detect() 里加信号灯周期组逻辑

在 `update()` 之后、ENSEMBLE 模式判断之前，插入信号灯周期组的检查：

```cpp|CODE_EDIT_BLOCK|/work/sunboxue/LiuLiang/traffic_flow/src/traffic_flow/traffic_flow_factory.cpp
// ... existing code ...
        for (const auto& one_section_detectors : _coil_detectors) {
            for (const auto& one_detector : one_section_detectors.second) {
                one_detector->update(detected_result);
            }
        }

        // 信号灯周期组 ×4（北/南/东/西）
        {
            std::vector<DirectionCode> directions = {
                DirectionCode::NORTH, DirectionCode::SOUTH,
                DirectionCode::EAST, DirectionCode::WEST
            };
            auto& global_config = traffic_flow::GlobalConfig::get_instance();
            auto road_sections = global_config.get_road_section_infos();

            for (auto dir : directions) {
                if (!SignalPhaseManager::get_instance().check_phase_boundary(dir)) {
                    continue;
                }
#ifdef DEBUG
                std::cout << "[SignalPhase] " << direction_code_to_string(dir)
                          << " phase boundary triggered." << std::endl;
#endif
                for (const auto& one_road_section : road_sections) {
                    if (one_road_section.second.direction != dir) {
                        continue;
                    }
                    std::vector<CoilInfo> coils = global_config.get_coil_infos(one_road_section.first);
                    for (const auto& one_coil : coils) {
#ifdef DEBUG
                        std::cout << "[DEBUG] signal coil: " << one_coil.tag << std::endl;
#endif
                        nlohmann::json signal_coil_result_j;
                        default_coil_result(signal_coil_result_j, one_coil);
                        for (const auto& one_detector : _coil_detectors[one_coil.tag]) {
                            one_detector->calculate_once(signal_coil_result_j);
                        }
                        // TODO: 组装信号灯周期结果并发布（与下游协商）
                    }
                }
            }
        }

        if (_working_mode == WorkingMode::ENSEMBLE) {
// ... existing code ...
```

---

### 逐段解释这段代码在干嘛

```
对每个方向 (北/南/东/西):
  │
  ├─ 问 SignalPhaseManager："这个方向完成一个周期了吗？"
  │     check_phase_boundary(dir)
  │     │
  │     ├─ false → continue，跳过这个方向
  │     └─ true  ↓
  │
  ├─ 遍历所有路段（road_sections）
  │     │
  │     ├─ 路段方向 ≠ 当前方向 → 跳过
  │     └─ 路段方向 = 当前方向 ↓
  │
  ├─ 拿到这个路段下的所有线圈（coils）
  │     │
  │     └─ 对每个线圈：
  │           ├─ 创建结果 JSON
  │           ├─ 对该线圈的所有 detector 调用 calculate_once()
  │           └─ 把结果放进列表（TODO: 发布）
```

**关键点：**

| 点                                           | 说明                                                   |
| -------------------------------------------- | ------------------------------------------------------ |
| `check_phase_boundary()`                   | 返回 true 说明这个方向刚完成一个绿→红→绿周期         |
| `one_road_section.second.direction != dir` | 通过路段的方向字段过滤，只处理对应方向的线圈           |
| `calculate_once()`                         | 跟固定周期组用的是同一个方法，统计这个周期内的流量数据 |
| `// TODO: 组装信号灯周期结果并发布`        | 你哥说下游不用你管，所以发布逻辑留 TODO                |

---

### 整体数据流现在长这样

```
DDS 收到信号灯 JSON
  → on_data_available(SIGNAL_JSON_SUB)
    → parse_signal_json() 解析
      → SignalPhaseManager::update() 缓存 + 检测边界
        → _boundary_triggered[NORTH] = true  ✅

detect() 每帧循环:
  → 取感知数据 → update 所有 detector
  → 检查4个方向相位边界
    → NORTH 触发了！
      → 找到 NORTH 方向的所有线圈
        → calculate_once() 统计流量
          → (TODO: 发布结果)
```

---

### 任务 6 改完的文件清单

| 文件                                          | 改动                                            |
| --------------------------------------------- | ----------------------------------------------- |
| `src/traffic_flow/traffic_flow_factory.cpp` | 加 2 个 include + detect() 里加信号灯周期组逻辑 |

---

## 6 个任务全部完成！最终改动汇总

| 任务 | 文件                                                 | 操作                         |
| ---- | ---------------------------------------------------- | ---------------------------- |
| 1    | `config/traffic_flow/traffic_flow_service_5s.json` | 加 subscriber                |
| 1    | `config/traffic_flow/traffic_flow_service_5m.json` | 加 subscriber                |
| 2    | `src/plugin/traffic_flow_service_impl.h`           | 加枚举值                     |
| 3    | `src/plugin/traffic_flow_service_impl.cpp`         | 加 case + 日志分支 + include |
| 4    | `src/common/signal_phase_data.h`                   | **新建**               |
| 4    | `src/common/signal_phase_data.cpp`                 | **新建**               |
| 4    | `src/common/CMakeLists.txt`                        | 加头文件                     |
| 5    | `src/common/signal_phase_manager.h`                | **新建**               |
| 5    | `src/common/signal_phase_manager.cpp`              | **新建**               |
| 6    | `src/traffic_flow/traffic_flow_factory.cpp`        | 加 include + detect() 逻辑   |

改完了可以试着编译看看有没有报错。有问题随时问我！

<chrono>
