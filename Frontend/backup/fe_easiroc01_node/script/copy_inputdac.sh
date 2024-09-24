#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)

param_dir=$bin_dir/Input8bitDAC

#-- Befor E70 experiment --#
#temp_dir=$param_dir/inputdac_g30_temp19.0
#temp_dir=$param_dir/inputdac_g30_temp20.0
#temp_dir=$param_dir/inputdac_g30_temp21.0
#temp_dir=$param_dir/inputdac_g30_temp22.0
#temp_dir=$param_dir/inputdac_g30_temp23.0
#temp_dir=$param_dir/inputdac_g30_temp24.0
#temp_dir=$param_dir/inputdac_g30_temp25.0
#----------------------------------------------#

#-- After E70 experiment --#
#temp_dir=$param_dir/inputdac_e42final_20221129 ## Befor changing InputDAC for E70 setup. (E42 final status) see k18lognotebook #105 p.105??
temp_dir=$param_dir/inputdac_e70_20221129

log_file=$bin_dir/last_param.log
echo >> $log_file
date >> $log_file
echo "" >> $log_file
echo "bin_dir   : $bin_dir"   >> $log_file
echo "param_dir : $param_dir" >> $log_file
echo "temp_dir  : $(basename $temp_dir)"  >> $log_file

for i in $(seq 0 9)
  do
  nickname=easiroc`printf "%02d" $i`
  param_file=$temp_dir/Input8bitDAC_$i.txt
  
  if [ ! -f $param_file ]; then
      echo -e "\e[35mcannot find parameter\e[m : $param_file"
      echo -e "\e[35mcannot find parameter\e[m : $param_file" >> $log_file
      continue;
  fi

  echo "$nickname : $(basename $param_file)" >> $log_file
  cp -v $param_file $bin_dir/$nickname/setup/Input8bitDAC.txt
done
