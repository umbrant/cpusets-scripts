#!/bin/bash

# Testing under no load
echo -n "Testing under no load..."
for i in {1..10}
do
	./thrasher -1 3 -t 1 -m >> mem_noload.txt
	./thrasher -1 3 -t 1 -p >> cpu_noload.txt
	./thrasher -1 3 -t 1 -c >> cache_noload.txt
	echo -n "$i..."
done

# Testing under loaded conditions
./thrasher -t 10 -m &
sleep 5; # Let it spin up a bit
echo -n "Testing under load..."
for i in {1..10}
do
	./thrasher -1 3 -t 1 -m >> mem_load.txt
	./thrasher -1 3 -t 1 -p >> cpu_load.txt
	./thrasher -1 3 -t 1 -c >> cache_load.txt
	echo -n "$i..."
done
killall thrasher

# Testing under loaded conditions, but isolated
./thrasher -t 10 -n 3 -m &
sleep 5; # Let it spin up a bit
echo -n "Testing under load with isolation..."
for i in {1..10}
do
	./thrasher -1 3 -t 1 -m >> mem_isolated.txt
	./thrasher -1 3 -t 1 -p >> cpu_isolated.txt
	./thrasher -1 3 -t 1 -c >> cache_isolated.txt
	echo -n "$i..."
done
killall thrasher
