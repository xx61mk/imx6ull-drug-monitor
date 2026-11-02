#!/bin/sh
#
# 自检脚本：不需要开发板，用仿真后端把四条主链路跑一遍并断言结果。
#   1. 控温       温度进入 2~8℃，继电器按回差动作，且不误报
#   2. 报警       开门超时 / 碰撞 / 泄漏 / 超温 四类都能触发
#   3. 断网补传   断网窗口内入队，恢复后按序补传，最后队列清空
#   4. 断电恢复   强杀进程后重启，队列从文件恢复并补传
#
# 用法：  make && sh tools/verify.sh
#

set -u

BIN="$(cd "$(dirname "$0")/.." && pwd)/drug-monitor"
TMP="$(mktemp -d 2>/dev/null || echo /tmp/dm_verify)"
mkdir -p "$TMP"

PASS=0
FAIL=0

pass() { printf "  [PASS] %s\n" "$1"; PASS=$((PASS + 1)); }
fail() { printf "  [FAIL] %s\n" "$1"; FAIL=$((FAIL + 1)); }

check_has() {   # name pattern file
    if grep -q "$2" "$3" 2>/dev/null; then pass "$1"; else fail "$1 (missing: $2)"; fi
}

check_absent() {
    if grep -q "$2" "$3" 2>/dev/null; then fail "$1 (unexpected: $2)"; else pass "$1"; fi
}

num_of() {      # file key  -> 取 "key=数字" 里的数字，取不到返回空
    grep -o "$2=[0-9]*" "$1" 2>/dev/null | tail -1 | cut -d= -f2
}

check_num_gt() {  # name file key min
    v=$(num_of "$2" "$3")
    if [ -n "${v:-}" ] && [ "$v" -gt "$4" ]; then
        pass "$1 (= $v)"
    else
        fail "$1 (got '${v:-none}', want > $4)"
    fi
}

check_num_eq() {  # name file key expected
    v=$(num_of "$2" "$3")
    if [ -n "${v:-}" ] && [ "$v" -eq "$4" ]; then
        pass "$1 (= $v)"
    else
        fail "$1 (got '${v:-none}', want $4)"
    fi
}

last_temp() {
    grep -o 'T=[ ]*[0-9.]*' "$1" 2>/dev/null | tail -1 | sed 's/T=//; s/ //g'
}

echo "== build check =="
if [ ! -x "$BIN" ]; then
    echo "  drug-monitor not found, run 'make' first"
    exit 1
fi
echo "  binary: $BIN"

cd "$TMP" || exit 1

# ---------------------------------------------------------------- 1. 控温
echo
echo "== test 1: temperature control =="
"$BIN" --sim --scenario none --interval 100 --speed 10 --seconds 25 \
       --db ctrl.db > ctrl.log 2>&1

check_has   "cooler started"            "CLR=1"           ctrl.log
check_num_gt "relay switched at least twice" ctrl.log "relay_switches" 1

t=$(last_temp ctrl.log)
if [ -n "${t:-}" ]; then
    ok=$(awk -v v="$t" 'BEGIN { print (v >= 1.0 && v <= 9.0) ? 1 : 0 }')
    if [ "$ok" = "1" ]; then pass "final temp in control band (= $t)"; else fail "final temp out of range (= $t)"; fi
else
    fail "no temperature line in log"
fi

check_num_eq "no temp_high alarm in normal run" ctrl.log "alarm_high" 0
check_num_eq "no temp_low alarm in normal run"  ctrl.log "alarm_low"  0

samples=$(num_of ctrl.log "samples")
rows=$(num_of ctrl.log "rows")
if [ -n "${samples:-}" ] && [ "${samples:-0}" -ge 20 ]; then
    pass "samples recorded (= $samples)"
else
    fail "too few samples ('${samples:-none}')"
fi
if [ -n "${rows:-}" ] && [ "${rows:-0}" -ge 20 ]; then
    pass "rows persisted (= $rows)"
else
    fail "db rows too few ('${rows:-none}')"
fi

# ---------------------------------------------------------------- 2. 报警
echo
echo "== test 2: alarm coverage (door / tilt / leak / over-temp) =="
"$BIN" --sim --scenario demo --interval 100 --speed 10 --seconds 45 \
       --db alarm.db > alarm.log 2>&1

check_num_gt "door_open alarm fired"  alarm.log "alarm_door" 0
check_num_gt "tilt alarm fired"       alarm.log "alarm_tilt" 0
check_num_gt "leak alarm fired"       alarm.log "alarm_leak" 0
check_num_gt "over-temp alarm fired"  alarm.log "alarm_high" 0
check_has    "alarm event printed"    "\[ALARM\]"      alarm.log

# ---------------------------------------------------------------- 3. 断网补传
echo
echo "== test 3: offline cache and replay =="
"$BIN" --sim --scenario none --interval 200 --speed 5 --seconds 25 \
       --net-down 3:9 --db net.db > net.log 2>&1

check_num_gt "messages were cached while offline" net.log "queued"   0
check_num_gt "messages were replayed after link up" net.log "replayed" 0
check_num_eq "queue drained at exit"              net.log "pending"  0
check_has    "replay line printed"                "replay"           net.log

# ---------------------------------------------------------------- 4. 断电恢复
echo
echo "== test 4: power loss recovery =="
"$BIN" --sim --scenario none --interval 200 --speed 5 --seconds 30 \
       --net-down 0:9999 --db crash.db > crash1.log 2>&1 &
PID=$!
sleep 6
kill -9 "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
sleep 1

if [ -f uplink_queue.bin ]; then
    pass "queue file written before crash"
else
    fail "queue file missing, nothing to recover"
fi

"$BIN" --sim --scenario none --interval 200 --speed 5 --seconds 6 \
       --db crash.db > crash2.log 2>&1

check_has    "cached messages recovered on restart" "recovered" crash2.log
check_num_gt "recovered messages replayed"          crash2.log "replayed" 0
check_num_eq "queue empty after replay"             crash2.log "pending"  0
check_has    "database still opens after kill -9"   "ready"     crash2.log

# ---------------------------------------------------------------- summary
echo
echo "================ result ================"
printf "  pass: %d\n" "$PASS"
printf "  fail: %d\n" "$FAIL"
echo "  logs: $TMP"
echo "========================================"

[ "$FAIL" -eq 0 ] || exit 1
