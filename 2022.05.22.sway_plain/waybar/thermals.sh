#!/bin/sh

cpu1=$(sysctl dev.cpu.15.temperature | cut -d" " -f2 | cut -d"C" -f1)
cpu2=$(sysctl dev.cpu.16.temperature | cut -d" " -f2 | cut -d"C" -f1)

temp=$(echo -e "$cpu1\n$cpu2" | sort -g -r | head -n1)
baseval=$(echo $temp | cut -d'.' -f1)
sym=""

if [ "$baseval" -gt 20 ]; then
	sym=""
fi

if [ "$baseval" -gt 30 ]; then
	sym=""
fi

if [ "$baseval" -gt 40 ]; then
	sym=""
fi

if [ "$baseval" -gt 50 ]; then
	sym=""
fi

echo "$sym $temp"
