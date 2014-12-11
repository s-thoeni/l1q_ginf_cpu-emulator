/*
CIA (Complex Interface Adapter) 
Simulation of a cia chip
-timer
-two parallel ports
- no serial

initial: 29.6.2013

*/


/* stopwatch_timer.c */
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include "cpu-util.h" 
 #include <unistd.h>

#include "stopwatch_timer.h"

char PRA[9]="00000000"; 
char PRB[9]="00000000"; 
char cia_port_a_buffer[9]="00000000"; 
char cia_port_b_buffer[9]="00000000"; 


char dataportbread[9]="00000000"; 
char dataportbwrite[9]="00000000"; 

char DDRA[9]="00000000"; 
char DDRB[9]="00000000"; 
char timer_latch_low_a[9]="00000000"; 
char timer_latch_low_b[9]="00000000"; 
char timer_latch_high_a[9]="00000000"; 
char timer_latch_high_b[9]="00000000"; 

char timer_counter_low_a[9]="00000000"; 
char timer_counter_low_b[9]="00000000"; 
char timer_counter_high_a[9]="00000000"; 
char timer_counter_high_b[9]="00000000"; 

char icr_read[9]="00000000"; 
char icr_write[9]="00000000"; 

char control_a[9]="00000000"; 

char control_b[9]="00000000"; 

void read(int adr){
switch adr
 case 0: // dataport a
   return read_PRA
   break;
 case 2:
   break;
 case 3:
   break;
 case 4:
   break;
 case 5:
   break;
 case 6:
   break;
 case 7:
   break;
 case 8:
   break;
 case 9:
   break;
 case 10:
   break;
 case 11:
   break;
 case 12:
   break;
 case 13:
   break;
 case 14:
   break;
 case 15:
   break;

}

/*
  read Controlregister A
*/
void write_cra(char*data){
  cp_register(data,control_a);
  if(control_a[9-1]=='1'){
    cia_stop_timer_a(control_a);
  } else { 
    cia_start_timer_a(control_a);
  }
}
void read_PRA(){
  cp_Register(cia_porta_buffer,PRA);
}

cia_start_timer_a(){
  int time=0;
  cp_register(timer_latch_low_a,timer_counter_low_a);
  cp_register(timer_latch_high_a,timer_counter_high_a);
  time = conv_2register2int(timer_counter_high_a,timer_counter_low_a);
  start_timer(time);
}

Uint8 conv_byte2int(char*byte){
  int i=0;
  int sum = 0;
  for(i=0; i<8;i++)
    if(byte[i] == '1'){
      sum = (sum << 1) + 1;
    }else
      sum = sum << 1;
  return sum;
}
/* prototypes */
int* g_start_timer(void *secs);
 
static clock_t _current_time = 0;
 
/* create the thread */
void start_timer(int seconds)
{
    pthread_t thread_id;
    int rc = 0;
 
    rc = pthread_create(&thread_id, NULL, g_start_timer, (void*) seconds);
 
    if(rc)
    {
    printf("=== Error Creating thread\n");
    } 
}
 
/* start the timing in another thread */
int* g_start_timer(void *secs)
{
    printf("Starting thread\n");
    int seconds = (int) secs;
    printf("g_start_timer: %d\n", (int) seconds);
    while(cra[0]=='0'){
      _current_time = clock() + seconds * CLOCKS_PER_SEC;
      int usleep(1000 * seconds);
      if(conv_2register2int(timer_counter_high_a,timer_counter_low_a) ==0 ){
	dec_2register(timer_counter_high_a,timer_counter_low_a);
	timer_a_underflow();
	if(cra[3]=='0'){
	  cp_register(timer_latch_low_a,timer_counter_low_a);
	  cp_register(timer_latch_high_a,timer_counter_high_a);      
	}else{
	  cp_register(timer_latch_low_a,timer_counter_low_a);
	  cp_register(timer_latch_high_a,timer_counter_high_a);   
	  cra[0]='1';
	}
      }  else {
	dec_2register(timer_counter_high_a,timer_counter_low_a);
      }
      /* loop until the 10 seconds has reached */
      //    while(clock() < _current_time){}
    }
    pthread_exit(NULL);
}
void timer_a_underflow(){
    icr_read[0]= '1';
    if(ics_write[0]== '1'){
      shm->irq='1';
    }
    if(cra[1]== '1'){
      if(cra[2]== '1'){
	dpb[6]= dpb[6] || dpb[6];
      }else {
	dpb[6]= '1';
      }
    }
}
/* get the current time of work */
int current_time()
{
    return (int) _current_time / CLOCKS_PER_SEC;
}
