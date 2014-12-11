#!/bin/bash

OBELISK=$(mktemp /tmp/output.obelisk.XXXXXXXXXX)
awk '/See also:/ { readline; print "\n"} $0!~/See also:/{ printf"%s|",$0}' obelisk.demon.co.uk_6502.txt | \
#>$OBELISK

awk -F"|" '{printf("%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|", substr($2,1,3), $2,$4,$6, $8,$9,$10,$11,$12,$13,$14); for(i=21;i<NR;i++){printf("%s|",$i); } print "\n"} '


