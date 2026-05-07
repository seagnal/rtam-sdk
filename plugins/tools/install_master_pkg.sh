echo "install_master_pkg.sh : $RTAM_EVAL_VERSION"
version=$RTAM_EVAL_VERSION


# Désinstallation si l'argument est --remove
if [[ "$1" == "--remove" ]]; then
  echo "Désinstallation des paquets RTAM ..."
  read -p " > Êtes-vous sûr de vouloir continuer ? (Y/N) : " confirmation
  if [[ "$confirmation" == "y" || "$confirmation" == "Y" ]]; then
          echo "Désinstallation en cours ..."

	  sudo dpkg -P master-standalone
	  sudo dpkg -P plugin-rtam-rtam-obfuscated
	  sudo dpkg -P plugin-rtam-eval-rtam-obfuscated
	  sudo dpkg -P bml-parser-rtam
	  #sudo dpkg -P master-rtam-obfuscated
    #sudo dpkg -P plugin-rtam-eval-linear
    #sudo dpkg -P plugin-rtam-eval-sample

	  #sudo rm -R /usr/share/master
	  #sudo rm -R /usr/lib/master
    sudo rm /etc/udev/rules.d/99-master-udev-net-config.rules
    sudo netplan apply
	  echo "Désinstallation des paquets RTAM terminée !"
	  return 1 2>/dev/null || exit 1
   else
        echo "Désinstallation annulée !"
   	return 1 2>/dev/null || exit 1
   fi
fi


  sudo apt-get update
  sudo apt-get upgrade
  sudo apt-get install octave libboost-program-options1.74.0 libmatio-dev

  sudo dpkg -i master-standalone-$version-amd64-jammy.deb
  sudo dpkg -i plugin-rtam-rtam-$version-amd64-jammy.deb
  sudo dpkg -i plugin-rtam-eval-rtam-$version-amd64-jammy.deb
  sudo dpkg -i bml-parser-rtam-$version-amd64-jammy.deb
  #sudo dpkg -i plugin-rtam-eval-sample-$version-amd64-jammy.deb
  #sudo dpkg -i plugin-rtam-eval-linear-$version-amd64-jammy.deb
  sudo apt install -f


  # Get the mac address of the first ethernet adaptater and configure the PC to rename the interface
  mac=`ip a s | grep link/ether | tr -s ' ' | cut -d ' ' -f3 | head -n 1`;

  sudo cp /etc/udev/rules.d/99-master-udev-net-config.rules.example /etc/udev/rules.d/99-master-udev-net-config.rules
  sudo sed -i "s/00:40:45:3a:72:f9/${mac}/g" /etc/udev/rules.d/99-master-udev-net-config.rules
  sudo sed -i "s/#//g" /etc/udev/rules.d/99-master-udev-net-config.rules
  echo "Mac address replacement is successfull into file /etc/udev/rules.d/99-master-udev-net-config.rules"
  echo "Card with Mac address ${mac} will be renamed to ethm"

  sudo chmod 600 /etc/netplan/01-network-manager-all.yaml
  sudo chmod 600 /etc/netplan/master-net.yaml

  sudo netplan apply
  sudo mkdir -p /root/.rtam/licenses
  sudo cp *.lic /root/.rtam/licenses

  sudo chmod +r /usr/share/master/octave/bml-parser/*.m
