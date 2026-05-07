#!/bin/bash
set -e

MAC_ADDRESS="cc:aa:ee:00:00:00"
MTU="8192"
INTERFACE=$2

echo Generating /tmp/rtam.xml with proper device $INTERFACE

cp /rtam/plugins/config/$1.xml /tmp/rtam.xml
sed -i "s/ethm/$INTERFACE/g" /tmp/rtam.xml

echo Configuring device $INTERFACE with mac:$MAC_ADDRESS and mtu:$MTU
# Bring the interface down
ifconfig $INTERFACE down
# Set MAC address
ifconfig $INTERFACE hw ether $MAC_ADDRESS
# Set MTU
ifconfig $INTERFACE mtu $MTU
# Bring the interface up
ifconfig $INTERFACE up


mkdir -p /rtam/RUN
pushd /rtam/RUN
LD_LIBRARY_PATH=/usr/share/master:/usr/lib/plugin-rtam:/usr/lib/master:/usr/lib/nvidia-384 MODULE_PATH=/usr/share/master\;/usr/share/plugin-rtam-eval\;/usr/share/plugin-rtam\;/usr/share/plugin-transport-rawsocket MASTER_CONFIG=/tmp/rtam.xml master
popd