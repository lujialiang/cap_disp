ABI=$(adb shell getprop ro.product.cpu.abi | tr -d '\r')
SDK=$(adb shell getprop ro.build.version.sdk | tr -d '\r')

MINICAP_PATH=/home/hisino/Desktop/ivsc_20161105_v5
echo ABI $ABI
echo SDK $SDK

adb push $MINICAP_PATH/$ABI/ivscreen /data/local/tmp/
adb push $MINICAP_PATH//$ABI/libminicap-common.so /data/local/tmp/
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/ivscreen -n minicap & #-S -P 1720x1200@480x800/90 &

# LJL 
adb forward tcp:9002 localabstract:minicap


./sdl_sim #2>/dev/null 1>/dev/null

MINICAP_PS=$(adb shell "ps | grep ivscreen")
MINICAP_PID=$(echo $MINICAP_PS | sed 's/[^0-9]*\([0-9]\+\).*/\1/')
echo $MINICAP_PID
adb shell kill $MINICAP_PID

MINITOUCH_PS=$(adb shell "ps | grep hsntouch")
MINITOUCH_PID=$(echo $MINITOUCH_PS | sed 's/[^0-9]*\([0-9]\+\).*/\1/')
echo $MINITOUCH_PID
adb shell kill $MINITOUCH_PID

