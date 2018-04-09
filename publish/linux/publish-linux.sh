#!/bin/bash

PATH=$PATH:/opt/Qt/Tools/QtInstallerFramework/3.0/bin/

##################### AppImage creation #####################

# Create a new dist folder
rm -rf dist
mkdir -p dist/opt/QT-AntennaAnalyzer

# Include binary and dist files
cp ../../build-analyzer-Desktop_Qt_5_7_1_GCC_64bit-Release/analyzer dist/opt/QT-AntennaAnalyzer/QT-AntennaAnalyzer
cp ../../analyzer/antenna-charge-radio-512.png dist/opt/QT-AntennaAnalyzer/QT-AntennaAnalyzer.png
cp QT-AntennaAnalyzer.desktop dist/opt/QT-AntennaAnalyzer

# Create AppImage
echo "Creating AppImage.."
linuxdeployqt dist/opt/QT-AntennaAnalyzer/QT-AntennaAnalyzer -appimage -no-translations -bundle-non-qt-libs 
mkdir packages/com.vendor.product/data
mv *.AppImage packages/com.vendor.product/data
cp ../../analyzer/sark110/99-sark110.rules packages/com.vendor.product/data/99-sark110.rules
cp ../../analyzer/antenna-charge-radio-128.png packages/com.vendor.product/data/QT-AntennaAnalyzer.png
rm -rf dist

# Create Installer
echo "Creating installer.."
mkdir ../../installer/linux
binarycreator --offline-only -c config/config.xml -p packages ../../installer/linux/setup
rm -rf packages/com.vendor.product/data

echo "Done"




