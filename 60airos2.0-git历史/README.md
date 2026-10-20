# airos2.0 Git 历史档案

从 **193 机（`miivii-tegra`）`/work/sunboxue/airos2.0`** 导出的完整提交历史，用于事后翻看「别人是怎么写这块代码的」。

- 导出时间：2026-10-20 11:00
- 全量提交：**1040** 条（`--all` 去重，含 11 个分支引用）
- 时间跨度：**2024-02-20 ~ 2026-09-16**
- 作者：12 个（实际约 6 个人，重名见下）
- 标签：42 个

---

## 一、文件怎么用

| 文件 | 内容 | 什么时候看 |
|---|---|---|
| `00-仓库总览.md` | 分支表、作者提交数、全部标签 | 先看这个建立全局印象 |
| `01-全部提交日志.md` | 1040 条提交，一行一条（日期 / hash / 作者 / 标题） | 扫时间线、找某天改了什么 |
| `02-按作者分组.md` | 按人分组的提交列表 | 跟某个人的改动脉络 |
| `03-提交文件变更.tsv` | 每条提交改了哪些文件、增删多少行（5312 行记录） | **Excel 打开**，按人/模块/文件筛 |
| `04-目录改动热点.tsv` | 各目录被改的总次数与增删行数 | 判断哪些模块是主战场 |
| `05-改动最大的提交.tsv` | 按增删行数排前 120 的提交 | 找大重构、大功能落地 |
| `06-作者模块矩阵.md` | 作者 × 模块的改动分布 | **想知道该向谁学哪块，先看这个** |
| `patches/*.patch` | 按作者切分的代码 diff | 真正看代码怎么写的地方 |
| `_全量patch.patch` | 上面所有 patch 的合并版（32MB） | 全局 grep 用 |

> `03` / `04` / `05` 是制表符分隔，Excel 直接双击打开即可；`06` 是 Markdown 表格，编辑器里看。

---

## 二、人和模块的对应关系（偷师地图）

| 谁 | 主战场 | 说明 |
|---|---|---|
| **zhangenwei**（594 条，2681 次改文件） | `middleware/protocol`（1740 次） | 协议层绝对主力。设备接入、消息编解码基本都在这 |
| **changxuhui**（110 条 + 常序辉 67 条 = 177 条，1523 次） | `app/modules`(321) / `middleware/device_service`(266) / `air_service/modules`(207) | 业务层+设备服务，覆盖面最广 |
| **hezhiyong**（`LAPTOP-0HQ2NSHK\hezhiyong` 117 + `贺志勇\hezhiyong` 81 + `贺志勇` 25 = 223 条） | `middleware/protocol`(258) / `device_service`(83) | 协议层第二人，和 zhangenwei 有重叠 |
| **常序辉** | `package/package`(73) / `package/template`(72) | 打包与模板，和 changxuhui 是同一人 |
| **libo**（9 条） | `device_service`(19) / `protocol`(8) | 提交少但都是近期收尾 |
| 其他人 | `test_tools`、`base/common` 等 | tianpuxuan/tianchuan 偏测试工具 |

**给你的建议**：你简历里写的是「设备接入与通信中间件」，对应的正是 `middleware/device_service` + `middleware/protocol`。所以优先看 **zhangenwei** 和 **hezhiyong** 的 patch；`app/modules`（交通参数那类业务）看 **changxuhui**。

> ⚠️ 同一个人有多个身份：`hezhiyong` 拆成 3 个（换机器/改 git 配置导致），`changxuhui` 和 `常序辉` 是同一人。统计时记得合并。

---

## 三、patch 怎么看

`patches/zhangenwei_594条.patch` 这种文件，格式是：

```
@@@C	作者	完整commit hash	日期	提交标题
diff --git a/xxx b/xxx
...（标准 git diff）
```

**这不是能直接 `git am` 的补丁**，是给人读的 diff 记录。要点：

1. 编辑器直接打开就能读（VS Code 会把 `diff --git` 之后的部分按代码高亮）。
2. 想看某个文件的历史改动，在 patch 里搜文件名，例如：
   ```
   grep -n "device_service" patches/zhangenwei_594条.patch | head -50
   ```
   或者直接搜函数名、类名。
3. 想只看某个提交，用 `01-全部提交日志.md` 里的短 hash 在 patch 里搜（patch 里是完整 40 位 hash，搜前 8 位即可）。

---

## 四、想继续深挖怎么办

档案是快照，要查更细的（比如某文件逐行 blame、某版本的完整代码），直接上 193 机：

```bash
ssh 172.20.65.193
cd /work/sunboxue/airos2.0

# 某文件的所有历史改动
git log --oneline --follow -- app/modules/xxx/yyy.cpp

# 某一行是谁在哪个提交里写的
git blame -L 100,140 app/modules/xxx/yyy.cpp

# 某两个版本之间到底改了什么
git diff airos-v3.3-v2.3.3-shunyi-ccindex-new_algorithm-debug-202506061706 \
         airos-v3.3-v2.5.9-shunyi-debug-202511241657 -- middleware/

# 某次提交的完整内容
git show <hash>
```

> 别人（hzy / cxh / lht / libo）目录下的仓库属于各自账号，`git` 会报 `dubious ownership`。临时绕过：
> `git -c safe.directory=/work/hzy/airos2.0_optimization -C /work/hzy/airos2.0_optimization log --oneline | head`

---

## 五、⚠️ 安全提醒

这个仓库的 remote URL 里存着**明文账号密码**：

```
http://sunboxue:******@10.255.10.29/cv2x_application/airos2.0.git
```

本档案所有文件都已过滤，没有写进去。但建议你自己改掉：

```bash
cd /work/sunboxue/airos2.0
git remote set-url origin ssh://git@10.255.10.29/cv2x_application/airos2.0.git
# 或者换成 token 方式
```

---

## 六、本次导出做了什么（可复现）

- 远程脚本：`.workbuddy/export_git.sh`（纯只读：`git log` / `rev-list` / `numstat`，未 fetch、未切分支）
- 本地拆分脚本：`.workbuddy/split_patch.py`
- 导出目录：193 机 `/tmp/airos_git_export/`
