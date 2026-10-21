"""Expose the local magpie gateway to the LAN.

magpie only listens on 127.0.0.1:3425, so the ESP32 cannot reach it. This
forwards 0.0.0.0:3426 -> 127.0.0.1:3425 with a plain TCP relay, then you point
the dashboard's setup page at <PC LAN IP>:3426.

It also answers one path itself, GET /dash/agents: the coding agents magpie
manages on this PC (~/.config/magpie/library.json "applied"), each with the
model magpie set for it (applied.json) and its 30-day usage. magpie's public
API has no such list, and the agent page needs it.

    python tools/gw_relay.py [listen_port] [target_host] [target_port]
"""
import datetime
import json
import os
import socket
import sys
import threading
import urllib.request

LISTEN_PORT = 3426
TARGET_HOST = "127.0.0.1"
TARGET_PORT = 3425
BUF = 65536
MAGPIE_DIR = os.path.join(os.path.expanduser("~"), ".config", "magpie")

# usage rows from these are scripts poking the gateway, not coding agents
NOT_AGENTS = {"generic", "curl", "Python-urllib"}


def pump(src, dst):
    try:
        while True:
            data = src.recv(BUF)
            if not data:
                break
            dst.sendall(data)
    except Exception:
        pass
    finally:
        try:
            dst.shutdown(socket.SHUT_WR)
        except Exception:
            pass


def read_json(name):
    try:
        with open(os.path.join(MAGPIE_DIR, name), encoding="utf-8") as f:
            return json.load(f)
    except Exception:
        return {}


def gw_get(path):
    url = f"http://{TARGET_HOST}:{TARGET_PORT}{path}"
    with urllib.request.urlopen(url, timeout=5) as r:
        return json.load(r)


def iso_epoch(t):
    try:
        return int(datetime.datetime.fromisoformat(t.replace("Z", "+00:00")).timestamp())
    except Exception:
        return 0


def dash_agents():
    installed = list((read_json("library.json").get("applied") or {}).keys())
    applied = read_json("applied.json")
    names, stats, last = {}, {}, {}
    try:
        # /usage only counts gateway calls; the request log's per-day series
        # also has the calls magpie read from local session logs (Claude Code)
        d = gw_get("/v1/magpie/usage/requests?period=30d&limit=300")
        series = d.get("series") or []
        for i, b in enumerate(series):
            for aid, v in ((b.get("by") or {}).get("agent") or {}).items():
                st = stats.setdefault(aid, {"calls": 0, "tokens": 0, "cost": 0.0, "today": 0})
                st["calls"] += v.get("calls", 0)
                st["tokens"] += v.get("tokens", 0)
                st["cost"] += v.get("cost", 0) or 0
                if i == len(series) - 1:
                    st["today"] = v.get("calls", 0)
        for row in d.get("rows") or []:
            aid = row.get("agent", "")
            if aid and aid not in last:
                last[aid] = iso_epoch(row.get("t", ""))
                if row.get("agentName"):
                    names[aid] = row["agentName"]
    except Exception as e:
        print(f"[relay] usage for /dash/agents failed: {e}", flush=True)
    try:
        for a in gw_get("/v1/magpie/usage?period=30d").get("agents") or []:
            if a.get("icon") in NOT_AGENTS:
                NOT_AGENTS.add(a.get("id", ""))
            names.setdefault(a.get("id", ""), a.get("name", ""))
    except Exception:
        pass

    ids = installed + [i for i in stats if i not in installed]
    rows = []
    for aid in ids:
        if aid in NOT_AGENTS:
            continue
        st = stats.get(aid, {})
        model = ((applied.get(aid) or {}).get("fields") or {}).get("model", "")
        if model.startswith("magpie/"):
            model = model[len("magpie/"):]
        rows.append({
            "id": aid,
            "name": names.get(aid, ""),
            "model": model,
            "calls": st.get("calls", 0),
            "tokens": st.get("tokens", 0),
            "cost": round(st.get("cost", 0.0), 2),
            "today": st.get("today", 0),
            "last": last.get(aid, 0),
        })
    # used agents first (busiest on top), then the rest in magpie's order
    rows.sort(key=lambda r: -r["calls"])
    return {"agents": rows}


def serve_local(client, path):
    if path.split("?")[0] == "/dash/agents":
        body = json.dumps(dash_agents(), ensure_ascii=False).encode("utf-8")
        head = "HTTP/1.1 200 OK\r\nContent-Type: application/json"
    else:
        body = b'{"error":"unknown dash path"}'
        head = "HTTP/1.1 404 Not Found\r\nContent-Type: application/json"
    head += f"\r\nContent-Length: {len(body)}\r\nConnection: close\r\n\r\n"
    client.sendall(head.encode("ascii") + body)


def handle(client, addr):
    # Peek at the request line: /dash/* is answered here, the rest is relayed.
    try:
        client.settimeout(10)
        first = client.recv(BUF)
        client.settimeout(None)
    except Exception:
        client.close()
        return
    line = first.split(b"\r\n", 1)[0].decode("latin-1", "replace").split()
    if len(line) >= 2 and line[0] == "GET" and line[1].startswith("/dash/"):
        try:
            serve_local(client, line[1])
            print(f"[relay] {addr} {line[1]} (local)", flush=True)
        except Exception as e:
            print(f"[relay] {line[1]} failed: {e}", flush=True)
        client.close()
        return

    try:
        upstream = socket.create_connection((TARGET_HOST, TARGET_PORT), timeout=10)
        upstream.settimeout(None)
    except Exception as e:
        print(f"[relay] upstream refused for {addr}: {e}", flush=True)
        client.close()
        return
    print(f"[relay] {addr} connected", flush=True)
    if first:
        upstream.sendall(first)
    t = threading.Thread(target=pump, args=(upstream, client), daemon=True)
    t.start()
    pump(client, upstream)
    t.join(timeout=5)
    upstream.close()
    client.close()


def local_ips():
    ips = []
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ips.append(s.getsockname()[0])
        s.close()
    except Exception:
        pass
    return ips


def main():
    global LISTEN_PORT, TARGET_HOST, TARGET_PORT
    LISTEN_PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 3426
    TARGET_HOST = sys.argv[2] if len(sys.argv) > 2 else "127.0.0.1"
    TARGET_PORT = int(sys.argv[3]) if len(sys.argv) > 3 else 3425
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("0.0.0.0", LISTEN_PORT))
    srv.listen(16)
    print(f"[relay] 0.0.0.0:{LISTEN_PORT} -> {TARGET_HOST}:{TARGET_PORT}", flush=True)
    for ip in local_ips():
        print(f"[relay] ESP32 should use http://{ip}:{LISTEN_PORT}", flush=True)
    while True:
        c, a = srv.accept()
        threading.Thread(target=handle, args=(c, a), daemon=True).start()


if __name__ == "__main__":
    main()
