// Chinese / English string table for the dashboard.
//
// The Chinese text is the key: T("主页") returns the translation, or the Chinese
// text itself when the language is Chinese or the string is not in the table.
// Callers never need an id, which keeps the page builders readable, and a string
// that is missing from the table degrades to Chinese instead of "".
//
// Budgets: the panel is 480 px wide with a 6 px font cell, so English has to stay
// about twice the character count of the Chinese it replaces; the entries were
// written to fit the columns in ui.cpp (see the layout notes there).
#include "i18n.h"

#include <string.h>

static uint8_t s_lang = LANG_EN;

struct Entry {
    const char *zh;
    const char *en;
};

// Ordered roughly as the UI is read: chrome, home, state pages, agents, quota,
// usage, routes, settings, link panel, formatters, WiFi setup page.
static const Entry kTbl[] = {
    {"主页", "Home"},
    {"额度", "Quota"},
    {"用量", "Usage"},
    {"路由", "Routes"},
    {"设置", "Config"},
    {"周日", "Sun"},
    {"周一", "Mon"},
    {"周二", "Tue"},
    {"周三", "Wed"},
    {"周四", "Thu"},
    {"周五", "Fri"},
    {"周六", "Sat"},
    {"等待网关", "waiting for gw"},
    {"今天调用", "today calls"},
    {"30 天调用", "30d calls"},
    {"费用", "cost"},
    {"费用 %s", "cost %s"},
    {"会话", "sessions"},
    {"最近", "last"},
    {"暂无数据", "no data"},
    {"%s 次", "%s calls"},
    {"-- 个会话", "-- sessions"},
    {"%d 个会话", "%d sessions"},
    {"> 斟酌中", "> thinking"},
    {"> 编写代码", "> coding"},
    {"! 等你确认", "! need confirm"},
    {"5 小时额度", "5h quota"},  // placeholders shown before the first data update
    {"7 天额度", "7d quota"},
    {"%d 小时额度", "%dh quota"},
    {"%d 天额度", "%dd quota"},
    {"%d 周额度", "%dw quota"},
    {"未连接", "disconnected"},
    {"已连接", "connected"},
    {"已用时 %s", "elapsed %s"},
    {"运行命令", "running a command"},
    {"运行命令 : %s", "exec: %s"},
    {"额度来源 %s · %s", "quota: %s · %s"},
    {"暂无调用", "no calls"},
    {"%d 个", "%d agents"},
    {"轻触头像", "tap avatar"},
    {"关闭", "Close"},
    {"跟随", "Pin"},
    {"取消跟随", "Unpin"},
    {"今天", "today"},
    {"默认模型", "default model"},
    {"占全部调用 %.0f%%", "%.0f%% of calls"},
    {"● 工作中", "● working"},
    {"最近 %s", "last %s"},
    {"还没用过", "never used"},
    {"工作中", "working"},
    {"已完成", "done"},
    {"空闲", "idle"},
    {"未使用", "unused"},
    {"错误 %u", "err %u"},
    {"工具调用", "tool call"},
    {"回复", "reply"},
    {"暂无额度数据", "no quota data"},
    {"7 天", "7d"},
    {"30 天", "30d"},
    {"全部", "all"},
    {"src: 网关 + 会话日志", "src: gw + logs"},
    {"请求", "requests"},
    {"缓存命中", "cache hit"},
    {"占提示词", "of prompt"},
    {"token / 时段", "tok / bucket"},
    {"%u 次失败", "%u failed"},
    {"均价 $%.4f", "avg $%.4f"},
    {"今天 -- 次调用 · -- token", "today -- calls · -- tok"},
    {"%d 条", "%d rows"},
    {"收起 «", "collapse «"},
    {"展开 »", "expand »"},
    {"今天 %u 次调用 · %s token", "today %u calls · %s tok"},
    {"显示名称", "display name"},
    {"用量显示", "quota view"},
    {"刷新间隔", "refresh rate"},
    {"头像动效", "avatar anim"},
    {"隐藏金额", "hide cost"},
    {"连接 magpie", "magpie link"},
    {"语言", "language"},
    {"已用", "used"},
    {"剩余", "left"},
    {"%u 秒", "%us"},
    {"开", "on"},
    {"关", "off"},
    {"USB · 已连接", "USB · connected"},
    {"USB · 未连接", "USB · offline"},
    {"WiFi · 未连接", "WiFi · offline"},
    {"WiFi · 已连接", "WiFi · connected"},
    {"保存", "Save"},
    {"取消", "Cancel"},
    {"删", "Del"},
    {"清空", "Clear"},
    {"● USB · 等待电脑端桥接", "● USB · waiting host"},
    {"● WiFi · 未连接", "● WiFi · not connected"},
    {"● %s · 已连接", "● %s · connected"},
    {"网关 %s:%u", "gw %s:%u"},
    {"插上 USB 线就能用，不用路由器", "Plug USB cable; no router needed"},
    {"手机连 WiFi magpie-dash-setup 配网", "Join SSID magpie-dash-setup"},
    {"USB 一根线", "USB cable"},
    {"WiFi 局域网", "WiFi LAN"},
    {"重新连接", "Retry"},
    {"WiFi 配网", "WiFi setup"},
    {"别戳了 >_<", "stop poking >_<"},
    {"做完了", "all done!"},
    {"刚刚", "just now"},
    {"%ld 分钟前", "%ldm ago"},
    {"%ld 小时前", "%ldh ago"},
    {"%ld 天前", "%ldd ago"},
    {"%u分%u秒", "%um%us"},
    {"%.1f 秒", "%.1fs"},
    {"将重置", "resets soon"},
    {"%ld 天 %ld 小时后重置", "resets in %ldd %ldh"},
    {"%ld 小时后重置", "resets in %ldh"},
    {"%ld 分钟后重置", "resets in %ldm"},
    {"%d月%d日 %02d:%02d", "%d/%d %02d:%02d"},
    {"已保存，正在重启…", "Saved. Rebooting..."},  // WiFi setup page only
};

static const unsigned kTblN = sizeof(kTbl) / sizeof(kTbl[0]);

// Same literals come back on every repaint (page rebuilds, 15 s data updates),
// and string literals are pooled, so remembering the last few answers turns the
// table into a pointer compare for almost every call.
struct Hit {
    const char *zh;
    const char *en;
};
static Hit s_hits[4];

uint8_t lang_get(void) { return s_lang; }

void lang_set(uint8_t lang) {
    s_lang = (lang == LANG_ZH) ? LANG_ZH : LANG_EN;
    for (int i = 0; i < 4; i++) s_hits[i].zh = s_hits[i].en = nullptr;
}

const char *T(const char *zh) {
    if (!zh || !*zh) return "";
    if (s_lang == LANG_ZH) return zh;
    for (int i = 0; i < 4; i++)
        if (s_hits[i].zh == zh) return s_hits[i].en;  // same literal, cached
    static unsigned next = 0;
    const char *out = zh;
    for (unsigned i = 0; i < kTblN; i++) {
        if (!strcmp(kTbl[i].zh, zh)) {
            out = kTbl[i].en;
            break;
        }
    }
    s_hits[next++ & 3] = {zh, out};
    return out;
}

// Avatar catchphrases: the shipped table in agent_sprites.h is Chinese, so the
// English lines live here, keyed by agent id, and fall back to the shipped ones.
struct Say {
    const char *id;
    const char *line[2];
};
static const Say kSay[] = {
    {"claude", {"Clauding...", "on it!"}},
    {"codex", {"> codex exec", "patch ready"}},
    {"grok", {"Grokking...", "answer is 42"}},
    {"dsh", {"deep thinking", "whale breached"}},
    {"hermes", {"routing...", "cached!"}},
    {"pi", {"tiny pi run", "pi done"}},
    {"zcode", {"Z ready", "GLM online"}},
    {"claude-desktop", {"hi Claude here", "desktop idle"}},
    {"opencode", {"opencode run", "open source!"}},
    {"cursor", {"Tab Tab Tab", "completed"}},
    {"agy", {"liftoff!", "antigrav on"}},
    {"codebuddy", {"meow~ coding", "slacking off"}},
    {"commandcode", {"cmd received", "running..."}},
    {"mimocode", {"MiMo online", "all shipped"}},
    {"generic", {"hello", "right here"}},
};

const char *say_line(const char *id, int idx) {
    if (s_lang == LANG_ZH) return nullptr;
    for (unsigned i = 0; i < sizeof(kSay) / sizeof(kSay[0]); i++)
        if (!strcmp(kSay[i].id, id)) return kSay[i].line[idx & 1];
    return nullptr;
}
