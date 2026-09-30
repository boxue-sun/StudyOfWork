# 智路OS 简历四条 · 讲解手册

> 用法：面试前按第五节「面试话术」背；平时按章节翻。
> 阅读顺序：**先读第零节（整体）**，再读你简历上的四条。
> 本文原则：**不堆代码细节，只讲"干啥、输入输出、为什么这么设计"**——因为面试考的是思路。

---

# 第零节 · 先看整体：智路OS 到底是干啥的

## 0.1 一句话说清

**智路OS（AirOS）是一个装在路口铁盒子里的操作系统。**
这个铁盒子通常是一台 NVIDIA AGX Orin 或 Jetson 边缘计算设备，挂在路口的杆子上。

它干的活是：

```
接上路口的所有设备（相机、雷达、信号机、路侧单元RSU）
      ↓
把数据处理成"这个路口发生了什么"（有几辆车、在哪、往哪走、红灯还有几秒）
      ↓
通过 V2X 无线发给路过的车，或者通过 4G/5G 上传到云端
```

**为什么需要它？** 因为单车智能（车自己装摄像头雷达）有盲区、看不远。让路口"长眼睛"再告诉车，比车自己看更安全。这就是"车路协同"。

## 0.2 你在哪一层？

智路OS 分成**五层**，你简历上四条全部集中在**下面三层**：

```
【应用层】    做具体业务：闯红灯预警、绿波车速引导……        ← 你没碰
【服务层】    感知服务：检测、跟踪、融合、回3D              ← 你没碰
─────────────────────────── 以上是算法，以下是你碰的 ───────────────────────────
【框架层】    骨架和管子：调度、通信中间件、设备加载框架      ← 你简历 1、3 条
【硬件抽象层】把不同厂商的设备包装成统一接口                  ← 你简历 2 条
【内核层】    操作系统（Linux）                             ← 你简历 4 条（运维）
```

**记住这张图，面试时你能一句话定位自己**：

> "我参与的是**框架层和硬件抽象层**——也就是把路口的设备接进来、把数据在系统内部传起来。算法层不是我的范围。"

## 0.3 三级解耦架构（简历原文里那句"三级解耦"是什么意思）

简历上写的是「以『框架层+组件层+算法插件层』三级解耦架构组织」。这三级的**分工**是：

| 层级                 | 目录                                     | 它负责                                                       | 类比成一家餐厅                         |
| -------------------- | ---------------------------------------- | ------------------------------------------------------------ | -------------------------------------- |
| **框架层**     | `middleware/device_service/framework/` | 定义"一个设备应该怎么被管起来"，提供调度、通信、配置能力     | **大堂经理**：定规矩、派活、传菜 |
| **组件层**     | 同上（`xxx_component.cc`）             | 一个具体设备类型的"管家"：读配置、创建设备、起线程、转发数据 | **某个菜品的负责人**             |
| **算法插件层** | `middleware/device_service/modules/`   | 真正干活的代码：拉流、解码、解析协议。编译成`.so`，可插拔  | **厨师**：真正做菜的人           |

**为什么要分三级？** 核心一句话：

> **换设备（厨师换人）不用改框架（大堂经理的规矩不变）。**

这就是"解耦"的含义，也是你简历第 1 条的立身之本。

## 0.4 数据在主链路里怎么流（最重要的一张图）

以**相机**举例，从拍到车、到发出去：

```
【物理世界】
路口相机（海康/大华）—— 输出 RTSP 视频流
        ↓ RTSP over 网络
【硬件抽象层】 你简历第 2 条
StandardIpCamera：每路流开一个线程，用 FFmpeg 拉流
        ↓ 把 H.264 码流封装成 CompressedImage（protobuf）
        通过回调函数 sender_(topic, 数据) 往上一抛
【组件层】 你简历第 1、3 条的交叉点
IpCameraComponent：回调进来 → Send(topic, 数据)
        ↓
【框架层 / 通信中间件】 你简历第 3 条
CyberRT Channel：数据进入"话题总线"，比如 /sensor/ipcamera/h264/xxx
        ↓
【服务层】
检测算法订阅这个 Channel → 输出目标框
        ↓
【应用层】
发给车 / 上传云
```

**输入输出要记牢**（面试常问）：

| 环节               | 输入                     | 输出                                    |
| ------------------ | ------------------------ | --------------------------------------- |
| 设备接入           | 一个 IP 地址 + RTSP 地址 | 标准化的`CompressedImage`             |
| 通信中间件         | 一个话题名 + 一份数据    | 数据被放到话题总线上                    |
| **整条链路** | **相机的 RTSP 流** | **下游算法能订阅的 Channel 消息** |

---

# 第一节 · 简历第 1 条：设备抽象与插件化接入

## 1.1 简历原文

> 基于抽象基类 IpCameraDevice 定义设备统一接口（Init / Start / WriteToDevice / GetState），通过注册宏将具体设备注册进工厂，新增设备类型无需改动框架代码，实现设备接入配置化（读 device.yaml 由框架动态加载驱动）。

主程序启动
   │
   ▼
DynamicLoader 扫描插件目录（比如 /opt/airos/device_libs/）
   │
   ▼
遍历每个子目录（hikvision/、dahua/、standard/ ...）
   │
   ▼
每个子目录里读 device_lib_cfg.pb
   │
   ▼
解析出 so_name（比如 "libstandard_ipcamera.so"）
   │
   ▼
拼出完整路径，用 LibraryHolder 加载 .so
   │
   ▼
.so 里的静态对象构造
   │
   ▼
注册宏 V2XOS_IPCAMERA_REG_FACTORY 自动执行
   │
   ▼
工厂 map_ 里多了一条：
   "standard_ipcamera" → [](cb) { return new StandardIpCamera(cb); }

## 1.2 这一条整体在干啥（先看这个）

### 要解决的问题

路口设备的**厂商是混着用的**：这家路口装海康，那个路口装大华，测试环境用仿真数据。

每家的 SDK 长得完全不一样：

```cpp
// 海康的初始化函数（C 风格，十几个参数，一堆魔法字符串）
api_init_instance(API_MODE_OFFLINE, ip, name, gpu_id, img_mode,
                  "hikvision", stream_num, channel_num, user, password, callback);

// 大华的又是另一套完全不同的函数名和参数
// 仿真的又没有 SDK，就是自己造数据
```

**如果让上层算法直接调这些 SDK，会怎样？**

- 上层代码里到处是 `if (厂商 == 海康) ... else if (厂商 == 大华) ...`
- 新增一家厂商，所有上层代码都要改、都要重新测
- 换设备 = 改代码 = 重新发版。**这是灾难。**

### 解决方案：定义一个"设备应该长什么样"的抽象

**核心思路：不管你是什么厂商，你都必须长成这四副样子。**

```
        抽象基类 IpCameraDevice
        （定义"一个相机应该具备什么能力"）
                ↑  继承
    ┌───────────┼───────────┐
海康相机      大华相机     仿真相机
（各不相同，但都长得像基类）
                ↓  注册
     工厂（一张表：名字 → 造对象的方法）
                ↓  查表
        上层：给我一个"hikvision"
```

**输入输出**：

|                | 内容                                                                            |
| -------------- | ------------------------------------------------------------------------------- |
| **输入** | 一个**字符串设备名**（如 `"standard_ipcamera"`）+ 一个回调函数          |
| **输出** | 一个**基类指针** `IpCameraDevice*`，上层用它调 `Init()` / `Start()` |

## 1.3 涉及的类与文件（先记住这几个名字）

| 文件                                                        | 里面的东西                | 一句话作用                                                |
| ----------------------------------------------------------- | ------------------------- | --------------------------------------------------------- |
| `base/device_connect/ipcamera/device_base.h`              | `IpCameraDevice`        | **抽象基类**：定义设备必须有的 4 个能力             |
| `base/device_connect/ipcamera/device_factory.h/.cc`       | `IpCameraDeviceFactory` | **注册工厂**：一张表，存"名字 → 怎么造设备"        |
| `base/plugin/modules_loader/dynamic_loader.h/.cc`         | `DynamicLoader`         | **动态加载器**：扫目录、读配置、把 `.so` 加载进来 |
| `base/plugin/modules_loader/library-holder.h`             | `LibraryHolder`         | **动态库句柄的包装**（管着 dlopen 出来的东西）      |
| `modules/ipcamera/standard_ipcamera/standard_ipcamera.cc` | `StandardIpCamera`      | **具体设备**（简历第 2 条的主角）                   |
| `modules/ipcamera/conf/device_lib_cfg.pb`                 | 配置文件                  | 告诉框架"这个 so 里有哪些设备"                            |

## 1.4 每个函数是干嘛的、参数是啥

### ① 抽象基类的 4 个方法 —— 定义"设备必须会做的 4 件事"

```cpp
class IpCameraDevice {
  virtual bool Init(const std::string& config_file) = 0;
  virtual void Start() = 0;
  virtual IpCameraDeviceState GetState() = 0;
  virtual void WriteToDevice(const std::shared_ptr<const IpCameraReceiveData>& data) = 0;
 protected:
  IpCameraCallBack sender_;   // 用来"往上抛数据"的回调
};
```

| 方法                    | 干什么                                     | 参数是什么                                                                                    | 为什么这么设计                                                                                        |
| ----------------------- | ------------------------------------------ | --------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| `Init(config_file)`   | 读配置、连相机、准备资源                   | `config_file`：**配置文件的路径**（字符串）。因为这个路径不同设备不一样，只能运行时给 | 把"初始化"和"构造"分开。构造函数里不该失败，但连相机可能失败——所以用`Init` 返回 `bool` 表示成败 |
| `Start()`             | 真正开始工作（拉流）                       | 无参数                                                                                        | 单独一个方法，是因为**必须先 Init 成功才能 Start**。分开两个阶段，调用方好控制                  |
| `GetState()`          | 查当前状态                                 | 无参数，返回枚举`RUNNING` / `STOP` / `UNKNOWN`                                          | 让上层能"体检"。运维监控需要它                                                                        |
| `WriteToDevice(data)` | 往设备**下发**指令（如控制云台转动） | `data`：下发的数据                                                                          | 数据流是双向的：设备往上**抛数据**用 `sender_`，上层往下**发指令**用这个方法            |

**记忆口诀：初始化 → 启动 → 查状态 → 反向控制。这四步覆盖了一个设备的完整生命周期。**

### ② `sender_` 这个回调是干什么的

```cpp
using IpCameraDataType = std::shared_ptr<const CompressedImage>;
using IpCameraCallBack = std::function<void(const std::string& stream_id, const IpCameraDataType&)>;
```

**一句话：这是设备"往上抛数据"的电话线。**

| 组成                             | 含义                                                                                         |
| -------------------------------- | -------------------------------------------------------------------------------------------- |
| `const std::string& stream_id` | **哪一路流**。因为一个设备可能同时管 4 路相机（4 个通道）                              |
| `const IpCameraDataType&`      | **数据本体**。`shared_ptr` = 引用计数自动管理；`const` = 只读，禁止别人偷改        |
| `std::function<...>`           | **通用函数包装**。上层可以塞任何函数进来（普通函数、lambda、成员函数），只要签名对得上 |

**为什么签名是"引用 + const"？**

- 用引用：避免拷贝（一帧图像拷贝一次要好几 MB）
- 用 const：这帧数据会被多个线程读（检测、跟踪、录像都要读），加 const 让编译器帮你保证"没人能改"

**为什么不用"上层主动调用 GetImage()"来取数据？**

这是个经典设计问题，面试爱问：

|        | 推模式（本项目用的回调）                         | 拉模式（上层轮询 GetImage） |
| ------ | ------------------------------------------------ | --------------------------- |
| 实时性 | 数据一到立刻触发，零延迟                         | 得等下一次轮询              |
| CPU    | 没事干就睡着，零空转                             | 得开线程不停问"有数据吗"    |
| 缺点   | **回调里不能做耗时操作**（会卡住拉流线程） | 实时性差                    |

**答话术**：

> "相机数据是异步到达的，用推模式（回调）实时性最好、不空转 CPU。代价是回调函数跑在拉流线程上，所以必须轻量——本项目里回调只做一件事：往话题上转发。"

### ③ 工厂的方法

```cpp
using CONSTRUCT = std::function<IpCameraDevice*(const IpCameraCallBack&)>;

class IpCameraDeviceFactory {
 public:
  static IpCameraDeviceFactory& Instance();                                    // 单例
  std::shared_ptr<IpCameraDevice> GetShared(const std::string& key, const IpCameraCallBack& cb);
  std::unique_ptr<IpCameraDevice> GetUnique(const std::string& key, const IpCameraCallBack& cb);
 private:
  IpCameraDevice* Produce(const std::string& key, const IpCameraCallBack& cb);
  std::map<std::string, CONSTRUCT> map_;      // ★ 核心：名字 → 造对象的方法
};
```

| 成员                   | 干什么                                                                 | 参数                                    | 为什么这么设计                                                                          |
| ---------------------- | ---------------------------------------------------------------------- | --------------------------------------- | --------------------------------------------------------------------------------------- |
| `map_`               | **一张字典表**：左边是设备名字符串，右边是一个"能造出设备"的函数 | —                                      | 有这张表，就**不需要 if-else 分支**了。查表比写分支好，因为加新设备不用改表的结构 |
| `Instance()`         | 返回**全局唯一的那个工厂**（单例）                               | 无                                      | 全系统只能有一张表。否则 A 文件注册进 A 表、B 文件查 B 表，两边对不上                   |
| `GetUnique(key, cb)` | 查表造对象，返回**独占所有权**的智能指针                         | `key`：设备名字符串；`cb`：数据回调 | 一台设备通常只属于一个使用者，`unique_ptr` 语义最准、零开销                           |
| `GetShared(key, cb)` | 查表造对象，返回**共享所有权**的智能指针                         | 同上                                    | 多个人要用同一台设备时用                                                                |
| `Produce(key, cb)`   | 真正干活的：查表 → 调用表里的函数 → 返回裸指针                       | 同上                                    | 查不到时返回`nullptr`                                                                 |

**为什么要同时提供 unique 和 shared？**

答话术：

> "看谁拥有这台设备。如果只有一个组件用它，给 `unique_ptr` 更合适——语义清晰、还没有 `shared_ptr` 原子计数的开销。如果多个模块要共享同一台设备，才用 `shared_ptr`。**能说清'什么时候不该用 shared_ptr'，比夸 shared_ptr 更能体现水平。**"

### ④ 注册宏 —— 最巧妙的一块

```cpp
#define V2XOS_IPCAMERA_REG(T)   v2xos_reg_func_str_##T##_

#define V2XOS_IPCAMERA_REG_FACTORY(T, key)                          \
  static IpCameraDeviceFactory::Register_t<T> V2XOS_IPCAMERA_REG(T)(key);
```

**用法**：在具体设备的 `.cc` 文件末尾写一行：

```cpp
V2XOS_IPCAMERA_REG_FACTORY(StandardIpCamera, "standard_ipcamera");
```

**就这一行，这个设备就"自动"能被系统找到了。** 上层代码一个字都不用改。

**展开后它变成什么？**

```cpp
static IpCameraDeviceFactory::Register_t<StandardIpCamera>
       v2xos_reg_func_str_StandardIpCamera_("standard_ipcamera");
```

**翻译成人话**：

> 定义一个**全局静态变量**，名字叫 `v2xos_reg_func_str_StandardIpCamera_`，它构造的时候会往工厂的表里塞一条记录：`"standard_ipcamera" → new StandardIpCamera(cb)`。

**为什么"全局静态变量构造"就等于"注册"？**

这是 C++ 的一条规则：**所有全局/静态变量在 `main()` 之前就构造完毕**。

所以：

```
程序启动（main 还没跑）
   ↓
所有 .so 里的静态变量开始构造
   ↓
v2xos_reg_func_str_StandardIpCamera_ 构造
   ↓
它的构造函数执行 map_.emplace("standard_ipcamera", ...)
   ↓
工厂表里多了一条记录 ✅
   ↓
（之后）上层 GetShared("standard_ipcamera", cb) 就能查到
```

**这个技巧叫「自注册」（self-registration）。**

**为什么宏定义里要做 `##T##_` 这种拼接？**

`##` 是 C++ 预处理器里的"记号粘贴符"，把左右两边拼成一个新名字。

- `V2XOS_IPCAMERA_REG(StandardIpCamera)` → `v2xos_reg_func_str_StandardIpCamera_`

**为什么要把类名嵌进变量名？**
因为宏会在**每个 .cc 文件里展开**。如果都用同一个名字（比如 `reg`），同一个文件里注册两个设备就会**重复定义、编译报错**。把类名拼进去，**天然保证每个设备的名字唯一**。

### ⑤ `DynamicLoader`：从"有 .so 文件"到"设备能用"的最后一公里

```cpp
bool LoadDevicePackage(const std::string& device_lib_path);
bool LoadAppPackage(const std::string& app_lib_path);
static DynamicLoader& GetInstance();
```

| 方法                        | 干什么                                                                                                      | 参数                                         |
| --------------------------- | ----------------------------------------------------------------------------------------------------------- | -------------------------------------------- |
| `LoadDevicePackage(path)` | 扫描`path` 下的所有子目录，每个目录里读 `device_lib_cfg.pb`，按里面写的 `so_name` 把 `.so` 加载进来 | 设备库的根目录，如`"device/lib/ipcamera/"` |
| `LoadAppPackage(path)`    | 同上，但加载的是应用层的库                                                                                  | 应用库目录                                   |
| `GetInstance()`           | 单例                                                                                                        | 无                                           |

**它的内部流程（4 步）**：

```
1. 列出 device_lib_path 下的所有子目录
2. 每个子目录里找 device_lib_cfg.pb 这个文件
     读出来是这样的内容：
        so_name: "libipcamera_device.so"
        device_name: "standard_ipcamera"
3. 拼出完整路径，调用 dlopen 加载这个 .so
4. .so 一加载，里面的静态变量构造 → 注册宏执行 → 设备进工厂表 ✅
```

**这一步是全套设计的收口。** 到这里，"新增设备不用改框架"才真正成立——因为框架是**扫目录扫出来的**，不是硬编码的。

## 1.5 为什么这么设计（面试核心，必背）

### 问题：这套设计到底解决了什么？

**答案：开闭原则（对扩展开放、对修改关闭）的工程落地。**

|                  | 传统写法                                    | 本项目的做法                                                  |
| ---------------- | ------------------------------------------- | ------------------------------------------------------------- |
| 新增一家相机厂商 | 改上层代码，加`else if`，重新测试所有分支 | **新增一个文件 + 一行注册宏**，重新编译。上层代码零改动 |
| 谁决定用哪个设备 | 写死在代码里                                | **写在配置文件里**（`device: "standard_ipcamera"`）   |
| 设备从哪来       | 编译时链接                                  | **运行时扫目录加载 `.so`**                            |

### 三处"字符串"是怎么对上的（这是整条链路的灵魂）

```
①  modules/ipcamera/conf/device_lib_cfg.pb
        so_name: "libipcamera_device.so"
        device_name: "standard_ipcamera"        ← 告诉框架：这个 so 提供哪些设备
                     ↓
②  framework/ipcamera/conf/config.pb.txt
        device: "standard_ipcamera"             ← 配置说：我要用这个设备
                     ↓
③  standard_ipcamera.cc 最后一行
        V2XOS_IPCAMERA_REG_FACTORY(StandardIpCamera, "standard_ipcamera")
                                                ↑ 注册宏说：我叫这个名字
```

**三处的字符串完全一致，链路就闭合了。**
面试时你讲这一段，面试官会觉得你是真的读通了代码，而不是背的。

### 面试官可能追问的问题

**Q：这套设计有什么缺点或可以改进的地方？**

A（挑 2 个说就够，这部分是加分项）：

1. **配置写错时是"静默失败"**。如果配置文件里写了 `"hikvision"` 但这个设备没被注册，工厂 `Produce` 返回 `nullptr`，代码里如果没有检查，会在后面某一处莫名其妙崩溃。**更好的做法是查不到时立刻报错并打日志，把"运行期玄学崩溃"变成"启动期明确报错"。**
2. **`Init(const std::string& config_file)` 用字符串传配置不够安全**。更现代的做法是传一个**结构化的配置对象**（protobuf 或 struct），这样字段写错编译期就能发现。项目里确实还并存着一套用 `CameraInitConfig` 结构体的旧接口，说明团队也意识到了这点。

**Q：为什么用 `enum class` 而不是普通 `enum`？**

A：普通 `enum` 的枚举值会泄漏到外层作用域，而且能隐式转成 int。项目里有相机、雷达、信号机等多个状态枚举，如果都用普通 enum，多个文件里都有 `RUNNING` 就会重定义冲突。`enum class` 把它关在 `IpCameraDeviceState::` 里，还能禁止意外的隐式转换。

**Q：为什么基类析构函数必须是 virtual？**

A：因为上层是拿**基类指针**去操作子类的（`IpCameraDevice* dev = ...`，实际指向 `StandardIpCamera`）。如果析构不 virtual，用基类指针 delete 时只会调基类析构，子类的析构**不执行**——子类里的 FFmpeg 句柄、拉流线程、文件句柄全部泄漏。**线程泄漏最致命**：线程还在跑，但它引用的对象已经被销毁，一访问就崩。

## 1.6 这条简历的面试话术（30 秒版）

> "智路OS 要接各种厂商的相机，每家 SDK 完全不一样。如果让上层直接调 SDK，代码里会到处是厂商分支，加一家就要改一遍。
>
> 我的工作是参与设备抽象层的开发：定义了一个抽象基类 IpCameraDevice，规定任何设备都要实现初始化、启动、查状态、下发指令这四个接口；然后搭了一个注册工厂，用注册宏让各家设备'自注册'进一张名字到构造方法的表里；最后通过动态加载器扫目录加载 .so，配置里写什么名字就用什么设备。
>
> 效果是新增一家相机厂商，只要新增一个文件加一行注册宏，**框架代码一行都不用改**。"

---

# 第二节 · 简历第 2 条：多路视频流接入

## 2.1 简历原文

> 参与多路 RTSP 拉流模块开发与链路调试，每路流独立线程 + atomic 状态标志 + mutex 保护流表，基于 FFmpeg 解码封装为框架标准数据经 Channel 发布，支持断线自动重连与帧间隔统计。

4 个 RTSP 相机
   │
   ▼
StandardIpCamera 为每路开一个线程
   │
   ▼
每个线程用 FFmpeg 拉流，读 H.264 帧
   │
   ▼
封装成 CompressedImage（H.264 数据 + 元信息）
   │
   ▼
调用 sender_(output_topic, compressed_image)
   │
   ▼
回调把数据写到对应 Channel
   │
   ▼
订阅这些 Channel 的模块收到图像
   │
   ├── AI 识别模块：解码后送模型
   ├── 录像模块：存成文件
   └── 推流模块：转发到别处







## 2.2 这一条整体在干啥（先看这个）

### 要解决的问题

一个路口要接 **4 路甚至更多相机**，而且相机是**实时视频流**，特点是：

1. **不会停**——你得一帧一帧不停地读
2. **随时会断**——网络抖一下、相机重启一下，流就没了
3. **读一帧要等**——不能打断，得阻塞等着
4. **4 路要同时进行**——一起读，不能排队

**这四件事决定了必须用多线程。** 单线程串行读 4 路流 = 第 2 路会积压延迟。

### 这一条的整体链路（背下来）

```
输入：4 个 RTSP 地址（rtsp://user:pwd@ip:port/stream1 ...）
      ↓
StandardIpCamera::Init()   读 YAML 配置，为每路流建一个"流信息"结构体，存入流表
      ↓
StandardIpCamera::Start()  遍历流表，为每路流起一个独立线程
      ↓
【线程1】读流1   【线程2】读流2   【线程3】读流3   【线程4】读流4   ← 4 个线程同时跑
      ↓ 每个线程内部：
      用 FFmpeg 读一个包
      → 判断是不是关键帧
      → 解析时间戳
      → 打包成标准数据 CompressedImage
      → 调用回调 sender_(话题名, 数据) 抛给上层
      ↓
输出：4 个话题（Channel）上的视频消息流
```

**输入输出**：

|                | 内容                                                                               |
| -------------- | ---------------------------------------------------------------------------------- |
| **输入** | 4 个 RTSP 直播流地址                                                               |
| **输出** | 4 个 CyberRT Channel，每个上面是连续的`CompressedImage`（H.264 压缩帧 + 元信息） |

## 2.3 涉及的文件

| 文件                                                       | 作用                                                            |
| ---------------------------------------------------------- | --------------------------------------------------------------- |
| `modules/ipcamera/standard_ipcamera/standard_ipcamera.h` | 类定义 +**`StreamInfo` 结构体**（每路流的状态都装在这） |
| `...standard_ipcamera.cc`                                | 全部实现：Init / Start / 拉流线程 / 关键帧判断 / 时间戳解析     |
| `modules/ipcamera/conf/ipcamera_config.yaml`             | 配置文件：4 个 RTSP 地址 + 4 个输出话题                         |

## 2.4 关键数据：`StreamInfo` —— "一路流的全部状态"

**先理解这个结构体，后面的代码就都懂了。** 它描述"一路流在运行时的所有信息"：

```cpp
struct StreamInfo {
    // 【身份】这路流是谁、数据往哪去
    std::string rtsp_url;        // 从哪拉
    std::string output_topic;    // 往哪个话题发

    // 【控制】线程之间用来"喊停"
    std::atomic<bool> stop;      // 停止信号
    std::atomic<bool> connected; // 连上了没

    // 【线程】这路流的专属工作线程
    std::unique_ptr<std::thread> pull_thread;

    // 【FFmpeg 句柄】
    AVFormatContext* format_ctx;   // FFmpeg 的流上下文
    int video_stream_index;        // 视频流在文件里的下标

    // 【计数与统计】
    uint32_t sequence_num;         // 发出的第几条消息
    uint64_t frame_counter;        // 这路流的第几帧
    double last_send_time;         // 上一帧什么时候发的
    double total/min/max_interval_sec;  // 帧间隔统计
    int64_t interval_count;

    // 【落盘，默认关】
    std::ofstream h264_file;
    int64_t saved_frame_count;
};
```

**面试重点：为什么 `stop` 和 `connected` 必须用 `std::atomic<bool>`？**

答话术：

> "因为这两个标志是**跨线程**的：主线程写（要停止）、拉流线程读（判断要不要退出循环）。如果用普通 `bool`，编译器优化时可能把它缓存在 CPU 寄存器里，拉流线程永远读不到 `stop` 变成了 `true`——线程就退不出来，程序卡死。`atomic` 保证每次读写都真正访问内存，并且是原子的。"

**面试重点：为什么 `pull_thread` 要用 `unique_ptr`？**

答：`std::thread` 这个类**不可拷贝、不可赋值**，只能移动。而且项目里需要在 `StartStream` 里重新给它赋值（`reset`），所以必须用指针包一层。

**面试重点：`frame_counter` 和 `sequence_num` 有什么区别？**

- `frame_counter`：**这路流的第几帧**（业务含义）
- `sequence_num`：**这个通道发出的第几条消息**（传输含义）

虽然值通常一样，但语义不同。**能区分这两个，说明你理解"业务语义 vs 传输语义"。**

## 2.5 每个函数是干嘛的、参数是啥

### ① `Init(conf)` —— 读配置、建流表

```cpp
bool StandardIpCamera::Init(const std::string& conf);
```

|                  | 内容                                                                 |
| ---------------- | -------------------------------------------------------------------- |
| **干什么** | 读 YAML 配置文件 → 校验 → 为每路流建一个`StreamInfo` → 存进流表 |
| **参数**   | `conf`：YAML 配置文件的路径                                        |
| **返回**   | 成功/失败                                                            |

**内部做的 4 件事**：

```
1. 用 yaml-cpp 解析配置文件
   读出：rtsp_urls（4 个地址）、output_topics（4 个话题）、重连间隔、超时时间等

2. ★ 校验数量一致
   if (rtsp_urls_.size() != output_topics_.size()) { 报错，返回 false }

3. 初始化 FFmpeg（av_register_all / avformat_network_init）

4. 循环 4 次，为每个 URL 建一个 StreamInfo，key 用话题名存进 streams_ 这张表
```

**第 2 步的校验是重点，面试可以讲：**

> "配置里 URL 和话题必须是**一一对应**的，因为后面是用下标去取的。如果数量不一致，取的时候就会越界——`std::vector` 的 `operator[]` 不做边界检查，是未定义行为，可能读到垃圾数据、也可能直接段错误。所以我在 Init 里加了校验，**把'运行到一半神秘崩溃'提前成'启动时明确报错'**。"

### ② `Start()` / `Stop()` —— 启动和停止所有流

```cpp
void Start();   // 遍历流表，为每路流起一个线程
void Stop();    // 停止所有流，清理资源
```

|             | 干什么                                                  | 关键点                                     |
| ----------- | ------------------------------------------------------- | ------------------------------------------ |
| `Start()` | 遍历`streams_`，对每一路调 `StartStream()`          | 每个`StartStream` 里 `new std::thread` |
| `Stop()`  | 先置`stop_ = true`，再遍历停止每一路，最后清理 FFmpeg | **顺序很重要**                       |

**为什么 `Stop()` 要先置标志再遍历？**

因为拉流线程在循环里检查 `stop_`。先置标志，线程看到后就会自己往外退；如果顺序反了（先遍历 join、后置标志），线程可能还在跑就被要求 join，造成等待甚至死锁。

### ③ `StartStream(stream_id)` —— 给一路流起线程

```cpp
void StartStream(const std::string& stream_id);
```

|                  | 内容                                                                                     |
| ---------------- | ---------------------------------------------------------------------------------------- |
| **干什么** | 找到这条流的`StreamInfo`，起一个线程去跑 `TaskPullRtspStream`                        |
| **参数**   | `stream_id`：流的名字。**注意实际传进来的是话题名**（因为流表的 key 就是话题名） |

**为什么流表的 key 用话题名，不用 RTSP 地址？**

答话术：

> "因为数据最终要按话题发出去，回调 `sender_` 的第一个参数就是话题名。用话题名当 key，找到流信息后直接拿 key 就能发，不用再从流信息里取一次。"

### ④ `TaskPullRtspStream(stream_id)` —— 拉流线程的主循环（核心）

```cpp
void TaskPullRtspStream(const std::string& stream_id);
```

**这是整个模块的心脏。它跑在独立线程上，是一个"双层循环"：**

```
┌─ 外层循环：管"连接" ─────────────────────────────┐
│                                                   │
│   ConnectRtspStream()  ← 尝试连接                 │
│   ├─ 失败 → 睡 5 秒 → 回到循环开头重试            │
│   └─ 成功 → 设 connected = true                   │
│                                                   │
│   ┌─ 内层循环：管"读数据" ──────────────────┐    │
│   │                                          │    │
│   │  av_read_frame()  读一个包               │    │
│   │  ├─ 读失败/流结束 → break，跳出内层        │    │
│   │  └─ 成功：                                │    │
│   │       判关键帧 → 解析时间戳 → 打包         │    │
│   │       → sender_(话题, 数据)  发给上层      │    │
│   │       av_packet_unref()  释放这个包        │    │
│   └──────────────────────────────────────────┘    │
│                                                   │
│   关掉连接 → 睡 5 秒 → 回到外层循环重试            │
└───────────────────────────────────────────────────┘
```

**为什么要分两层循环？这是必须理解的：**

| 循环           | 负责                         | 为什么单独一层                                                                                        |
| -------------- | ---------------------------- | ----------------------------------------------------------------------------------------------------- |
| **外层** | 连接的生命周期：连、断、重连 | RTSP 流**随时会断**（网络抖动、相机重启）。如果只有一层，读到流结束就只能退出线程，永远不重连了 |
| **内层** | 数据读取：连上以后疯狂读     | 一旦连上就专心读数据，不用一直检查连接                                                                |

**答话术**：

> "拉流必须有重连。路口的相机风吹雨淋、供电不稳，经常掉线。如果读到流结束就让线程退出，那相机重启后就再也收不到数据了，得重启整个程序。所以设计成两层循环：外层管连接和重连，内层只管读数据。这样相机重启后 5 秒内自动恢复，运维不用上去手动操作。"

### ⑤ `ConnectRtspStream(stream_id)` —— 建立连接

```cpp
bool ConnectRtspStream(const std::string& stream_id);
```

**干什么**：用 FFmpeg 打开 RTSP 流，找到视频流的位置。

```
1. avformat_open_input()     打开 RTSP 地址
2. avformat_find_stream_info()  读取流信息（有几个流、什么编码）
3. 遍历所有流，找 codec_type == VIDEO 的那个，记下它的下标
   存进 stream_info->video_stream_index
```

**为什么要专门找"视频流的下标"？**

因为一个 RTSP 流里可能同时有视频、音频、甚至多个视频轨道。**必须记住视频是第几个，读数据时才能过滤掉非视频的包。**

**（这是 ffmpeg 使用的三个核心调用之一，面试如果问"你怎么用 FFmpeg 的"，就说这三个：打开 → 找流信息 → 找视频流下标。）**

### ⑥ `IsKeyFrame(data, size)` —— 判断是不是关键帧

```cpp
bool IsKeyFrame(const uint8_t* data, size_t size);
```

|                  | 内容                                              |
| ---------------- | ------------------------------------------------- |
| **干什么** | 扫描这段 H.264 数据，判断它是不是"关键帧（I 帧）" |
| **参数**   | `data`：H.264 原始字节；`size`：字节数        |
| **返回**   | 是关键帧就`true`                                |

**背景知识（必须懂，面试会问）：**

H.264 视频不是每一帧都独立的，分三种帧：

| 帧类型         | 别名                   | 特点                                                   |
| -------------- | ---------------------- | ------------------------------------------------------ |
| **I 帧** | **关键帧 / IDR** | **完整的一幅图**，可以独立解码                   |
| P 帧           | 前向预测帧             | 只存"和上一帧的差异"，**必须依赖前面的帧才能解** |
| B 帧           | 双向预测帧             | 依赖前后帧                                             |

**为什么必须判断关键帧？**

因为 P/B 帧不能独立解码。如果下游解码器从 P 帧开始解，画面会花屏/绿屏。**必须从 I 帧开始才能正常解码。**

**怎么判断？（实现原理）**

H.264 的每个数据块长这样：

```
[起始码 00 00 01] [1 字节头部] [数据内容...]
                   ↑
                   低 5 位 = 类型编号
```

代码做的事：

```
1. 扫描数据，找 00 00 01 这个起始码
2. 取起始码后面那个字节，与 0x1F 做位与（& 0x1F = 只保留低 5 位）
3. 结果是 5 → 就是关键帧（IDR）
```

**类型编号速查（记住 5 就够了）**：1 = 普通帧；**5 = 关键帧**；6 = SEI（时间戳藏在这）；7 = SPS；8 = PPS。

**判断结果怎么用？** 写进消息里：

```cpp
compressed_image->set_frame_type(is_key_frame ? 1 : 0);
```

下游拿到消息，看到 `frame_type == 1` 就知道"可以从这开始解码"。

### ⑦ `ExtractExposureTimestamp(data, size)` —— 解析曝光时间戳

```cpp
int64_t ExtractExposureTimestamp(const uint8_t* data, size_t size);
```

|                  | 内容                                                |
| ---------------- | --------------------------------------------------- |
| **干什么** | 从 H.264 数据里把**相机曝光的真实时刻**抠出来 |
| **参数**   | 同上                                                |
| **返回**   | 毫秒级时间戳；没找到返回 -1                         |

**为什么要这个？这是个很好的面试素材。**

```
一帧图像的真实时间线：
   相机曝光  →  编码  →  网络传输  →  我们收到
      ↑                                  ↑
  真实时间戳                      系统时间（晚了 50~300ms，还抖）
```

**如果下游用"收到的时间"去做多传感器融合（相机 + 雷达 + 信号机对齐），就错了**——因为传输延迟大且不稳定。

**所以要从 SEI 里读相机自己的时间戳**，这样时间对齐精度能到毫秒级。

**答话术**：

> "多传感器融合的前提是时间对齐。如果用系统接收时间，里面包含了传输抖动。相机在编码时会把曝光时刻写进 SEI 补充信息里，我从那里读，拿到的是真实的曝光时间，对齐精度高得多。"

**（注意：这段代码里的偏移量是硬编码的，是个可以改进的点。面试如果被问"这段有什么问题"，可以说"不同厂商 SEI 布局不同，硬编码偏移不够健壮，标准做法是按 payloadType 解析"。）**

### ⑧ 为什么用 `frame_counter` 统计帧间隔

```cpp
double interval = 当前时间 - last_send_time;
// 累加、取最小、取最大
```

|                  | 内容                                                                          |
| ---------------- | ----------------------------------------------------------------------------- |
| **干什么** | 记录相邻两帧发出的时间间隔，累积统计平均值/最小值/最大值，每 100 帧打一次日志 |
| **为什么** | **这是运维监控需要的数据**——帧率稳不稳、有没有卡顿，看这个 log 就知道 |

**答话术**：

> "现场排查问题时，光看'有没有数据'不够，还得看'帧率稳不稳'。所以我在拉流循环里加了帧间隔统计，每 100 帧打一次日志，平均/最小/最大间隔一目了然。**流卡顿时能立刻定位是相机问题还是网络问题。**"

## 2.6 多线程 + 锁，具体怎么用的（简历原文那三个词）

简历上写的是「每路流独立线程 + atomic 状态标志 + mutex 保护流表」。拆开讲：

| 技术                          | 用在哪                                 | 为什么                                                                       |
| ----------------------------- | -------------------------------------- | ---------------------------------------------------------------------------- |
| **每路流独立线程**      | `StartStream` 里 `new std::thread` | 4 路流必须并发读。串行读会导致后面的流延迟累积                               |
| **`atomic` 状态标志** | `StreamInfo::stop`、`connected`    | 跨线程的标志位。普通`bool` 可能被编译器缓存，线程读到旧值退不出来          |
| **`mutex` 保护流表**  | `streams_mutex_` 保护 `streams_`   | 流表是**多线程共享**的：主线程遍历它、各个拉流线程查找它。同时读写会崩 |

**面试可能追问：`streams_` 这个 map 在多线程下怎么保证安全？**

答：

> "所有对 `streams_` 的读写都在 `streams_mutex_` 保护下。原则是**锁的粒度尽量小**——只锁真正访问 map 的那几行。另外注意一个细节：从 map 里取流信息时要用 `auto& x = it->second`（引用），不要用 `auto x = it->second`（拷贝），否则你改的是副本。"

## 2.7 这条简历的面试话术（30 秒版）

> "一个路口要接 4 路相机，相机是实时流——不会停、随时会断、读一帧要阻塞等待。这决定了必须并发。
>
> 我的工作是参与拉流模块：每路流起一个独立线程，用 FFmpeg 拉 RTSP；线程用一个原子标志位来控制退出，用互斥锁保护流表；每个线程内部从码流里识别关键帧、解析相机的 SEI 曝光时间戳，然后打包成框架的标准数据结构，通过回调发布到对应的 Channel。
>
> 关键设计是**双层循环**：外层管连接和断线重连，内层只管读数据。因为路口相机经常掉线，必须能自动恢复，不能靠人为重启程序。
>
> 另外我在循环里加了帧间隔统计，平均/最大/最小间隔，排查现场卡顿问题很有用。"

---

# 第三节 · 简历第 3 条：通信中间件封装

## 3.1 简历原文

> 参与 CyberRT 统一接口封装层开发（Node / Reader / Writer / Component），编写进程内与跨进程 pub-sub 通信示例，通过 DAG 配置声明组件订阅拓扑，调整链路无需修改代码。

## 3.2 这一条整体在干啥（先看这个）

### 要解决的问题

系统里各模块之间要传数据：相机模块要发图像、检测模块要收图像发目标、跟踪模块要收目标……**这些数据怎么传？**

智路OS 选了 **CyberRT**（Apollo 开源的自动驾驶通信框架）作为通信中间件。

**问题来了**：

1. 直接用 CyberRT 的 API，那**全项目的代码都被 CyberRT 绑死**了。将来想换中间件（比如换成 ROS2、或者自己写一套），就要改遍所有文件。
2. CyberRT 的 API 有点底层，每个业务开发者都要学一遍。

**所以要做一层"统一接口封装"：让业务代码不直接碰 CyberRT。**

### 一句话理解这条简历

> **做一层"翻译中介"，把 CyberRT 包起来，业务代码只说自己的话，不直接和 CyberRT 打交道。**

### 输入输出

|                | 内容                                                                                           |
| -------------- | ---------------------------------------------------------------------------------------------- |
| **输入** | 对业务开发者的价值：**一套与具体中间件无关的接口**（Node / Reader / Writer / Component） |
| **输出** | 业务代码零 CyberRT 依赖；换中间件只改一个文件                                                  |

## 3.3 涉及的文件（`middleware/runtime/` 目录）

| 文件                               | 里面的东西                 | 作用                                                |
| ---------------------------------- | -------------------------- | --------------------------------------------------- |
| `src/air_middleware_common.h`    | 一堆`#define`            | ★**核心**：把 CyberRT 的名字"改名"成通用名字 |
| `src/air_middleware_node.h`      | `AirMiddlewareNode`      | 节点：用来创建 reader 和 writer                     |
| `src/air_middleware_reader.h`    | `AirMiddlewareReader<T>` | 订阅者（收数据）                                    |
| `src/air_middleware_writer.h`    | `AirMiddlewareWriter<T>` | 发布者（发数据）                                    |
| `src/air_middleware_component.h` | `ComponentAdapter`       | 组件适配器：业务写这个基类                          |
| `src/cyberrt_component.h`        | 两个宏                     | ★**核心**：自动生成桥接类                    |
| `demo/`                          | 5 个 demo                  | 进程内通信、跨进程通信、组件通信                    |

## 3.4 五个核心概念（先记名字，再看怎么包装的）

| 名字                | 是什么                                                           | 类比 ROS2                          |
| ------------------- | ---------------------------------------------------------------- | ---------------------------------- |
| **Channel**   | 数据通道。发和收用同一个通道名就能通信                           | **Topic**                    |
| **Node**      | 节点。一个节点可以创建多个 reader/writer                         | **Node**                     |
| **Writer**    | 发布者。往一个话题写数据                                         | **Publisher**                |
| **Reader**    | 订阅者。订阅一个话题，需要绑一个回调函数                         | **Subscription**             |
| **Component** | 组件。**把 Node + Reader 打包**，靠 DAG 文件配置它订阅什么 | **Component / 生命周期节点** |

**注意 Component 和 Node 的区别（面试爱问）**：

- 用 **Node**：自己写代码创建 reader/writer，订阅哪个话题**写死在代码里**
- 用 **Component**：订阅哪个话题**写在 DAG 配置文件里**，改配置就行、不用改代码

## 3.5 每个函数/类/宏是干嘛的

### ① `air_middleware_common.h` —— 整个封装的"总开关"

```cpp
#define IMPL_NAMESPACE        apollo::cyber
#define WRITER_IMPL(MessageT) apollo::cyber::Writer<MessageT>
#define READER_IMPL(MessageT) apollo::cyber::Reader<MessageT>
#define NODE_IMPL             apollo::cyber::Node
#define RATE_IMPL             apollo::cyber::Rate
```

**这是整个设计的核心思想，20 行代码搞定了"可替换中间件"。**

**怎么工作的？** 别的地方写 `NODE_IMPL`，预处理器会把它替换成 `apollo::cyber::Node`。

**所以将来要换成别的中间件，只要改这个文件**：

```cpp
#define NODE_IMPL  MyOwnMiddleware::Node     // 改这一行就够了
```

**答话术**：

> "封装的核心是**用宏做实现替换**。所有上层代码只写抽象的 `NODE_IMPL`、`WRITER_IMPL`，不写具体的 `apollo::cyber::`。要换中间件只改这一个头文件。项目 README 里也写了'目前只封装了 Apollo CyberRT，后续将支持更多消息中间件'——这套设计就是为那个目标做的准备。"

### ② 三个包装类（Reader / Writer / Rate）

```cpp
template <typename MessageT>
class AirMiddlewareWriter {
 public:
  bool Write(const std::shared_ptr<MessageT>& msg_ptr) {
    return writer_impl_->Write(msg_ptr);      // 转手给真正的 cyber writer
  }
 private:
  std::shared_ptr<WRITER_IMPL(MessageT)> writer_impl_;
};
```

**看明白这个模式了吗？这叫「包装器」/「代理」：**

- 对外叫 `AirMiddlewareWriter`，对内其实是 `apollo::cyber::Writer`
- 方法名 `Write` 简单好记，内部转手给底层
- **业务代码只认识 `AirMiddlewareWriter`，不认识 `apollo::cyber`**

| 类                         | 对外方法         | 作用                           |
| -------------------------- | ---------------- | ------------------------------ |
| `AirMiddlewareWriter<T>` | `Write(msg)`   | 发数据                         |
| `AirMiddlewareReader<T>` | （构造时绑回调） | 收数据                         |
| `AirMiddlewareRate`      | `Sleep()`      | 按固定频率循环（控制循环节奏） |

**`AirMiddlewareRate` 是干嘛的？** 控制循环频率。比如"我要 10Hz 处理"，就 `Rate r(10.0); while(...) { 干活; r.Sleep(); }`。**没有它就得手写 `sleep_for` 算时间，容易累积漂移。**

### ③ `AirMiddlewareNode` —— 节点，工厂式创建

```cpp
class AirMiddlewareNode {
 public:
  explicit AirMiddlewareNode(const std::string& node_name);   // 用名字创建

  template <typename MessageT>
  std::shared_ptr<AirMiddlewareReader<MessageT>>
  CreateReader(const std::string& channel,
               std::function<void(const std::shared_ptr<const MessageT>&)> callback);

  template <typename MessageT>
  std::shared_ptr<AirMiddlewareWriter<MessageT>>
  CreateWriter(const std::string& channel);
};
```

| 方法                                   | 干什么         | 参数                                                                |
| -------------------------------------- | -------------- | ------------------------------------------------------------------- |
| 构造函数                               | 创建一个节点   | `node_name`：节点名（用于日志和调试）                             |
| `CreateReader<T>(channel, callback)` | 订阅一个话题   | `channel`：话题名；`callback`：**收到数据时调用哪个函数** |
| `CreateWriter<T>(channel)`           | 创建一个发布者 | `channel`：话题名                                                 |

**注意 `template <typename MessageT>` 是什么意思？**

这叫**模板方法**。`MessageT` 是"消息类型"的占位符。用的时候**必须显式指定**：

```cpp
auto writer = node->CreateWriter<CompressedImage>("/sensor/ipcamera/h264/xxx");
//                                ↑ 告诉它消息类型是 CompressedImage
```

**为什么用模板？** 因为不同话题传的数据类型不同。用模板就能"一套代码支持所有消息类型"，不用为每种类型写一个函数。

### ④ `ComponentAdapter` —— 业务开发真正要继承的基类

```cpp
template <typename M0 = NullType>
class ComponentAdapter {
 public:
  virtual bool Init() = 0;                              // 你要实现：初始化
  virtual bool Proc(const std::shared_ptr<const M0>& msg) = 0;  // 你要实现：收到数据怎么处理

  template <typename ProtoT>
  bool LoadConfig(ProtoT* config);                      // 直接用：读配置

  template <typename M>
  bool Send(const std::string& channel, const std::shared_ptr<M>& msg);  // 直接用：发数据

  void RegisterNode(std::shared_ptr<NODE_IMPL>& node_impl);      // 框架用
  void RegisterComponent(IMPL_NAMESPACE::Component<M0>* component); // 框架用
};
```

**这是给业务开发者用的"开发模板"。** 你只需要写两个函数：

| 方法                   | 谁来写               | 干什么                                   | 参数                               |
| ---------------------- | -------------------- | ---------------------------------------- | ---------------------------------- |
| `Init()`             | **业务开发者** | 初始化：读配置、创建设备、起线程         | 无                                 |
| `Proc(msg)`          | **业务开发者** | **收到一条消息时怎么处理**         | `msg`：收到的那条消息            |
| `LoadConfig(config)` | 框架提供             | 把配置文件反序列化到一个 protobuf 对象里 | `config`：输出的配置对象指针     |
| `Send(channel, msg)` | 框架提供             | 把数据发到某个话题上                     | `channel`：话题名；`msg`：数据 |

**`Send` 内部的实现有个聪明的地方（懒创建 + 缓存）：**

```cpp
bool Send(const std::string& channel, const std::shared_ptr<M>& msg) {
  static std::unordered_map<std::string, std::shared_ptr<AirMiddlewareWriter<M>>> writers_map_;
  std::lock_guard<std::mutex> lg(writers_mutex_);
  if (writers_map_.find(channel) == writers_map_.end()) {
    auto writer = node_->CreateWriter<M>(channel);   // 第一次用才创建
    writers_map_.insert(std::make_pair(channel, writer));
  }
  return writers_map_[channel]->Write(msg);          // 第二次直接用缓存的
}
```

**为什么这么做？**

> 因为 `Send` 是在**拉流线程里高频调用**的（每秒几十次）。如果每次都 `CreateWriter`，开销大且会重复创建资源。**第一次用的时候创建并缓存起来，后面直接查表拿来用。**

**答话术（能讲出"懒创建"是加分项）**：

> "`Send` 是热路径，每帧都要调。所以里面做了 writer 的懒创建 + 缓存：第一次发某个话题时创建 writer 并缓存，之后直接复用。避免高频创建对象。"

### ⑤ `cyberrt_component.h` 的两个宏 —— 最精妙的部分

```cpp
#define AIROS_COMPONENT_CLASS_NAME(class_name) class_name##Adapter

#define REGISTER_AIROS_COMPONENT_CLASS(class_name, datatype)              \
  class class_name : public apollo::cyber::Component<datatype> {          \
   public:                                                                \
    virtual bool Init() override {                                        \
      impl_.RegisterNode(node_);                                          \
      impl_.RegisterComponent(this);                                      \
      return impl_.Init();                                                \
    }                                                                     \
   protected:                                                             \
    AIROS_COMPONENT_CLASS_NAME(class_name) impl_;                         \
   private:                                                               \
    virtual bool Proc(const std::shared_ptr<datatype>& msg) override {    \
      return impl_.Proc(msg);                                             \
    }                                                                     \
  };                                                                      \
  CYBER_REGISTER_COMPONENT(class_name);
```

**这段宏是什么？——「桥接模式」的自动化生成。**

**背景**：CyberRT 要求你的组件必须继承 `apollo::cyber::Component<T>`，并实现它的 `Init()` 和 `Proc()`。

**问题**：如果业务开发者直接继承 `apollo::cyber::Component`，代码就被 CyberRT 绑死了。

**解法**：让**宏自动生成一个中间类**。

**展开来看**（以 `IpCameraComponent` 为例）：

```cpp
// ① 业务开发者手写的类（只依赖 airos::middleware，不依赖 cyber）
class IpCameraComponentAdapter : public airos::middleware::ComponentAdapter<IpCameraReceiveData> {
  bool Init() override;                                        // 业务逻辑
  bool Proc(const std::shared_ptr<const IpCameraReceiveData>& msg) override;  // 业务逻辑
};

// ② 宏自动生成的桥接类（依赖 cyber，但业务不用管）
class IpCameraComponent : public apollo::cyber::Component<IpCameraReceiveData> {
 public:
  virtual bool Init() override {
    impl_.RegisterNode(node_);        // 把 cyber 的 node 交给业务类
    impl_.RegisterComponent(this);    // 把 this 交给业务类
    return impl_.Init();              // 调用业务类的 Init
  }
 private:
  virtual bool Proc(const std::shared_ptr<IpCameraReceiveData>& msg) override {
    return impl_.Proc(msg);           // ★ 收到数据 → 转发给业务类
  }
 protected:
  IpCameraComponentAdapter impl_;     // ← 业务开发者写的那个类
};
CYBER_REGISTER_COMPONENT(IpCameraComponent);   // 注册进 cyber 框架
```

**看懂了吗？「桥接」就是：**

- 外层（`IpCameraComponent`）：长得完全符合 CyberRT 要求，CyberRT 认识它
- 内层（`IpCameraComponentAdapter`）：业务代码，只依赖 `airos::middleware`
- **`Proc` 一转发，数据就从 CyberRT 世界流进了业务世界**

**这就是为什么你在 `ipcamera_component.h` 里看到的是这样写的**：

```cpp
class AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)   // = IpCameraComponentAdapter
    : public airos::middleware::ComponentAdapter<...> {
  bool Init() override;
  bool Proc(const std::shared_ptr<const IpCameraReceiveData>& recv_data) override;
  ...
};

REGISTER_AIROS_COMPONENT_CLASS(IpCameraComponent, os::v2x::device::ipcamera::IpCameraReceiveData);
// ↑ 这一行自动生成了继承 cyber Component 的桥接类
```

**答话术（这段讲出来效果最好）**：

> "封装层最核心的是用**桥接模式**把业务代码和中间件隔开。业务开发者只继承 `ComponentAdapter`，写 `Init` 和 `Proc` 两个函数；宏会自动生成一个继承 CyberRT Component 的桥接类，把 CyberRT 的 `Init`/`Proc` 转发给业务类。
>
> 好处是业务代码里**看不到任何 `apollo::cyber` 的字样**。将来换中间件，业务代码不用改。"

### ⑥ 三个 demo —— 对应简历里"进程内与跨进程 pub-sub 通信示例"

| demo 文件                                           | 演示什么                                                  |
| --------------------------------------------------- | --------------------------------------------------------- |
| `node_demo.cc`                                    | **进程内**通信：一个进程里同时创建 reader 和 writer |
| `node_talker_demo.cc` + `node_listener_demo.cc` | **跨进程**通信：两个进程，一个发一个收              |
| `component_demo.cc` + `demo_cyberrt.dag`        | **组件方式**：靠 DAG 文件配置订阅关系               |

**进程内 vs 跨进程有什么区别？（面试可能问）**

|                  | 进程内                 | 跨进程                                    |
| ---------------- | ---------------------- | ----------------------------------------- |
| 数据怎么传       | 直接传指针（共享内存） | 需要**序列化**再通过网络/共享内存传 |
| 速度             | 快                     | 慢一些                                    |
| 对消息类型的要求 | 什么类型都行           | **必须是 protobuf**（因为要序列化） |

**注意 README 里那句**："如果是跨进程通信，消息类型需要为 protobuff 格式。"——**这是个很重要的知识点**。

### ⑦ DAG 文件 —— "调整链路无需修改代码"

```yaml
module_config {
    module_library: "/home/airos/os/lib/libipcamera_component.so"   # 加载哪个库
    components {
        class_name: "IpCameraComponent"                              # 哪个类
        config {
            name: "IpCameraComponent"
            readers {
                channel: "/v2x/device/ipcamera/receive_data"          # ★ 订阅哪个话题
            }
            config_file_path: ".../config.pb.txt"                     # 参数文件
        }
    }
}
```

**DAG 文件是什么？—— 系统的"接线图"。**

**它解决的问题**：

```
不用 DAG：订阅哪个话题写死在 C++ 代码里 → 改链路要改代码、重新编译、重新发布
用   DAG：订阅哪个话题写在配置文件里   → 改链路只改配置、重启进程
```

**答话术**：

> "DAG 是组件的接线图。它声明了要加载哪个库、实例化哪个类、订阅哪些话题、参数文件在哪。**把'谁订阅谁'从代码里挪到了配置里**——调整链路拓扑只改配置，不用改代码也不用重新编译。"

## 3.6 这条简历的面试话术（30 秒版）

> "智路OS 底层用 CyberRT 做通信。如果业务代码直接调 CyberRT 的 API，整个项目就被绑死在这个中间件上了。
>
> 我参与的是统一接口封装层：提供 Node / Reader / Writer / Component 这套与中间件无关的接口，业务开发只继承 `ComponentAdapter` 写两个函数——`Init` 和 `Proc`。
>
> 封装的核心手法有两个：一是**用宏做实现替换**，上层只写抽象的 `NODE_IMPL`，换中间件只改一个头文件；二是**桥接模式**，用宏自动生成一个继承 CyberRT Component 的桥接类，把 CyberRT 的回调转发给业务类。业务代码里看不到任何 CyberRT 的字样。
>
> 另外我写了进程内和跨进程的通信示例，用 DAG 文件声明组件订阅关系——**改链路拓扑只改配置，不用改代码**。"

---

# 第四节 · 简历第 4 条：设备与平台运维

## 4.1 简历原文

> 读 Tegra sysfs 采集 GPU 利用率/显存/温度并周期上报；配置信号机（GA/T 1743）通信工参表；优化部署工具前置检测，提升异构环境部署成功率。

## 4.2 这一条整体在干啥

**这条跟前三条性质不同：前三条是"数据怎么流"，这条是"机器活得好不好"。**

三个子任务：

| 子任务                        | 干什么                          | 为什么需要                                                                                 |
| ----------------------------- | ------------------------------- | ------------------------------------------------------------------------------------------ |
| **① GPU 状态采集**     | 读板子的 GPU 利用率、显存、温度 | 边缘盒子算力有限，跑深度学习模型。GPU 温度过高会降频、显存爆了会崩。**必须实时监控** |
| **② 信号机通信配置**   | 配好和路口信号机通信的参数      | 信号机是国标设备（GA/T 1743），要按标准的 IP/端口/协议配好才能拿到红绿灯数据               |
| **③ 部署工具前置检测** | 部署前先检查环境                | 不同现场硬件型号不一样，环境不满足就装，装到一半失败，排查成本高                           |

## 4.3 子任务①：GPU 采集 —— 每个函数是干嘛的

**文件**：`middleware/protocol/om_common/performence_utils.cc` → `getGpuInfo()`

```cpp
bool PerformenceUtils::getGpuInfo(GpuInfo& gpuInfo);
```

|                  | 内容                                                             |
| ---------------- | ---------------------------------------------------------------- |
| **干什么** | 采集 4 项 GPU 指标：**负载率、显存占用、显存占用率、温度** |
| **参数**   | `gpuInfo`：**输出参数**（引用传入，函数把结果填进去）    |
| **返回**   | 温度是否读取成功                                                 |

**它采了 4 个指标，每个的方法都不一样——这正是这条简历的价值所在：**

| 指标                 | 怎么采集                                                                                | 关键点                                                                                                |
| -------------------- | --------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| **负载率**     | 读文件`/sys/devices/gpu.0/load`                                                       | 值是千分之一，要`/1000*100` 转成百分比。**代码里试了两个路径**——因为不同 Tegra 型号路径不同 |
| **显存占用**   | 执行命令`cat /sys/kernel/debug/nvmap/iovmm/maps` 解析                                 | nvmap 是 Tegra 特有的显存管理器。**用 `popen` 执行命令**                                      |
| **显存占用率** | 调 CUDA API`cudaMemGetInfo(&free, &total)`                                            | 用占用量 ÷ 总量                                                                                      |
| **温度**       | 遍历`/sys/class/thermal/thermal_zone0~9/type`，找到写 `GPU` 的那个，读它的 `temp` | **不能硬编码 zone 号**——不同板子 GPU 在哪个 zone 不一样                                       |

**面试核心：为什么这段代码要写得这么"绕"？**

答话术：

> "Tegra 平台的 sysfs 路径**不是固定的**——不同 Jetson 型号、不同 JetPack 版本，GPU 的路径、thermal zone 编号都不一样。所以代码里做了三件事：
>
> 1. **多路径探测**：负载率试两个路径，哪个能打开用哪个
> 2. **不硬编码 zone 号**：遍历 thermal_zone0 到 9，读每个的 `type` 字段，找到内容包含 `GPU` 的那个，才用它的编号
> 3. **失败兜底**：所有指标初始化为 -1，采集失败就保持 -1，不要给出错误的 0
>
> 这个思路就叫**异构环境适配**——同一份代码在不同硬件上都能跑，而不是写死一个型号。"

**"周期上报"是怎么实现的？** 在 `DeviceMonitor` 里：

```cpp
class DeviceMonitor {
  std::unique_ptr<std::thread> ping_thread_;     // 探活线程
  std::unique_ptr<std::thread> upload_thread_;   // 上报线程
  const int PING_INTERNAL   = 2;                 // 2 秒探活一次
  const int UPLOAD_INTERNAL = 5;                 // 5 秒上报一次
  void SetPostCallBack(const PostCallBack& cb);  // 设置上报回调
};
```

**两个线程，两个周期**：

- 探活线程：每 2 秒 ping 一次所有设备（信号机、RSU），看活着没
- 上报线程：每 5 秒把状态打包，通过 MQTT 上报到云端（话题前缀 `upload/status/`）

**注意它用了和相机一样的模式**：`SetPostCallBack(cb)` —— 也是"回调向上抛数据"。**说明这是项目的通用范式。**

## 4.4 子任务②：信号机配置（GA/T 1743）

**GA/T 1743-2020 是什么？** 《道路交通信号控制机信息发布接口规范》——**国家标准**，规定了信号机怎么对外发布红绿灯信息。

**"通信工参表"是什么？** 就是和信号机通信的参数配置。代码里是这份 YAML：

```yaml
# device/traffic_light/gat_device/device.yaml
ip: 127.0.0.1        # 信号机的 IP 地址
port: 10023          # 信号机的接收端口
local_port: 10050    # 本机用来接收的端口
protocol: udp        # 通信协议，默认 UDP，支持 TCP
```

**别小看这 4 行，每一项都有讲究：**

| 参数           | 为什么需要                                                                                                    |
| -------------- | ------------------------------------------------------------------------------------------------------------- |
| `ip`         | 信号机在网络上是哪台设备                                                                                      |
| `port`       | 信号机在哪个端口发布数据                                                                                      |
| `local_port` | **本机要用哪个端口收**。UDP 通信必须两头都指定端口                                                      |
| `protocol`   | UDP 还是 TCP。**UDP 快但不可靠，TCP 可靠但有延迟**。默认选 UDP 是因为信号灯数据要实时，偶尔丢一帧没关系 |

**还有一个配套的"相位转换表"：**

```json
// device/traffic_light/phase_to_direction.json
{ "phase": [1, 2, 5, 6, 9, 10, 13, 14],
  "direction": [27, 26, 37, 36, 7, 6, 17, 16] }
```

**这是干什么的？** 把信号机自己的"相位编号"翻译成标准的方向编号。

**为什么要翻译？** 因为不同厂商信号机的相位编号方式不一样。**配置里用一张映射表把差异吸收掉——又是"配置化适配异构"的思路**（和 GPU 多路径、和插件化是同一个设计哲学）。

**答话术**：

> "信号机遵循国标 GA/T 1743。配置主要是通信工参——IP、信号机端口、本地接收端口、协议类型（默认 UDP 保证实时性）。另外还有一张相位映射表，把厂商各自的相位编号统一成标准方向编号。**这些都放在配置文件里而不是写死在代码里，所以不同厂商的信号机接入时不用改代码。**"

## 4.5 子任务③：部署工具前置检测

**这一条我在开源代码里没有找到对应实现**（可能是内部的部署工具，未包含在这份源码里）。

**如果面试被问到，诚实的回答方式是**：

> "部署前置检测这部分我参与的是使用和问题反馈，具体实现是在内部的部署工具里。它做的事是在部署前检查环境是否满足——比如系统版本、依赖库、硬件型号，不满足就提前报错，而不是装到一半失败。**这样做的好处是排查成本低很多：装到一半失败时，环境已经被改了一半，很难回滚也很难定位。**"

**⚠️ 不要编造细节。** 简历上写"优化部署工具前置检测"，你就说"参与优化"，讲清楚"为什么要做前置检测"这个思路就够了。**面试官更看重思路，编细节反而容易被追问问穿。**

## 4.6 这条简历的面试话术（30 秒版）

> "这条是我在平台运维侧的参与。三件事：
>
> 一是**硬件状态监控**。边缘盒子跑深度学习模型，必须盯着 GPU。我参与采集 GPU 利用率、显存、温度——有意思的是 Tegra 平台上这些路径不固定，不同 Jetson 型号、不同 JetPack 版本，GPU 路径和 thermal zone 编号都不同，所以做了多路径探测和遍历查找，不能硬编码。采集后每几秒周期上报到云端。
>
> 二是**信号机接入配置**。信号机是国标 GA/T 1743 设备，配好 IP、端口、协议这些通信参数，还有一张相位编号映射表——把各家信号机的编号差异用配置吸收掉。
>
> 三是**部署前置检测**，把环境问题提前暴露在部署之前，而不是装到一半失败。"

---

# 第五节 · 四条怎么串起来讲（面试自我介绍用）

## 5.1 一段完整的话术（约 60 秒）

> "我在智路OS 项目里参与的是**框架层和设备接入层**，简单说就是让路口的各种设备能接进来、让数据在系统内部流起来。
>
> 具体四块：
>
> 第一，**设备抽象和插件化**。路口相机厂商混着用，每家 SDK 都不一样。我参与定义了一套统一的设备接口，用注册工厂 + 注册宏让设备自注册，再用动态加载器扫目录加载。效果是新增一家厂商只要加一个文件加一行宏，框架代码不用动。
>
> 第二，**多路视频流接入**。一个路口 4 路相机，每路一个独立线程用 FFmpeg 拉 RTSP，用原子标志和互斥锁做线程安全，做成了双层循环——外层重连、内层读数据，因为路口相机经常掉线。还在循环里做了帧间隔统计，排查卡顿很有用。
>
> 第三，**通信中间件封装**。底层是 CyberRT，我参与做了一层统一接口，业务代码只继承 ComponentAdapter 写两个函数就行，看不到任何 CyberRT 的字样。核心手法是用宏做实现替换、用桥接模式隔离。同时写了进程内和跨进程的通信示例，用 DAG 声明链路拓扑，改链路不用改代码。
>
> 第四，**平台运维侧**，主要是采集 Tegra 的 GPU 状态上报云端，还有信号机的接入配置。
>
> 这个项目让我完整理解了**一个机器人/自动驾驶系统是怎么把设备、通信、调度组织起来的**——这也是我为什么想往这个方向走。"

## 5.2 四条之间的内在联系（一句话串起来）

```
第1条（设备抽象）  定义了"设备长什么样"      →  是第2条的容器
第2条（多路视频流）是设备的一个具体实现      →  数据在这里产生
第3条（通信封装）  把数据送进系统总线        →  是第1、2条的出口
第4条（平台运维）  保证上面三条跑得健康      →  是外围保障
```

**面试时如果被问"你在项目里做了那么多，它们之间什么关系？"**，就用这句话回答。

## 5.3 四条的风险与诚实边界（防被追问）

| 条目       | 你可能被追问              | 稳妥的回答                                                                  |
| ---------- | ------------------------- | --------------------------------------------------------------------------- |
| 设备抽象   | 注册宏具体怎么实现的？    | 用`##` 拼接类名生成唯一变量名，静态变量在 main 之前构造时把自己塞进工厂表 |
| 多路视频流 | FFmpeg 的 API 你熟悉吗？  | 熟悉三个核心调用：打开输入、找流信息、读帧。更深层的解码过滤我还在学        |
| 通信封装   | 你自己写过 Component 吗？ | 我参与了封装层的开发，也照着写通信示例跑通了链路                            |
| 平台运维   | 部署工具具体怎么改的？    | 前置检测是内部工具，我参与的是使用和反馈；思路是让环境问题提前暴露          |

**⚠️ 原则：技术细节可以讲深，但不要声称"我从零设计了这个系统"。** 说"参与开发"、"负责/参与某个模块"最稳。

---

# 附录 A · CyberRT ↔ ROS2 概念对照表

| CyberRT              | ROS2                      | 说明                         |
| -------------------- | ------------------------- | ---------------------------- |
| Channel              | Topic                     | 数据通道                     |
| Node                 | Node                      | 节点                         |
| Writer               | Publisher                 | 发布者                       |
| Reader               | Subscription              | 订阅者                       |
| Component            | Component / 生命周期节点  | 可配置加载的计算单元         |
| DAG 文件             | launch 文件               | 声明加载哪些组件、配什么参数 |
| `.proto` 消息      | `.msg` 消息             | 数据结构定义                 |
| CyberRT 用宏注册组件 | ROS2 用`rclcpp` 宏/继承 | 注册方式不同但目的相同       |

**面试可以说的**："CyberRT 和 ROS2 在通信模型上几乎一一对应。我因为看了 CyberRT 的封装层，学 ROS2 的时候概念迁移很快——Topic/Publisher/Subscription 这些概念是通的，主要新学的是 TF2、Service/Action 和生命周期节点的写法。"

---

# 附录 B · 术语速查

| 术语                 | 一句话解释                                                                       |
| -------------------- | -------------------------------------------------------------------------------- |
| **RTSP**       | 实时流传输协议。相机对外提供视频流的标准方式，地址形如`rtsp://ip:port/stream1` |
| **H.264**      | 一种视频压缩编码格式。分 I 帧（关键帧，完整）和 P/B 帧（差异帧，需依赖前面的帧） |
| **NALU**       | H.264 码流的基本单元，格式是`00 00 01 + 头部 + 数据`                           |
| **IDR / I 帧** | 关键帧，可以独立解码。NALU 类型编号 = 5                                          |
| **SEI**        | 补充增强信息，相机可以在里面塞额外数据（比如曝光时间戳）。NALU 类型编号 = 6      |
| **FFmpeg**     | 最常用的音视频处理库。用来拉流、解码                                             |
| **RTSP 拉流**  | 从相机主动把视频流读过来                                                         |
| **CyberRT**    | Apollo 开源的自动驾驶通信中间件                                                  |
| **DAG**        | 有向无环图。在这里指描述组件依赖和订阅关系的配置文件                             |
| **protobuf**   | Google 的数据序列化格式。`.proto` 是定义，编译后生成 C++ 类                    |
| **插件化**     | 把功能做成可以独立加载的`.so`，主体程序不用改                                  |
| **抽象基类**   | 只定义接口（纯虚函数），不实现，由子类各自实现                                   |
| **工厂模式**   | 用一张表 + 一个统一接口来创建对象，避免上层写 if-else 分支                       |
| **自注册**     | 利用"全局静态变量在 main 之前构造"这个特性，让插件自动把自己登记到工厂           |
| **dlopen**     | Linux 上运行时加载动态库的系统调用                                               |
| **sysfs**      | Linux 的一个虚拟文件系统（`/sys/...`），读文件就能拿到硬件信息                 |
| **Tegra**      | NVIDIA 的嵌入式芯片系列。Jetson AGX Orin 用的就是 Tegra                          |
| **MQTT**       | 一种轻量级的物联网通信协议，常用于设备和云端通信                                 |
| **GA/T 1743**  | 国标《道路交通信号控制机信息发布接口规范》                                       |
| **RSU**        | Road Side Unit，路侧单元。车路协同里负责无线通信的设备                           |
| **V2X**        | Vehicle to Everything，车与万物互联                                              |
| **桥接模式**   | 一种设计模式：把"抽象"和"实现"分开，让两者可以独立变化                           |
