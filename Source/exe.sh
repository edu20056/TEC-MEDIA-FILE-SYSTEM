#!/bin/bash

echo "Compiling..."
make || { echo "!ERROR: Compilation failed..."; exit 1; }

CONTROLLER_EXEC="./TECMFS-CTRL"
GUI_EXE="./TECMFS-GUI"
NODE_EXEC="./TECMFS-NODE"

echo "Starting CTRL..."
gnome-terminal -- bash -c "$CONTROLLER_EXEC Config/CtrlConfig.xml; exec bash"

echo "Starting NODEs..."
for i in {1..4}; do
    DISK_CONFIG="Config/DiskNodeConfig$i.xml"
    gnome-terminal -- bash -c "$NODE_EXEC $DISK_CONFIG; exec bash"
done

sleep 2

echo "Starting GUI..."
gnome-terminal -- bash -c "$GUI_EXE; exec bash"