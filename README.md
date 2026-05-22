# rtam-sdk

A comprehensive Software Development Kit (SDK) for the RTAM Digitization Module is designed to empower developers to efficiently integrate, configure, and extend the capabilities of the module within their applications.

# Usage 

## Run and use rtam eval board (with Docker)

* Clone this git repository
git clone https://github.com/seagnal/rtam-sdk.git

* Move into the cloned repo
cd rtam-sdk

* Download sw version from
https://drive.google.com/drive/folders/1EsHkGC5Ct3mzHd5N81RAdkl6-Ldl9rlQ?usp=sharing
or other procedure available on www.seagnal.fr on Digitizer web page.

* Build your docker image with appropriate rtam version parameter: (0.0-112-g0f1613d)
```
docker-compose -f ./docker-compose-rtam-eval.yml build --build-arg RTAM_VERSION="0.0-112-g0f1613d" --build-arg UID=$(id -u) --build-arg GID=$(id -g)
```
use  --build-arg PROXY=xxxxx if you are behind a Proxy

* Configure you network device
```
sudo bash ./plugins/tools/configure_device.sh enxXXXX 8192 1000
```
where:
- enxXXXX is your device
- 8192 is the mtu (can be 1500 with no pulser or vga)
- 1000 is the speed of the ling (can be 100 with SGMII HW)

* Perform simple acquisition (need root user)
```
docker run -u 0 -it --cap-add=NET_ADMIN --net=host -v ".:/rtam" -v "/tmp:/tmp" --env-file ./docker-scripts/env-file-config-continuous-mtu-1500 rtam-sdk_rtam-eval bash /rtam/docker-scripts/launch.sh rtam-eval
```

* Perform sample loop test (need root user)
```
docker run -u 0 -it --cap-add=NET_ADMIN --net=host -v ".:/rtam" -v "/tmp:/tmp" --env-file ./docker-scripts/env-file-config-continuous-mtu-1500 rtam-sdk_rtam-eval bash /rtam/docker-scripts/launch.sh rtam-eval-sample
```

## Run and use rtam eval board (without Docker / ubuntu 22.04)

* Clone this git repository
git clone https://github.com/seagnal/rtam-sdk.git

* Move into the cloned repo
cd rtam-sdk

* Download sw version from
https://drive.google.com/drive/folders/1EsHkGC5Ct3mzHd5N81RAdkl6-Ldl9rlQ?usp=sharing
or other procedure available on www.seagnal.fr on Digitizer web page.

* Install all package with appropriate rtam version parameter ex: 0.0-96-g8dfa936
```
sudo bash ./docker-scripts/configure.sh . 0.0-96-g8dfa936
```

* Configure you network device
```
sudo bash ./plugins/tools/configure_device.sh enxXXXX 8192 1000
```
where:
- enxXXXX is your device
- 8192 is the mtu (can be 1500 with no pulser or vga)
- 1000 is the speed of the ling (can be 100 with SGMII HW)

* Perform simple acquisition (need root user)
```
sudo bash /rtam/docker-scripts/launch.sh rtam-eval
```

* Perform sample loop test (need root user)
```
sudo bash /rtam/docker-scripts/launch.sh rtam-eval-sample
```

# Docker build procedure (with docker)
If you want to quickly modify and try rtam-eval plugin or rtam-eval-client in order to improve or customose processing of recored data. 
* Generate the build docker image
```
docker-compose -f ./docker-compose-rtam-build.yml build --build-arg UID=$(id -u) --build-arg GID=$(id -g)
```
use  --build-arg PROXY=xxxxx if you are behind a Proxy

* adapt according to your need plugins/rtam-eval or plugins/rtam-eval-sample contents

* Build custom rtam-eval plugin on your side
```
docker run -it -v ".:/rtam" rtam_rtam-build scons config=rtam plugin-rtam-eval.modInstall
```
or 
```
docker run -it -v ".:/rtam" rtam_rtam-build scons config=rtam plugin-rtam-sample.modInstall
```

* You should obtain a deb file at the root of the repository

# Build a custom alternative of plugin-rtam (without docker / ubuntu 22.04)
1.	Install dependencies

apt-get install -y --no-install-recommends dpkg net-tools scons pkg-config python3-dev libusb-1.0-0-dev libboost-program-options1.74-dev libboost-thread1.74-dev libboost-filesystem1.74-dev libmatio-dev python3-pyparsing python3-termcolor python3-reportlab g++-9 gcc-9 git build-essential python3-distro libeigen3-dev python3-gdbm libboost-regex1.74-dev

2. adapt according to your need plugins/rtam-eval or plugins/rtam-eval-sample contents

3.	Build the plugins
```
scons config=rtam plugin-rtam-eval.modInstall
scons config=rtam plugin-rtam-sample.modInstall
```
4. You should obtain a deb file at the root of the repository