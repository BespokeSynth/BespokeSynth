#!/bin/sh
# requires appdmg, which can be installed via: npm install -g appdmg
rm -r BespokeSynth
rm Bespoke-Mac.dmg
cp -r Release BespokeSynth
mkdir BespokeSynth/Bespoke.app/Contents/Frameworks/
cp Python38 BespokeSynth/Bespoke.app/Contents/Frameworks/
install_name_tool -change /Library/Frameworks/Python.framework/Versions/3.8/Python @executable_path/../Frameworks/Python38 BespokeSynth/Bespoke.app/Contents/MacOS/Bespoke
codesign --force --options runtime --timestamp --verbose=4 --entitlements entitlements.plist --sign "Developer ID Application: Ryan Challinor (J5RJ562GN5)" BespokeSynth/Bespoke.app/Contents/MacOS/Bespoke
appdmg bespoke_dmg.json Bespoke-Mac.dmg
codesign -s "Developer ID Application: Ryan Challinor (J5RJ562GN5)" --timestamp Bespoke-Mac.dmg