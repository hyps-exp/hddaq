#!/bin/sh

for i in $(seq 0 9)
  do
  cp /home/DAQ/bft_adc/inputdac_g30_temp21.7/Input8bitDAC_$i.txt ./easiroc0$i/setup/Input8bitDAC.txt
done