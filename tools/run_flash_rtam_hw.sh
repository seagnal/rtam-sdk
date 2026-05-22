#!/bin/bash
##################################################
####  FLASH HW - RTAM MODULE  ####################
##################################################

MAC=$1
if [ -z "$MAC" ]; then
  MAC=82
fi
echo ""
echo " # MAC Address : $MAC"
echo ""


CURRENT_DIR=$(pwd)
PROJECT_DIR="../project"

HW_CONFIG_DIR=$(dirname "$CURRENT_DIR")
echo " # HW CONFIG directory : $HW_CONFIG_DIR"
echo " # Current directory   : $CURRENT_DIR"

echo ""

# Compte les fichiers .tcl
tcl_count=$(find "$CURRENT_DIR" -type f -name '*.tcl' | wc -l)
project_count=$(ls -d "$PROJECT_DIR"/*/ | wc -l)

# Test : on attend *au moins* un fichier
if [ "$tcl_count" -lt 1 ]; then
    echo "❌ Erreur : aucun fichier .tcl trouvé dans '$CURRENT_DIR'." >&2
    exit 1
fi

# Test : on attend *au moins* un fichier
if [ "$project_count" -lt 1 ]; then
    echo "❌ Erreur : aucun fichier .tcl trouvé dans '$PROJECT_DIR'." >&2
    exit 1
fi

# Récupérer le fichier .tcl
TCL_FILE=$(find "$CURRENT_DIR/" -name "*.tcl" | head -1)
TCL_NAME=$(basename "$TCL_FILE" | sed 's/[0-9]*\.tcl$//')
echo " # TCL FILE      : $TCL_FILE"
echo " # TCL FILE NAME : $TCL_NAME"

echo ""

# Récupérer le nom du projet dynamiquement
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
