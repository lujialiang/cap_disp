MINICAP_PATH=../minicap-800x480
MINITOUCH_PATH=../minitouch
ABI=$(adb shell getprop ro.product.cpu.abi | tr -d '\r')
SDK=$(adb shell getprop ro.build.version.sdk | tr -d '\r')

echo ABI $ABI
echo SDK $SDK

adb push $MINICAP_PATH/libs/$ABI/minicap /data/local/tmp/
adb push $MINICAP_PATH/libs/$ABI/mini_info /data/local/tmp/
adb push $MINICAP_PATH/libs/$ABI/mini_info_ex /data/local/tmp/
adb push $MINICAP_PATH/jni/minicap-shared/aosp/libs/android-$SDK/$ABI/minicap.so /data/local/tmp/

adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/minicap -z 600@1280  & #-P 720x1280@480x800/0 &

adb forward tcp:9002 localabstract:minicap

#adb push $MINITOUCH_PATH/libs/$ABI/hsntouch /data/local/tmp/
#adb shell /data/local/tmp/hsntouch &
#adb forward tcp:9003 localabstract:hsntouch

./sdl_sim 2>/dev/null 1>/dev/null

MINICAP_PS=$(adb shell "ps | grep minicap")
MINICAP_PID=$(echo $MINICAP_PS | sed 's/[^0-9]*\([0-9]\+\).*/\1/')
echo $MINICAP_PID
adb shell kill $MINICAP_PID

MINITOUCH_PS=$(adb shell "ps | grep hsntouch")
MINITOUCH_PID=$(echo $MINITOUCH_PS | sed 's/[^0-9]*\([0-9]\+\).*/\1/')
echo $MINITOUCH_PID
adb shell kill $MINITOUCH_PID
