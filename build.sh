#!/bin/bash

if [ $1 == "rk" ]; then
	source ./rk3328_env.sh
fi

make distclean;make
