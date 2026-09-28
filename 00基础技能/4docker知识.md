
好，压缩成**基础版**，适合“用过但不多”的情况。重点放在：能答上来、不露怯、项目题能自圆其说。

---

## 一、Docker 基础必会题（先背这些）

**1. Docker 是什么？**
把应用和依赖打包成镜像，再用容器运行起来。核心是“一次构建，到处运行”。

**2. Docker 解决什么问题？**
环境不一致、依赖冲突、部署麻烦、新人搭环境慢。

**3. 镜像和容器有什么区别？**
镜像是只读模板；容器是镜像跑起来的实例。一个镜像可以起多个容器。

**4. 容器和虚拟机有什么区别？**
虚拟机有独立操作系统，重、启动慢；容器共享宿主机内核，轻、启动快。

**5. 什么是 Docker 仓库？**
存镜像的地方，比如 Docker Hub、Harbor、阿里云镜像仓库。

**6. Dockerfile 是什么？**
用来构建镜像的脚本文件，里面写基础镜像、复制文件、安装依赖、启动命令。

**7. Docker Compose 是什么？**
用一个 YAML 文件定义多个容器，比如应用 + MySQL + Redis，一条命令一起启动。

**8. 常用 Docker 命令有哪些？**
`docker pull` 拉镜像
`docker images` 看镜像
`docker build -t 名字:标签 .` 构建镜像
`docker run` 跑容器
`docker ps -a` 看容器
`docker logs` 看日志
`docker exec -it 容器 sh` 进容器
`docker stop/rm/rmi` 停止、删除容器、删除镜像

**9. `docker run -d -p 8080:80 --name web nginx` 什么意思？**
`-d` 后台运行，`-p 8080:80` 宿主机 8080 映射容器 80，`--name web` 容器名叫 web，镜像是 nginx。

**10. `-v` 是干什么的？**
挂载数据卷，让容器数据持久化，或者把宿主机目录挂进容器。容器删了，卷里的数据还在。

**11. `-e` 是干什么的？**
传环境变量，比如数据库地址、密码、时区。

**12. 容器删了数据会丢吗？**
容器可写层会丢；如果用 volume 挂载，数据不丢。

**13. `docker logs` 和 `docker exec` 区别？**
`logs` 看容器标准输出日志；`exec` 进容器里执行命令。

**14. `CMD` 和 `ENTRYPOINT` 基础区别？**
`ENTRYPOINT` 是固定入口，`CMD` 是默认参数。`docker run` 后面的参数会覆盖 `CMD`，一般不覆盖 `ENTRYPOINT`。

**15. `COPY` 和 `ADD` 区别？**
`COPY` 只复制文件；`ADD` 还能解压 tar、下载 URL。平时优先用 `COPY`。

**16. `.dockerignore` 有什么用？**
构建时忽略文件，比如 `.git`、`node_modules`、日志、密钥，减小构建上下文。

**17. 为什么镜像要分层？**
层可以复用和缓存，构建更快，存储和传输更省。

**18. 为什么不建议用 `latest` 标签上生产？**
`latest` 会变，不知道具体版本，回滚困难。最好用 Git commit 或版本号。

**19. Docker 网络默认是什么？**
默认 bridge。自定义 bridge 网络里，容器可以用容器名互相访问。

**20. `docker compose up -d` 和 `down` 区别？**
`up -d` 后台启动整套服务；`down` 停止并删除容器和网络，默认保留命名卷。

---

## 二、知道就行的加分题

**21. 多阶段构建是什么？**
Dockerfile 里多个 `FROM`，编译阶段和运行阶段分开，最终镜像只带运行需要的文件，体积更小。

**22. 容器里 PID 1 是什么？**
容器启动的主进程。它要能处理停止信号，否则 `docker stop` 可能等超时后强杀。

**23. `depends_on` 能保证依赖服务就绪吗？**
不能，只保证启动顺序。要配合 healthcheck 判断是否真的可用。

**24. 容器内存超了会怎样？**
可能被 OOM Killer 杀掉，退出码常见 137。

**25. Docker 和 Kubernetes 什么关系？**
Docker 负责打包和跑单机容器；K8s 负责跨多机编排、扩缩容、自愈、滚动更新。

---

## 三、预留题：你在项目中怎么用到 Docker 的？

这题最重要，也最容易被追问。**不要背通用答案，按你真实情况填。**

### 回答结构

1. 项目是做什么的，你负责什么
2. 为什么用 Docker
3. 具体怎么用
4. 解决了什么问题
5. 遇到什么问题，怎么处理
6. 如果用得不多，就诚实说用到哪，不展开 K8s

### 填空模板

> 在我参与的 **XX 项目** 中，我主要负责 **XX**。
> 我们用 Docker 主要是为了解决 **环境不一致 / 本地依赖多 / 部署麻烦** 的问题。
> 具体做法是：用 Dockerfile 把 **XX 服务** 打成镜像，基础镜像用 **XX**，暴露 **XX 端口**；
> 本地或测试环境用 Docker Compose 启动 **MySQL / Redis / 应用**；
> 部署时在服务器上执行 `docker run -d -p XX:XX -v XX:/XX --name XX 镜像`，或者推到镜像仓库再由 CI/CD 部署。
> 好处是 **新人一条命令就能起环境，测试和生产环境一致，部署回滚更方便**。
> 遇到的问题主要是 **端口冲突 / 数据卷权限 / 内存限制 / 时区 / 日志查看**，后来通过 **改端口映射 / 配 volume / 加环境变量 / 限制内存** 解决。
> 更深层的 K8s 编排我接触不多，但 Docker 基础构建和运行流程我是熟悉的。

### 如果你用得真的很少，可以这样答

> 我在项目里对 Docker 的使用不算深，主要集中在本地开发和测试环境。
> 比如用 Docker Compose 起 MySQL、Redis，应用本身有时本地跑，有时打成镜像在测试机跑。
> 我会写简单的 Dockerfile，常用命令像 build、run、logs、exec、ps 都没问题。
> 生产环境的 K8s 编排我没有深度参与，但如果需要，我可以很快上手。

### 最好准备一个最小例子

Dockerfile：

```dockerfile
FROM openjdk:17
WORKDIR /app
COPY target/app.jar app.jar
EXPOSE 8080
CMD ["java", "-jar", "app.jar"]
```

Compose：

```yaml
services:
  app:
    build: .
    ports:
      - "8080:8080"
  mysql:
    image: mysql:8
    environment:
      MYSQL_ROOT_PASSWORD: 123456
    volumes:
      - mysql-data:/var/lib/mysql

volumes:
  mysql-data:
```

面试官如果追问，你就说：

- 镜像怎么构建：`docker build -t myapp:1.0 .`
- 怎么跑：`docker run -d -p 8080:8080 --name myapp myapp:1.0`
- 数据怎么持久化：MySQL 用 volume 挂到 `/var/lib/mysql`
- 日志怎么看：`docker logs -f myapp`
- 怎么进容器：`docker exec -it myapp sh`

---

总结：你先把 **第一部分的 20 题** 背熟，再把 **项目题模板** 换成你自己的项目。这样面试基本够用。
如果你告诉我你的项目技术栈，比如 Java/Spring Boot、Python、Node、MySQL，我可以直接帮你把那道项目题填成你的版本。
