#!/bin/bash

# copy the files to the ns-3 repository location

if [ -z "$NS3_HOME" ] ; then
export NS3_HOME=/opt/network-study/repos/ns-3-allinone/ns-3-dev
fi

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


function install_dependancies() {
  sudo apt-get install gcc g++ python python-dev mercurial bzr gdb valgrind gsl-bin libgsl0-dev libgsl0ldbl \
    flex bison tcpdump sqlite sqlite3 libsqlite3-dev libxml2 libxml2-dev libgtk2.0-0 libgtk2.0-dev \
    uncrustify doxygen graphviz imagemagick \
    texlive texlive-pdf texlive-latex-extra texlive-generic-extra texlive-generic-recommended \
    texinfo dia texlive texlive-latex-extra texlive-extra-utils texlive-generic-recommended texi2html \
    python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev \
    libboost-signals-dev libboost-filesystem-dev
}

function clone_ns3() {
  mkdir repos
  cd repos
  hg clone 'http://code.nsnam.org/ns-3-allinone'
  cd 'ns-3-dev'
}

function configure_ns3() {
  ./waf configure --enable-examples --enable-tests --doxygen-no-build
}

function build_ns3() {
  ./waf 
  # ./waf doxygen
  # ./waf install
}

function test_ns3() {
  ./test.py 
}


cat <<EOM
Before sourcing this script set the NS3_HOME environment 
parameter otherwise it will default to ${NS3_HOME}.
The functions in this script can be made available 
to the interactive command shell by running
> source setup.sh

Doing so provides the following (which you should run in order):
- install_dependancies : installs the ubuntu packages needed by ns-3
- clone_ns3 : prepares a repos directory in the NS3_HOME 
- configure_ns3 : configure ns3 for tests and examples
- build_ns3 : build ns3 
- test_ns3 : build ns3 
- linkup : makes links from the source files into the ns-3-dev tree
- unlinkup : reverses what linkup does (it removes the symbolic links)

A sample of running these can be found in 'configure-ns3.txt' 
EOM

