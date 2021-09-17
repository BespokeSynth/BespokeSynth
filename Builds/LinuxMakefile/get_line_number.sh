#!/usr/bin/env bash
#pass the address to get the line number where the crash was
#the address is this part of backtrace: "./BepokeSynth(+0xADDRESS)"
addr2line -e BespokeSynth $1