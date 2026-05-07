To use master software (to communicate with RTAM eval board and generate a record file), you need :
- a PC where to install UBUNTU 22.04 :
   * Follow detailed steps  : https://ubuntu.com/tutorials/install-ubuntu-desktop
   * a step 2, click on "check out our alternative downloads"
   * choose Ubuntu 22.04.4 Desktop (64 bit)
   * continue with proposed steps
   * restart
- open a terminal window (CTRL+ALT+T)
   * change directory (cd) to where you placed the .deb files
   * edit the script /usr/share/master/install_master_pkg.sh and replace the version accoding to the files .deb
   * in the terminal, execute
sudo bash /usr/share/master/install_master_pkg.sh
- connect the eval board with an Ethernet cable to the PC
- in the terminal, execute master software :
sudo TEST_FULL_SCALE=1000 ANTENNA=maquette16 TEST_RTAM=0 bash /usr/share/master/launch-rtam-eval.sh
- in the terminal, execute data plotting with octave
cd /usr/share/master/octave
octave-cli

RTAM_fast_analyse
