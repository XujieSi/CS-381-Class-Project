#!/bin/bash

# copy the files to the ns-3 repository location

if [ -z "${NS3_REPOS}" ] ; then
export NS3_REPOS=/opt/ns3/repos
fi
export NS3_HOME="${NS3_REPOS}/ns-3-allinone/ns-3-dev"

# missing texlive-pdf

function sim-source() {
   source ./sims.sh
}

function sim-strict() {
  EXAMPLE=./ns-3-dev/src/simple-wireless-tdma/examples/
  ln -s  ${EXAMPLE}/tdma-example.cc ${NS3_HOME}/scratch/tdma-example.cc
  ln -s  ${EXAMPLE}/tdmaSlots.txt ${NS3_HOME}/scratch/tdmaSlots.txt
  pushd "${NS3_HOME}"
  ./waf --run=tdma-example 
  popd
}

cat <<EOM
This script is used to run the project simulations.

- sim-strict : installs the ubuntu packages needed by ns-3

A sample of running these can be found in 'configure-ns3.txt' 
EOM

