#!/bin/sh
# requires appdmg, which can be installed via: npm install -g appdmg
rm -r BespokeSynth
rm Bespoke-Mac.dmg
cp -r Release BespokeSynth
cp Python38 BespokeSynth/Bespoke.app/Contents/MacOS/
install_name_tool -change /Library/Frameworks/Python.framework/Versions/3.8/Python @executable_path/Python38 BespokeSynth/Bespoke.app/Contents/MacOS/Bespoke
appdmg bespoke_dmg.json Bespoke-Mac.dmg
