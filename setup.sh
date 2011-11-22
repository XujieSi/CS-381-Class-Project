#!/bin/bash

# copy the files to the ns-3 repository location

if [ -z "${NS3_REPOS}" ] ; then
export NS3_REPOS="/opt/ns3/repos/ns-3-dev"
fi
export NS3_HOME="${NS3_REPOS}"

# missing texlive-pdf

function ns3-source() {
   source ./setup.sh
}

function ns3-dependancies() {
  sudo apt-get install \
    gcc g++ python python-dev mercurial bzr gdb valgrind gsl-bin libgsl0-dev libgsl0ldbl \
    flex bison tcpdump sqlite sqlite3 libsqlite3-dev libxml2 libxml2-dev libgtk2.0-0 libgtk2.0-dev \
    uncrustify doxygen graphviz imagemagick \
    texlive texlive-latex-extra texlive-generic-extra texlive-generic-recommended \
    texinfo dia texlive texlive-latex-extra texlive-extra-utils texlive-generic-recommended texi2html \
    python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev \
    libboost-signals-dev libboost-filesystem-dev

 sudo apt-get install python-pygccxml gccxml
}

function ns3-get() {
  mkdir -p ${NS3_REPOS}
  pushd ${NS3_REPOS}
  hg clone 'http://code.nsnam.org/ns-3-dev' 'ns-3-dev'
  popd
}

function ns3-update() {
  mkdir -p ${NS3_REPOS}
  pushd ${NS3_REPOS}
  hg update
  popd
}

function ns3-configure() {
  pushd "${NS3_HOME}"
  ./waf configure --enable-examples --enable-tests --doxygen-no-build
  popd
}

function ns3-build() {
  pushd "${NS3_HOME}"
  ./waf 
  popd
}

function ns3-install() {
  pushd "${NS3_HOME}"
  # ./waf doxygen
  sudo ./waf install
  popd
}

function ns3-test() {
  pushd "${NS3_HOME}"
  ./test.py 
  popd
}

cat <<EOM
Before sourcing this script set the NS3_REPOS environment 
parameter otherwise it will default to ${NS3_REPOS}.
The functions in this script can be made available 
to the interactive command shell by running
> source setup.sh

Doing so provides the following (which you should run in order):
- ns3-source : same as 'source setup.sh'
- ns3-dependancies : installs the ubuntu packages needed by ns-3
- ns3-get : prepares a repos directory in the NS3_HOME 
- ns3-configure : configure ns3 for tests and examples
- ns3-link : makes links from the source files into the ns-3-dev tree
- ns3-build : build ns3 
- ns3-install : install ns3 
- ns3-test : build ns3 
- ns3-unlink : reverses what linkup does (it removes the symbolic links)

A sample of running these can be found in 'configure-ns3.txt' 
EOM

function ns3-link() {
CWD=$(pwd)
rm -f ${NS3_HOME}/src/simple-wireless-tdma 
ln -f -s ${CWD}/ns-3-dev/src/simple-wireless-tdma ${NS3_HOME}/src/simple-wireless-tdma 

rm -f ${NS3_HOME}/src/serial-tdma
#ln -f -s ${CWD}/ns-3-dev/src/serial-tdma ${NS3_HOME}/src/serial-tdma

rm -f ${NS3_HOME}/examples/wireless 
ln -f -s ${CWD}/ns-3-dev/examples/wireless ${NS3_HOME}/examples/wireless 

ln -f ${CWD}/ns-3-dev/tdma.h ${NS3_HOME}/tdma.h
meld ns-3-dev/src/wifi/wscript ${NS3_HOME}/src/wifi/wscript
}

