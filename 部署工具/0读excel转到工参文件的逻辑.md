
让我先找到 Excel 读取和工参文件生成的相关代码。

让我继续查看工参配置的核心逻辑文件。

让我找到核心的"读取Excel并生成工参文件"的逻辑。

好的，我已经完整阅读了所有相关代码。下面给你一份通俗易懂的总结，可以直接贴到笔记里：

---

## 📖 Excel → 工参配置文件 的完整流程

### 一、整体思路（一句话版）

> 从 Excel 工参表的多个 Sheet 中按 IP 逐行读取数据，填入预定义的 JSON 模板中，最终为每台设备生成一个 `work_param_config.flag` 配置文件。

---

### 二、涉及的文件和角色

| 文件                                 | 角色                   | 通俗理解                                       |
| ------------------------------------ | ---------------------- | ---------------------------------------------- |
| `work_param_column.py`             | **列名定义**     | 定义了每个 Sheet 有哪些列（枚举）              |
| `work_param_mapping.py`            | **翻译字典**     | Excel 中文列名 → JSON 英文字段名的映射关系    |
| `work_param_struct_new.py`         | **JSON 模板**    | 一个空的配置文件骨架（全是默认值）             |
| `get_work_param.py`                | **读取 Excel**   | 把 Excel 各 Sheet 读到内存（pandas DataFrame） |
| `create_work_param_config_file.py` | **生成配置文件** | 从内存取数据，填到 JSON 模板里，写成文件       |

---

### 三、流程图

```
┌──────────────────────────────────────────────────────────────┐
│                    第1步：读取 Excel                          │
│                                                              │
│  工参表.xlsx                                                 │
│    ├── MEC工参表      → g_all_mec_data        (MEC设备信息)   │
│    ├── 传感工参表     → g_all_sensor_data      (雷达/相机)    │
│    ├── 信控工参表     → g_all_ccindex_data     (信号控制)     │
│    ├── 信号机工参表   → g_all_trafficlight_data(可选)         │
│    └── 信控路网       → g_all_ccindex_road_info_data(可选)    │
│                                                              │
│  每个Sheet读取后：过滤掉"MEC-IP=default"的行，空值填充       │
└──────────────────────────┬───────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│                第2步：生成 JSON 骨架文件                      │
│                                                              │
│  遍历 MEC 表中每个 IP，为每个 IP 创建目录：                   │
│    local/{路口编号}/{设备类型}_{IP}/work_param_config.flag   │
│                                                              │
│  先把 work_param_struct_new 的模板（全默认值）写成 JSON 文件  │
└──────────────────────────┬───────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│            第3步：用 Excel 数据填充 JSON 各字段               │
│                                                              │
│  对每个 IP 的 JSON 文件，依次填入 6 大块参数：                │
│                                                              │
│  ① write_mec_para       → 填 MEC 基础参数 + 协议参数        │
│  ② write_sensor_para    → 填 传感器（雷达/相机）参数         │
│  ③ write_ccindex_para   → 填 信控（车道/流向）参数           │
│  ④ write_ptp_and_ftp_para → 填 PTP/FTP 参数                │
│  ⑤ write_camera_event_para → 填 相机违法事件参数            │
│  ⑥ write_Trafficlight_para → 填 信号机参数（可选）           │
│                                                              │
│  填完后保存 JSON 文件                                        │
└──────────────────────────────────────────────────────────────┘
```

---

### 四、核心机制详解

#### 1️⃣ Excel 是怎么读的？

```python
df = pd.read_excel(工参表路径, sheet_name="MEC工参表", engine="openpyxl")
```

- 用 pandas 的 `read_excel` 读取，每个 Sheet 变成一个 DataFrame（类似表格）
- 读完后做三件事：
  - **过滤**：删掉 `MEC-IP == "default"` 的无效行
  - **补空**：NaN 值按类型填充（整数填0，浮点填0.0，字符串填""）
  - **存全局变量**：如 `g_all_mec_data` 供后续使用

#### 2️⃣ "翻译字典"是怎么工作的？

Excel 里是中文列名，JSON 里要英文字段名，靠 `work_param_mapping.py` 做翻译：

```
Excel 列名（中文）          →    JSON 字段名（英文）
─────────────────────────────────────────────────────
"路口编号"                  →    "B_routeId"
"MEC-IP"                   →    "G_mecIp"
"感知云控Ip"                →    "O_senseCloudIp"
"mqtt用户名"                →    "V_maintenanceMqttUsername"
```

#### 3️⃣ JSON 模板长什么样？

`work_param_struct_new.py` 定义了一个完整的 JSON 骨架：

```json
{
    "A_hasedInit": false,
    "B_mecDeviceWorkParam": {
        "A_serialNumber": 1,
        "B_routeId": "",          ← 待填：路口编号
        "G_mecIp": "",            ← 待填：MEC-IP
        "O_senseCloudIp": "",     ← 待填：感知云控IP
        ...共50多个字段
    },
    "C_sensorDeviceNum": 0,
    "D_sensorDeviceWorkParamList": [ ... 12个传感器槽位 ... ],
    "E_ccInexWorkParam": {
        "A_laneList": [ ... 10个车道 ... ],
        "B_flowList": [ ... 10个流向 ... ]
    },
    "F_ptpLogParam": { ... },
    "G_cameraEventParam": { ... },
    "H_trafficLightParam": { ... }
}
```

#### 4️⃣ 最终生成的文件在哪？

```
local/
├── SY-200/                          ← 路口编号
│   ├── MEC_a_172_20_65_193/         ← 设备类型_IP（点号变下划线）
│   │   └── work_param_config.flag   ← 最终的 JSON 配置文件
│   └── MEC_b_172_20_65_129/
│       └── work_param_config.flag
├── SY-201/
│   └── ...
```

---

### 五、一句话总结每个文件的作用

| 文件                                 | 一句话                                                      |
| ------------------------------------ | ----------------------------------------------------------- |
| `get_work_param.py`                | 读 Excel 各 Sheet 到内存，做清洗和容错                      |
| `create_work_param_config_file.py` | 先为每个 IP 生成 JSON 骨架，再按映射关系把 Excel 数据填进去 |
| `work_param_column.py`             | 定义 Excel 每个 Sheet 的列名枚举                            |
| `work_param_mapping.py`            | Excel 中文列名 → JSON 英文字段名的翻译对照表               |
| `work_param_struct_new.py`         | JSON 配置文件的空模板（骨架）                               |

---

### 六、数据流向一句话

```
Excel中文列 → pandas读取 → 过滤+补空 → 存全局DataFrame → 遍历每个IP 
→ 复制JSON模板 → 按mapping字典逐个填字段 → 保存为 .flag 文件
```

---

这就是整个"读 Excel → 转工参文件"的完整逻辑，可以直接贴到你的笔记里！
