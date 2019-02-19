# Uboot_Smart210
U-Boot(2012.10) for Smart210(S5PV210) based on goni(S5PC1X0).

修改内容：
1. 基于goni开发板移植到Smart210开发板
2. 加入LED点灯模块便于调试
3. 加入BL1阶段串口打印内存信息模块便于调试
4. 移植Nand Flash驱动，便于保持环境变量
5. 移植DM9000网卡驱动，以便使用tftp下载内核到内存


使用步骤：
1. 执行 make s5p_goni_config 生产开发板相关配置文件
2. 执行 make 编译
3. PC插入SD卡，执行 ./burnSD.sh 脚本生成前U-Boot的前16K二进制文件，并将其与整个U-Boot烧录到SD卡中
4. 将烧录好的SD卡插入Smart210开发板, 开机
