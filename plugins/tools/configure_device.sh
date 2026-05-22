#!/bin/bash
ip link set dev $1 down
ip link set dev $1 name ethm
ethtool -s ethm speed $3 duplex full autoneg on
ip link set dev ethm mtu $2
ip link set dev ethm address cc:aa:ee:00:00:00
ip link set dev ethm up

