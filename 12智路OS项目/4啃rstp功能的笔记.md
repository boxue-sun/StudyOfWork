1.读取配置文件

先读.dag 加载so文件 **一个 `.dag` = 一个进程**

而so文件又能注册对应的相机类

组件配置里面又写了驱动配置 和文件路径

> ```
> module_config {
>     module_library: "/home/airos/os/lib/libipcamera_component.so"   // ① 加载这个 so
>     components{
>         class_name: "IpCameraComponent"                            // ② 起这个类
>         config{
>             name: "IpCameraComponent"                              //    实例名
>             readers{
>                 channel: "/v2x/device/ipcamera/receive_data"       // ③ 订阅：接收控制指令
>             }
>             config_file_path: "/home/airos/os/conf/ipcamera/config.pb.txt"  // ④ 组件配置
>         }
>     }
> }
> ```


2.init初始化

	这一步进行读取配置conf: 如下所示有来源的rtsp的地址 还有输出的topic的地址 以及其他的设置时间 一同都被读取出来。

```
# IP Camera 配置文件
# 多路RTSP拉流地址配置
rtsp_urls:
  - "rtsp://root:root@172.20.65.193:8554/stream1"
  - "rtsp://root:root@172.20.65.193:8554/stream2"
  - "rtsp://root:root@172.20.65.193:8554/stream3"
  - "rtsp://root:root@172.20.65.193:8554/stream4"
# 输出话题配置（与rtsp_urls一一对应）
output_topics:
  - "/sensor/ipcamera/h264/172_20_65_193"
  - "/sensor/ipcamera/h264/172_20_65_194"
  - "/sensor/ipcamera/h264/172_20_65_195"
  - "/sensor/ipcamera/h264/172_20_65_196"
reconnect_interval: 5  # 重连间隔（秒）
buffer_size: 1024000   # 缓冲区大小
connection_timeout: 10 # 连接超时时间（秒）
enable_sei_timestamp: true  # 是否启用SEI时间戳解析
enable_h264_save: false  # 是否保存H.264文件到磁盘（默认关闭） 

```
