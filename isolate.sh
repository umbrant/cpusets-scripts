#!/bin/bash

if [[ $EUID -ne 0]]; then
	echo "This script must be run as root."
	exit 1
fi

# Only make and mount if it's not already mounted
cpuset=`mount | grep cpuset`
if [ -z "$cpuset" ]; then
	mkdir -p /dev/cpuset
	mount -t cpuset cpuset /dev/cpuset
fi

cd /dev/cpuset

mkdir -p best_effort
mkdir -p isolated

# Put all existing tasks into the best_effort cpuset
cd best_effort
/bin/echo 0-1 > cpus
/bin/echo 0 > mems
for pid in `ps -e -o pid`
do
	/bin/echo $pid > tasks
done

# Assign the other CPUs to isolated set, make the CPUs exclusive
cd ../isolated
/bin/echo 2-3 > cpus
/bin/echo 0 > mems
/bin/echo 1 > cpu_exclusive

echo "All done!"
