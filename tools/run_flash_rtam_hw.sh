##################################################
####  FLASH HW - RTAM MODULE  ####################
##################################################
# Ce script doit être dans le dossier 'tools' :  ~/RTAM/rtam-<version>/hw_config_1x_eval_tag_rtam-<version>-<extension>/tools"
# Ce script doit être dans le dossier 'tools' :  ~/RTAM/despot-<version>/hw_config_eval-top_tag_despot-<version>-<extension>/tools"

#!/bin/bash


MAC=$1
if [ -z "$MAC" ]; then
  MAC=82
fi
echo ""
echo " # MAC Address : $MAC"
echo ""


CURRENT_DIR=$(pwd)
HW_CONFIG_DIR=$(dirname "$CURRENT_DIR")
echo " # HW CONFIG directory : $HW_CONFIG_DIR"
echo " # Current directory   : $CURRENT_DIR"

echo ""

# Vérifie qu'on est bien dans un dossier "tools" de RTAM
if [[ "$CURRENT_DIR" != *"/RTAM/"*"/tools" ]]; then
    echo -e "\n/!\ Erreur : ce script doit être lancé depuis un dossier 'tools' : ~/RTAM/rtam-<version>/hw_config_1x_eval_tag_rtam-<version>-<extension>/tools"
    echo -e "/!\ Erreur : --------------------------------------------------- : ~/RTAM/despot-<version>/hw_config_eval-top_tag_rtam-<version>-<extension>/tools\n"
    exit 1
fi

# Récupérer le fichier .tcl
TCL_FILE=$(find "$CURRENT_DIR/" -name "*.tcl" | head -1)
TCL_NAME=$(basename "$TCL_FILE" | sed 's/[0-9]*\.tcl$//')
echo " # TCL FILE      : $TCL_FILE"
echo " # TCL FILE NAME : $TCL_NAME"

echo ""

# Récupérer le nom du projet dynamiquement
PROJECT_DIR="../project"
PROJECT_NAME=$(ls -d "$PROJECT_DIR"/*/ | head -1 | xargs basename)
PROJECT_DIR="$PROJECT_DIR/$PROJECT_NAME"
echo " # PROJECT directory : $PROJECT_DIR"
echo " # PROJECT NAME      : $PROJECT_NAME"

echo ""

# Récupérer le fichier .bit
BIT_FILE=$(find "$PROJECT_DIR/$PROJECT_NAME.runs/impl_1/" -name "*.bit" | head -1)
BIT_NAME=$(basename "$BIT_FILE" | sed 's/[0-9]*\.bit$//')
echo " # BIT FILE      : $BIT_FILE"
echo " # BIT FILE NAME : $BIT_NAME"

echo ""

# Charge l'environnement Vivado Lab
rm vivado_lab*
rm -R .Xil

echo ""

echo "---------- CONFIGURATION ----------"
echo "HW DIR       : $HW_CONFIG_DIR"
echo "PROJECT NAME : $PROJECT_NAME "
echo "BIT FILE     : $BIT_NAME$MAC "
echo "TCL FILE     : $TCL_NAME$MAC "
echo "BIT FILE     : $BIT_NAME$MAC "
echo "-----------------------------------"

# Affichage de la commande Vivado Lab
echo "vivado_lab -mode batch -source "$TCL_NAME$MAC".tcl -tclargs $PROJECT_NAME "$BIT_NAME$MAC" "$HW_CONFIG_DIR""

# Exécute la commande Vivado Lab en mode batch
vivado_lab -mode batch -source "$TCL_NAME$MAC".tcl \
    -tclargs $PROJECT_NAME "$BIT_NAME$MAC" "$HW_CONFIG_DIR"

echo ""
echo "Fin du script !"
echo ""
