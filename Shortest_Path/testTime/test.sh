#!/bin/bash

for sitenu in $(seq 1 4)
do
    echo -e "----------------------------" >> ./dense_apsp.rpt
	srun -c12 -n${sitenu} -N${sitenu} -pbatch ./apsp /home/ta/hw3/cases/input/dense_5000.in ./a.out >> ./dense_apsp.rpt
	rm -rf ./a.out
done

for sitenu in $(seq 1 4)
do
    echo -e "----------------------------" >> ./skew_apsp.rpt
	srun -c12 -n${sitenu} -N${sitenu} -pbatch ./apsp /home/ta/hw3/cases/input/skew_5000.in ./a.out >> ./skew_apsp.rpt
	rm -rf ./a.out
done

for sitenu in $(seq 1 4)
do
    echo -e "----------------------------" >> ./random_apsp.rpt
	srun -c12 -n${sitenu} -N${sitenu} -pbatch ./apsp /home/ta/hw3/cases/input/random_5000.in ./a.out >> ./random_apsp.rpt
	rm -rf ./a.out
done
