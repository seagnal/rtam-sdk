#!/bin/bash
set -e

pathdebs=$1
version=$2

echo "RTAM_EVAL_VERSION : $version"

echo "Installing Octave, liboost and matio"
apt-get install -y libboost-program-options1.74.0 libboost-thread1.74.0 libboost-filesystem1.74.0 libmatio11 libusb-1.0-0

echo "Installing all rtam packages"

cd $pathdebs
dpkg -i master-standalone-$version-amd64-jammy.deb
dpkg -i plugin-rtam-rtam-$version-amd64-jammy.deb
dpkg -i plugin-rtam-eval-rtam-$version-amd64-jammy.deb
dpkg -i plugin-rtam-eval-sample-$version-amd64-jammy.deb
dpkg -i bml-parser-rtam-tools-$version-amd64-jammy.deb

#FIXME dpkg -i plugin-rtam-eval-linear-$version-amd64-jammy.deb
#FIXME dpkg -i rtam-eval-client-rtam-service-$version-amd64-jammy.deb

apt install -f