#!/bin/sh

for i in $(seq 0 9)
  do
  cp ./easiroc0$i/setup/Input8bitDAC.txt ~/bft_adc/inputdac/Input8bitDAC_$i.txt
done