#!/bin/bash

numP=8
for rT in {2..8}
do	
	for mT in {2..8}
	do
		
		for s in {0..6}
		do
			echo "Running for $numP nodes $rT reader $mT mapper threads and $s data size"
			mpirun -n $numP ./dbg.o $rT $mT $s
		done
	done
done

exit 0
