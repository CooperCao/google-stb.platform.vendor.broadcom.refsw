#!/bin/sh -

# check platform
if ! dmidecode | grep -m 1 "Product Name: MKLP7AI-B9"; then
    echo >&2 "Skipping non-skylake board"
    exit 1
fi

case "$1" in
    disable)
	# disable ASPM
	echo "---disable EP ASPM---"
	setpci -s 1:0.0  0xbc.b=0x40
	setpci -s 1:0.0  0xbc.l
	echo "---disable RC ASPM---"
	setpci -s 0:1c.0  0x50.b=0x40
	setpci -s 0:1c.0  0x50.l
	echo "---set RC offset 0x208 to 0x40a02810---"
	setpci -s 0:1c.0 0x208.l=0x40a02810
	setpci -s 0:1c.0 0x208.l
	echo "---set RC offset 0x20c to 0x29(50us)---"
	setpci -s 0:1c.0 0x20c.l=0x29
	setpci -s 0:1c.0 0x20c.l
	echo "---set EP offset 0x248 to 0x40a00000---"
	setpci -s 1:0.0 0x248.l=0x40a00000
	setpci -s 1:0.0 0x248.l
	echo "---set EP offset 0x24c to 0x29(50us)---"
	setpci -s 1:0.0 0x24c.l=0x29
	setpci -s 1:0.0 0x24c.l
	#echo "---set RC ASPM----"
	#setpci -s 0:1c.0 0x50.b=0x43
	#setpci -s 0:1c.0 0x50.l
	#echo "---set EP ASPM----"
	#setpci -s 1:0.0 0xbc.w=0x0143
	#setpci -s 1:0.0 0xbc.l
	;;
    enable)
	# enable ASPM
	echo "---disable EP ASPM---"
	setpci -s 1:0.0  0xbc.b=0x40
	setpci -s 1:0.0  0xbc.l
	echo "---disable RC ASPM---"
	setpci -s 0:1c.0  0x50.b=0x40
	setpci -s 0:1c.0  0x50.l
	echo "---set RC offset 0x208 to 0x40a0281f---"
	setpci -s 0:1c.0 0x208.l=0x40a0281f
	setpci -s 0:1c.0 0x208.l
	echo "---set RC offset 0x20c to 0x29(50us)---"
	setpci -s 0:1c.0 0x20c.l=0x29
	setpci -s 0:1c.0 0x20c.l
	echo "---set EP offset 0x248 to 0x40a0000f---"
	setpci -s 1:0.0 0x248.l=0x40a0000f
	setpci -s 1:0.0 0x248.l
	echo "---set EP offset 0x24c to 0x29(50us)---"
	setpci -s 1:0.0 0x24c.l=0x29
	setpci -s 1:0.0 0x24c.l
	echo "---set RC ASPM----"
	setpci -s 0:1c.0 0x50.b=0x43
	setpci -s 0:1c.0 0x50.l
	echo "---set EP ASPM----"
	setpci -s 1:0.0 0xbc.w=0x0143
	setpci -s 1:0.0 0xbc.l
	;;
    *)
	echo >&2 "Usage: $0 enable"
	echo >&2 "       $0 disable"
	;;
esac

exit

