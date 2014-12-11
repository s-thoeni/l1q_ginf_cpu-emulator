/*
  src_ripper.h

*/
#define LINE_LENGTH 80

void init_src();
void src_read(char* fname);
void print_src_line(int linenr);
void src_print(int adr);
char * src_str(int adr);
  //void src_str(int adr);
