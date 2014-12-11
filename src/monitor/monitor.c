/*
  monitor.c
  bho - init 11.12.2009
  GPL applies
 */

//#include <config.h>
//#include "monitor.h"


#include <ncurses.h>
//#include <menu.h>
#include <panel.h>

#include "register.h"
#include "cpu.h"
#include "flags.h"
#include "alu.h"
#include "decode.h"
#include "memory.h"
#include "cpu-util.h"
#include "printers.h"
#include "src-ripper.h"
#include "sid.h"
#include <signal.h>

extern char src[MEMORY_SIZE][LINE_LENGTH];
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

#include <panel.h>                                                                                
                                                                                                  
#define NLINES 10                                                                                 
#define NCOLS 40                                                                                  

WINDOW *my_wins[3];                                                                                                                                                           
void init_wins();//WINDOW **wins, int n);                                                             
void win_show(WINDOW *win, char *label, int label_color);                                         
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color); 

void mon_print_opcodeinfo0(){
    struct cpu_operator* optr;
	optr = (struct cpu_operator*) getop(ir);
	printw("CMD:    %s\nopcode: %02x\nfct:    %s\nflags: %s\n", optr?optr->mnemonic:"XXX", conv_bitstr2int(ir,0,7), optr->action, optr->flagsettings);

}
void mon_print_src_line(int adr){
  int i;
  //  if(adr+i<MEMORY_SIZE && adr[i]){
  for(i=0;i<8;i++){
	if(src[adr+i][0]){
	  printw("%s",src[adr+i]);
	}
  }
}

void mon_print_registers(){
  printw(" PC  AC XR YR SP NV-BDIZC IR \n");
  printw("%02x%02x %02x %02x %02x %02x %c%c%c%c%c%c%c%c %02x \n",
		 conv_bitstr2int(pch,0,7),
		 conv_bitstr2int(pcl,0,7),
		 conv_bitstr2int(acc,0,7),
		 conv_bitstr2int(idx,0,7),
		 conv_bitstr2int(idy,0,7),
		 conv_bitstr2int(sp,0,7),

		 getSignflag(flags),
		 getOverflowflag(flags),
		 '1',
		 getBRKflag(flags),
		 getDflag(flags),
		 getIRQflag(flags),
		 getZeroflag(flags),
		 getCarryflag(flags),  
		 
		 conv_bitstr2int(ir,0,7));
}

/* void mon_print_SID_registers(struct sid_6581 *sid){ */
/*   printw(" PC  AC XR YR SP NV-BDIZC IR \n"); */
/*   printw("%02x%02x %02x %02x %02x %02x %c%c%c%c%c%c%c%c %02x \n", */
/* 		 conv_bitstr2int(pch,0,7), */
/* 		 conv_bitstr2int(pcl,0,7), */
/* 		 conv_bitstr2int(acc,0,7), */
/* 		 conv_bitstr2int(idx,0,7), */
/* 		 conv_bitstr2int(idy,0,7), */
/* 		 conv_bitstr2int(sp,0,7), */

/* 		 getSignflag(flags), */
/* 		 getOverflowflag(flags), */
/* 		 '1', */
/* 		 getBRKflag(flags), */
/* 		 getDflag(flags), */
/* 		 getIRQflag(flags), */
/* 		 getZeroflag(flags), */
/* 		 getCarryflag(flags),   */
		 
/* 		 conv_bitstr2int(ir,0,7)); */
/* } */

void monitor_print_mem(int mem_index, int rows, int cols){
   int i,j;
  int pc = pc2int();
  clear();//move(0,0);

  mon_print_opcodeinfo0();
  mon_print_registers();
  mon_print_src_line(pc);
	
  printw("\n     ");
  for(i=0;i<cols;i++){
	printw("%02x ",(mem_index+i)%256);
  }
  i=0;
  for(j=0; j<rows; ++j) {
    printw("\n%04x ",	mem_index +j*cols);
    for(i=0; i<cols; ++i) {
	  if(pc==mem_index+ j*cols+i){
		printw("%02x ",conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
		//print_color(conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
	  //printf(RED "%02x " RESET ,conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
	  } else {
		printw("%02x ",conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
		//		mvprintw(0,0,"%02x",conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
		
		//print_color(conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
		//		printf("%02x ",	conv_bitstr2int(memory[mem_index + j*cols+i],0,7));
	  }
    }
  }
  printw("\n");
  doupdate();
  refresh();
}
void quit_monitor(){
  endwin();
}
int init_monitor() {
			  
        PANEL  *my_panels[3];                                                                     
        PANEL  *top;                                                                              
        int ch;                                                                                   
                                                                                                  
        /* Initialize curses */                                                                   
        initscr();                                                                                
        start_color();                                                                            
        cbreak();                                                                                 
		echo();                                                                                 
        keypad(stdscr, TRUE);                                                                     
                                                                                                  
        /* Initialize all the colors */                                                           
        init_pair(1, COLOR_RED, COLOR_BLACK);                                                     
        init_pair(2, COLOR_GREEN, COLOR_BLACK);                                                   
        init_pair(3, COLOR_BLUE, COLOR_BLACK);                                                    
        init_pair(4, COLOR_CYAN, COLOR_BLACK);                                                    
                                                                                                  
		init_wins();//my_wins, 3);                                                                    
                                                                                                  
                                                                                                  
        /* Show it on the screen */                                                               
        attron(COLOR_PAIR(4));                                                                    
		//     mvprintw(LINES - 2, 0, "Use tab to browse through the windows (F1 to Exit)");             

		//		monitor_print_mem(pc2int(),4, 8);                                                                            
        doupdate();
		attroff(COLOR_PAIR(4));
		//		getch();
}                                                                                                 
void quit_monitor_handler(int dummy) {
  dummy++;
  quit_monitor();
}
                                                                      
/* Put all the windows */                                                                         
void init_wins()//WINDOW **wins, int n)                                                              
{       int x, y, i;                                                                              
        char label[80];                                                                           
		signal(SIGSEGV, quit_monitor_handler);                                                                                        
        y = 2;                                                                                    
        x = 10;                                                                                   
		newwin(NLINES, NCOLS, y, x);
		/*
		  for(i = 0; i < n; ++i)                                                                    
        {       wins[i] = newwin(NLINES, NCOLS, y, x);                                            
                sprintf(label, "Window Number %d", i + 1);                                        
                win_show(wins[i], label, i + 1);                                                  
                y += 3;                                                                           
                x += 7;                                                                           
				}
		*/
}                                                                                                 
                                                                                                  
/* Show the window with a border and a label */                                                   
void win_show(WINDOW *win, char *label, int label_color)                                          
{       int startx, starty, height, width;                                                        
                                                                                                  
        getbegyx(win, starty, startx);                                                            
        getmaxyx(win, height, width);                                                             
                                                                                                  
        box(win, 0, 0);                                                                           
        mvwaddch(win, 2, 0, ACS_LTEE);                                                            
        mvwhline(win, 2, 1, ACS_HLINE, width - 2);                                                
        mvwaddch(win, 2, width - 1, ACS_RTEE);                                                    
                                                                                                  
        print_in_middle(win, 1, 0, width, label, COLOR_PAIR(label_color));                        
}                                                                                                 
                                                                                                  
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)  
{       int length, x, y;                                                                         
        float temp;                                                                               
                                                                                                  
        if(win == NULL)                                                                           
                win = stdscr;                                                                     
        getyx(win, y, x);                                                                         
        if(startx != 0)                                                                           
                x = startx;                                                                       
        if(starty != 0)                                                                           
                y = starty;                                                                       
        if(width == 0)                                                                            
                width = 80;                                                                       
                                                                                                  
        length = strlen(string);                                                                  
        temp = (width - length)/ 2;                                                               
        x = startx + (int)temp;                                                                   
        wattron(win, color);                                                                      
        mvwprintw(win, y, x, "%s", string);                                                       
        wattroff(win, color);                                                                     
        refresh();                                                                                
}

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

void init_monitor0(){
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
  mvprintw(gp_register_row+0,gp_register_col,"ax : %s  ", acc);
  mvprintw(gp_register_row+1,gp_register_col,"bx : %s  ", idx);
  mvprintw(gp_register_row+2,gp_register_col,"cx : %s  ", idy);
  mvprintw(gp_register_row+3,gp_register_col,"dx : %s  ", ir); 
}

void monitor_memory(unsigned int start, unsigned int count){
  unsigned int i=0;
  int pc_adr = 256 * conv_addr2int(pch) + conv_addr2int(pcl);;
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
	ax_back[i] = acc[i];
	bx_back[i] = idx[i];
	cx_back[i] = idy[i];
	dx_back[i] = pcl[i];
	ir_back[i] = ir[i];
	pc_back[i] = pch[i];
	sp_back[i] = sp[i];
	abr_back[i] = abrl[i];
	dbr_back[i] = dbr[i];
	ir_back[i] = ir[i];
  }
}

