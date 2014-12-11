#!/bin/bash
 
awk -F"\t" -e '$5 != "" { print " PC  AC XR YR SP NV-BDIZC IR"
print  $6 " " $7 " " $8 " " $9 " " $10 " " $11 "\n" 
if($11!="nv‑BdIzc") { print "Flag change "}
}' < visual6502.org.disco.trace

echo "e88a990002990003990004990005c898c510d004c84c0006c8c8c8c84c0006" | awk  -e '
 function nibble2int( c){
    switch(c){
    case /[aA]/:{
       ret = 10;
       break
    }
    case /[bB]/:{
       ret = 11;
       break
    }
    case /[cC]/:{
       ret = 12;
       break
    }
    case /[dD]/:{
       ret = 13;
       break
    }
    case /[eE]/:{
       ret = 14;
       break
    }
    case /[fF]/:{
       ret = 15;
       break
    }
    case /[[:digit:]]/:{
       ret = c;
       break
    }
   default: ret="";
}
return ret;
}
#e88a990002990003990004990005c898c510d004c84c0006c8c8c8c84c0006
## val = or(lshift(val,4), nibble2int(chars[i+1]));
{ 
 split($0, chars, "")

 for (i=1; i <= length($0); i+=2) {

      val = nibble2int(chars[i]);
      printf("%c", or(lshift(val,4),  nibble2int(chars[i+1])))
#      print "i: " i " val " val " lshift(val,4) : " lshift(val,4)  " chars[i] : " chars[i] " chars[i+1] : " chars[i+1] " nibble " nibble2int(chars[i+1]) " OR : " or(lshift(val,4),  nibble2int(chars[i+1]))

#    printf("%x", or(lshift(val,4),  nibble2int(chars[i+1])))
#     if((i+1) <= length($0)){
#       print "i: " i " val " val " lshift(val,4) : " lshift(val,4)  " chars[i] : " chars[i] " chars[i+1] : " chars[i+1] " nibble " nibble2int(chars[i+1]) " OR : " or(lshift(val,4),  nibble2int(chars[i+1]))
#      i++;
# print "val : " val
#     }
#     printf("%i\n", val)
  }
}' > disco.bin
