#!/bin/sh

script_dir=$(cd $(dirname $0); pwd -P)

if [ $# != 3 ]; then
    echo "Usage : `basename $0` [EASIROC#] [PEAK_INTERVAL] [PHOTON]"
    echo "EASIROC# : 1) easiroc11 for SCH 00-31"
    echo "           2) easiroc12 for SCH 32-63"
    echo "           3) easiroc13 for FBH"
else
    easiroc=easiroc`expr 10 + $1`
    setup_dir=$script_dir/$easiroc/setup
    param_dir=$setup_dir/DAC
    
    if [ ! -d $setup_dir ]; then
	echo "- cannot find $setup_dir"
	exit
    fi

    cd $setup_dir

    peak_interval=$2
    vth_pe=$3

    slink_file=DAC.txt
    param_file=DAC_${easiroc}_${peak_interval}_${vth_pe}.txt
    
    if [ -f $param_dir/$param_file ]; then
	echo "- change Input8bitDAC : $param_file"
	ln -sf $param_dir/$param_file $setup_dir/$slink_file
    else
	echo "- cannot find $param_file"
    fi
fi
