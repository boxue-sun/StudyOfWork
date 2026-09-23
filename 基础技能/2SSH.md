
好的，给你整理一份 SSH 基础面试 QA，覆盖高频考点：

---

## SSH 基础面试 QA

### Q1：SSH 是什么？

SSH（Secure Shell）是一种**加密的网络协议**，用于安全地远程登录和管理另一台计算机。它通过加密传输所有数据，防止被窃听或篡改。

默认端口：**22**

---

### Q2：SSH 和 Telnet 有什么区别？

|            | SSH      | Telnet                                   |
| ---------- | -------- | ---------------------------------------- |
| 安全性     | 加密传输 | **明文传输**（密码都能被抓包看到） |
| 默认端口   | 22       | 23                                       |
| 现在用哪个 | ✅ 主流  | ❌ 基本淘汰                              |

---

### Q3：怎么通过 SSH 连接远程服务器？

```bash
# 基本连接（默认22端口）
ssh root@192.168.1.100

# 指定端口
ssh root@192.168.1.100 -p 2222

# 指定密钥文件连接
ssh -i ~/.ssh/id_rsa root@192.168.1.100
```

---

### Q4：SSH 的两种认证方式？

**1. 密码认证**

- 输入用户名密码登录
- 简单但不安全（密码可能被暴力破解）

**2. 密钥认证**（推荐）

- 生成一对密钥：公钥（放服务器）+ 私钥（留本地）
- 登录时自动匹配，不需要输密码
- 更安全，防暴力破解

```bash
# 生成密钥对
ssh-keygen -t rsa

# 把公钥传到服务器
ssh-copy-id root@192.168.1.100
```

---

### Q5：怎么通过 SSH 传文件？

```bash
# 上传：本地 → 远程
scp local_file.txt root@192.168.1.100:/remote/dir/

# 下载：远程 → 本地
scp root@192.168.1.100:/remote/file.txt ./local_dir/

# 传整个目录（加 -r）
scp -r local_dir/ root@192.168.1.100:/remote/dir/
```

---

### Q6：SFTP 和 SCP 有什么区别？

|          | SCP         | SFTP                                           |
| -------- | ----------- | ---------------------------------------------- |
| 全称     | Secure Copy | SSH File Transfer Protocol                     |
| 功能     | 只能传文件  | 可以传文件 +**浏览/删除/重命名**远程文件 |
| 底层     | 走 SSH 通道 | 走 SSH 通道的独立协议                          |
| 使用场景 | 简单传文件  | 需要交互式文件管理                             |

```bash
# 进入 SFTP 交互模式
sftp root@192.168.1.100

# 然后在里面可以：
# put 本地文件    → 上传
# get 远程文件    → 下载
# ls / cd / pwd  → 浏览远程目录
```

---

### Q7：SSH 免密登录的原理是什么？（高频！）

```
1. 本地生成密钥对：ssh-keygen
   → 得到 id_rsa（私钥，自己留着）和 id_rsa.pub（公钥）

2. 把公钥放到服务器的 ~/.ssh/authorized_keys 文件里

3. 连接时：
   服务器用公钥加密一段随机字符串发给客户端
   客户端用私钥解密，发回给服务器
   服务器验证正确 → 登录成功

整个过程不需要输密码，而且私钥永远不会离开本地
```

---

### Q8：SSH 连接慢/超时，怎么排查？

```
1. 检查网络是否通：ping 192.168.1.100
2. 检查端口是否开放：telnet 192.168.1.100 22
3. 检查 SSH 服务是否运行：systemctl status sshd
4. 检查防火墙是否放行：iptables -L | grep 22
5. 加 -v 参数看详细日志：ssh -v root@192.168.1.100
```

---

### Q9：Python 中怎么用 SSH？（结合你的项目经验）

用 **paramiko** 库：

```python
import paramiko

client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
client.connect("192.168.1.100", port=22, username="root", password="xxx")

# 执行命令
stdin, stdout, stderr = client.exec_command("ls -la")
print(stdout.read().decode())

# SFTP 上传
sftp = client.open_sftp()
sftp.put("本地文件", "/远程路径/文件")

client.close()
```

---

### Q10：什么是 SSH 端口转发/隧道？

把一个端口"借"到另一台机器上用，常用于访问内网服务：

```bash
# 本地端口转发：把远程的 3306 端口映射到本地 13306
ssh -L 13306:localhost:3306 root@192.168.1.100

# 之后访问本地 13306 就等于访问远程的 3306（比如 MySQL）
```

---

### 速记卡片

| 知识点        | 记住这个                                      |
| ------------- | --------------------------------------------- |
| SSH 默认端口  | **22**                                  |
| SCP 传文件    | `scp 源 目标`                               |
| 密钥认证      | 公钥放服务器，私钥留本地                      |
| 免密登录      | `ssh-keygen` + `ssh-copy-id`              |
| Python SSH 库 | **paramiko**                            |
| SFTP vs SCP   | SFTP 功能更多，SCP 更简单                     |
| 排查连接问题  | ping → telnet端口 → systemctl状态 → ssh -v |

这些够应付大部分 SSH 相关的面试问题了，结合你项目中用 paramiko 做批量部署的经验，面试官问起来会很有说服力。
