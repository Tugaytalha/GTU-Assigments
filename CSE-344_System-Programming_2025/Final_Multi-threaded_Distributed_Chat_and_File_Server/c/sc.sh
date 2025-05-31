#!/usr/bin/env bash
# Start the chat server and stress-test the upload queue.
# ------------------------------------------------------
# Requirements:
#   - chatserver  : already built (./chatserver <port>)
#   - chatclient  : already built (./chatclient <server_ip> <port>)
#   - test file   : ./test_files/large.jpg  (≤3 MB, any allowed type)
#
# What it does:
#   1. Starts chatserver in the background.
#   2. Waits until the port is open.
#   3. Fires 10 clients *at once*; each:
#         • logs in with a unique username,
#         • immediately runs /sendfile,
#         • stays connected a few extra seconds to keep
#           the slot busy (so later uploads must queue),
#         • exits cleanly.
#   4. Waits for every client to finish, prints where the
#      logs are, and stops the server.

set -euo pipefail

########################################
# >>> CONFIGURABLE SECTION  <<<
########################################
PORT=12345
SERVER_EXEC=./chatserver
CLIENT_EXEC=./chatclient
FILE=./test_files/large.jpg       # must exist and be ≤3 MB (or auto‐created below)
TARGET_USER=receiver1             # the recipient username (must exist or be connected)
TOTAL_CLIENTS=10                  # number of simultaneous clients to launch
HOLD_CONN_SEC=1                # how long each client stays connected after /sendfile
CLIENT_TIMEOUT=$((HOLD_CONN_SEC + 4))  # if a client hangs, kill it after this many seconds
LOGDIR=./test_logs
########################################

mkdir -p "$LOGDIR"

# --------------------------------------------------------------------
# 0. Make sure the test file is present; if not, create a 2 MB dummy.
# --------------------------------------------------------------------
if [[ ! -f "$FILE" ]]; then
  echo "[$(date +%T)] Test file $FILE not found – creating 2 MB dummy."
  mkdir -p "$(dirname "$FILE")"
  dd if=/dev/urandom of="$FILE" bs=2M count=1 status=none
fi

#------------------------------------------------------------------------
# 1. Start the chatserver in the background and redirect logs:
#------------------------------------------------------------------------
echo "[$(date +%T)] Starting chatserver on port $PORT …"
"$SERVER_EXEC" "$PORT" >"$LOGDIR/server.out" 2>&1 &
SERVER_PID=$!

# Ensure we kill the server on exit, even if the script is interrupted:
cleanup() {
  echo -e "\n[$(date +%T)] Cleaning up: killing server (PID $SERVER_PID)."
  kill -TERM "$SERVER_PID" 2>/dev/null || true
}
trap cleanup EXIT

# --------------------------------------------------------------------
# 2. Wait until the server’s TCP port is listening:
#------------------------------------------------------------------------
printf "[$(date +%T)] Waiting for server to accept connections"
until nc -z 127.0.0.1 "$PORT" 2>/dev/null; do
  printf "."
  sleep 0.2
done
echo " ready!"

# 2.5 Launch the receiver
echo "[$(date +%T)] Starting receiver..."
echo $TARGET_USER | "$CLIENT_EXEC" 127.0.0.1 "$PORT" >"$LOGDIR/receiver.out" 2>&1 &
RECEIVER_PID=$!

#------------------------------------------------------------------------
# 3. Launch all clients *concurrently*, each under a timeout:
#------------------------------------------------------------------------
echo "[$(date +%T)] Firing $TOTAL_CLIENTS clients under timeout …"

for i in $(seq 1 "$TOTAL_CLIENTS"); do
  USER="user$i"

  # Build a small script that:
  #   1. Sends the username
  #   2. Sleeps 0.5s to let login settle
  #   3. Sends “/sendfile …”
  #   4. Sleeps HOLD_CONN_SEC to occupy the upload slot
  #   5. Sends “/exit”
  #
  # Then pipe that into “timeout $CLIENT_TIMEOUT chatclient …”
  {
    echo "$USER"
    sleep 0.5
    echo "/sendfile $FILE $TARGET_USER"
    sleep "$HOLD_CONN_SEC"
    echo "/exit"
  } | timeout "$CLIENT_TIMEOUT" "$CLIENT_EXEC" 127.0.0.1 "$PORT" \
      >"$LOGDIR/client_${USER}.out" 2>&1 &

  # (No sleep here—fire them all “at once” for max concurrency.)
done


sleep 8
trap 'echo -e "\nStopping server (PID $SERVER_PID)." && kill -TERM $SERVER_PID' EXIT

# --------------------------------------------------------------------
# 4. Wait for every background job (clients) to finish.
# --------------------------------------------------------------------
wait
echo "[$(date +%T)] All clients finished."

# --------------------------------------------------------------------
# 5. Show where to look for evidence of queuing.
# --------------------------------------------------------------------
echo
echo "=== Test complete ==="
echo " • Server log:   $LOGDIR/server.out"
echo " • Client logs:  $LOGDIR/client_user*.out"
echo
echo "Search the server log for lines such as:"
echo "   [FILE-QUEUE] Upload 'large.jpg' from user6 added to queue."
echo "   [FILE] 'large.jpg' from user9 started upload after N seconds in queue."
echo
echo "You should see the first five uploads start immediately and"
echo "the remaining uploads enter the queue, then start after slots free up."
