#!/bin/bash

export LD_LIBRARY_PATH=../BitfiltrateBencode/SharedLib:../ConcurrentQueue/Debug:../LinkedList/SharedLib

while read p; do
	found=0
	for f in $(ls assets)
	do
		cp "assets/$f" peers.txt
		echo "Doing $f"
		./Debug/Bitfiltrate $p

		exitCode=$?
		echo $exitCode
		if [ $exitCode -eq 67 ]; then
			echo $p",true"
			found=1
			break
		fi
	done

	if [ $found -eq 0 ]; then
		echo $p",false"
	fi
done < hashes.txt
#timeout 5s ./Debug/Bitfiltrate 2>&1

echo $?
