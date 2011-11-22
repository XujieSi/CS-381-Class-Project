#!/bin/bash

# copy the files to the ns-3 repository location

if [ -z "${NS3_REPOS}" ] ; then
export NS3_REPOS=/opt/ns3/repos
fi
export NS3_HOME="${NS3_REPOS}/ns-3-allinone/ns-3-dev"

function sim-source() {
   source ./sims.sh
}


function sim-sync() {
  EXAMPLE=./ns-3-dev/src/simple-wireless-tdma/examples/
  rm ${NS3_HOME}/scratch/tdma-example.cc
  rm ${NS3_HOME}/scratch/tdmaSlots.txt
  ln ${EXAMPLE}/tdma-example.cc ${NS3_HOME}/scratch/tdma-example.cc
  ln ${EXAMPLE}/tdmaSlots.txt ${NS3_HOME}/scratch/tdmaSlots.txt
}

function sim-strict() {
  EXAMPLE=./ns-3-dev/src/simple-wireless-tdma/examples/
  pushd "${NS3_HOME}"
  ./waf --run "scratch/tdma-example ${1}";
  popd
}

cat <<EOM
This script is used to run the project simulations.

- sim-sync : sync up the source code 
- sim-strict : runs the test application
- sim-args : runs the test application with specified args

A sample of running these can be found in 'configure-ns3.txt' 
EOM

