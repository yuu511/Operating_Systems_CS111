#!/bin/sh

# ./mode.sh [NUMBER]
# must be run as root
# vm_pageout: Original Source File
# vm_pageout_log.c : Original Source File + Logging
# vm_pageout_fifo_log.c : FIFO memory management + Logging

VM_KERNEL_PATH="/usr/src/sys/vm/vm_pageout.c"
GENERIC_PATH="/usr/src/sys/amd64/conf/GENERIC"
CUSTOM_CONF_PATH="/usr/src/sys/amd64/conf/MYKERNEL"
OPTION_0="../usr/src/sys/vm/vm_pageout.c"
OPTION_1="../usr/src/sys/vm/vm_pageout_log.c"
OPTION_2="../usr/src/sys/vm/vm_pageout_fifo_log.c"

echo " PLEASE RUN THIS SCRIPT AS ROOT or use sudo $0, in the top level directory of the repository! Progress:"
echo "COPYING THE GENERIC AMD64 CONF FILE, OUTPUT $CUSTOM_CONF_PATH"
cp -v $GENERIC_PATH $CUSTOM_CONF_PATH

if [ "$#" -ne 1 ]; then
  echo "Wrong number of arguments : $# Arguments"  
  exit 1
fi

echo "REPLACING CURRENT $OPTION_0 WITH OPTION $1"
if [ "$1" -eq 0 ]; then
	echo "YOU SELECTED OPTION 0: $OPTION_0"
	cp -v $OPTION_0 $VM_KERNEL_PATH
fi

if [ "$1" -eq 1 ]; then
	echo "YOU SELECTED OPTION 1: $OPTION_1 "
	cp -v $OPTION_1 $VM_KERNEL_PATH
fi

if [ "$1" -eq 2 ]; then
	echo "YOU SELECTED OPTION 2: $OPTION_2 "
	cp -v $OPTION_2 $VM_KERNEL_PATH
fi

