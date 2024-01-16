#!/bin/bash

#this is pulled from surge and adapted: https://github.com/surge-synthesizer/surge/blob/ccf5d842755279c3db0003b674e059c611fd39e5/scripts/installer_mac/make_installer.sh

# Documentation for pkgbuild and productbuild: https://developer.apple.com/library/archive/documentation/DeveloperTools/Reference/DistributionDefinitionRef/Chapters/Distribution_XML_Ref.html

# preflight check
INDIR=$1
SOURCEDIR=$2
TARGET_DIR=$3
VERSION=$4

echo "MAKE from $INDIR $SOURCEDIR into $TARGET_DIR with $VERSION"


if [ "$VERSION" == "" ]; then
    echo "You must specify the version you are packaging!"
    echo "eg: ./make_installer.sh 1.0.6b4"
    exit 1
fi


OUTPUT_BASE_FILENAME="BespokeSynth-Mac-$VERSION"

mkdir -p "${TARGET_DIR}"
echo "{ \"title\": \"Bespoke\", \"background\": \"${SOURCEDIR}/background.png\", \"icon-size\": 80," > dmg_manifest.json
echo "\"contents\": [ { \"x\": 192, \"y\": 338, \"type\": \"file\", \"path\": \"${INDIR}/BespokeSynth.app\"}, " >> dmg_manifest.json 
echo "{ \"x\": 448, \"y\": 338, \"type\": \"link\", \"path\": \"/Applications\" }]}" >> dmg_manifest.json 
appdmg "dmg_manifest.json" "${TARGET_DIR}/$OUTPUT_BASE_FILENAME.dmg"

echo "dmg created at ${TARGET_DIR}/$OUTPUT_BASE_FILENAME.dmg"

if [[ ! -z $MAC_SIGNING_CERT ]]; then
  echo "running codesign with cert $MAC_SIGNING_CERT"
  codesign --force -s "$MAC_SIGNING_CERT" --timestamp "${TARGET_DIR}/$OUTPUT_BASE_FILENAME.dmg"
  echo "checking codesign"
  codesign -vvv "${TARGET_DIR}/$OUTPUT_BASE_FILENAME.dmg"
  echo "running notarytool"
  xcrun notarytool submit "${TARGET_DIR}/$OUTPUT_BASE_FILENAME.dmg" \
                          --apple-id ${MAC_SIGNING_ID} \
                          --team-id ${MAC_SIGNING_TEAM} \
                          --password ${MAC_SIGNING_1UPW} \
                          --wait

  echo "running stapler"
  xcrun stapler staple "${TARGET_DIR}/${OUTPUT_BASE_FILENAME}.dmg"
fi

# clean up

