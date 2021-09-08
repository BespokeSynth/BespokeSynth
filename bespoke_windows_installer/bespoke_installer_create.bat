"C:\Program Files (x86)\WiX Toolset v3.11\bin\heat" dir ..\Builds\MacOSX\build\Release\resource -o HarvestedResourceDir.wxs -scom -frag -srd -sreg -gg -cg BespokeResourceDir -dr RESOURCE_DIR_REF
"C:\Program Files (x86)\WiX Toolset v3.11\bin\candle" BespokeSynth.wxs HarvestedResourceDir.wxs -arch x64
"C:\Program Files (x86)\WiX Toolset v3.11\bin\light" BespokeSynth.wixobj HarvestedResourceDir.wixobj -b ..\Builds\MacOSX\build\Release\resource -out BespokeSynth.msi -ext WixUIExtension

@pause