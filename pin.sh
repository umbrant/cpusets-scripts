#!/bin/bash

cd /dev/cpuset/isolated

PID=`ps -e | grep nameserver | head -1 |  cut -d " " -f 1`
echo "$PID"
echo "$PID" > tasks
PID=`ps -e | grep DummyServer | head -1 | cut -d " " -f 1`
echo "$PID"
echo "$PID" > tasks
PID=`ps -e | grep textedit | head -1 | cut -d " " -f 1`
echo "$PID"
echo "$PID" > tasks
