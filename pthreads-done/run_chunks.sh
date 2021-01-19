#/bin/bash

module load compilers/solarisstudio-12.5  compilers/gnu-5.4.0

NUM_TH=24
STATIC="300 200 100 90 50 40 30 20 10 9 8 5 4 2 1"

rm -rf $(ls *_chunk.t 2> /dev/null) compiler_output exec_output_*

make clean > /dev/null && make >> compiler_output

for s in $STATIC
do
    echo $s >> exec_output_julia
    { time ./fractastic J 1800 1800 -2 2 -2 2 200 1 -0.4 0.6 2 $NUM_TH C $s output.pnm >> exec_output_julia; } 2>> tmp
done
cat tmp | grep real | cut -d $'\t' -f 2 > time_julia_chunk.t
rm tmp

for s in $STATIC
do
    echo $s >> exec_output_mandle
    { time ./fractastic M 1800 1800 -2.5 1.5 -2 2 200 20 2 $NUM_TH C $s output.pnm >> exec_output_mandle; } 2>> tmp
done
cat tmp | grep real | cut -d $'\t' -f 2 > time_mandlebrot_chunk.t
rm tmp
