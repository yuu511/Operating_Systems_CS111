#!/bin/sh

SCHE_KERNEL_PATH="/sys/kern/sched_ule.c"
KERN_KERNEL_PATH="/sys/kern/kern_switch.c"
GENERIC_PATH="/usr/src/sys/amd64/conf/GENERIC"
CUSTOM_CONF_PATH="/usr/src/sys/amd64/conf/MYKERNEL"
OPTION_00="sched_ule.c"
OPTION_11="kern_switch_1.c"
OPTION_01="sched_ule_2.c"
OPTION_10="kern_switch.c"

echo " PLEASE RUN THIS SCRIPT AS ROOT or use sudo $0, in the top level directory of the repository! Progress:"
echo "COPYING THE GENERIC AMD64 CONF FILE, OUTPUT $CUSTOM_CONF_PATH"
cp -v $GENERIC_PATH $CUSTOM_CONF_PATH

if [ "$#" -ne 1 ]; then
  echo "Wrong number of arguments : $# Arguments"  
  exit 1
fi

echo "REPLACING CURRENT SCHEDULER WITH OPTION $1"
if [ "$1" -eq 0 ]; then
	echo "YOU SELECTED OPTION 0: $OPTION_00 AND $OPTION_10 "
	cp -v $OPTION_00 $SCHE_KERNEL_PATH
	cp -v $OPTION_10 $KERN_KERNEL_PATH
fi

if [ "$1" -eq 1 ]; then
	echo "YOU SELECTED OPTION 1: $OPTION_00 AND $OPTION_11 "
	cp -v $OPTION_00 $SCHE_KERNEL_PATH
	cp -v $OPTION_11 $KERN_KERNEL_PATH
fi

if [ "$1" -eq 2 ]; then
	echo "YOU SELECTED OPTION 2: $OPTION_01 AND $OPTION_10 "
	cp -v $OPTION_01 $SCHE_KERNEL_PATH
	cp -v $OPTION_10 $KERN_KERNEL_PATH
fi

if [ "$1" -eq 3 ]; then
	echo "YOU SELECTED OPTION 3: $OPTION_01 AND $OPTION_11 "
	cp -v $OPTION_01 $SCHE_KERNEL_PATH
	cp -v $OPTION_11 $KERN_KERNEL_PATH
fi
