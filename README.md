# magpie-dash

[![CI](https://github.com/dreamfarer-space/magpie-dash/actions/workflows/ci.yml/badge.svg)](https://github.com/dreamfarer-space/magpie-dash/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

**A terminal-style 480×272 touch dashboard for the magpie LLM gateway, running on an ESP32-S3 (Guition JC4827W543).**
Ten LVGL pages: which agent is working right now, quota left per provider, token usage and cost, and a live log of
routed requests. The screen talks to the gateway over one USB cable (a small Python bridge on the host, nothing to
configure) or over WiFi (on-device setup page).

一块 4.3 寸触摸屏，常驻桌边显示 magpie 网关的实时状态：哪个智能体在干活、各家额度还剩多少、今天烧了多少 token、每一次请求被路由到了哪里。

<p align="center">
  <img src="docs/demo.gif" alt="在真实板子上点一遍：主页 → 状态页 → Agent 列表与详情 → 额度 → 用量 → 路由日志 → 设置" width="800">
</p>
<p align="center"><a href="docs/demo.mp4">高清 MP4</a> · 画面是板子上逐帧抓下来的真实渲染（演示数据），镜头跟随每一次点击</p>

## 界面

终端风格：顶部一行 shell 提示符（`user@magpie:~/页面$`、闪烁光标、链路状态、时间），底部一行 htop 式功能键（`1 主页` … `6 设置`），
中间是带标题的线框窗格。字体是 **Ark Pixel 等宽**（半角 6 px 一格，全角 12 px），每一列都落在这张 80 列网格上，所以跨行、跨页都对得齐。
颜色只用来表达状态：绿 = 正常 / 有余量，琥珀 = 忙 / 偏紧，红 = 出错 / 快用完，外加各智能体的品牌色。

![十二个界面总览](docs/overview.png)

| 页 | 内容 |
|---|---|
| **主页** | 左：正在干活的智能体（头像、模型、状态、两档额度、今天 / 30 天调用、重置倒计时）；右：magpie 今天的调用、token、费用、会话、30 天 token 柱状图 |
| **状态页 ×3** | 斟酌中 / 编写代码 / 等你确认：最近一次请求的耗时、模型、额度消耗；头像下三个小条互相切换 |
| **Agent** | magpie 管理的全部智能体：名称 · 模型 · 状态（工作中 / 已完成 / 空闲 / 未使用）· 30 天调用；点一行看详情，可“跟随” |
| **额度** | 每个上游账号一个窗格，每个额度窗口一条 `■` 进度条 + 百分比 + 重置时间 |
| **用量** | 今天 / 7 天 / 30 天 / 全部：token（输入 / 输出 / 缓存）、请求与失败、费用与均价、缓存命中率、分时段柱状图 |
| **路由** | 最近一条的拓扑 `agent ──▶ magpie ──▶ 上游账号`，下面是可滚动的请求日志；点顶部一行展开成整页日志 |
| **设置** | 显示名称（自带键盘）、剩余 / 已用、刷新间隔、头像动效、隐藏金额、连接方式（USB / WiFi） |

| | | |
|---|---|---|
| ![主页](docs/screen-home.png) | ![Agent 列表](docs/screen-agents.png) | ![Agent 详情](docs/screen-agent-sheet.png) |
| ![额度](docs/screen-quota.png) | ![用量](docs/screen-usage.png) | ![路由](docs/screen-routes.png) |

**交互**

- 底部功能键在**手指按下的瞬间**换页（不等抬起）；按下到整页画完 51 ms（板上实测，见下文）。
- 主页左窗格 → 状态页，右窗格 → Agent 列表；点提示符（带着你的名字）→ 设置。
- 戳头像：跳一下并说一句它自己的台词（Claude Code “Clauding...”、Codex “> codex exec”……），连戳 5 次会抱怨。
- Agent 详情里的“跟随”把主页和状态页固定到这个智能体，10 分钟后自动回到“跟随最新请求”。
- 长按顶部提示符一栏进入两点触摸校准。

## 硬件

| 项 | 值 |
|---|---|
| 板子 | Guition **JC4827W543**（4.3" 480×272 IPS） |
| 芯片 | ESP32-S3-WROOM-1 N4R8：4 MB flash、8 MB PSRAM |
| 屏幕 | NV3041A，QSPI：CS45 / SCK47 / D0-21 / D1-48 / D2-40 / D3-39，背光 1，TE 0 |
| 触摸 | GT911，I²C：SCL4 / SDA8 / INT3 / RST38，地址 0x5D |
| 调试 | 板载 USB-Serial/JTAG（无 CH340），`CDCOnBoot=cdc`，921600 |

> GPIO0 接的是 LCD 的 TE 信号，**不是按键**：这块板没有能用的 BOOT 键，导航全靠触摸。

## 快速开始

工具链（CI 用的就是这一套，见 [`.github/workflows/ci.yml`](.github/workflows/ci.yml)）：arduino-cli、esp32 core **3.3.12**、
lvgl **8.3.7**、ArduinoJson **7.4.3**、GFX Library for Arduino **1.6.9**；把 [`firmware/lv_conf.h`](firmware/lv_conf.h) 放到 Arduino `libraries/` 目录下、和 `lvgl/` 同级。

```bash
bash flash.sh            # 只编译
bash flash.sh flash      # 编译 + 烧录 + 抓 10 秒串口日志
```

`flash.sh` 顶部写着本机的 arduino-cli / 数据目录路径，换机器改那几行即可。烧录时它会先停掉 USB 桥接的自启任务，烧完再拉起来。

### 连接 magpie：USB 一根线（默认）

屏插在电脑上，USB 同时供电和传数据；不开 WiFi、不开端口，电脑继续用原来的网络。电脑侧跑桥接（只在 127.0.0.1 读 magpie）：

```bash
py -3.11 tools/usb_bridge.py --port COM9                       # 前台跑
powershell -File tools/bridge_autostart.ps1 -Action install    # 开机自启（pythonw，无窗口）
powershell -File tools/bridge_autostart.ps1 -Action status
```

- 按 VID/PID `0x303a:0x1001` 找板子，COM 号变了也能连上；重复启动会被命名互斥量挡掉。
- 状态在 `build_logs/usb_bridge_status.json`，日志在 `build_logs/usb_bridge.log`。
- 协议是长度 + CRC32 帧（`!MG1 <H|R|C> <seq> <len> <crc>\n` + body），请求路径白名单只有只读的 dashboard 接口；帧外字节一律丢弃，串口噪声不会误触发调试指令。

### 连接 magpie：WiFi（可选）

设置 →「连接 magpie」→「WiFi 配网」：屏开热点 `magpie-dash-setup`（密码 `magpie12345`），手机连上打开 `http://192.168.4.1` 填 SSID / 密码 / 电脑 IP。两个坑：

1. **magpie 只监听 127.0.0.1**，先在电脑上跑转发 `python tools/gw_relay.py 3426 127.0.0.1 3425`，屏上填 `<电脑IP>:3426`。
2. **ESP32-S3 只有 2.4 GHz、不支持 802.1X 企业认证**，校园网这类连不上；用手机热点或普通路由器（`tools/start_hotspot.ps1` 可开 Windows 热点）。

入网是非阻塞的，开机后 UI 立刻可用，连上后数据自己出现；配置存 NVS，重烧固件不丢。

## 数据从哪来

| 请求 | 用到的字段 |
|---|---|
| `/v1/magpie/quotas` | `provider/name/plan/user`、`windows[].name/used/remaining/resetsAt` |
| `/v1/magpie/usage?period=` | `calls/errors/input/output/cache_read/cost`、`series[]` |
| `/v1/magpie/usage/requests?period=&limit=12` | `rows[]`（agent / provider / model / stop / token / 耗时 / 状态）、`total` |
| `/dash/agents`（桥接 / relay 本地应答） | 智能体列表：`id/name/model/calls/tokens/cost/today/last` |

- magpie 的 `period` 只认 `today/7d/30d/all`（传 `1d` 会静默回退成 30d）；用量页的档位同时作用于 `/usage` 和 `/usage/requests`。
- magpie 公开接口没有智能体列表，`/dash/agents` 由桥接自己拼：`~/.config/magpie/library.json` 的 `applied` + `applied.json` 的模型 + 30 天按天 `by.agent` 统计 + 最近请求里的最后调用时间。
- “斟酌中 / 编写代码 / 等你确认”这类会话级状态接口里没有，状态页用**最近一次请求**推导：耗时、模型、stop 原因、状态码。
- 头像是各智能体的真实 logo（LobeHub 图标集，`tools/agent_logos/*.svg`），`tools/gen_agents.py` 预渲染成 20 / 56 / 88 px 写进 `agent_sprites.h`，屏上 1:1 显示。logo 版权归各自厂商。

## 延迟：实测数据

所有数字都是在板子上用串口指令量出来的（`P` / `p` / `q` / `l`），不是估算。

**换页（触摸按下 → 整页上屏）**，串口 `P` 通过 LVGL 输入驱动按下各个功能键：

| | 旧界面 | 现在 |
|---|---|---|
| 按下 → 整页画完 | 抬起后 46 ms（加上手指停留，真实手指约 150 ms） | **51 ms 平均 / 57 ms 最差**，与按多久无关 |
| 换页重绘区域 | 480×272 整屏 | 480×226 页面带 + 两个功能键格子 |

做法：

- **按下即换页**：手指落下到抬起通常 80–150 ms，以前要等抬起才开始画。功能键栏不滚动，按下就是点击，所以可以提前。
- **只重画中间那一带**：提示符栏和功能键栏是全局共享的，换页时不动；页面本身只有 480×226。
- **减少昂贵的绘制**：去掉了额度圆环（arc）和缩放位图；分隔线从“110 个点的折线”（每段一次带遮罩的线段绘制）改成一个 1 px 矩形；进度条是一个 `■` 标签而不是 20 个对象；呼吸动画不再对没变的位移反复失效重绘。
- **连点合并**：150 ms 内的连续换页只画最后一页（串口 `b` 复现：4 连点只产生 1 次绘制）。
- **数据刷新不压在点击上**：手指在屏上时推迟数据重绘，最多 3 秒。

底层的显示与触摸调优（之前几轮测出来的，仍然有效）：

| 版本 | 整页重绘 | 其中 flush (QSPI) |
|---|---|---|
| 480×20 双缓冲，内部 RAM | 134–157 ms | 41 ms |
| 整屏缓冲放 PSRAM | 71–78 ms | 40 ms |
| + QSPI 40 MHz（80 MHz APB 精确二分频；请求 32 MHz 实际只有约 20 MHz） | 62–71 ms | 32 ms |
| + 72 行 × 2 内部 DMA RAM 双缓冲 + 异步刷新任务（渲染第 N+1 带时传第 N 带） | 34–40 ms | 与渲染重叠 |

- 条带数比缓冲放哪更重要：480×20 的缓冲让 LVGL 每次重绘把对象树走 14 遍。
- LVGL 堆放 PSRAM（对象树太大放不进内部 RAM），内部堆至少留给 WiFi 驱动。
- 触摸轮询与 LVGL 解耦：主循环、LVGL 读取回调、每条刷新带之间都会读 GT911（12 ms 限频），按下 / 抬起只锁存边沿，整屏绘制期间的点击不会丢。
- `LV_DISP_DEF_REFR_PERIOD` / `LV_INDEV_DEF_READ_PERIOD` 都是 15 ms（默认 30）。

### 串口日志绝不能阻塞

主机不读串口时（桥接停了、USB 挂起），CDC 发送环填满，`Serial.printf` 会**阻塞调用它的任务**，取数任务和主循环一起卡死，看起来像死机。
现在 `Serial.setTxTimeoutMs(0)`（发不出去就丢），周期性日志先看 `availableForWrite()`；逐事件日志默认关闭（串口 `V` 打开）。
回归测试 `tools/test_usb_disconnect.py`：断开 100 秒再重连，板子照常回话。

## 开发

```bash
python tools/gen_fonts.py          # 重建字体子集（Ark Pixel 等宽 10/12/24，1bpp）
python tools/check_glyphs.py       # 检查屏上每个字串的字形都在字体里（CI 也跑）
python tools/extract_sprites.py --write
python tools/gen_agents.py         # 智能体 logo -> agent_sprites.h（需要 resvg-py、pillow）
```

- 字体子集直接从固件源码里的字符串字面量收集，新加文案重跑 `gen_fonts.py` 即可；来自网关的运行时文字（账号名等）写进它的 `STRINGS`。
  Ark Pixel 12 px 里没有的字（例如“即”“热”）会被 `check_glyphs.py` 拦下，只能换个说法。
- CI（每个 PR）：固件用和上面完全相同的工具链编译，产物 `.bin` 作为 artifact；工具脚本编译检查、USB 协议单元测试、字形检查。
  `master` 受保护：必须走 PR、CI 通过、分支是最新的。详见 [CONTRIBUTING.md](CONTRIBUTING.md)。

### 把屏幕“抓”到电脑上

先停掉 USB 桥接（它占着串口）。`tools/board_shot.py` 在占用串口期间会替桥接应答板子的数据请求，所以链路一直在线：

```bash
py -3.11 tools/board_shot.py --demo --all --out shots          # 演示数据，十页全截
py -3.11 tools/board_shot.py --cmd 4 --cmd o --shot --out shots # 打开 Agent 详情后截当前屏
py -3.11 tools/board_shot.py --tap 120,258 --shot --out shots   # 真实点击后截图
py -3.11 tools/demo_reel.py capture && py -3.11 tools/demo_reel.py render   # 重新录 README 里的演示
```

帧缓冲按行带偏移和校验和发送（`OOOOO:<hex>*SS`），损坏的行会被重新请求，所以截图要么逐像素准确，要么报告不完整。

### 串口调试指令

USB 桥接在跑时，原始字节会被丢弃，指令要用 `C` 帧发（`usb_bridge.py --command`、`board_shot.py --cmd`）；`@x,y` 帧是一次真实点击。

| 键 | 作用 |
|---|---|
| `0`..`9` | 跳到第 N 页 |
| `m` / `r` | 载入演示数据 / 回到实时数据并立刻刷新 |
| `s` / `S` | 十页截图 / 只截当前屏（保留弹窗、气泡） |
| `P` | 换页延迟：按下功能键 → 整页上屏，12 次取平均和最差 |
| `p` / `l` / `q` | 每页整屏重绘耗时（flush / render 拆分）/ 每页空闲重绘负载 / 每 2 秒的主循环与刷新计时 |
| `A` / `Z` / `B` | 回归：点击命中测试（66 项）/ 设置页（6 项）/ Agent 详情（6 项），都经过真实的 LVGL 输入驱动 |
| `b` | 4 连点（验证合并） |
| `f` / `k` / `o` | 换下一个智能体头像 / 戳当前头像 / 打开第一行的详情 |
| `F` / `L` `X` / `y` `Y` | 循环切换用量档位 / 打开、关闭连接面板 / 打开、关闭键盘 |
| `G` / `H` | 路由日志向上 / 向下滚 |
| `T` / `c` `x` / `K` | 颜色 + 触摸方向测试图 / 打开、关闭校准画面 / 清掉 NVS 里的触摸校准 |
| `t` / `d` / `V` / `D` | 触摸原始点 / 对象树 / 逐事件日志开关 / 设置恢复默认 |

`A` / `Z` / `B` 证明的是 LVGL 命中和导航链路，不经过 GT911，实物触摸手感以真机为准。

### 颜色与触摸方向

- 颜色用驱动默认的 `MADCTL = 0xC0`（MX|MY|RGB）。红蓝看起来互换时先用 `T` 的四色块确认是面板通道顺序、字节序还是反显，不要去改素材。
- 触摸是 1:1 直通（`ax=1 bx=0 ay=1 by=0`）。NVS 里曾存过一份上下左右都镜像的校准，表现为“点了没反应”；`K` 清掉即可。

## 目录

```
firmware/magepie_dash/   Arduino 工程（ui.cpp 界面、gateway.cpp 取数、usb_link.cpp 协议、display_port.cpp 屏和触摸）
firmware/lv_conf.h       LVGL 配置，放到 Arduino libraries/ 下
tools/                   USB 桥接、relay、字体 / 精灵图生成、截图与录屏、回归测试
docs/                    README 用的截图和演示
.github/                 CI、PR 模板、dependabot
```

## License

[MIT](LICENSE)。智能体 logo 版权归各自厂商；Ark Pixel 字体使用 SIL OFL 1.1。
