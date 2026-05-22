#!/bin/bash
set -e
mkdir -p /rtam/RUN
pushd /rtam/RUN
LD_LIBRARY_PATH=/usr/share/master:/usr/lib/plugin-rtam:/usr/lib/master:/usr/lib/nvidia-384 MODULE_PATH=/usr/share/master\;/usr/share/plugin-rtam-eval\;/usr/share/plugin-rtam\;/usr/share/plugin-transport-rawsocket MASTER_CONFIG=/rtam/plugins/config/$1.xml master
popd