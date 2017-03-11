#!/bin/sh

if [ $# -gt 0 ]; then
    run_dir=$1
else
    run_dir=.
fi

script_dir=$(dirname `readlink -f $0`)
misc_dir=$(dirname `readlink -f $0`)/../misc

#_______________________________________________________________________________
function makelink()
{
    source=$1
    target=$run_dir/$(basename $1)
    if [ -e $target ];then
	echo $target already exist
    else
	ln -sv $source $target
    fi
}

#_______________________________________________________________________________
function copytxt()
{
    source=$1
    target=$run_dir/$(basename $1)
    if [ -e $target ];then
	echo $target already exist
    else
	cp -v $source $target
    fi
}

#_______________________________________________________________________________
echo
echo "run_dir    = $run_dir"
echo "script_dir = $script_dir"
echo "misc_dir   = $misc_dir"
echo

makelink $script_dir/launcher.py
makelink $script_dir/tmplauncher.py
makelink $script_dir/kill.sh
copytxt $misc_dir/datanode.txt
copytxt $misc_dir/msgnode.txt

echo
echo done
echo
