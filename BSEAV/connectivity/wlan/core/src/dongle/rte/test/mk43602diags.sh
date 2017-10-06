#!/tools/bin/bash

function cm3_build_copy ()
{
	make run-arm-thumb-cm3-int-ai-uc-43602 PROGRAMS=cm3_43602 CHIP=43602
	cp -v run-arm-thumb-cm3-int-ai-uc-43602/cm3_43602.exe ../../../tools/47xxtcl/  
} 



function cr4_build_copy ()
{
	make run-arm-thumb-cr4-43602 PROGRAMS=cr4_43602 CHIP=43602
	cp -v run-arm-thumb-cr4-43602/cr4_43602.bin ../../../tools/47xxtcl/  
} 

if [ "$1" == "cm3" ] 
then
	cm3_build_copy
elif [ "$1" == "cr4" ] 
then
	cr4_build_copy
else
	cm3_build_copy
	cr4_build_copy
fi
