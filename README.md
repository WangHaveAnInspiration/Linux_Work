# 并发游戏服务器  2023.11-2024.01  项目负责人
- 通过Linux平台的系统调用实现服务器的开发，游戏名"五子棋对战"，完成服务器端规则与游戏数据的初始化。
- 采用epoll技术，实现Linux内核的IO多路复用、进程的同步和互斥，测试服务器代码的可用性与高并发操作。
- 开启多个终端页面使用telnet分别拨入服务器，使得高并发的服务器处理机制得以实现。
- 建立连接模拟客户端，实现不同客户端异步操作，玩家可交替操作，服务器与客户端数据基于命令行的文本交互，并设计算法执行下棋逻辑。
