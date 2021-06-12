#!/bin/sh
# requires appdmg, which can be installed via: npm install -g appdmg
rm -r BespokeSynth
rm Bespoke-Mac.dmg
cp -r Release BespokeSynth
appdmg bespoke_dmg.json Bespoke-Mac.dmg
