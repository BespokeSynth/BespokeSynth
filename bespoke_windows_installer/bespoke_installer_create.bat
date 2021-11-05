"C:\Program Files (x86)\WiX Toolset v3.11\bin\heat" dir ..\ignore\build\Source\BespokeSynth_artefacts\Release\resource -o HarvestedResourceDir.wxs -scom -frag -srd -sreg -gg -cg BespokeResourceDir -dr RESOURCE_DIR_REF
"C:\Program Files (x86)\WiX Toolset v3.11\bin\candle" BespokeSynth.wxs HarvestedResourceDir.wxs -arch x64
"C:\Program Files (x86)\WiX Toolset v3.11\bin\light" BespokeSynth.wixobj HarvestedResourceDir.wixobj -b ..\ignore\build\Source\BespokeSynth_artefacts\Release\resource -out Bespoke-Windows.msi -ext WixUIExtension

@pause