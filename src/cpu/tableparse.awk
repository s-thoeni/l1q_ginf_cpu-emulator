# call with awk -F"\t" 
'BEGIN { split("imp imm zp zpx zpy izx izy abs abx aby ind rel", adrmodes, " "); } 
{ for( adrm in adrmodes) { 
	printf("cmd %s adrmodes %s has value %s\n", $1, adrm, adrmodes[adrm] ) ;
    } 
    for( i=2; i<14; i++){
	printf("%s %s\n",$1, $14);
    }
}' laa-commands.txt 
						     
