rm -f Bespoke-Linux.zip
rm -rf BespokeSynth/
mkdir BespokeSynth
cp build/BespokeSynth BespokeSynth/BespokeSynth
strip BespokeSynth/BespokeSynth
cp Bespoke-GLSLfix.sh BespokeSynth/Bespoke-GLSLfix.sh
cp bespoke_dependencies.sh BespokeSynth/bespoke_dependencies.sh
cp readme.txt BespokeSynth/readme.txt
cp -r ../MacOSX/build/Release/resource BespokeSynth/resource
zip -r Bespoke-Linux.zip BespokeSynth
rm -rf BespokeSynth/