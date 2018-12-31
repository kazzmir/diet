#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>

static int run(int argc, char **argv){
    struct rlimit memory_limit;
    int ok = getrlimit(RLIMIT_AS, &memory_limit);
    if (ok == 0){
        printf("Current memory limit:\n");
        printf("  soft: %lu\n", memory_limit.rlim_cur);
        printf("  hard: %lu\n", memory_limit.rlim_max);

        uint64_t limit = 20 * 1024 * 1024;
        memory_limit.rlim_cur = limit;
        memory_limit.rlim_max = limit;
        ok = setrlimit(RLIMIT_AS, &memory_limit);
        if (ok != 0){
            printf("Unable to set the memory limit: %d %s\n", ok, strerror(errno));
        } else {
            printf("Memory limit set to %lu\n", limit);
        }
    } else {
        printf("Unable to get the memory limit: %d %s\n", ok, strerror(errno));
    }

    ok = execv(argv[0], argv);
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
