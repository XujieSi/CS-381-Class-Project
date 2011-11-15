!#/bin/bash

# copy the files to the ns-3 repository location

export NS3_HOME=/opt/network-study/repos/ns-3-allinone/ns-3-dev

function linkup() {
ln -s ns-3-dev/src/simple-wireless-tdma ${NS2_HOME}/src/simple-wireless-tdma 
ln -s ns-3-dev/examples/wireless ${NS2_HOME}/examples/wireless/wifi-tdma.cc 
ln -s ns-3-dev/src/wifi/wscript ${NS2_HOME}/src/wifi/wscript
ln -s ns-3-dev/tdma.h ${NS2_HOME}/tdma.h
}

function unlinkup() {
rm ${NS2_HOME}/src/simple-wireless-tdma 
rm ${NS2_HOME}/examples/wireless/wifi-tdma.cc 
rm ${NS2_HOME}/src/wifi/wscript
rm ${NS2_HOME}/tdma.h
}


linkup


