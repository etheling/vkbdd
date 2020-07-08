/* BSD 2-Clause License

Copyright (c) 2020, Etheling Lydas (alias). All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <signal.h>

#include <fcntl.h>
#include <unistd.h> 
#include <sys/stat.h> 
#include <sys/types.h> 

#define VERSION "1.0"

// Minimum terminal size to display kbd
#define MINX 75
#define MINY 16

// for handling sigint, sigpipe
static void finish(int sig);
static void finish(int sig) {  
  endwin();
  exit(0);
}

char * myfifo = "/tmp/vkbdd.fifo"; 

int bpos=0; // position in display
int toosmall=0; // if display is too small

#define KBD_ROWS 11
char kbd[KBD_ROWS][74] = {
		  //        1         2         3         4         5         6         7         8
		  //12345678901234567890123456789012345678901234567890123456789012345678901234567890
		   " [                                                                      ]",   // 1
		   "                                                                        ",    // 2
		   "  . -------------------------------------------------------------------.",    // 3
		   "  | [Esc] [F1][F2][F3][F4][F5][F6][F7][F8][F9][F10][F11][F12] :...     |",    // 4
		   "  |                                                                    |",    // 5
		   "  | [`][1][2][3][4][5][6][7][8][9][0][-][=][_<_] [I][H][U] [N][/][*][-]|",    // 6
		   "  | [|-][q][w][e][r][t][y][u][i][o][p][{][}][ |] [D][E][D] [7][8][9][+]|",    // 7
		   "  | [CAP][a][s][d][f][g][h][j][k][l][;]['][entr]           [4][5][6]| ||",    // 8
		   "  | [_^_] [z][x][c][v][b][n][m][,][.][/] [__^__]    [^]    [1][2][3]|e||",    // 9
		   "  | [c]   [a][________________________][a]   [c] [<][v][>] [ 0  ][.]|_||",    // 10
		   "  `--------------------------------------------------------------------'"     // 11
};

#define HELP_ROWS 5
char hlp[HELP_ROWS][74] = {		   	   " [                                                                      ]",    // 1
                   "                                                                         ",    // 2			   
		   "   CTRL+a: capslock    CTRL+e: left shift    CTRL+h: right alt           ",	   // 3
		   "   CTRL+b: left ctrl   CTRL+f: right shift                               ",    // 4
		   "   CTRL+d: right ctrl  CTRL+g: left alt      CTRL+c: exit                "     // 5
};

#define DWIDTH 69
int display[DWIDTH];


struct ck {
  int x;
  int y;
  int i; // index to color
  char key[5];
};

#define CKEYS 42
struct ck ckmap[CKEYS] = {
		6,5,7,"Esc",                       //1
		12,5,7,"F1",          
		16,5,7,"F2",
		20,5,7,"F3",
		24,5,7,"F4",
		28,5,7,"F5",
		32,5,7,"F6",
		36,5,7,"F7",
		40,5,7,"F8",
		44,5,7,"F9",
		48,5,7,"F10",
		53,5,3,"F11",
		58,5,3,"F12",

		51,7,7, "I", // Insert
		51,8,7, "D", // Delete
		54,7,7, "H", // Home
		54,8,7, "E", // End
		57,7,5, "U", // PgUp
		57,8,5, "D",   // PgDown

		44,9,3, "entr",  // Enter

		46,7,5, "<",  // Backspace /delete
		
		61,8,5, "7",  // keypad
		64,8,5, "8",  // 
		67,8,5, "9",  // 
		61,9,5, "4",  // 
		64,9,5, "5",  // 
		67,9,5, "6",  // 
		61,10,5, "1", //
		64,10,5, "2", // 
		67,10,5, "3", //
		70,10,5, "e", // 		
		62,11,5, "0",  //

		7,10,7,  "^",  // left shift
		45,10,5, "^",  // right shift
		6,11,7,  "c",  // left ctrl
		47,11,5, "c",  // right ctrol
		12,11,7, "a",  // left alt
		41,11,5, "a",  // right alt		
		
		51,11,3, "<", // left arrow
		54,10,3, "^", // 
		54,11,3, "v", // 
		57,11,3, ">"  //
};

int capsflag = 0;

#define MAPS 512
char maps[MAPS][2];
void init_maps() {
  for (int c = 0 ; c < MAPS ; c++) {
    maps[c][0]= '.' ; maps[c][1] = 3; 
  }

  maps[1][0]   = 'C' ; maps[1][1] = 7; // CAPS LOck / CTRL+A
  maps[2][0]   = 'c' ; maps[2][1] = 7; // left ctrl
  maps[4][0]   = 'c' ; maps[4][1] = 5; // right ctrl
  maps[5][0]   = '^' ; maps[5][1] = 7; // left shift
  maps[6][0]   = '^' ; maps[6][1] = 5; // right shift 
  maps[7][0]   = 'a' ; maps[7][1] = 7; // left alt
  maps[8][0]   = 'a' ; maps[8][1] = 5; // right alt 
  
  // ..
  maps[9][0]   = 'T' ; maps[9][1] = 3; // TAB
  maps[13][0]  = 'e' ; // Enter 
  maps[27][0]  = 'E' ; maps[27][1] = 7; // ESC
  //...
  maps[258][0] = 'v' ; // down arrow
  maps[259][0] = '^' ; // up arrow
  maps[260][0] = '<' ; // left arrow
  maps[261][0] = '>' ; // right arrow
  maps[262][0] = 'H' ; maps[262][1] = 7; // Home
  maps[263][0] = '<' ; maps[263][1] = 7; // delete
  // ...
  maps[265][0] = '1' ; maps[265][1] = 7; // F1
  maps[266][0] = '2' ; maps[266][1] = 7; // F2
  maps[267][0] = '3' ; maps[267][1] = 7; // F3
  maps[268][0] = '4' ; maps[268][1] = 7; // F4
  maps[269][0] = '5' ; maps[269][1] = 7; // F5    
  maps[270][0] = '6' ; maps[270][1] = 7; // F6
  maps[271][0] = '7' ; maps[271][1] = 7; // F7
  maps[272][0] = '8' ; maps[272][1] = 7; // F8
  maps[273][0] = '9' ; maps[273][1] = 7; // F9
  maps[274][0] = '0' ; maps[274][1] = 7; // F10
  maps[275][0] = '1' ; maps[275][1] = 3; // F12
  maps[276][0] = '2' ; maps[276][1] = 3; // F12  

  // ...

  
  maps[330][0] = '^' ; // backspace
  maps[330][0] = 'D' ; maps[330][1] = 7; // Del
  maps[331][0] = 'I' ; maps[331][1] = 7; // Insert      
  maps[338][0] = 'D' ; maps[338][1] = 5; // PgDn
  maps[339][0] = 'U' ; maps[339][1] = 5; // PgUp
  maps[343][0] = 'e' ; maps[343][1] = 5; // keypad Enter
  maps[360][0] = 'E' ; maps[360][1] = 7; // End
}

void ui_mvaddstr (int cp, int y, int x, char* message) {
  attron(COLOR_PAIR(cp));
  mvaddstr (y,x,message);
  attroff(COLOR_PAIR(cp));
}

void ui_banner (int xx) {
  for (int p=0; p<xx; p++) {
    ui_mvaddstr (8,0,p," ");
  }
  ui_mvaddstr (8,0,2,"Virtual Keyboard for RetroPie. CTRL+C to exit.");
}

void ui_draw_keyboard(void) {
  for (int i = 0 ; i<KBD_ROWS; i++) {
    mvaddstr(2+i, 1, kbd[i]);
  }
}

void ui_color_keys(void) {
  for (int y=0; y<CKEYS; y++) {
    ui_mvaddstr (ckmap[y].i, ckmap[y].y,ckmap[y].x,ckmap[y].key);
  }
}

void ui_update_emitchr (int c) {
  int tc;
  char chstr[6];
  int y = 5;
  int x1 = 67;
  int x2 = 68;

  if (toosmall) {
    y=1; x1 = 4; x2=5;
  } 

  tc =c;
  if (c<255) {
    mvaddstr (y, x1, "    ");
  } else {
    ui_mvaddstr (7,y, x1, ">");
    tc = c - 127;
    snprintf(chstr, 5, "%d", tc); // always 3 chars; no need to erase?
    ui_mvaddstr (2,y, x2, chstr);    
  }

  if (!toosmall) {
    int octno = 0, i = 1;
    char chbuffer[80];
    while (tc != 0)
    {
        octno += (tc % 8) * i;
        tc /= 8;
        i *= 10;
    }
    snprintf (chbuffer, 80, "printf '\\%03d' > %s", octno, myfifo);

    ui_mvaddstr (4,14, 4, "$");
    ui_mvaddstr (0,14, 6, chbuffer);


  }
  
}

void ui_update_display_buff (int cpos) {
  char plot[2];
  plot[0]=0; plot[1]=0;
  for (int i = 0; i<DWIDTH; i++) {
    if (display[i] >= 32 && display[i] <= 127) {
      plot[0] = (char) display[i];
      mvaddstr (2,3+i,plot);
    } else {
      attron(COLOR_PAIR(maps[display[i]][1]));
      mvaddch (2,3+i,maps[display[i]][0]);
      attroff(COLOR_PAIR(maps[display[i]][1]));
    }
  }
  move (2,3+cpos); // cursor position
}

void ui_rotate_display_buff (int ch) {
  if (bpos==DWIDTH) {
    for (int j=0; j<DWIDTH-1; j++) {
      display[j] = display[j+1];
    }
    display[DWIDTH-1]=ch;
  } else {
    display[bpos]=ch;
    bpos++;
  }
}

void ui_update_ch_on_kb (int ch) {
  int y = 5;
  int x = 64;
  if (toosmall) {
    y=1; x=1; mvaddstr (y, x, "                            "); // clean away too long message
  }
  // print character code on keyboard
  char chstr[5];
  snprintf(chstr, 5, "%d", ch);
  ui_mvaddstr (2, y, x, chstr);
  for (int z = strlen(chstr); z<3 ; z++) {
    addch(' ');
  }
}

void ui_help_banner (void) {
  int base=14;
  for (int i = 0 ; i<HELP_ROWS; i++) {
    //if (i+base > yy) { break; } // don't go below di
    mvaddstr(base+i, 1, hlp[i]);
  }
}

void write_fifo (char * fifo, int fck) {
  int fd;
  char cz;

  if (fck>381) { return ; } // if > 255+127 or resize etc. -> do early exit
                            // FIXME: DEBUG LOGGING ON WHAT WE EMIT
  
  if (fck<255) {
    cz = (char) fck;
  } else {
    cz = (char) fck - 127;
  }
  
  fd = open(fifo, O_WRONLY);
  if (fd < 0) {
    // FIXME: wont print because ncurses
    printf ("ERROR: cannot connect to vkbdd via %s. Aborting.\n", fifo);
    finish (9);
  }
  
  write(fd, &cz, 1);   
  close (fd);
}

void ui_update_capslockflag (void) {
  if (capsflag == 0) {
    capsflag = 1;
  } else {
    capsflag = 0;
  }
}
void ui_update_capslockui (void) {
  if (capsflag == 1) {
    ui_mvaddstr (7, 9, 6, "CAP");
  } else {
    ui_mvaddstr (0,9, 6, "CAP");
  }
}


int main(int argc, char *argv[])
{
  WINDOW * mainwin;
  (void) signal(SIGINT, finish);
  (void) signal(SIGTERM, finish);
  (void) signal(SIGPIPE, finish);   

  int fd; // FIFO file descriptor

  int cl;
  int simulate=0;
  
  int newy, newx;
  char minmsg[80];	
  snprintf (minmsg, 80, "Minimum terminal is %dx%d", MINX, MINY);

    while ((cl = getopt (argc, argv, "hp:sv")) != -1) {
      switch (cl)
	{
	case 'h':
	  printf ("Virtual Keyboard for RetroPie - https://github.com/etheling/vkbdd\n");
	  printf ("\nUsage: %s [OPTION..]\n", argv[0]);
	  printf ("  -h        print this help message\n");
	  printf ("  -p <file> path/file to named pipe to read (default /tmp/vkbdd.fifo)\n");
	  printf ("  -s        simulate and don't connnect to or send keypresses to named pipe\n");
	  printf ("  -v        print program version and exit\n\n");   	  	  
	  return 1;
	case 'p':
	  if (strlen(optarg) > 255) {
	    printf ("\nERROR: path/file too long (max is 255). Aborting.\n");
	    return 1;
	  }
	  myfifo = optarg;
	  break;
	case 's':
	  simulate=1;
	  break;
	case 'v':
	  printf ("Version %s\n", VERSION);
	  return 1;
	case '?':
	  return 1;
	default:
	  abort ();
	}
    }


  
    // test that we can connect - if FIFO exists, but there isn't reader, we stuck
    if (!simulate) {
      printf ("Connecting to vkbdd FIFO %s. CTRL+C to abort.\n", myfifo);
      fd = open(myfifo, O_WRONLY);
      if (fd < 0) {
	printf ("ERROR: cannot connect to vkbdd via %s. Aborting.\n", myfifo);
	return 1;
      }
      close (fd);
    }

    if ( (mainwin = initscr()) == NULL ) {
      fprintf(stderr, "Error initialising ncurses. Aborting.\n");
      return 1;
    }
  
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    nonl();         /* tell curses not to do NL->CR/NL on output */
    cbreak();       /* take input chars one at a time, no wait for \n */
    noecho();   
    getmaxyx(mainwin, newy, newx);
    
    if (has_colors()) {
      start_color();
      
      // https://www.linuxjournal.com/content/about-ncurses-colors-0
      init_pair(1, COLOR_RED,     COLOR_BLACK);
      init_pair(2, 10,   COLOR_BLACK); // Bright Green
      init_pair(3, 11,  COLOR_BLACK); // Bright Yellow
      init_pair(4, COLOR_GREEN,    COLOR_BLACK);
      init_pair(5, 14,    COLOR_BLACK); // Bright Cyan
      init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
      init_pair(7, 15,   COLOR_BLACK);
      init_pair(8, 15,   COLOR_BLUE);    
    } else {
      endwin();
      printf("Your terminal does not support color. Aborting.\n");
      return (1);
    }

    init_maps();     // initialize ...
    int cpos=0; // cursor position
    for (int i = 0 ; i<DWIDTH; i++) { display[i]=32; }  // init display buffer  
    
    if (newy < MINY || newx < MINX) {
      toosmall = 1;
      wclear(mainwin);
      mvaddstr (1, 1, minmsg);    
    } else {
      ui_draw_keyboard();  // draw keyboard on screen
      ui_color_keys(); // color-keys on kbd
      ui_banner(newx);
      ui_help_banner();
      ui_update_display_buff(cpos);      
    }
    
    for (;;) {
    
      int ch = getch();        // read key
      if (!simulate) {
	write_fifo (myfifo, ch); // write read character to FIFO
      }
      
      if (ch < 382) { // 255+127 > resize etc. 
	if (cpos<DWIDTH) { cpos++; } /* increment until 'display' right edge      */
	if (ch == 1) { ui_update_capslockflag (); }
	
	/* If ctrl+a update capslock flag */
	ui_rotate_display_buff (ch); // add character to display[]
	
	if (!toosmall) {
	  ui_update_ch_on_kb (ch); // print character code on kb
	  ui_update_emitchr(ch); // print translated code
	  ui_update_capslockui (); // redraw ui	
	  ui_update_display_buff(cpos); // redraw display buffer	
	} else { ui_update_ch_on_kb (ch); ui_update_emitchr(ch); }      
      } else {
	getmaxyx(mainwin, newy, newx);
	if (newy < MINY || newx < MINX) {
	  toosmall = 1;
	  wclear(mainwin);
	  mvaddstr (1, 1, minmsg);
	} else {
	  if (toosmall) {
	    toosmall=0;
	    wclear(mainwin);
	    // redraw
	    ui_draw_keyboard();  // draw keyboard on screen
	    ui_banner(newx);
	    ui_help_banner();
	    ui_color_keys(); // color-keys on kbd
	    ui_update_capslockui ();
	    ui_update_display_buff(cpos);	  
	  }
	}
      }
    }
    
    endwin(); // finish
}

