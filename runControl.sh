#!/bin/bash

if [ -z "$1" ]; then
 exit
fi

IP=192.168.10.$1
YAML_DIR=../../ELPH.2020.07/citiroc_param
YAML1=$YAML_DIR/RegisterValue_$1.yml
YAML2=$YAML_DIR/InputDAC_$1.yml
YAML3=$YAML_DIR/DiscriMask_$1.yml

OPT1=$2
OPT2=$3
OPT3=$4

echo ./bin/veasiroc_control -ip=$IP -yaml=$YAML1 -yaml=$YAML2 -yaml=$YAML3 $OPT1 $OPT2 $OPT3
./bin/veasiroc_control -ip=$IP -yaml=$YAML1 -yaml=$YAML2 -yaml=$YAML3 $OPT1 $OPT2 $OPT3
