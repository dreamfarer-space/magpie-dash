"""Generate LVGL 8 bitmap fonts (Ark Pixel 10/12/24, 1bpp) for the dashboard.

All UI strings live here so the font subset and the firmware text stay in sync.
Run:  python gen_fonts.py
Out:  ../firmware/magepie_dash/fonts/*.c
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
TTF_DIR = r"D:\esp32\tools\fonts"
OUT_DIR = os.path.join(ROOT, "firmware", "magepie_dash")  # sketch root: Arduino only compiles there
LVCONV = ["node", r"D:\esp32\tools\node_modules\lv_font_conv\lv_font_conv.js"]

# ---------------------------------------------------------------- UI strings
STRINGS = [
    # header / home
    "@bruc3van", "10/15 周四", "21 49", "周一", "周二", "周三", "周四", "周五", "周六", "周日",
    "Claude Buddy", "运行命令", "magpie", "今天", "次", "已用时", "秒", "小时", "分钟", "天",
    # claude code status pages
    "CLAUDE CODE", "个会话", "斟酌中", "编写代码", "等你确认", "运行命令 : git push origin main",
    "小时额度", "天额度", "小时后重置", "已连接", "未连接", "连接中", "在线", "离线", "等待网关",
    # tabs
    "Agent", "额度", "用量", "路由", "设置", "刷新", "退出",
    # agent rows
    "Claude Code", "Codex", "Pi", "DeepSeek Harness", "Grok Build", "ZCode", "Claude Desktop",
    "opus", "effort", "high", "low", "thinking", "provider", "gpt-6.1-sol",
    "magpie/grok/grok-4.6", "—",
    # quota cards
    "pro", "plus", "剩余", "GROK (SUPERGROK)", "额度卡", "重置",
    # usage page
    "7 天", "30 天", "全部", "网关 + 会话日志", "Token 总量", "输入", "输出", "缓存",
    "请求次数", "次失败", "费用支出", "生效价格", "缓存命中率", "占提示词",
    "无数据", "今日无调用", "读取中", "调用",
    # route page
    "次调用", "提示词建议", "路由记录", "秒", "token", "网关",
    "→", "·", "≈", "↻", "■", "●", "★", "%", "$", "+", "/", ":", "-", ".", "_", "(", ")", "[", "]",
    "?", "!", "·",
    # misc runtime
    "WiFi 设置", "AP 模式", "正在启动", "已保存", "重启中", "IP", "网关地址", "请输入",
    "错误", "空", "连接", "断开", "重试", "版本", "内存",
    # glyphs used by runtime formatting (dates, counts)
    "年月日时分", "个会话今天次调用失败", "额度已用剩余重置在线稳定刷新读取",
    "提示词建议工具路由记录择梦舟余解待确认位",
    # agent buddies / redesigned pages
    "Hermes Agent", "OpenCode", "Cursor", "Antigravity", "CodeBuddy", "Command Code", "MiMo Code",
    "@Dreamfarer", "额度来源", "暂无调用", "今日暂无调用", "暂无额度数据", "暂无数据", "未使用",
    "默认模型", "最近", "空闲", "回复", "工具调用", "错误", "周额度", "即将重置", "时", "请求", "费用",
    # agent avatars: poke lines, sheet, activity
    "这个文件交给我", "> codex exec", "补丁已就绪", "让我想想宇宙的答案", "深度思考中", "鲸鱼出海",
    "消息送达", "记住了，下次用", "最小的 agent", "pi 正在运行", "Z 已就位", "GLM 在线",
    "你好，我是 Claude", "桌面待命", "开源万岁", "已补全", "起飞！", "反重力已开启", "喵~写好了",
    "摸鱼中", "命令已收到", "跑起来了", "MiMo 在线", "小米加步枪", "你好", "在这儿", "别戳了 >_<",
    "轻触头像", "30 天调用", "今天", "费用", "占全部调用", "关闭", "跟随", "取消跟随", "工作中",
    "刚刚", "分钟前", "小时前", "天前", "还没用过", "最近 30 天未使用", "默认模型",
    # settings page
    "设置", "显示名称", "用量显示", "刷新间隔", "头像动效", "隐藏金额", "连接", "已用", "开", "关",
    "保存", "清空", "删", "点击重试", "USB", "秒", "立即刷新", "循环切换", "键盘", "名称", "其余",
    # link panel (USB cable / WiFi LAN)
    "连接 magpie", "一根线", "局域网", "已连接", "等待电脑端桥接", "电脑端运行", "插线即用",
    "不需要路由器", "屏幕只读本机", "不开端口", "手机连热点", "网关", "立即重试", "配网", "填写",
    "WiFi", "等待", "跑", "桥接",
    "插上 USB 线就能用，不用路由器", "电脑端跑", "重新连接", "网关地址", "等待电脑端桥接",
    "均价", "次",
]

# ASCII printable is always included
CHARS = set(chr(c) for c in range(0x20, 0x7F))
for s in STRINGS:
    CHARS |= set(s)
CHARS |= set("·—→↻≈■●★")
CPS = sorted(ord(c) for c in CHARS)
print("unique glyphs:", len(CPS))

FONTS = [
    # name, ttf, size, bpp
    ("ark10", "ark-pixel-10px-proportional-zh_hans.ttf", 10, 1),
    ("ark12", "ark-pixel-12px-proportional-zh_hans.ttf", 12, 1),
    ("ark24", "ark-pixel-12px-proportional-zh_hans.ttf", 24, 1),
    ("ark16", "ark-pixel-16px-proportional-zh_cn.ttf", 16, 1),
]

os.makedirs(OUT_DIR, exist_ok=True)

from fontTools.ttLib import TTFont  # noqa: E402


def font_cmap(path):
    f = TTFont(path)
    cm = set()
    for t in f["cmap"].tables:
        cm |= set(t.cmap.keys())
    return cm


for name, ttf, size, bpp in FONTS:
    out = os.path.join(OUT_DIR, name + ".c")
    cm = font_cmap(os.path.join(TTF_DIR, ttf))
    wanted = [cp for cp in CPS if cp in cm]
    dropped = [cp for cp in CPS if cp not in cm]
    print(f"  {name}: wanted {len(wanted)}, not in font: {''.join(chr(c) for c in dropped) or '-'}")
    cmd = LVCONV + [
        "--font", os.path.join(TTF_DIR, ttf),
        "--size", str(size),
        "--bpp", str(bpp),
        "--format", "lvgl",
        "--lv-include", "lvgl.h",
        "--no-compress",
        "--force-fast-kern-format",
        "-r", "0x20-0x7e",
        "-o", out,
    ]
    for cp in wanted:
        if cp > 0x7E:
            cmd += ["-r", hex(cp)]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print(r.stdout[-3000:])
        print(r.stderr[-3000:])
        sys.exit(1)
    src = open(out, encoding="utf-8").read()
    got = set(range(0x20, 0x7F))
    for m in re.finditer(r"unicode_list_\d+\[\]\s*=\s*\{(.*?)\};", src, re.S):
        got |= {int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]+)", m.group(1))}
    missing = [cp for cp in CPS if cp not in got]
    print(f"  {name}: {os.path.getsize(out)} bytes, glyphs={len(got)}, "
          f"missing={''.join(chr(c) for c in missing) or '-'}")
