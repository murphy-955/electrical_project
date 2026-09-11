# 电工电子实训 Arduino 项目

本项目包含三个实训日的 Arduino 练习代码 (`.c` 草图),使用 **arduino-cli** 进行命令行编译/烧录,并支持在 **CLion**
中获得代码补全和一键编译烧录。

## 目录结构

```
├── day1/  day2/  day3/     # 每日实训内容
│   ├── p*/                 # 各练习的 Arduino 草图(.c)
│   └── 要求.md             # 练习要求
├── .tools/
│   ├── arduino-cli/        # arduino-cli 及 AVR 核心(项目内独立安装)
│   └── compile.sh          # 编译/烧录脚本
└── CMakeLists.txt          # CLion 配置(代码补全 + 编译烧录目标)
```

## 环境说明

- **arduino-cli 1.3.1** + **arduino:avr 1.8.8** 核心 (支持 Uno/Nano 等 AVR 开发板),全部安装在项目 `.tools/` 目录内,不影响系统环境。
- 默认目标板:`arduino:avr:uno`,默认串口:`/dev/ttyUSB0`。

## 从零配置 .tools 环境

`.tools/` 目录已被 `.gitignore` 忽略,**克隆本仓库后需要按以下步骤重新配置**。所有内容都安装在项目目录内,不会污染系统环境。

### 1. 下载 arduino-cli

前往 [arduino-cli Releases](https://github.com/arduino/arduino-cli/releases) 下载 Linux 64bit 版本,解压后将可执行文件放入 `.tools/arduino-cli/`:

```bash
mkdir -p .tools/arduino-cli
cd .tools/arduino-cli
# 以 1.3.1 为例
wget https://github.com/arduino/arduino-cli/releases/download/v1.3.1/arduino-cli_1.3.1_Linux_64bit.tar.gz
tar -xzf arduino-cli_1.3.1_Linux_64bit.tar.gz
rm arduino-cli_1.3.1_Linux_64bit.tar.gz
cd ../..
```

### 2. 创建配置文件

新建 `.tools/arduino-cli/arduino-cli.yaml`,把核心、库等数据全部指向项目内目录 (**必须写绝对路径**):

```yaml
board_manager:
    additional_urls: []
directories:
    data: <项目绝对路径>/.tools/arduino-cli/data
    downloads: <项目绝对路径>/.tools/arduino-cli/data/staging
    user: <项目绝对路径>/.tools/arduino-cli/user
```

> ⚠️ 把 `<项目绝对路径>` 替换为实际路径,例如 `/home/你的用户名/.../electrical-project`。  
> **项目目录移动/重命名后必须同步修改此文件**,否则 arduino-cli 找不到已安装的核心。

### 3. 安装 AVR 核心

```bash
CLI=".tools/arduino-cli/arduino-cli --config-file .tools/arduino-cli/arduino-cli.yaml"

# 更新包索引(需要联网)
$CLI core update-index

# 安装 AVR 核心(含 avr-gcc、avrdude 等工具链,约 380MB)
$CLI core install arduino:avr
```

安装完成后可用 `$CLI core list` 确认显示 `arduino:avr 1.8.8`。

### 4. 安装依赖库

部分练习使用了第三方库,安装到项目内目录:

```bash
$CLI lib install "Adafruit NeoPixel"      # 灯环/灯带控制 (day5)
$CLI lib install "LiquidCrystal"          # LCD 显示屏
$CLI lib install "LiquidCrystal I2C"      # I2C 接口 LCD 显示屏
```

库会安装到 `.tools/arduino-cli/user/libraries/`,`CMakeLists.txt` 中的头文件路径已指向该目录。

### 5. 编译/烧录脚本

`.tools/compile.sh` 已随仓库提交,无需配置。它的作用是:把 `.c` 草图复制到临时目录并改名为 `.ino`(arduino-cli 只认 `.ino`),再调用上面的配置文件编译,结束后自动清理临时文件。

### 6. 串口权限 (仅首次需要)

Linux 下烧录/监视串口需要 `dialout` 组权限:

```bash
sudo usermod -aG dialout $USER
# 重新登录后生效
```

### 7. 验证安装

接上 Arduino 板子,运行:

```bash
# 确认能看到板子
$CLI board list

# 编译一个已有草图做冒烟测试
.tools/compile.sh day1/p1/led_sos.c
```

编译输出 `Sketch uses ... bytes` 即表示环境配置成功。

## 命令行使用

```bash
# 编译检查任意草图
.tools/compile.sh day3/p8/led_8_control.c

# 编译并烧录到开发板
.tools/compile.sh --upload day3/p8/led_8_control.c

# 指定其他板型(如 Nano)
.tools/compile.sh day1/p1/led_sos.c arduino:avr:nano

# 指定其他串口
ARDUINO_PORT=/dev/ttyUSB1 .tools/compile.sh --upload day3/p8/led_8_control.c
```

> 脚本会自动把 `.c` 复制为临时 `.ino` 再编译,源文件不会被修改。

## CLion 配置与使用

### 1. 打开项目

- CLion → `File` → `Open` → 选择项目根目录 `electrical-project` → 信任项目
- CLion 会自动识别 `CMakeLists.txt` 并加载,等待右下角索引完成

### 2. 代码补全

打开任意 `.c` 草图,`pinMode`、`digitalWrite`、`Arduino.h` 等都有补全和跳转 (已在 `CMakeLists.txt` 中配置好 AVR 核心头文件路径和
`ARDUINO_AVR_UNO` 等宏定义)。

### 3. 编译 / 烧录

右上角运行配置下拉框中,每个草图都有两个目标,例如:

- `compile_day3_p8_led_8_control` — 只编译检查
- `upload_day3_p8_led_light_memory` — 编译并烧录到板子

选中后点绿色 ▶️ 即可运行,输出显示在下方面板。

> ⚠️ **不要点 Build All**——`sketch_index` 目标只是为了让 CLion 索引代码,不能用本机 gcc 构建。只使用 `compile_*` /
> `upload_*` 目标。

### 4. 串口监视器

在 CLion 底部 Terminal 里运行 (以 9600 波特率为例):

```bash
.tools/arduino-cli/arduino-cli --config-file .tools/arduino-cli/arduino-cli.yaml \
  monitor -p /dev/ttyUSB0 -c baudrate=9600
```

按 `Ctrl+C` 退出。

## 常见问题

| 问题                             | 解决方法                                                                                      |
|----------------------------------|-----------------------------------------------------------------------------------------------|
| 串口权限不足 (Permission denied) | 把当前用户加入 `dialout` 组:`sudo usermod -aG dialout $USER`,重新登录生效                     |
| 板子换了串口(如 ttyUSB1)         | 修改 `CMakeLists.txt` 中的 `ARDUINO_PORT`,或用环境变量指定(见上文)                            |
| 查看当前连接的板子               | `.tools/arduino-cli/arduino-cli --config-file .tools/arduino-cli/arduino-cli.yaml board list` |
| 板子只运行最后烧录的程序         | 开发板一次只能存一个程序,重新 `upload_*` 即可切换                                             |
