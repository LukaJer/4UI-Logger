#!/usr/bin/env fish
rsync -av --exclude=".*/" --exclude="README.md" /home/luka/Documents/PlatformIO/Projects/4UI-Logger/ /home/luka/4UI-Logger/Software
mv /home/luka/4UI-Logger/Software/src/LoRaMini.cpp //home/luka/4UI-Logger/Software/src/4UILogger.cpp



