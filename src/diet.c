#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static int run(int argc, char **argv){
    int ok = execv(argv[0], argv);
    /* Shouldn't get here */
    printf("Unable to execute '%s'. execve returned %d: %s\n", argv[0], ok, strerror(errno));
    return 1;
}

int main(int argc, char **argv){
    if (argc < 2){
        printf("Give a program to execute\n");
        return 1;
    }

    return run(argc - 1, &argv[1]);
}
