#/bin/bash

module load compilers/solarisstudio-12.5  compilers/gnu-5.4.0

rm -rf $(ls *_smart.t 2> /dev/null) compiler_output exec_output_*

NUM_TH="1 2 4 6 8 12 24 48"

make clean > /dev/null && make >> compiler_output

for t in $NUM_TH
do
    echo $t >> exec_output_julia
    { time ./fractastic J 1800 1800 -2 2 -2 2 200 1 -0.4 0.6 2 $t S 100 output.pnm >> exec_output_julia; } 2>> tmp
done

cat tmp | grep real | cut -d $'\t' -f 2 > time_julia_smart.t
rm tmp

for t in $NUM_TH
do
    echo $t >> exec_output_mandle
    { time ./fractastic M 1800 1800 -2.5 1.5 -2 2 200 20 2 $t S 100 output.pnm >> exec_output_mandle; } 2>> tmp
done
cat tmp | grep real | cut -d $'\t' -f 2 > time_mandlebrot_smart.t
rm tmp
