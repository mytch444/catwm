static void message_full(char* args[], int argc);
static void message(char *message);
static void message_wait(char *message, int t);

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
    char *command[] = {NULL};

    command[0] = message;

    message_full(command, 1);
}

void message_wait(char *message, int t) {
    char *command[] = {NULL, "-t", NULL, "-w"};

    command[0] = message;
    command[2] = malloc(sizeof(char) * 20);
    sprintf(command[2], "%i", t);
        
    message_full(command, 4);
}
