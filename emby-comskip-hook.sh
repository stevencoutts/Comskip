#!/bin/sh
# Emby Live TV recording post-process hook.
# Emby passes the recording path. This only accepts that path when it is
# under the recordings library, then waits for the host worker.
set -eu

LOG=/config/logs/comskip.log
QUEUE=/config/comskip/queue
ROOT=/mnt/recordings

log() {
  printf '%s hook: %s\n' "$(date '+%Y-%m-%dT%H:%M:%S')" "$*" >> "$LOG"
}

if [ "$#" -eq 0 ] || [ -z "$1" ]; then
  log "no path"
  exit 1
fi

if [ "$#" -eq 1 ]; then
  raw=$1
else
  raw=$*
fi
case $raw in
  \"*\")
    raw=${raw#\"}
    raw=${raw%\"}
    ;;
esac

real=$(readlink -f "$raw") || {
  log "cannot resolve $raw"
  exit 1
}
case $real in
  "$ROOT"|"$ROOT"/*) ;;
  *)
    log "refused $real"
    exit 1
    ;;
esac

base=${real%.*}
ext=$(printf '%s' "$real" | tr '[:upper:]' '[:lower:]')
case $ext in
  *.mp4)
    log "already mp4 $real"
    exit 0
    ;;
  *.ts) ;;
  *)
    log "ignored $real"
    exit 0
    ;;
esac

if [ ! -f "$real" ]; then
  if [ -f "$base.mp4" ]; then
    log "already swapped $base.mp4"
    exit 0
  fi
  log "missing $real"
  exit 1
fi

mkdir -p "$QUEUE"
id=$(date '+%s')-$$
printf '%s\n' "$real" > "$QUEUE/$id.job"
log "queued $id $real"

i=0
while [ "$i" -lt 21600 ]; do
  if [ -f "$QUEUE/$id.ok" ]; then
    log "ok $id"
    rm -f "$QUEUE/$id.job" "$QUEUE/$id.ok" "$QUEUE/$id.running"
    exit 0
  fi
  if [ -f "$QUEUE/$id.fail" ]; then
    log "fail $id"
    rm -f "$QUEUE/$id.job" "$QUEUE/$id.fail" "$QUEUE/$id.running"
    exit 1
  fi
  sleep 2
  i=$((i + 2))
done
log "timeout $id"
exit 1
