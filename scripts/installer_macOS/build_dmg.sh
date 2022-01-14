#!/bin/sh
# requires appdmg, which can be installed via: npm install -g appdmg

rm Bespoke-Mac.dmg

rm -rf BespokeSynth.app
ditto ../../ignore/build/Source/BespokeSynth_artefacts/Release/BespokeSynth.app BespokeSynth.app

#codesign --force --options runtime --timestamp --verbose=4 --entitlements entitlements.plist --sign "Developer ID Application: Ryan Challinor (J5RJ562GN5)" BespokeSynth.app/Contents/MacOS/BespokeSynth
#codesign --force --options runtime --timestamp --verbose=4 --entitlements entitlements.plist --sign "Developer ID Application: Ryan Challinor (J5RJ562GN5)" BespokeSynth.app/Contents/Frameworks/Python.framework/Versions/3.9/Python
#codesign --force --options runtime --timestamp --verbose=4 --entitlements entitlements.plist --sign "Developer ID Application: Ryan Challinor (J5RJ562GN5)" BespokeSynth.app/Contents/Frameworks/Python.framework/Versions/3.9/bin/python3.9
#codesign --force --options runtime --timestamp --verbose=4 --entitlements entitlements.plist --sign "Developer ID Application: Ryan Challinor (J5RJ562GN5)" BespokeSynth.app/Contents/Frameworks/Python.framework/Versions/3.9/Resources/Python.app/Contents/MacOS/Python

appdmg bespoke_dmg.json Bespoke-Mac.dmg

codesign -s "Developer ID Application: Ryan Challinor (J5RJ562GN5)" --timestamp Bespoke-Mac.dmg