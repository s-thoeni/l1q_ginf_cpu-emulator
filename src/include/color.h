/*
  terminal codes for 16 colors
*/

/*
  #define BLACK "\e[1;38;05;000m"
#define WHITE "\e[1;38;05;001m"
#define RED " \e[1;38;05;31m"
#define CYAN " \e[1;38;05;003m"
#define PURPLE " \e[1;38;05;004m"
#define GREEN " \e[1;38;05;005m"
#define BLUE " \e[1;38;05;006m"
#define YELLOW " \e[1;38;05;007m"
#define ORANGE " \e[1;38;05;008m"
#define BROWN " \e[1;38;05;009m"
#define LIGHT " \e[1;38;05;010m"
#define DARK " \e[1;38;05;011m"
#define GRAY " \e[1;38;05;012m"
#define LIGHT_GREEN "\e[1;38;05;013m"
#define LIGHT_BLUE "\e[1;38;05;014m"
#define LIGHT_GRAY "\e[1;38;05;015m"
#define RESET "\e[0;37;40m"
*/
//"\e[m"
#define RESET		0
#define BRIGHT 		1
#define DIM		2
#define UNDERLINE 	3
#define BLINK		4
#define REVERSE		7
#define HIDDEN		8

#define BLACK 		0
#define RED		1
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define CYAN		6
#define	WHITE		7
void print_color(int color);
