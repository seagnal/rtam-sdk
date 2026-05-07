#!/bin/bash
mkdir -p ~/RTAM
pushd ~/RTAM
LD_LIBRARY_PATH=/usr/share/master:/usr/lib/plugin-rtam:/usr/lib/master:/usr/lib/nvidia-384 MODULE_PATH=/usr/share/master\;/usr/share/plugin-rtam-eval\;/usr/share/plugin-rtam\;/usr/share/plugin-transport-rawsocket MASTER_CONFIG=/usr/share/master/config/rtam-eval.xml master
popd
