#!/bin/sh

script_dir=$(cd $(dirname $0); pwd -P)

for i in $(seq 1 3); do
    easiroc=easiroc`expr 10 + $i`
    setup_dir=$script_dir/$easiroc/setup
    param_file=$setup_dir/Input8bitDAC.txt

    if [ -f $param_file ]; then
	input_dac=$(readlink $param_file)
	if [ ! -z $input_dac ]; then
	    echo "- $easiroc : $(basename $input_dac)"
	else
	    echo "- $easiroc : $(basename $param_file) is not symbolic link"
	fi
    else
	echo "- $easiroc : cannot find $param_file"
    fi
done