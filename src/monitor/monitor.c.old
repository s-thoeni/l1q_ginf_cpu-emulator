/*
  monitor.c
  bho - init 11.12.2009
  GPL applies
 */

//#include <config.h>
//#include "monitor.h"


#include <ncurses.h>
#include <menu.h>

#include "alu.h"
#include "cpu.h"
#include "memory.h"
#include "register.h"

int gp_register_row = 3;
int gp_register_col = 1;

int mem_row = 3;
int mem_col = 40;

int stack_row = 3;
int stack_col = 64;


// backup register
char dbr_back[17];
char abr_back[17];
char rw_back;
char ir_back[17];
char ax_back[17], bx_back[17], cx_back[17],dx_back[17];
char sp_back[17];
char pc_back[17];

int reg2val(char* reg){
  int i=0;
  int sum = 0;
  for(i=0; i<REG_WIDTH;i++)
    if(reg[i] == '1'){
      sum = (sum << 1) + 1;
    }else
      sum = sum << 1;
  return sum;
}

void init_monitor(){
  initscr();			/* Start curses mode 		  */

  start_color();			/* Start color 			*/

  cbreak();
  noecho();
  keypad(stdscr, TRUE);

}

void exit_monitor(){
  endwin();			/* End curses mode		  */
}


void print_general_purpose_register(){
  mvprintw(gp_register_row+0,gp_register_col,"ax : %s  ", ax);
  mvprintw(gp_register_row+1,gp_register_col,"bx : %s  ", bx);
  mvprintw(gp_register_row+2,gp_register_col,"cx : %s  ", cx);
  mvprintw(gp_register_row+3,gp_register_col,"dx : %s  ", dx); 
}

void monitor_memory(unsigned int start, unsigned int count){
  unsigned int i=0;
  int pc_adr = conv_addr2int(pc);
  mvprintw(mem_row-1, mem_col, "mem adr.  binary ");
  for(i=0; i < count ; i++) 
	if( start+i == pc_adr )
	  mvprintw(mem_row+i, mem_col-5, "pc-->%04x   %s ", start+i, memory[start+i]);
	else
	  mvprintw(mem_row+i, mem_col, "%04x   %s ", start+i, memory[start+i]);
}

void monitor_stack(unsigned int size){
  unsigned int i=0;
  int sp_adr = conv_addr2int(sp);
  int start = sp_adr - size/2;
  start = start<0?0:start;
  mvprintw(stack_row-1, stack_col, "stack adr.  binary ");
  for(i=0; i < size ; i++) 
	if( start+i == sp_adr )
	  mvprintw(stack_row+i, stack_col-5, "sp-->%04x   %s ", start+i, memory[start+i]);
	else
	  mvprintw(stack_row+i, stack_col, "%04x   %s ", start+i, memory[start+i]);
}


void save_monitor(){
  int i;
  for (i=0; i< 16; i++){
	ax_back[i] = ax[i];
	bx_back[i] = bx[i];
	cx_back[i] = cx[i];
	dx_back[i] = dx[i];
	ir_back[i] = ax[i];
	pc_back[i] = ax[i];
	sp_back[i] = ax[i];
	abr_back[i] = addressbus_register[i];
	dbr_back[i] = databus_register[i];
	ir_back[i] = ir[i];
	rw_back = read_write;
  }
}


void print_registers(){
  struct monitored_reg {
	char *reg;
	char reg_name[4];
  };

  struct monitored_reg r[9] =
	{ {reg: ax, reg_name: "ax"},
	  {reg: bx, reg_name: "bx"},
	  {reg: cx, reg_name: "cx"},
	  {reg: dx, reg_name: "dx"},
	  {reg: ir, reg_name: "ir"},
	  {reg: pc, reg_name: "pc"},
	  {reg: sp, reg_name: "sp"},
	  {reg: addressbus_register, reg_name: "ABR"},
	  {reg: databus_register, reg_name: "DBR"}
	};
  
  //  char* regs[] = { ax, bx, cx, dx, ir, pc, sp, addressbus_register, databus_register};
  unsigned int i;
  int val;
  int size = 9;//sizeof(regs)/sizeof(char *);
  for(i=0; i<size; i++){
	val = reg2val(r[i].reg);
	mvprintw(gp_register_row+i,gp_register_col,"%9s : %s %04x", r[i].reg_name, r[i].reg,val);
  }
  /*
    int size = sizeof(regs)/sizeof(char *);
	for(i=0; i<size; i++){
	val = reg2val(regs[i]);
	mvprintw(gp_register_row+i,gp_register_col,"ir : %s 0x%x", regs[i],val);
	}
  */
}
  
void monitor(char what[]){
  save_monitor();
  
  clear();  //screen

  mvprintw(1, 20, "%s", what);

  init_pair(1, COLOR_RED, COLOR_BLACK);
  
  attron(COLOR_PAIR(1));
  


  int i = conv_addr2int(pc) - 5;
	
  monitor_memory(i<0?0:i,10);

  monitor_stack(16);
  /*
  print_general_purpose_register();
  attroff(COLOR_PAIR(1));
  
  mvprintw(gp_register_row+4,gp_register_col,"ir : %s ", ir);
  mvprintw(gp_register_row+5,gp_register_col,"sp : %s ", sp);
  mvprintw(gp_register_row+6,gp_register_col,"PC : %s ", pc);

  mvprintw(gp_register_row+7,gp_register_col,"DBR: %s ", databus_register);
  mvprintw(gp_register_row+8,gp_register_col,"ABR: %s ", addressbus_register);
  */
  
  print_registers(); 

    
  refresh();			/* Print it on to the real screen */

  /* HANDLE USER KEYSTROKES */
  //  set_curses(1);
  int my_choice = -1;
  int c;

  
  while(my_choice == -1)
	{
	  c = getch();
	  switch(c)
		{
		case KEY_DOWN:
		  mvprintw(gp_register_row+5,gp_register_col,"key : keadownKEYDOWN %i ",c);
		  i = 12345;
		  move(3,4);
		  break;
		case KEY_UP:
		  move(3,5);
		  break;
		case 10:	/* Enter */
		  my_choice = 1;
		  
		  /* RESET CURSOR IN CASE MORE SELECTION IS NECESSARY */
		  refresh(); 
		  //		  pos_menu_cursor(my_menu);
		  break;
		}
	  printw("i= %i :: KEY_DOWN= %i :: key : %i ",i,KEY_DOWN, c);
	  mvprintw(gp_register_row+4,gp_register_col,"i= %i :: KEY_DOWN= %i :: key : %i ",i,KEY_DOWN, c);
	  i = 9;
	}
	//getch();			/* Wait for user input */


}
