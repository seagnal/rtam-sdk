
# Vérification qu'un fichier .deb existe
filename=$(ls *.deb 2>/dev/null | head -1)
if [[ -z "$filename" ]]; then
    echo "Erreur: aucun fichier .deb trouvé"
    exit 1
fi

# Récupération de la version
version_auto=$(echo "$filename" | grep -oP '\d+\.\d+-\d+-g[a-f0-9]+')

# Vérification que la version a été trouvée
if [[ -z "$version_auto" ]]; then
    echo "Erreur: version non trouvée dans '$filename'"
    exit 1
fi
#version="${1:-$version_auto}"
version=$version_auto

# Recherche de tous les paquets contenant 'rtam' et 'master-' installés
rtam_packages=$(dpkg -l | grep -i "rtam" |grep "^ii" | awk '{print $2}')
master_packages=$(dpkg -l | grep -i "master-" |grep "^ii" | awk '{print $2}')
if [[ -z "$rtam_packages" ]]; then
  echo "Aucun paquet RTAM trouvé sur le système."
else
  echo "Paquets RTAM actuellement installés sur le systèmes :"
  for pkg in $rtam_packages; do
    version=$(dpkg -l "$pkg" | grep "^ii" | awk '{print $3}')
    echo " - $pkg ($version)"
  done
fi
echo ""
if [[ -z "$master_packages" ]]; then
  echo "Aucun paquet MASTER trouvé sur le système."
else
  echo "Paquets RTAM actuellement installés sur le systèmes : "
  for pkg in $master_packages; do
    version=$(dpkg -l "$pkg" | grep "^ii" | awk '{print $3}')
    echo " - $pkg ($version)"
  done
fi
echo ""

echo "install_master_pkg : $version"


# Désinstallation si l'argument est --remove
if [[ "$1" == "--remove" ]]; then
  echo "Désinstallation des paquets RTAM ..."
  read -p " > Êtes-vous sûr de vouloir continuer ? (Y/N) : " confirmation
  if [[ "$confirmation" == "y" || "$confirmation" == "Y" ]]; then
          echo "Désinstallation en cours ..."

	  sudo dpkg -P rtam-eval-client-rtam-service
	  sudo dpkg -P plugin-rtam-rtam #-obfuscated
	  sudo dpkg -P plugin-rtam-eval-rtam #-obfuscated
	  #sudo dpkg -P plugin-rtam-eval-linear-rtam #-obfuscated
    #sudo dpkg -P plugin-rtam-eval-sample #-obfuscated
    sudo dpkg -P master-standalone
    sudo dpkg -P master-rtam-service
	  sudo dpkg -P master-rtam #-obfuscated
    sudo dpkg -P debug-jtag-rtam #-obfuscated
    sudo dpkg -P bml-parser-rtam

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


# Installation
  echo "Installation des paquets RTAM ..."
  sudo apt-get update
  sudo apt-get upgrade
  sudo apt-get install octave libboost-program-options1.74.0 libmatio-dev
 
  #sudo dpkg -i rtam-eval-client-rtam-service-$version-amd64-jammy.deb
  sudo dpkg -i plugin-rtam-rtam-$version-amd64-jammy.deb #-obfuscated
  sudo dpkg -i plugin-rtam-eval-rtam-$version-amd64-jammy.deb #-obfuscated
  #sudo dpkg -i plugin-rtam-eval-linear-rtam-$version-amd64-jammy.deb #-obfuscated
  #sudo dpkg -i plugin-rtam-eval-sample-rtam-$version-amd64-jammy.deb #-obfuscated
  sudo dpkg -i master-standalone-$version-amd64-jammy.deb
  #sudo dpkg -i master-rtam-service-$version-amd64-jammy.deb
  #sudo dpkg -i master-rtam-$version-amd64-jammy.deb #-obfuscated
  sudo dpkg -i debug-jtag-rtam-$version-amd64-jammy.deb #-obfuscated
  sudo dpkg -i bml-parser-rtam-$version-amd64-jammy.deb

echo ""
echo "=== Synthèse des paquets ==="
for deb in "$deb_dir"/*.deb; do
  pkg=$(dpkg-deb --field "$deb" Package)
  version_deb=$(dpkg-deb --field "$deb" Version)

  if dpkg -l "$pkg" 2>/dev/null | grep -q "^ii"; then
    version_installed=$(dpkg -l "$pkg" | grep "^ii" | awk '{print $3}')

    if [[ "$version_installed" == "$version_deb" ]]; then
      echo "✔ $pkg ($version_deb) : installation réussie"
    elif dpkg --compare-versions "$version_installed" lt "$version_deb"; then
      echo "↑ $pkg ($version_deb) : mis à jour (ancienne version : $version_installed)"
    else
      echo "⚠ $pkg ($version_deb) : version antérieure déjà présente ($version_installed)"
    fi
  else
    echo "✘ $pkg ($version_deb) : installation échouée ou non installé"
  fi
done
echo "============================"
  
  sudo dpkg -i
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
  sudo chmod +r /usr/share/master/octave/bml-parser/*.m

  # Gestion des licences
  sudo mkdir -p /root/.rtam/licenses
  sudo cp *.lic /root/.rtam/licenses
