#!/bin/sh

script_dir=$(cd $(dirname $0); pwd -P)

#_______________________________________________________________________________
echo "EASIROC# : 1) easiroc11 for SCH 00-31"
echo "           2) easiroc12 for SCH 32-63"
echo "           3) easiroc13 for FBH"
echo -n "Which easiroc do you want to change? "
read easiroc
echo

easiroc=easiroc1$easiroc
setup_dir=$script_dir/$easiroc/setup
param_dir=$setup_dir/Input8bitDAC

if [ ! -d $setup_dir ]; then
#    echo "- cannot find $setup_dir"
    exit
fi

param_list=(`ls $param_dir`)

#_______________________________________________________________________________
i=0
echo "PARAM# : "
for p in ${param_list[@]}
do
    echo "  $i) $p"
    i=`expr $i + 1`
done
echo -n "Which param do you want to use? "
read num
echo

if [ -z $num ]; then exit; fi

param=$param_dir/${param_list[$num]}

if [ ! -f $param ]; then
    exit
fi

#_______________________________________________________________________________
echo "- change Input8bitDAC : `basename $param`"
ln -sf $param $setup_dir/Input8bitDAC.txt
