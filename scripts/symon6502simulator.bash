#!/bin/bash

# generate binary memory for symon 6502 load program
echo "e88a990002990003990004990005c898c510d004c84c0006c8c8c8c84c0006" | xxd -r -p > disco.bin 

java -jar ~/Downloads/symon-0.8.5.jar 
