#!/bin/sh
#
# Start/stop ipc-daemon
#

[ -f /usr/bin/ipc-daemon ] || exit 0
PIDFILE=/var/run/ipc-daemon.pid

case "$1" in
  start)
	echo "Starting ipc-daemon..."
	mkdir -p /var/log/ipc-daemon /var/tmp/ipc-daemon
	start-stop-daemon -S -b -x ipc-daemon -p "$PIDFILE"
	;;
  stop)
	echo "Stopping ipc-daemon..."
	start-stop-daemon -K -x ipc-daemon -p "$PIDFILE" -o
	;;
  reload|force-reload)
	echo "Reloading ipc-daemon configuration..."
	ipc-daemon -s reload
	;;
  restart)
	"$0" stop
	sleep 1 # Prevent race condition: ensure ipc-daemon stops before start.
	"$0" start
	;;
  *)
	echo "Usage: $0 {start|stop|restart|reload|force-reload}"
	exit 1
esac
