#!/usr/bin/env bash
#running this script to install these dependencies should allow you to both run and compile bespoke
#getting all of these dependencies is definitely overkill if you only want to run the binary, and 
#don't care about compiling, but I'm not good enough at linux to narrow it down for you
sudo apt -y install g++ libfreetype6-dev libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev \
mesa-common-dev libasound2-dev freeglut3-dev libxcomposite-dev libcurl4 libusb-1.0-0-dev libgtk-3-dev \
python3-dev libcurl4-openssl-dev libwebkit2gtk-4.0-dev libjack-dev
