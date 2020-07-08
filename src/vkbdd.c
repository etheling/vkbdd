// FIXME: warnings on Ubuntu

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

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 

/* For file ownership/permission change */
#include <pwd.h>
#include <grp.h>

#include <linux/input.h>
#include <linux/uinput.h>

#define VERSION "1.0"
#define BUFFSIZE 1

// debug flag, logfile handle, and log buffer. 
#define LBSIZE 200
int debug = 1;
int dfd = 0;
char logbuffer[LBSIZE];

static void finish(int sig) {  
  if (dfd > 0) {
    snprintf (logbuffer, LBSIZE, "finish(): reveived signal %d. Exiting.", sig);
    write(dfd, logbuffer, strlen(logbuffer));
    close (dfd);
  }
  exit (1);
}

int open_debuglog (void) {
  int tfd = open("/tmp/vkbdd.debug.log",O_CREAT|O_RDWR|O_APPEND);
  lseek (tfd, 0, SEEK_END);

  chmod("/tmp/vkbdd.debug.log", 0644);
  return tfd;
}

int wlog (char* message) {
  if(!write(dfd, message, strlen(message))){
    printf("ERROR: unable to write to debug log. Aborting.\n");
    finish(10);
  }
  return 0;
}

// https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
int mapkey (int key) {
  switch (key) {

    // direct from ncurses
  case  0:  break;
  case  1:  return (KEY_CAPSLOCK);
  case  2:  return (KEY_LEFTCTRL);
    // 3 --> CTRL+C
  case  4:  return (KEY_RIGHTCTRL);
  case  5:  return (KEY_LEFTSHIFT);
  case  6:  return (KEY_RIGHTSHIFT);
  case  7:  return (KEY_LEFTALT);
  case  8:  return (KEY_RIGHTALT);        
  case  9:  return (KEY_TAB);     
  case 13:  return (KEY_ENTER);
  case 27:  return (KEY_ESC);
  case 32:  return (KEY_SPACE); // ' '
    // ...
  case 49:  return (KEY_1); // 1
  case 33:  return (KEY_1); // !
  case 50:  return (KEY_2); // 2
  case 64:  return (KEY_2); // @
  case 51:  return (KEY_3); // 3
  case 35:  return (KEY_3); // #
  case 52:  return (KEY_4); // 4
  case 36:  return (KEY_4); // $
  case 53:  return (KEY_5); // 5
  case 37:  return (KEY_5); // %
  case 54:  return (KEY_6); // 6
  case 94:  return (KEY_6); // ^
  case 55:  return (KEY_7); // 7
  case 38:  return (KEY_7); // &
  case 56:  return (KEY_8); // 8
  case 42:  return (KEY_8); // *
  case 57:  return (KEY_9); // 9
  case 40:  return (KEY_9); // (
  case 48:  return (KEY_0); // 0
  case 41:  return (KEY_0); // )
    //...
  case 39:  return (KEY_APOSTROPHE); // '
  case 44:  return (KEY_COMMA);
  case 45:  return (KEY_MINUS); // -
  case 46:  return (KEY_DOT);
  case 47:  return (KEY_SLASH);

  case 59:  return (KEY_SEMICOLON);

  case 61:  return (KEY_EQUAL); // =

  case 91:  return (KEY_LEFTBRACE); // [
  case 92:  return (KEY_BACKSLASH); //     
  case 93:  return (KEY_RIGHTBRACE); // ]
  case 96:  return (KEY_GRAVE); // ` 
    
  case 97:  return (KEY_A); // a
  case 65:  return (KEY_A); // A    
  case 98:  return (KEY_B); // b
  case 66:  return (KEY_B); // B    
  case 99:  return (KEY_C); // c
  case 67:  return (KEY_C); // C    
  case 100: return (KEY_D); // d
  case 68:  return (KEY_D); // D    
  case 101: return (KEY_E); // e
  case 69:  return (KEY_E); // E    
  case 102: return (KEY_F); // f
  case 70:  return (KEY_F); // F    
  case 103: return (KEY_G); // g
  case 71:  return (KEY_G); // G    
  case 104: return (KEY_H); // h
  case 72:  return (KEY_H); // H    
  case 105: return (KEY_I); // i
  case 73:  return (KEY_I); // I    
  case 106: return (KEY_J); // j
  case 74:  return (KEY_J); // J    
  case 107: return (KEY_K); // k
  case 75:  return (KEY_K); // K    
  case 108: return (KEY_L); // l
  case 76:  return (KEY_L); // L    
  case 109: return (KEY_M); // m
  case 77:  return (KEY_M); // M
  case 110: return (KEY_N); // n
  case 78:  return (KEY_N); // N
  case 111: return (KEY_O); // o
  case 79:  return (KEY_O); // O    
  case 112: return (KEY_P); // p
  case 80:  return (KEY_P); // P    
  case 113: return (KEY_Q); // q
  case 81:  return (KEY_Q); // Q   
  case 114: return (KEY_R); // r
  case 82:  return (KEY_R); // R    
  case 115: return (KEY_S); // s
  case 83:  return (KEY_S); // S    
  case 116: return (KEY_T); // t
  case 84:  return (KEY_T); // T    
  case 117: return (KEY_U); // u
  case 85:  return (KEY_U); // U    
  case 118: return (KEY_V); // v
  case 86:  return (KEY_V); // V    
  case 119: return (KEY_W); // w
  case 87:  return (KEY_W); // W    
  case 120: return (KEY_X); // x
  case 88:  return (KEY_X); // X    
  case 121: return (KEY_Y); // y
  case 89:  return (KEY_Y); // Y    
  case 122: return (KEY_Z); // z
  case 90:  return (KEY_Z); // Z    
    
  // ...
  // rempped from ncurses: FIXME formula
  case 131:  return (KEY_DOWN);
  case 132:  return (KEY_UP);
  case 133:  return (KEY_LEFT);
  case 134:  return (KEY_RIGHT);
  case 135:  return (KEY_HOME);
  case 136:  return (KEY_BACKSPACE); 
    
  case 138:   return (KEY_F1); // 59
  case 139:   return (KEY_F2); // ..
  case 140:   return (KEY_F3);
  case 141:   return (KEY_F4);
  case 142:   return (KEY_F5); // 
  case 143:   return (KEY_F6);
  case 144:   return (KEY_F7);
  case 145:   return (KEY_F8);
  case 146:   return (KEY_F9);
  case 147:   return (KEY_F10); // 68
  case 148:   return (KEY_F11);
  case 149:   return (KEY_F12);

  case 203:   return (KEY_DELETE);
  case 204:   return (KEY_INSERT);

  case 211:   return (KEY_PAGEDOWN);
  case 212:   return (KEY_PAGEUP);
  case 216:   return (KEY_KPENTER);
  case 233:   return (KEY_END);
  }

  if (debug) {
    snprintf (logbuffer, LBSIZE, "mapkey(): no mapping found for: %d\n", key);
    wlog (logbuffer);
  }
  
  return 0;
}

// inspired by https://www.kernel.org/doc/html/v4.12/input/uinput.html
int setup_uinput () {
  struct uinput_setup usetup;
  int ret;
  
  int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (fd <= 0) {
    printf ("ERROR: cannot open /dev/uinput. Are you not r00t or other permission problem?\n");
    return -1;
  }

  if (debug) {
    snprintf (logbuffer, LBSIZE, "setup_uinput(): Opened /dev/uinput handle %d\n", fd);
    wlog (logbuffer);
  }

  // FIXME: purpose of this line?
  ioctl(fd, UI_SET_EVBIT, EV_KEY);
  
  // allow all characters to come in
  for (int ch = 1; ch < 256; ch++) {
    ret = ioctl(fd, UI_SET_KEYBIT, ch);
    
  }
  
  memset(&usetup, 0, sizeof(usetup));
  usetup.id.bustype = BUS_USB;
  usetup.id.vendor = 0x1234; /* sample vendor */
  usetup.id.product = 0x5678; /* sample product */
  strcpy(usetup.name, "Virtual Keyboard for RetroPie");

  ret = ioctl(fd, UI_DEV_SETUP, &usetup);
  if (debug) {
    snprintf (logbuffer, LBSIZE, "setup_uinput(): UI_DEV_SETUP: %d\n", ret);
    wlog (logbuffer);
  }
  
  ret = ioctl(fd, UI_DEV_CREATE);
  if (debug) {
    snprintf (logbuffer, LBSIZE, "setup_uinput(): UI_DEV_CREATE: %d\n", ret);
    wlog (logbuffer);
  }
  
  snprintf (logbuffer, LBSIZE, "Keyboard setup complete (name: %s, USB-ID: %x:%x)\n",
	   usetup.name,usetup.id.vendor,usetup.id.product);
  printf ("%s", logbuffer);
  if (debug) { wlog (logbuffer); }
  return fd;
}

void emit(int fd, int type, int code, int val)
{
  struct input_event ie;
  int ret;
  
  ie.type = type;
  ie.code = code;
  ie.value = val;
  gettimeofday(&ie.time, 0);
  
  ret = write(fd, &ie, sizeof(ie));
  
  if (debug) {
    snprintf (logbuffer, LBSIZE,
	      "emit(): Wrote ie.type:%d, ie.code:%d, ie.value:%d, ie.time:%ld.%ld\nemit(): fd: %d. ret:%d \n",
	      type, code, val, ie.time.tv_sec, ie.time.tv_usec, fd,ret);
    wlog (logbuffer);
  }
}


int file_exists (char *filename) {
  if (access(filename, F_OK) != -1) { return 1; }
  return 0;
}

int do_chown (const char *file_path, const char *user_name, const char *group_name) {
  uid_t          uid;
  gid_t          gid;
  struct passwd *pwd;
  struct group  *grp;

  if (debug) {
    snprintf (logbuffer, LBSIZE, "do_chown(): chown %s:%s %s\n",user_name,group_name,file_path);
    wlog (logbuffer);
  }


  pwd = getpwnam(user_name);
  if (pwd == NULL) {
    wlog ("do_chown(): getpwnam() returned null.");
    return 1;
  }
  uid = pwd->pw_uid;

  grp = getgrnam(group_name);
  if (grp == NULL) {
    wlog ("do_chown(): getgrnam() returned null.");
    return 1;
  }
  gid = grp->gr_gid;

  if (chown(file_path, uid, gid) == -1) {
    wlog ("do_chown(): chown() returned error.");
    return 1;
  }

  return 0;
}

int send_keypress (int fd, char key) {
   /* Key press, report the event, send key release, and report again */
  int setkey = mapkey (key); // ncurses -> linux/uinput-events mapping
  emit(fd, EV_KEY, setkey, 1);
  emit(fd, EV_SYN, SYN_REPORT, 0);
  usleep(25*1000);  
  emit(fd, EV_KEY, setkey, 0);
  emit(fd, EV_SYN, SYN_REPORT, 0);
  
  return 0;
}


int main(int argc, char **argv) 
{ 
    int fd1; 
    int c, f=0;
    char * myfifo = "/tmp/vkbdd.fifo"; // FIFO file path
    char * chownuser = "";    
    
    snprintf (logbuffer, LBSIZE, "Virtual keyboard daemon for RetroPie - https://github.com/etheling/vkbdd\n");
    if (argc > 1) {
      sprintf (logbuffer, "%sArguments: ",logbuffer);
      for ( int u=1; u<argc; u++) { sprintf (logbuffer, "%s%s ", logbuffer, argv[u]); }
      sprintf (logbuffer, "%s\n", logbuffer);
    }
    printf ("%s",logbuffer);

    while ((c = getopt (argc, argv, "fhnvu:p:")) != -1) {
      switch (c)
	{
	case 'f':
	  f=1;	  
	  break;
	case 'h':
	  printf ("\nUsage: %s [OPTION..]\n", argv[0]);
	  printf ("  -f        if named pipe exists, overwrite\n");
	  printf ("  -h        print this help message\n");
	  printf ("  -n        disable debug logging to /tmp/vkbdd.debug.log\n");
	  printf ("  -p <file> path/file to named pipe to read (default /tmp/vkbdd.fifo)\n");
	  printf ("  -u <user> username to chown pipe (default current user)\n");
	  printf ("  -v        print program version and exit\n\n");   	  	  
	  return 1;
	case 'n':
	  debug = 0;
	  break;
	case 'p':
	  if (strlen(optarg) > 255) {
	    printf ("\nERROR: path/file too long (max is 255). Aborting.\n");
	    return 1;
	  }
	  myfifo = optarg;
	  break;
	case 'u':
	  if (strlen(optarg) > 32) {
	    printf ("\nERROR: username max length is 32 characters. Aborting.\n");
	    return 1;
	  }
	  chownuser = optarg;
	  break;
	case 'v':
	  printf ("Version %s\n", VERSION);
	  return 1;
	case '?':
	  // ..
	  return 1;
	default:
	  abort ();
	}
    }

    if (debug) {
      dfd = open_debuglog ();
      if (dfd < 1) {
	printf ("ERROR: cannot open debug log. Aborting. Run with -n to disable debug logging.\n");
	return 1;
      }
      wlog (logbuffer);
      printf ("Debug log enabled (/tmp/vkbdd.debug.log). Run with -n to disable.\n");
    }
    
    // create and setup FIFO/named pipe
    if (debug) {
      snprintf (logbuffer, LBSIZE, "main(): Setting up named pipe (FIFO): %s\n", myfifo);
      wlog (logbuffer);
    }
    if (file_exists(myfifo)) {
      if (!f) {
	printf ("ERROR: /tmp/retropie-vkbd named pipe exists. Run with -f to overwrite\n");
	return 1;
      } else {
	printf ("WARNING: %s exists. Overwriting...\n", myfifo);
	if (remove(myfifo) != 0) {
	  printf ("ERROR: couldn't remove fifo %s. Aborting.", myfifo);
	  return 1;
	}
      }
    }

    if (debug) { wlog ("main(): mkfifo... mode 0666\n"); }
    // 0666 https://stackoverflow.com/questions/1343144/mkfifo-file-permissions-not-being-executed-correctly
    mode_t oldmask = umask (0000); 
    if ( mkfifo(myfifo, 0666 ) != 0 ) {
      printf ("ERROR: cannot create named pipe %s\n", myfifo);
      return 1;
    }
    umask (oldmask);

    if (strlen(chownuser)>0) {
      // FIXME: separate paramter for the group?
      if (do_chown (myfifo, chownuser,  chownuser) != 0) {
	printf ("ERROR: cannot chown %s to user:group %s\n",myfifo, chownuser);
	return 1;
      }
    }

    // Setup and create keyboard device 
    if (debug) { wlog ("main(): Setting up /dev/uinput...\n"); }
    int uifd;
    uifd = setup_uinput();
    if (uifd <= 0) {
      return (1);
    }


    // Main loop
    char ch[BUFFSIZE];
    c = 0;
    if (debug) {
      snprintf (logbuffer, LBSIZE, "main(): Entering main loop (uifd %d, dfd %d)\n",uifd,dfd);
      wlog (logbuffer);
    }
    while (1) {
      fd1 = open(myfifo,O_RDONLY);
      
      int n;
      while( (n = read(fd1, ch, BUFFSIZE) ) > 0) {
        c++;
	if (debug) {
	  snprintf (logbuffer, LBSIZE, "main(): read: c#%d: ch:%d: ch:%c (no_read(n): %d)\n", c, ch[0], ch[0], n);
	  wlog (logbuffer);
	}
	send_keypress (uifd, ch[0]);
      }

      close (fd1);
    }
    
    return 0; 
} 

// TODO: do early exits via finish() to close file handles more properly.
// TODO: find appropriate VID:PID to register 
