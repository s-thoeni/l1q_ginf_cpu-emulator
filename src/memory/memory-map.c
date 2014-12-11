/*
  memory_map.c
  initial: 3.7.2013
*/

char * irq;
char * dbr;
char * abrl;
char * abrh;
char * rw;
#define CIA_BASE_ADR 0xd700

system_bus_cpu_mem_access_bus(int adr, int data, char* adr1, char* adr2, int rw,char*data){
  if(startaddr>=cia_register && startaddr<=cia_register+15){
    cp_register(cia_read(start_addr%15),dbr);
  }
  
  if(startaddr>=sid_register && startaddr<=sid_register+29){
	write_sid(reg2int(reg), value, sid, emu_sid);
  }
}	
