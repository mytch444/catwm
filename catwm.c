/*
  *   /\___/\
  *  ( o   o )  Made by cat...
  *  (  =^=  )
  *  (        )            ... for cat!
  *  (         )
  *  (          ))))))________________ Cute And Tiny Window Manager
  *  ______________________________________________________________________________
  *
  *  Copyright (c) 2010, Rinaldini Julien, julien.rinaldini@heig-vd.ch
  *
  *  Permission is hereby granted, free of charge, to any person obtaining a
  *  copy of this software and associated documentation files (the "Software"),
  *  to deal in the Software without restriction, including without limitation
  *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  *  and/or sell copies of the Software, and to permit persons to whom the
  *  Software is furnished to do so, subject to the following conditions:
  *
  *  The above copyright notice and this permission notice shall be included in
  *  all copies or substantial portions of the Software.
  *
  *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  *  DEALINGS IN THE SOFTWARE.
  *
  */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <X11/extensions/Xinerama.h>

#define TABLENGTH(X)    (sizeof(X)/sizeof(*X))

#define TILE 0
#define MONOCLE 1

#define RESIZE 1
#define MOVE 2
#define MINWSZ 50

typedef struct key key;
typedef struct button button;
typedef struct client client;
typedef struct desktop desktop;
typedef struct monitor monitor;

struct Arg {
  char** com;
  key *map;
  int i;
};

struct key {
  unsigned int mod;
  KeySym keysym;
  void (*function)(struct Arg arg);
  struct Arg arg;
};

struct button {
  unsigned int mask;
  unsigned int button;
  void (*function)(struct Arg arg);
  struct Arg arg;
};

struct client {
  client *next;
  client *prev;
  
  unsigned int bw;
  int floating;
  
  Window win;
};

struct desktop {
  int master_size;
  int mode;
  client *head;
  client *current;
};

struct monitor {
  int sw;
  int sh;
  int x;
  int y;
  
  desktop desktops[10];
  int current_desktop;
};

// Functions
static void add_window(Window w, int floating);
static void change_desktop(const struct Arg arg);
static void client_to_desktop(const struct Arg arg);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static void decrease();
static void destroynotify(XEvent *e);
static void die(const char* e);
static unsigned long getcolor(const char* color);
static void grabkeys();
static void increase();
static void keypress(XEvent *e);
static void kill_client();
static void maprequest(XEvent *e);
static void move_down();
static void move_up();
static void next_win();
static void prev_win();
static void quit();
static void remove_window(Window w);
static int remove_window_from_current(Window w);
static void save_desktop(int i);
static void select_desktop(int i);
static void send_kill_signal(Window w);
static void setup();
static void sigchld(int unused);
static void spawn(const struct Arg arg);
static void start();
static void swap_master();
static void togglemonocle();
static void tile();
static void update_current();
static int grabkeyboard();
static void releasekeyboard();
static void message_full(char* args[], int argc);
static void message(char *message);
static void message_wait(char *message, int t);
static void submap(struct Arg arg);
static void stickysubmap(struct Arg arg);
static int keyismod(KeySym keysym);
static void togglefloat(struct Arg arg);
static void mousemotion(struct Arg arg);
static void grabbuttons(Window w);
static void ungrabbuttons(Window w);
static void buttonpress(XEvent *e);
static void update_monitors();
static void select_monitor(int i);
static void change_monitor(struct Arg arg);
static void client_to_monitor(struct Arg arg);
static void init_desktops(monitor *mon);
static void copy_desktop(desktop *new, desktop old);
static int cycle_monitor(int n);
static void save_monitor(int i);

// Include configuration file (need struct key)
#include "config.h"

// Variable
static Display *dis;
static Window root;
static int screen;

static int bool_quit;
static int bs;
static unsigned int win_focus;
static unsigned int win_unfocus;

static int monitors_count;
static int current_monitor;
static int current_desktop;

static monitor *monitors;
static desktop desktops[10];

static int sh;
static int sw;
static int sx;
static int sy;
static int master_size;
static int mode;
static client *head;
static client *current;

// Events array
static void (*events[LASTEvent])(XEvent *e) = {
  [KeyPress] = keypress,
  [MapRequest] = maprequest,
  [DestroyNotify] = destroynotify,
  [ConfigureNotify] = configurenotify,
  [ConfigureRequest] = configurerequest,
  [ButtonPress] = buttonpress,
};

void setup() {
  // Install a signal
  sigchld(0);
    
  bool_quit = 0;
  mode = 0;
  head = NULL;
  current = NULL;
  bs = BORDER_SPACE;
  monitors_count = 0;
  current_monitor = 0;

  // Screen and root window
  screen = DefaultScreen(dis);
  root = RootWindow(dis,screen);

  update_monitors();
  select_monitor(0);
  
  // Colors
  win_focus = getcolor(FOCUS);
  win_unfocus = getcolor(UNFOCUS);
  
  grabkeys();
  
  // To catch maprequest and destroynotify (if other wm running)
  XSelectInput(dis,root,SubstructureNotifyMask|SubstructureRedirectMask);
}

void update_monitors() {
  int i, m, d;
  client *c;
  
  if (XineramaIsActive(dis)) {
    int count = 0;
    XineramaScreenInfo *info = NULL;
    
    if (!(info = XineramaQueryScreens(dis, &count)))
      die("Error xineramaQueryScreens!");
    
    if (count > monitors_count) {
      if (!(monitors = (monitor*) realloc(monitors, count * sizeof(monitor))))
	die("Error realloc!");

      for (i = monitors_count; i < count; ++i) {
	monitors[i].current_desktop = 0;
	monitors[i].sw = info[i].width;
	monitors[i].sh = info[i].height;
	monitors[i].x = info[i].x_org;
	monitors[i].y = info[i].y_org;
	init_desktops(&monitors[i]);
      }
    } else if (count < monitors_count) {
      if (count < 1)
	die("A monitor could help");

      select_monitor(0);
      for (m = 1; m < monitors_count; m++) {
	for (d = 0; d < 10; d++) {
	  for (c = monitors[m].desktops[d].head; c; c = c->next) {
	    remove_window(c->win);
	    add_window(c->win, c->floating);
	  }
	}
      }
      save_monitor(0);
      
      if (!(monitors = (monitor*)realloc(monitors, count * sizeof(monitor))))
	die("Error realloc!");
    }

    for (i = 0; i < count; ++i) {
      monitors[i].sw = info[i].width;
      monitors[i].sh = info[i].height;
      monitors[i].x = info[i].x_org;
      monitors[i].y = info[i].y_org;
    }
    
    monitors_count = count;
    XFree(info);
  } else {
    monitors_count = 1;
    if (!(monitors = (monitor*)realloc(monitors, sizeof(monitor))))
      die("Error realloc!");
       
    monitors[0].current_desktop = 0;
    monitors[0].sw = XDisplayWidth(dis, screen);
    monitors[0].sh = XDisplayHeight(dis, screen);
    monitors[0].x = 0;
    monitors[0].y = 0;
    init_desktops(&monitors[0]);
  }
}

void init_desktops(monitor *mon) {
  int i;
  for (i = 0; i < 10; i++) {
    mon->desktops[i].master_size = mon->sw * MASTER_SIZE;
    mon->desktops[i].mode = TILE;
    mon->desktops[i].head = NULL;
    mon->desktops[i].current = NULL;
  }
}

void select_monitor(int i) {
  int j;
  
  current_monitor = i;
  
  sh = monitors[i].sh;
  sw = monitors[i].sw;
  sx = monitors[i].x;
  sy = monitors[i].y;
  
  for (j = 0; j < 10; j++) {
    copy_desktop(&desktops[j], monitors[i].desktops[j]);
  }
  
  select_desktop(monitors[i].current_desktop);
}

void save_monitor(int i) {
  int j;
  
  monitors[i].current_desktop = current_desktop;
  save_desktop(current_desktop);
  for (j = 0; j < 10; j++) {
    copy_desktop(&(monitors[i].desktops[j]), desktops[j]);
  }
}

void copy_desktop(desktop *new, desktop old) {
  (*new).master_size = old.master_size;
  (*new).mode = old.mode;
  (*new).head = old.head;
  (*new).current = old.current;  
}

int cycle_monitor(int n) {
  int new = current_monitor + n;
  
  while (new < 0)
    new += monitors_count;
  while (new >= monitors_count)
    new -= monitors_count;

  return new;
}

void change_monitor(const struct Arg arg) {
  int new;
  client *c;
  
  if (monitors_count == 1) return;
  
  new = cycle_monitor(arg.i);
  
  save_monitor(current_monitor);
  
  // Unfocus the current window in the non focused monitora
  current = NULL;
  update_current();
  
  select_monitor(new);

  // Map all windows
  if(head != NULL)
    for(c=head;c;c=c->next)
      XMapWindow(dis,c->win);
  
  tile();
  update_current();
  
  message_wait("Moved to this monitor", 1);
}

void client_to_monitor(const struct Arg arg) {
  int new = current_monitor + arg.i;
  client *tmp;
  
  if (current == NULL)
    return;
  
  new = cycle_monitor(arg.i);
  
  tmp = current;
  
  remove_window(tmp->win);
  
  save_monitor(current_monitor);
  
  // Add client to desktop
  select_monitor(new);
  add_window(tmp->win, tmp->floating);
  
  tile();
  update_current();
}

void add_window(Window w, int floating) {
  client *c,*t;
  
  if(!(c = (client *)calloc(1,sizeof(client))))
    die("Error calloc!");
  
  c->next = NULL;
  c->win = w;
  c->floating = floating;
  
  if(head == NULL) {
    c->prev = NULL;
    head = c;
  } else {
    for(t=head;t->next;t=t->next);
    
    c->prev = t;
    t->next = c;
  }
  
  current = c;
}

void change_desktop(const struct Arg arg) {
  client *c;
  
  if(arg.i == current_desktop)
    return;

  // Save current "properties"
  save_desktop(current_desktop);
  
  // Unmap all window
  if(head != NULL)
    for(c=head;c;c=c->next)
      XUnmapWindow(dis,c->win);
    
  // Take "properties" from the new desktop
  select_desktop(arg.i);
  
  // Map all windows
  if(head != NULL)
    for(c=head;c;c=c->next)
      XMapWindow(dis,c->win);
  
  tile();
  update_current();
  
  char buf[10];
  sprintf(buf, "%i", arg.i);
  message_wait(buf, 1);
}

void client_to_desktop(const struct Arg arg) {
  client *tmp = current;
  int tmp2 = current_desktop;
  
  if(arg.i == current_desktop || current == NULL)
    return;
  
  XUnmapWindow(dis, current->win);
  remove_window(current->win);
  save_desktop(tmp2);
  
  // Add client to desktop
  select_desktop(arg.i);
  add_window(tmp->win, tmp->floating);
  save_desktop(arg.i);
  
  select_desktop(tmp2);
  
  tile();
  update_current();
}

/*
  If anyone could get this working that would be great. As it is, it's unimportant.
 */
void configurenotify(XEvent *e) {
  /*  XConfigureEvent *ev = &e->xconfigure;

  if (ev->window == root) {
    update_monitors();
    }*/
}

void configurerequest(XEvent *e) {
  /*  // Paste from DWM, thx again \o/
  XConfigureRequestEvent *ev = &e->xconfigurerequest;
  XWindowChanges wc;
  wc.x = ev->x;
  wc.y = ev->y;
  wc.width = ev->width;
  wc.height = ev->height;
  wc.border_width = ev->border_width;
  wc.sibling = ev->above;
  wc.stack_mode = ev->detail;
  XConfigureWindow(dis, ev->window, ev->value_mask, &wc);*/
}

void increase() {
  if(master_size < sw-50) {
    master_size += 10;
    tile();
  }
}

void decrease() {
  if(master_size > 50) {
    master_size -= 10;
    tile();
  }
}

void destroynotify(XEvent *e) {
  int i = 0;
  client *c;
  XDestroyWindowEvent *ev = &e->xdestroywindow;

  for (c = head; c; c = c->next)
    if (ev->window == c->win)
      i++;

  if (i == 0)
    return;
  
  remove_window(ev->window);

  update_current();
}

void die(const char* e) {
  fprintf(stderr,"catwm: %s\n",e);
  exit(1);
}

unsigned long getcolor(const char* color) {
  XColor c;
  Colormap map = DefaultColormap(dis,screen);
  
  if(!XAllocNamedColor(dis,map,color,&c,&c))
    die("Error parsing color!");
  
  return c.pixel;
}

void grabkeys() {
  int i;
  KeyCode code;
  
  for (i = 0; keys[i].function != NULL; i++) {
    if((code = XKeysymToKeycode(dis,keys[i].keysym))) {
      XGrabKey(dis,code,keys[i].mod,root,True,GrabModeAsync,GrabModeAsync);
    }
  }
}

void keypress(XEvent *e) {
  int i;
  XKeyEvent ke = e->xkey;
  KeySym keysym = XLookupKeysym(&ke, 0);
  
  for (i = 0; keys[i].function != NULL; i++) {
    if(keys[i].keysym == keysym && keys[i].mod == ke.state) {
      keys[i].function(keys[i].arg);
    }
  }
}

void send_kill_signal(Window w) {
  int n, i;
  XEvent ke;
  Atom *protocols;

  if (XGetWMProtocols(dis, w, &protocols, &n) != 0) {
    for (i = n; i >= 0; i--) {
      ke.type = ClientMessage;
      ke.xclient.window = w;
      ke.xclient.message_type = XInternAtom(dis, "WM_PROTOCOLS", False);
      ke.xclient.format = 32;
      ke.xclient.data.l[0] = XInternAtom(dis, "WM_DELETE_WINDOW", False);
      ke.xclient.data.l[1] = CurrentTime;
      XSendEvent(dis, w, False, NoEventMask, &ke);
    }
  }
}

/*
  I feel as though there could be a reason for having the same funtion call twice in as row
  as I cannot see any difference between the send_kill_signal and the code below.

  remove_window must be here and cannot be in unmapnotify (which also solves the problem of
  closing chromium crashing X) as if it is in unmapnotify then unmaping the window in change
  desktop removes it.
*/
void kill_client() {
  if(current) {
    send_kill_signal(current->win);
    remove_window(current->win);
  }
}

void maprequest(XEvent *e) {
  XMapRequestEvent *ev = &e->xmaprequest;

  // For fullscreen mplayer (and maybe some other program)
  client *c;
  for(c=head;c;c=c->next) {
    if(ev->window == c->win) {
      XMapWindow(dis,ev->window);
      return;
    }
  }
  
  add_window(ev->window, 0);
  XMapWindow(dis,ev->window);
  
  tile();
  update_current();
}

void move_down() {
  Window tmp;
  if(current == NULL || current->next == NULL || current->win == head->win || current->prev == NULL) {
    return;
  }
  if (current->floating) {
    XCirculateSubwindowsUp(dis, current->win);
  } else {
    tmp = current->win;
    current->win = current->next->win;
    current->next->win = tmp;
    //keep the moved window activated
    next_win();
    tile();
    update_current();
  }
}

void move_up() {
  Window tmp;
  if(current == NULL || current->prev == head || current->win == head->win) {
    return;
  }
  if (current->floating) {
    XCirculateSubwindowsDown(dis, current->win);
  } else {
    tmp = current->win;
    current->win = current->prev->win;
    current->prev->win = tmp;
    prev_win();
    tile();
    update_current();
  }
}

void next_win() {
  client *c;
  
  if(current != NULL && head != NULL) {
    if(current->next == NULL)
      c = head;
    else
      c = current->next;
    
    current = c;
    update_current();
  }
}

void prev_win() {
  client *c;
  
  if(current != NULL && head != NULL) {
    if(current->prev == NULL)
      for(c=head;c->next;c=c->next);
    else
      c = current->prev;
    
    current = c;
    update_current();
  }
}

void quit() {
  Window root_return, parent;
  Window *children;
  int i;
  unsigned int nchildren; 
  XEvent ev;
  
  /*
   * if a client refuses to terminate itself,
   * we kill every window remaining the brutal way.
   * Since we're stuck in the while(nchildren > 0) { ... } loop
   * we can't exit through the main method.
   * This all happens if MOD+q is pushed a second time.
   */
  if(bool_quit == 1) {
    XUngrabKey(dis, AnyKey, AnyModifier, root);
    XDestroySubwindows(dis, root);
    fprintf(stdout, "catwm: Thanks for using!\n");
    XCloseDisplay(dis);
    die("forced shutdown");
  }
  
  bool_quit = 1;
  XQueryTree(dis, root, &root_return, &parent, &children, &nchildren);
  for(i = 0; i < nchildren; i++) {
    send_kill_signal(children[i]);
  }
  //keep alive until all windows are killed
  while(nchildren > 0) {
    XQueryTree(dis, root, &root_return, &parent, &children, &nchildren);
    XNextEvent(dis,&ev);
    if(events[ev.type])
      events[ev.type](&ev);
  }
  
  XUngrabKey(dis,AnyKey,AnyModifier,root);
  fprintf(stdout,"catwm: Thanks for using!\n");
}

void remove_window(Window w) {
  int m, d, monitor_save, desktop_save;

  monitor_save = current_monitor;
  save_monitor(current_monitor);
  
  for (m = 0; m < monitors_count; m++) {
    select_monitor(m);
    
    desktop_save = current_desktop;
    
    for (d = 0; d < 10; d++) {
      select_desktop(d);
      
      if (remove_window_from_current(w)) {
	tile();
	update_current();
      }
      
      save_desktop(d);
    }
    
    select_desktop(desktop_save);
    save_monitor(m);
  }

  select_monitor(monitor_save);
}

/*
  Going back to old code (not sure why I changed in the first place)
  as it works. Fixed bug where closing windows with there own close
  button would crash wm.
 */
int remove_window_from_current(Window w) {
  client *c;
  for (c = head; c; c = c->next) {
    if (c->win == w) {
      if (c->prev == NULL && c->next == NULL) {
	free(head);
	head = NULL;
	current = NULL;
	return 1;
      }

      if (c->prev == NULL) {
	head = c->next;
	c->next->prev = NULL;
	current = c->next;
      }
      else if (c->next == NULL) {
	c->prev->next = NULL;
	current = c->prev;
      }
      else {
	c->next->prev = c->prev;
	c->prev->next = c->next;
	current = c->next;
      }

      free(c);
      return 1;
    }
  }
  
  return 0;
}

void save_desktop(int i) {
  desktops[i].master_size = master_size;
  desktops[i].mode = mode;
  desktops[i].head = head;
  desktops[i].current = current;
}

void select_desktop(int i) {
  head = desktops[i].head;
  current = desktops[i].current;
  master_size = desktops[i].master_size;
  mode = desktops[i].mode;
  current_desktop = i;
}

void sigchld(int unused) {
  // Again, thx to dwm ;)
  if(signal(SIGCHLD, sigchld) == SIG_ERR)
    die("Can't install SIGCHLD handler");
  while(0 < waitpid(-1, NULL, WNOHANG));
}

void spawn(const struct Arg arg) {
  if(fork() == 0) {
    if(fork() == 0) {
      if(dis)
	close(ConnectionNumber(dis));
      
      setsid();
      execvp((char*)arg.com[0],(char**)arg.com);
    }
    exit(0);
  }
}

void start() {
  XEvent ev;
  
  // Main loop, just dispatch events (thx to dwm ;)
  while(!bool_quit && !XNextEvent(dis,&ev)) {
    if(events[ev.type])
      events[ev.type](&ev);
  }
}

void swap_master() {
  Window tmp;
  
  if(head != NULL && current != NULL && current != head && mode == 0) {
    tmp = head->win;
    head->win = current->win;
    current->win = tmp;
    current = head;
    
    tile();
    update_current();
  }
}

void togglemonocle() {
  mode = (mode == TILE) ? MONOCLE:TILE;
  tile();
  update_current();
}

void tile() {
  client *c;
  int n = 0;
  int y = bs / 2;
  
  if(head != NULL) {
    for(c=head;c;c=c->next) {
      c->bw = 1;
      if (!c->floating) n++;
    }
    
    if (n > 1 && mode == TILE) {
      n--;
      
      for(c=head;c;c=c->next) {
	if (c->floating) continue;
	XMoveResizeWindow(dis,c->win, sx + bs, sy + bs, master_size - 2 - bs * 2, sh - 2 - bs * 2);
	break;
      }
      
      for(c=c->next;c;c=c->next) {
	if (c->floating) continue;
	XMoveResizeWindow(dis, c->win,
			  sx + master_size,
			  sy + y + bs / 2,
			  sw - master_size - 2 - bs,
			  ((sh - bs) / n) - 2 - bs);
	y += (sh - bs) / n;
      }
    } else if (n > 0) {
      for(c=head;c;c=c->next) {
	if (c->floating) continue;
	c->bw = 0;
	XMoveResizeWindow(dis,c->win,sx,sy,sw,sh);
      }
    }
  }
}


void update_current() {
  client *c;
  
  for(c=head;c;c=c->next) {
    XSetWindowBorderWidth(dis,c->win,c->bw);
    if(current == c) {
      // "Enable" current window
      XSetWindowBorder(dis,c->win,win_focus);
      XSetInputFocus(dis,c->win,RevertToParent,CurrentTime);
      XRaiseWindow(dis,current->win);
    } else {
      XSetWindowBorder(dis,c->win,win_unfocus);
    }
  }
}

void message_full(char* args[], int argc) {
  if (!MESSAGES) return;
  
  int i;
  
  char* command[argc + 4];
  command[0] = "showmessage";
  command[1] = "-x";
  command[2] = malloc(sizeof(char) * 20);
  sprintf(command[2], "%i", sx + sw - 20);
  
  for (i = 0; i < argc; i++) {
    command[i + 3] = args[i];
  }

  command[argc + 3] = NULL;

  struct Arg arg;
  arg.com = command;
  spawn(arg);
}

void message(char *message) {
  char* command[] = {NULL};

  command[0] = message;
  
  message_full(command, 1);
}

void message_wait(char *message, int t) {
  char* command[] = {NULL, "-t", NULL, "-w"};

  command[0] = message;
  command[2] = malloc(sizeof(char) * 20);
  sprintf(command[2], "%i", t);
  
  message_full(command, 4);
}

int grabkeyboard() {
  int i;
  for (i = 0; i < 100; i++) {
    if (XGrabKeyboard(dis, root, True, GrabModeAsync, GrabModeAsync, CurrentTime) == GrabSuccess)
      return 1;
    usleep(1000);
  }
  return 0;
}

void releasekeyboard() {
  XUngrabKeyboard(dis, CurrentTime);  
}

void submap(struct Arg arg) {
  int i;
  XEvent e;
  XKeyEvent ke;
  KeySym keysym;
  
  if (!grabkeyboard())
    return;
  
  for (i = 0; i < 1000; i++) {
    XNextEvent(dis, &e);
    if (e.type == KeyPress) {
      ke = e.xkey;
      keysym = XLookupKeysym(&ke, 0);
      
      if (!keyismod(keysym))
	goto gotkey;
    }
  }
  
  message("No key found in 1000 events... Umm");
  goto end;
  
 gotkey:
  
  for (i = 0; arg.map[i].function != NULL; i++) {
    if(arg.map[i].keysym == keysym && arg.map[i].mod == ke.state) {
      arg.map[i].function(arg.map[i].arg);
      goto end;
    }
  }
  
  message("No key shortcut maped to that key");
  
p end:
  releasekeyboard();
}

void stickysubmap(struct Arg arg) {
  int i;
  XEvent e;
  XKeyEvent ke;
  KeySym keysym;
  
  if (!grabkeyboard())
    return;
  
  message_wait("Sticky submap entered", 2);
  while (1) {
    XNextEvent(dis, &e);
    
    if (e.type == KeyPress) {
      ke = e.xkey;
      keysym = XLookupKeysym(&ke, 0);
      
      if (keyismod(keysym))
	continue;
      
      if (keysym == XK_Escape)
	break;
      
      if (keysym == XK_g && ke.state == ControlMask)
	break;
      
      for (i = 0; arg.map[i].function != NULL; i++) {
	if(arg.map[i].keysym == keysym && arg.map[i].mod == ke.state) {
	  arg.map[i].function(arg.map[i].arg);
	}
      }
    }
  }
  message_wait("Exited", 1);
  releasekeyboard();
}

int keyismod(KeySym keysym) {
  if (keysym == XK_Shift_L ||
      keysym == XK_Shift_R ||
      keysym == XK_Control_L ||
      keysym == XK_Control_R ||
      keysym == XK_Meta_L ||
      keysym == XK_Meta_R ||
      keysym == XK_Alt_L ||
      keysym == XK_Alt_R
      )
    return 1;
  return 0;
}

void togglefloat(struct Arg arg) {
  if (!current) return;
  
  current->floating = !current->floating;
  
  if (current->floating) {
    XMoveResizeWindow(dis, current->win,
		      sx + sw / 3, sy + sh / 3,
		      sw / 3, sh / 3);
    XRaiseWindow(dis,current->win);
    grabbuttons(current->win);
  } else {
    ungrabbuttons(current->win);
  }
  
  tile();
  update_current();
}

void mousemotion(struct Arg arg) {
  XWindowAttributes wa;
  XEvent ev;
  
  if (!current || !XGetWindowAttributes(dis, current->win, &wa)) return;
  
  if (arg.i == RESIZE) XWarpPointer(dis, current->win, current->win, 0, 0, 0, 0, --wa.width, --wa.height);
  int rx, ry, c, xw, yh; unsigned int v; Window w;
  if (!XQueryPointer(dis, root, &w, &w, &rx, &ry, &c, &c, &v) || w != current->win) return;
  
  if (XGrabPointer(dis, root, False, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync,
		   GrabModeAsync, None, None, CurrentTime) != GrabSuccess) return;
  
  do {
    XMaskEvent(dis, ButtonPressMask|ButtonReleaseMask|PointerMotionMask|SubstructureRedirectMask, &ev);
    
    if (ev.type == MotionNotify) {
      
      xw = (arg.i == MOVE ? wa.x:wa.width)  + ev.xmotion.x - rx;
      yh = (arg.i == MOVE ? wa.y:wa.height) + ev.xmotion.y - ry;
      
      if (arg.i == RESIZE)
	XResizeWindow(dis, current->win,
		      xw > MINWSZ ? xw:MINWSZ, yh > MINWSZ ? yh:MINWSZ);
      
      else if (arg.i == MOVE)
	XMoveWindow(dis, current->win, xw, yh);
      
    } else //if (ev.type == ConfigureRequest || ev.type == MapRequest)
      events[ev.type](&ev);
    
  } while (ev.type != ButtonRelease);
  
  XUngrabPointer(dis, CurrentTime);
}

void grabbuttons(Window w) {
  int b;
  for (b = 0; b < TABLENGTH(buttons); b++)
    XGrabButton(dis, buttons[b].button, buttons[b].mask, w,
 		False, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
}

void ungrabbuttons(Window w) {
  int b;
  for (b = 0; b < TABLENGTH(buttons); b++)
    XUngrabButton(dis, buttons[b].button, buttons[b].mask, w);
}

void buttonpress(XEvent *e) {
  unsigned int i;
  
  for (i = 0; i < TABLENGTH(buttons); i++)
    if (buttons[i].mask == e->xbutton.state &&
	buttons[i].function && buttons[i].button == e->xbutton.button) {
      buttons[i].function(buttons[i].arg);
    }
}

int main(int argc, char **argv) {
  // Open display
  if(!(dis = XOpenDisplay(NULL)))
    die("Cannot open display!");
  
  // Setup env
  setup();
  
  // Start wm
  start();
  
  // Close display
  XCloseDisplay(dis);
  
  return 0;
}
