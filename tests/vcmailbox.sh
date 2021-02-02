#!/bin/bash
# vcmailbox mockup to test RPI MAC address readout from VC

# expected input (get macaddr command with 6 bytes send/recv)
# $ vcmailbox 0x00010003 6 6 0 0

if [[ "$1" == "0x00010003" ]] && [[ "$2" == "6" ]] && [[ "$3" == "6" ]] && [[ "$4" == "0" ]] && [[ "$5" == "0" ]]; then
	echo "0x00000020 0x80000000 0x00010003 0x00000006 0x80000006 0x78563412 0x0000bc9a 0x00000000"
else
	echo "oops, wrong input params to vcmailbox"
fi
