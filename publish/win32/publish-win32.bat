md packages\com.vendor.product\data
md ..\..\installer\win32
copy ..\..\build-analyzer-Desktop_Qt_5_10_1_MinGW_32bit-Release\release\analyzer.exe packages\com.vendor.product\data\QT-AntennaAnalyzer.exe
C:\Qt\5.10.1\mingw53_32\bin\windeployqt packages\com.vendor.product\data\QT-AntennaAnalyzer.exe
copy "C:\Qt\5.10.1\mingw53_32\bin\libstdc++-6.dll" packages\com.vendor.product\data\
C:\Qt\Tools\QtInstallerFramework\3.0\bin\binarycreator --offline-only -c config\config.xml -p packages ..\..\installer\win32\setup.exe
RMDIR packages\com.vendor.product\data /S /Q 
pause