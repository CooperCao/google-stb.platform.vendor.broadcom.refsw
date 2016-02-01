
#rm -f *.pj
rm -f *.use
rm -f *.sln
rm -f *.vcproj

rm -rf BrcmUtils
cd build
rm -f tar7111

cd ..\
cd cm_bsp_v2/
rm -rf bsp_bcm97111
rm -rf bsp_common/os/vxWorks
rm -rf bsp_common/os/Win32

cd ..\

rm -f Common/IpHelpers/Vx*.*
rm -f Common/IpHelpers/Win32*.*
rm -f Common/OperatingSystem/vx*.*
rm -f Common/OperatingSystem/Win32*.*
rm -rf Common/vxWorks

rm -rf StbApp_DsgCC/vxWorks
rm -rf Stb/StbDocsisApi/vxWorks
rm -rf Stb/StbDocsisApi/Win32


:EXIT
