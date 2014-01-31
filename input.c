static void grabkeys();
static int grabkeyboard();
static void releasekeyboard();
static int keyismod(KeySym keysym);

void grabkeys() {
    int i;
    KeyCode code;

    for (i = 0; keys[i].function != NULL; i++) {
        if ((code = XKeysymToKeycode(dis, keys[i].keysym))) {
            XGrabKey(dis, code, keys[i].mod, root, True, GrabModeAsync, GrabModeAsync);
        }
    }
}

int grabkeyboard() {
    int i;
    for (i = 0; i < 100; i ++) {
        if (XGrabKeyboard(dis, root, True, GrabModeAsync, GrabModeAsync, CurrentTime) == GrabSuccess)
            return 1;
        usleep(1000);
    }
    return 0;
}

void releasekeyboard() {
    XUngrabKeyboard(dis, CurrentTime);
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
