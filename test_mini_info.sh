MINICAP_PATH=~/Desktop/minicap
ABI=$(adb shell getprop ro.product.cpu.abi | tr -d '\r')
SDK=$(adb shell getprop ro.build.version.sdk | tr -d '\r')

echo ABI $ABI
echo SDK $SDK

adb push $MINICAP_PATH/libs/$ABI/mini_info /data/local/tmp/
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mini_info

