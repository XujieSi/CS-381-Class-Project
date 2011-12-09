#!/bin/bash
nodes=2
OneKB=8192
iniDR=1
appDR=1
CsmaDR=1


function RunTest {
echo NumofNodes $1 AppDataRate $2 CsmaDataRate $3 >> ExperimentData.txt
`./waf --run "scratch/test_csma --NumOfNodes=$1  --ns3::OnOffApplication::DataRate=$2 --ns3::CsmaChannel::DataRate=$3 --ns3::CsmaNetDevice::Mtu=65535"` 
./analyseAsciiTrace < test-csma-broadcast.tr >> ExperimentData.txt
}

function Node_App_Csma {
while [ $nodes -le 20 ]; do
	let appDR=iniDR
	echo num_of_nodes is $nodes
	App_Csma $nodes
	let nodes=nodes+2
done
}

function App_Csma {
while [ $appDR -le 10 ]; do
	let t_appdr=$[$appDR*$OneKB]
	echo t_appdr is $t_appdr
	let CsmaDR=iniDR
	Csma $1 $t_appdr
	let appDR=appDR+1
done
}

function Csma {
while [ $CsmaDR -le 10 ]; do
	let t_csmadr=$[$CsmaDR*$OneKB]
	echo t_csmadr is $t_csmadr
	RunTest $1 $2 $t_csmadr
	let CsmaDR=CsmaDR+1
done
}

Node_App_Csma 
