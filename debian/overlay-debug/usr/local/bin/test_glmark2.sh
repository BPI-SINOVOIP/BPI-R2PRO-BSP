#!/bin/sh

echo "running glmark2 for testing GPU!!"
echo "please use the 'test_glmark2.sh fullscreen' or 'test_glamrk2.sh offscreen'"

case "$1" in

fullscreen)
	/usr/local/bin/test_glmark2_fullscreen.sh
	;;
offscreen)
	/usr/local/bin/test_glmark2_offscreen.sh
	;;
*)
	/usr/local/bin/test_glmark2_normal.sh
	;;
esac
shift
