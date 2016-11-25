#!/bin/bash
function test {
	echo "$1" > /proc/first
	echo "$2" > /proc/operand
	echo "$3" > /proc/second
	res=`sudo cat /dev/result`
	echo "$1$2$3=${res}"
	if [ "${res}" == "$4" ]; then
		echo "OK"
	else 
		echo "NOT CORRECT"
	fi
}

test 3 + 13 16
test 1 + 2 3
test 4 + 10 14
test 12 / 2 6
test 12 / -2 -6
test 8 / 2 4
test 10 '*' 4 40
test 10 '*' -4 -40
test -3 '*' -3 9
test 15 - 30 -15
test 10 - -10 20
test 10 - 20 -10
test 1 - 1 0
