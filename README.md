# magpie-dash — ESP32-S3 十页仪表盘

**A 480×272 touch dashboard for the [magpie](https://github.com/) LLM gateway, running on an ESP32-S3 (Guition JC4827W543).**
Ten LVGL pages, animated agent avatars, live quota/usage/routing data. The screen talks to the gateway either over
one USB cable (a small Python bridge on the host — nothing to configure) or over WiFi (on-device setup page).
Firmware and host tooling are MIT licensed; the Chinese docs below go into the details, the pitfalls and the measured numbers.

按 `real/HUrv4sEagAADfnF.png` 里的九页像素设计，复刻成跑在 **JC4827W543（Guition 4.3" 480×272 / ESP32-S3-WROOM-1 N4R8）** 上的固件，数据实时取自 magpie 网关。

```
magepie-esp32/
├── real/                     原始设计图 + 切好的九页（pages/page1..9.png，480×272 原生）
├── tools/                    生成器与工具（字体子集、精灵图、截图、转发、热点、USB 桥接）
├── firmware/magepie_dash/    实际烧录的 Arduino 工程
├── shots/                    从板子抓回来的真实渲染（本地生成，不入库）
├── flash.sh                  编译 / 烧录 / 抓图
├── LICENSE                   MIT
└── README.md
```

## 硬件（从板子实测读出，不是猜的）

| 项 | 值 |
|---|---|
| 芯片 | ESP32-S3 QFN56 rev0.2，4MB flash，8MB PSRAM（`esptool flash_id` + 串口日志） |
| 屏幕 | NV3041A，QSPI，480×272 IPS |
| 引脚 | CS45 / SCK47 / D0-21 / D1-48 / D2-40 / D3-39 / 背光 1 / TE 0 |
| 触摸 | GT911，I2C：SCL4 / SDA8 / INT3 / RES38（实测地址 0x5D） |
| 调试 | 板载 USB-Serial/JTAG（COM9），无 CH340；`CDCOnBoot=cdc`，串口 **921600** |

型号判定依据：设计稿九宫格正好是 480×272 的两倍放大；板子报出 ESP32-S3 + 4MB flash + 8MB PSRAM；读出的原厂固件与网盘 `JC4827W543` 的烧录镜像有 70% 字节一致，引脚表、NV3041A、GT911 地址全部吻合。

## 九个页面对应的固件页

| 设计图 | 固件页 | 数据来源 |
|---|---|---|
| ① 主页双卡 | PG_HOME | `quotas`(codex 5h) + `usage?period=today` |
| ② 斟酌中 | PG_STATE_THINK | 最近一次调用：耗时 |
| ③ 编写代码 | PG_STATE_CODE | 最近一次调用：模型 / 路由 |
| ④ 等你确认 | PG_STATE_CONFIRM | 最近一次调用（橙框告警态） |
| ⑤ Agent 列表 | PG_AGENTS | relay `/dash/agents`（magpie 管理的全部智能体） |
| ⑥ 额度 | PG_QUOTA | `/v1/magpie/quotas` |
| ⑦ 用量 | PG_USAGE | `/v1/magpie/usage?period=` |
| ⑧ 路由拓扑 | PG_ROUTES | `/v1/magpie/usage/requests` |
| ⑨ 路由列表 | PG_ROUTES_LIST | 同上，3 条 |

**导航**（全触摸）：主页左卡 → 状态页（点吉祥物在 ②③④ 循环），右卡 → Agent 列表；Agent 列表点某一行 → 状态页换成该智能体的形象（10 分钟后回到"跟随最近一次调用"）；状态页左上角房子回主页；第 5~9 页顶部四个 tab 互切；主页**长按顶部 3 秒**清空 WiFi 配置并重启。

> 这块屏的 GPIO0 接的是 LCD TE 信号，**不是按键**，所以没有 BOOT 键可用来翻页。

## 连接 magpie：两种方式，设备上可切换

设置页 →「连接 magpie」打开面板，选 **USB 一根线** 或 **WiFi 局域网**，选择存 NVS，重启后保持。

| 方式 | 适合 | 需要什么 |
|---|---|---|
| USB 一根线（默认） | 屏幕就摆在电脑旁 | 电脑跑 `tools/usb_bridge.py`，一根 USB 线；不开热点、不开端口 |
| WiFi 局域网 | 想脱离电脑 | 2.4GHz WiFi + 电脑局域网 IP；屏上自带配网页填写，无需电脑脚本 |

面板里的「重新连接」：USB 模式下让电脑端立刻重发数据；WiFi 模式下立即重连（没配过则自动开热点进入配网）。「WiFi 配网」会切到 WiFi 并启动热点 `magpie-dash-setup`，手机连上打开 `http://192.168.4.1` 填 SSID/密码/网关 IP。

## USB 方式细节（默认）

固件默认 `config.h: USB_LINK_ONLY 1`：**不开 WiFi、不开配网页、不需要热点**。屏幕插在电脑上，USB 同时供电和传数据，电脑继续用原来的校园网，端口一个都不对外开。

电脑侧跑 `tools/usb_bridge.py`（只监听 127.0.0.1 读 magpie，不 bind 任何网络端口）：

```bash
py -3.11 tools/usb_bridge.py --port COM9     # 前台跑，Ctrl-C 停
py -3.11 tools/usb_bridge.py --command q     # 顺带把串口调试指令发进去
powershell -File tools/bridge_autostart.ps1 -Action install   # 开机自启（pythonw，无窗口）
powershell -File tools/bridge_autostart.ps1 -Action status    # 看任务状态
powershell -File tools/bridge_autostart.ps1 -Action remove    # 取消自启
```

- 端口不写死：桥接按 VID/PID `0x303a:0x1001` 找板子，COM 号变了也能连上。
- 状态随时看 `build_logs/usb_bridge_status.json`（connected / requests / responses / errors / last_board_data），日志在 `build_logs/usb_bridge.log`。
- 重复启动会被命名互斥量挡掉，不会两个进程抢 COM9。
- `bash flash.sh flash` 会在烧录前自动停掉自启任务、烧完再拉起来，不用手动让口。

协议是长度 + CRC32 帧（`!MG1 <H|R|C> <seq> <len> <crc>\n` + body），请求路径白名单只含 4 个只读 dashboard 路径。**帧外字节一律丢弃**，所以串口噪声不会误触发调试指令——之前一次 `S` 被误读，把整屏帧转储刷满了串口。

### 延时：从 ~1.9 s 降到 0.13–0.26 s

原来一次刷新要传 71 KB：`/usage/requests` 一份就 45 KB（`series` 占大头），`/usage` 12.8 KB，串口按 512 B/6 ms 限速，还要在板子上解析全部内容。切换“今天/7 天/30 天”时又把额度和智能体一起重取。三处改动：

1. **桥接按需裁剪**（仅 USB 模式）：`requests` 只发 12 行 + 三个 token 求和（45 KB → 2.9 KB），`usage` 只发 9 个标量 + agents + series（12.8 KB → 1.2–2.4 KB）。WiFi 模式拿不到这层裁剪，所以更慢。
2. **两段式上屏**：先取 `/usage` 就把 KPI 卡刷出来，再取列表；切换档位时列表先清空，避免短暂显示上一档的数据。
3. **慢文档轮转**：额度和智能体每 3 轮取一次；请求超时 8 s → 2.5 s，超时即判定链路断开（不再每份文档各等 8 s）。

实测（2026-10-20，数据在流）：切档位到 KPI 上屏 **0.13–0.26 s**，列表补齐 0.26–0.38 s；完整周期约 1.5 s 且每 3 轮一次；主循环 `gap` 仍稳定 7 ms。数据与电脑端 API 逐项一致（today 493 次 / 78.6M，7d 2465，30d 2471 / 426M）。

> 连接策略是“选的那条优先，另一条能通就自动用”：曾被误触存成 WiFi 后，板子就完全不理 USB 线（表现成“没数据”）。设置页显示的是**当前实际在用的链路**。

### 串口日志绝不能阻塞（2026-10-20 修）

现象：板子跑几小时后“卡住不动”，串口一个字都不出，只能硬复位。

根因：主机一旦不读串口（桥接被停、窗口关掉、USB 挂起），CDC 发送环填满，`Serial.printf` 就会**阻塞调用它的任务**。取数任务和主循环一起卡死，屏幕自然冻住，而且一条日志都打不出来——看起来像“死机”，其实是写日志把自己锁住了。

修复：`Serial.setTxTimeoutMs(0)`（发不出去就丢，绝不阻塞）+ 周期性日志前先看 `Serial.availableForWrite()`。回归测试 `tools/test_usb_disconnect.py`：断开 100 秒再重连，要求板子照常回话——当前 PASS。

## 无线方式（可选，不插电脑时用）

`config.h` 里把 `USB_LINK_ONLY` 改成 0 就回到原来的 WiFi 路线，需要下面的配网 + 需要注意的两个坑。

## 首次配网

> **入网是非阻塞的**：`netcfg_begin()` 只发起连接就返回，连接进度在 `netcfg_loop()` 里推进。之前在 `ui_init()` 之后同步等 WiFi（2×12 s），那段时间主循环完全不跑——屏幕停在开机画面、点什么都没反应，最容易被误判成"触摸坏了"。现在开机后 UI 立刻可用，WiFi 连上后数据自己出现。

第一次上电（或长按主页顶部）进入 AP 模式：

1. 手机/电脑连 WiFi **`magpie-dash-setup`**，密码 `magpie12345`
2. 打开 `http://192.168.4.1`
3. 填家里 WiFi 名称/密码 + 电脑局域网 IP（`ipconfig` 的 IPv4）+ 端口
4. 保存 → 自动重启 → 开始拉数据

配置存 NVS，重烧固件不丢。`config.h` 里的默认值只在 NVS 为空时生效。

## 两个必须知道的坑

1. **magpie 只监听 127.0.0.1**，屏连不上。先跑转发：
   ```bash
   python tools/gw_relay.py 3426 127.0.0.1 3425
   ```
   然后屏的网关地址填 `<电脑IP>:3426`（不是 3425）。
2. **ESP32-S3 只有 2.4GHz、不支持 802.1X**，连不上 `SUDA_WIFI`（5GHz + 企业认证）。需要一个 2.4GHz 普通网络：手机热点或宿舍路由器。本机联调时是用 `tools/start_hotspot.ps1` 开的 Windows 热点 `magpie-dash-net`（密码 `magpie12345`）。



## 智能体状态与动作

- 顶部 tab 只显示 **Agent**（数量留在列表里，不再挤在 tab 上）。
- 列表每行右侧明确写出状态：**工作中**（最近 2 分钟内有调用，琥珀色 + 呼吸的点）、**已完成**（今天有调用）、**空闲**（用过但今天没有）、**未使用**。
- 头像有持续动作：空闲时约 1.9 s 一次的三像素呼吸，工作状态加快到 0.8 s，并配合打点动画；一次工作结束时头像会跳一下并说「做完了」。设置页的「头像动效」可整体关闭。

## 设置页（第 10 页）

入口有两个：顶部 tab 条最右的 **设置**，或直接点主页左上角的 `@名字`。

| 项 | 取值 | 说明 |
|---|---|---|
| 显示名称 | 键盘输入（字母/数字/`-`/`_`） | 主页左上角 `@xxx`，最多 19 字符 |
| 用量显示 | 剩余 / 已用 | 额度百分比、环形进度、首页大数字一起切换 |
| 刷新间隔 | 15 / 30 / 60 秒 | 取数任务的周期，立即生效 |
| 头像动效 | 开 / 关 | 关掉后不再播放弹跳动画（省渲染） |
| 隐藏金额 | 开 / 关 | 花费显示成 `$••`，适合屏幕摆在公共场合 |
| 连接 magpie | USB · 已连接 / WiFi · 未连接 … | 点开面板：切 USB/WiFi、立即重连、WiFi 配网 |

选择存在 NVS 命名空间 `cfg`，重烧固件不丢。键盘是自绘的（不依赖系统输入法）：`删` 退格、`清空` 全清、`保存` 落盘。

调试入口：串口 `9` 直达设置页，`Z` 跑设置页自动点击自检（5 项），`y` / `Y` 开关键盘方便截图，`D` 恢复默认设置。

> 新增中文文案时，必须把字串加进 `tools/gen_fonts.py` 的 `STRINGS` 再跑一次，否则屏上是缺字方框；`ark16` 是纯拉丁字重，别拿来显示中文。

## 数据接口

每 15 秒（`config.h: REFRESH_MS`）拉这些：

| 请求 | 用到的字段 |
|---|---|
| `/v1/magpie/quotas` | `data[].provider/name/plan/user`、`windows[].name/used/remaining/resetsAt` |
| `/v1/magpie/usage?period=30d` | `calls/errors/input/output/cache_read/cost`、`series[]` |
| `/v1/magpie/usage?period=today` | 首页「今天 N 次 · token」的后备值 |
| `/v1/magpie/usage/requests?period=today&limit=20` | `rows[].t/agent/agentName/provider/model/stop/in/out/cache_read/ms/status`；`total` + `series` 给出今天含会话日志的调用数和 token |
| `/dash/agents`（relay 本地应答） | Agent 页：`id/name/model/calls/tokens/cost/today/last` |

magpie 的 `period` 只认 `today/7d/30d/all`，**没有 `1d`**（传 1d 会静默回退成 30d）。

> 用量页的四个档位（今天/7 天/30 天/全部）必须同时作用于 `/usage` **和** `/usage/requests`：早期版本把 requests 的 period 写死成 `today`，于是切到 7 天/30 天时 KPI 是区间值、列表和会话统计却还是今天的，看起来就是"数据乱"。桥接白名单也要跟着放行四个档位，否则请求直接被丢弃。

设计稿里「斟酌中 / 编写代码 / 等你确认」「2 个会话」「git push origin main」这类 Claude Code 会话级状态，magpie 的公开接口没有暴露，这三页用**最近一次请求**推导：耗时、模型、stop 原因、状态码。

## 调试：把屏幕「抓」到电脑上

固件带了两条串口指令（921600 波特率）：

| 指令 | 作用 |
|---|---|
| `s` | 逐页渲染并把 480×272 RGB565 帧缓冲以十六进制吐出来 |
| `r` | 立刻重新拉一次网关数据 |
| `d` | 打印界面对象树（坐标 + 背景色），排查布局用 |

```bash
python tools/grab_shots.py COM9 shots     # 生成 shots/shot_p0..p8.png
```

这是唯一能「看见」屏幕的办法 —— 渲染 bug 都是这么发现并修掉的。

## 自己改

```bash
python tools/gen_fonts.py                 # 改文案后重建中文字体子集
python tools/extract_sprites.py --write   # 重新从设计图抠吉祥物/喜鹊/气泡/星星
./flash.sh                                # 编译
./flash.sh flash                          # 编译 + 烧录 + 串口日志
```

字体是 **Ark Pixel**（10/12/16/24，1bpp 无抗锯齿），只打包固件用到的 225 个字形。**新加的中文必须出现在 `tools/gen_fonts.py` 的 `STRINGS` 里**，否则屏幕上显示空白。

## 已知限制 / 还能继续打磨的地方

- Agent 页图标是品牌色小方块，不是设计稿里每个产品各自的 Logo。
- 路由拓扑的连线是 `......` 文本点，不是设计稿那种精细虚线。
- 字体是子集，**网关返回的任意中文**（比如账号名里的生僻字）可能缺字显示空白；目前已补进 `提示词建议/工具调用/择梦舟` 等常出现的字，遇到新缺字要往 `gen_fonts.py` 里加。
- 拉数据在独立任务里跑（5 个 HTTP 请求，局域网 0.5–1.2 s 一轮），不阻塞 UI；但每 15 秒数据到达时仍有一次约 120–160 ms 的重绘顿挫。
- 触摸坐标用 NVS 里的两点校准；若发现点击偏移，串口 `c` 进校准画面重新点三点即可。

## 智能体头像与交互

头像用的是各智能体的**真实 logo**——就是 magpie 应用里显示的那套图标（LobeHub 图标集，SVG 存在 `tools/agent_logos/<agent id>.svg`）。`tools/gen_agents.py`（需要 `pip install resvg-py pillow`）把它们预渲染成 20 / 56 / 88 px 三种尺寸写进 `agent_sprites.h`，屏上 1:1 显示，不做缩放；预览图 `shots/agents_preview.png`。logo 版权归各自厂商。

Claude Code · Codex · Grok Build · DeepSeek Harness · Hermes Agent · Pi · ZCode · Claude Desktop · OpenCode · Cursor · Antigravity · CodeBuddy · Command Code · MiMo Code，未知智能体用通用终端图标。

交互：

- **头像 = logo 原图**，直接放在卡片上（不加底板、不加描边，保持品牌原色）。状态：斟酌中 / 工作中 = logo 下三个点轮流亮，编写代码 = 角标 `>_`，等你确认 = 琥珀色角标 `!`。主页上 agent 最近 2 分钟内有调用就显示"工作中"。没有常驻动画：之前的转圈状态环让 LVGL 空闲时也有 40–70% 在重绘，触摸明显卡，现在各页空闲负载 1–2%（串口 `l` 可测）。
- **戳一下头像**：跳一下并冒出它自己的台词（Claude Code "Clauding..."、Grok "Grokking..."、DeepSeek "深度思考中"……），连戳 5 次会说"别戳了 >_<"。主页点头像是戳，点卡片其它地方进状态页。
- **换 agent**：最近一次请求换了 agent（或你选了别的），所有头像跳一下、新 agent 打招呼。
- **Agent 页**：每行 logo + 名字 + 模型 + 30 天次数，正在工作的 agent 在 logo 角上有绿点。点一行弹出详情卡：30 天调用 / 今天 / Token / 费用 / 占全部调用比例 / 最近调用时间，"跟随"把主页和状态页固定到这个 agent（10 分钟后自动回到跟随最新请求），再点一次"取消跟随"。
- **状态页**：头像下面三个小条是三个状态页，点哪个去哪个。

**Agent 列表来源**：magpie 公开接口没有智能体列表，所以 `tools/gw_relay.py` 自己应答 `GET /dash/agents`：读 `~/.config/magpie/library.json` 的 `applied`（magpie 在本机管理的智能体）+ `applied.json`（magpie 给它配的模型）+ `/v1/magpie/usage/requests?period=30d` 的按天 `by.agent` 统计（调用次数 / token / 费用 / 今天次数；这个统计包含 magpie 从本地会话日志读到的 Claude Code 调用，`/usage` 的 `agents[]` 不含）+ 最近请求里每个 agent 的最后调用时间。旧版 relay 没有这个路径时，固件退回"用量里出现过的 agent + 内置目录"。**更新固件后记得重启 relay。**

## 串口调试命令（921600）

| 键 | 作用 |
|---|---|
| `s` | 九页截图（`tools/grab_shots.py`） |
| `m` | 载入演示数据（不连网关也能看完整页面；`tools/shot_mock.py` 一键 m+截图） |
| `r` | 立刻刷新并退出演示数据 |
| `S` | 只截当前屏（保留弹窗、气泡） |
| `0`..`8` | 跳到第 N 页（会打印 `painted N ms`） |
| `f` | 切换到下一个智能体头像（预览用） |
| `k` | 戳当前页的头像 |
| `l` | 每页空闲时 LVGL 的重绘负载（%） |
| `q` | 每 2 秒打印一次计时：`gap`(主循环最长停顿) `tick_max`(单次 LVGL 处理) `upd_max`(ui_update) `flush_*`(像素传输) |
| `V` | 打开/关闭逐事件日志（每次点击、每次换页、每个网关请求）。**默认关闭**：主机没在读串口时 USB-CDC 写入会反压 UI 线程，默认打开会引入莫名其妙的卡顿 |
| `T` | 打开/关闭**颜色 + 触摸方向测试画面**：左上红 / 右上绿 / 左下蓝 / 右下白，黄点跟手（复位后自动消失） |
| `K` | 清掉 NVS 里存的触摸校准，回到编译默认的 1:1 直通 |
| `o` | 打开 Agent 页第一行的详情卡 |
| `c` / `x` | 打开 / 关闭校准画面 |
| `t` / `p` / `d` | 触摸原始点 / 每页重绘耗时（含 flush 与 render 拆分）/ 对象树 |

## 点击失效回归测试（当前验证边界）

串口 `A` 在板子上向已注册的 LVGL 输入驱动注入按下/抬起，经过真实命中检测和点击回调，检查最终页；不直接调用页面按钮回调，也不依赖用户恰好在抓日志时点击。

```bash
py -3.11 tools/run_tap_regression.py --label check
```

已复现并修复：通用 `obj()` 使用 `lv_obj_create()`，装饰对象默认带 `CLICKABLE`，进度条容器和顶部色条会吞掉父卡片的点击。修改前 31 项中 3 项失败；清掉通用装饰的 `CLICKABLE`、只给真正按钮/头像/行/滚动区域显式开启后，31 项全部通过。证据在 `build_logs/tap-before.log` 和 `tap-after.log`。

这些测试证明 LVGL 命中及导航链路，**不证明 GT911 实物触摸或连续点击手感已经正常**。此前基于重绘耗时就宣称触摸修好、达到物理极限的说法不成立。颜色与坐标保持当前值，此次未改。

### Agent 详情弹窗

打开详情时不再自动播放 450 ms 头像弹跳；用户主动戳头像的跳动仍保留。串口 `B` 通过 LVGL 输入驱动依次点击前三行并关闭，检查目标 Agent、弹窗状态及末次刷新完成，记录首帧与后续 500 ms 的 UI 工作量。

- 修改前：6 次均成功开/关，但都有自动弹跳；后续 500 ms UI 处理耗时 34–55 ms。
- 去掉自动弹跳后：同测 6 次通过，复测 6 次通过；后续 UI 处理耗时 14–16 ms。
- 首帧完成变化很小：91–94 ms → 86–92 ms（包含测试固定的 30 ms 按压）；不能把动画工作量下降宣称为点击延迟大幅下降。
- 31 项导航回归仍全部通过。此测试绕过 GT911，最终手感以用户实物反馈为准。

复现命令：`py -3.11 tools/run_tap_regression.py --command B --label sheet-check`。

## 性能：实测数据与调优记录

换页（整屏重绘）在 JC4827W543 + LVGL 8.3 上的实测演进：

| 版本 | 整页重绘 | 其中 flush(QSPI) | 其中 render |
|---|---|---|---|
| 初始（480×20 双缓冲，内部 RAM） | 134–157 ms | 41 ms | 93–113 ms |
| 整屏缓冲放 PSRAM（1 条带） | 71–78 ms | 40 ms | 13–40 ms |
| + QSPI 40 MHz | 62–71 ms | 32 ms | 23–40 ms |
| + 96 行缓冲放内部 RAM（3 条带） | 50–65 ms | ~30 ms | 18–33 ms |
| + 72 行双缓冲 + 异步 DMA 刷新任务 | **34–40 ms** | 30–38 ms（与渲染重叠） | 6–37 ms |

关键结论（都不是猜的，是 `p`/`q`/`l` 在板子上量出来的）：

- **异步 DMA 刷新（双缓冲）实现渲染与传输流水线重叠**：采用 72 行 × 2 的内部 DMA RAM 双缓冲，结合 FreeRTOS 独立 `lcdflush` 队列任务，当第 N 带传输时，LVGL 并发渲染第 N+1 带，换页重绘时间压至 **34–40 ms** 的物理极值。
- **I²C 互斥锁保障**：异步刷新与主循环会并发轮询 GT911 触摸芯片，通过 `s_i2c_mtx` 互斥锁确保 I²C 读写线程安全，杜绝总线冲突。

- **条带数比缓冲放哪更重要**：480×20 的缓冲让 LVGL 每次重绘把对象树走 14 遍；改成整屏/96 行后只剩 1–3 遍，render 直接砍掉一半以上。
- **渲染像素写在内部 SRAM 更快**：整屏 PSRAM 缓冲（261 KB）虽然只用 1 条带，但每像素写入走 OPI PSRAM；96 行内部 SRAM 缓冲（92 KB，3 条带）再快 20%。
- **LVGL 堆放 PSRAM 不是瓶颈**：改成"内部优先、PSRAM 溢出"后整页只从 157 → 147 ms，反而把 Wi-Fi 驱动挤到分配失败（`Expected to init 4 rx buffer, actual is 0`）。对象树留在 PSRAM，但内部堆至少留 110 KB 给 Wi-Fi。
- **QSPI 32 MHz 是取整陷阱**：S3 从 80 MHz APB 分频，请求 32 MHz 实际只有 20 MHz 左右；请求 40 MHz（80/2）才精确，flush 41 → 32 ms。再往上提升有限且超出面板手册上限，不做。
- **空闲负载不是问题**：九页空闲重绘 1–3%，主循环周期 7 ms。卡顿只发生在"整屏重绘"和"每 15 秒数据到达"这两个瞬间。
- **每 15 秒有一次 ~120–160 ms 的顿挫**：`ui_update()` 本身 13–44 ms，加上可见页重绘。已让 `segbar_set`/`set_color`/`show()` 只在真变化时才写样式，未变化的数据不再反复失效重绘。
- **触摸判定没问题**：手指抬起能被 GT911 的 frame 正常捕获（不是靠 300 ms 超时），`[touch] up` → `[ui] page` 只隔 1–35 ms。曾经的"点完等半秒"是整屏重绘 + 30 ms 刷新定时器叠加，现在触摸轮询/刷新周期都降到 15 ms，并在换页时立即重绘（不等定时器）。
- **连点会"越点越卡"是因为每次换页都串行画一整屏**：连点 6 个 tab 时 `page 0 at 8042 → 1 at 8088 → 2 at 8134 → 3 at 8180 → 4 at 8240 → 5 at 8296`，每页 46–60 ms 累加，画面一页页跳且落后手指。现在触点走 `page_request()`：距上次绘制 >150 ms 的单点立即画；150 ms 内的连点只记住最后一个目标，由 `ui_page_settle()`（主循环里）合并成一次绘制。串口 `b` 可复现：4 连点只产生 1 次 `painted`，最终页在 39–60 ms 内出现。
- 程序化换页（串口 `0`..`8`、开机初始化、截图）仍走同步的 `ui_goto()`，保证随后 `lv_refr_now()`/抓图拿到的是目标页。
- **触摸轮询与 LVGL 解耦**：一次整屏绘制会阻塞 40–60 ms，只在 LVGL 的读取回调里读 GT911 的话，落在这段窗口里的"按下+抬起"会整段消失（或丢掉抬起帧，于是靠 `TOUCH_STALE_MS=300` 兜底，表现为"点几下没反应、过一会儿才跳"）。现在 `touch_poll()` 在主循环、LVGL 读取回调、以及**每条刷新带之间**都会被调用（12 ms 限频，空闲占用 2–3%），按下/抬起只做事件锁存，`touch_cb` 每次读消费一个边沿。日志里每次点击都是 `up (frame) after 6 ms`，不再出现 `stale timeout`。
- **小按钮要留点击余量**：tab 按钮 92×28，实测有一下点在 y=4（按钮顶边在 y=6）被吞掉。给 tab 加 `lv_obj_set_ext_click_area(b, 3)`（间距 6 px，不会串到邻居）、主页按钮加 4 px。
- **数据刷新的重绘不能压在点击上**：`ui_update()` 之后可见页会重绘 ~120–160 ms（实测 `[stall] gap 135 ms, LVGL tick 128 ms`）。主循环里现在用 `touch_recent(400)` 把它推迟到手指停下再做，最多压 3 秒，避免"正好在数据到达那一刻点，像没反应"。
- **默认不打日志**：逐事件日志（每次点击/换页/每个 HTTP 请求）要开 `V` 才输出。`Serial.printf` 走 USB-CDC，主机不在读时可能阻塞 UI 线程——这是"时好时坏"的一个来源。
- **`[stall]` 看门狗**：主循环停顿超过 120 ms 且刷新记录时打印一行，带上次 LVGL tick 与 data update 的耗时，能直接区分"卡在渲染"还是"卡在数据更新"。

颜色与方向（用测试画面定死的，不是猜的）：固件里有个 `T` 指令会画四个色块（左上红 / 右上绿 / 左下蓝 / 右下白）+ 一个跟手的黄点。这块板上：

- **颜色 = 驱动默认的 `MADCTL = MX|MY|RGB (0xC0)` 才对**。曾经以为是 BGR 接法、改成 0xC8，测试画面显示左上角变成蓝色（红蓝互换），所以已改回 0xC0。红蓝互换时**不要**去动 logo 素材或 LVGL 的 `lv_color_t`，先用色块确认到底是面板通道顺序、字节序还是反显。
- **触摸 = 1:1 直通（`ax=1, bx=0, ay=1, by=0`）**。之前 NVS 里存着一份"上下左右都镜像"的校准（`ax=-0.995, ay=-1.07`），点哪里都落在对角方向，看着像"点了没反应"。`K` 指令会清掉 NVS 里的旧校准并回到编译默认值。

`T` 和 `K` 是排障用的，正常使用不需要碰；`T` 画的画面在复位后自动消失。

