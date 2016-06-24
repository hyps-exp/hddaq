#!/bin/sh

script_dir=$(cd $(dirname $0); pwd -P)

if [ $# != 3 ]; then
    echo "Usage : `basename $0` [EASIROC#] [TEMP] [PEAK_INTERVAL]"
    echo "EASIROC# : 1) easiroc11 for SCH 00-31"
    echo "           2) easiroc12 for SCH 32-63"
    echo "           3) easiroc13 for FBH"
else
    easiroc=easiroc`expr 10 + $1`
    setup_dir=$script_dir/$easiroc/setup
    param_dir=$setup_dir/Input8bitDAC

    if [ ! -d $setup_dir ]; then
	echo "- cannot find $setup_dir"
	exit
    fi

    cd $setup_dir

    temp=$2
    peak_interval=$3

    slink_file=Input8bitDAC.txt
    param_file=Input8bitDAC_${easiroc}_${temp}_${peak_interval}.txt

    if [ -f $param_dir/$param_file ]; then
	echo "- change Input8bitDAC : $param_file"
	ln -sf $param_dir/$param_file $setup_dir/$slink_file
    else
	echo "- cannot find $param_file"
    fi
fi
