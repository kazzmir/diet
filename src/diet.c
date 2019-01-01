#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>

/* TODO
 * Fork child and set rlimits in the child before execing. Then attach to the child via ptrace to control
 * how the child behaves when making system calls. For example, we can slow down a system call by putting
 * a sleep before each system call so the user can feel what a slow system is like.
 */

typedef char bool;
static const int true = 1;
static const int false = 0;
#define KILOBYTE 1024
#define MEGABYTE (1024 * 1024)
#define GIGABYTE (1024 * MEGABYTE)

struct DietPlan{
    uint64_t memory_limit;
    uint64_t data_limit;
    uint64_t file_limit;
};

static const struct DietPlan DietLarge = {
    .memory_limit = 1 * GIGABYTE,
    .data_limit = 1 * GIGABYTE,
    .file_limit = 512
};

static const struct DietPlan DietMedium = {
    .memory_limit = 512 * MEGABYTE,
    .data_limit = 512 * MEGABYTE,
    .file_limit = 256
};

static const struct DietPlan DietSmall = {
    .memory_limit = 128 * MEGABYTE,
    .data_limit = 128 * MEGABYTE,
    .file_limit = 64
};

static const struct DietPlan DietStarving = {
    .memory_limit = 16 * MEGABYTE,
    .data_limit = 16 * MEGABYTE,
    .file_limit = 16
};

static int set_limit(const char* name, int limit_type, uint64_t limit, bool verbose){
    struct rlimit process_limit;
    int ok = getrlimit(limit_type, &process_limit);
    if (ok == 0){
        if (verbose){
            printf("Current %s limit:\n", name);
            printf("  soft: %lu\n", process_limit.rlim_cur);
            printf("  hard: %lu\n", process_limit.rlim_max);
        }

        process_limit.rlim_cur = limit;
        process_limit.rlim_max = limit;
        ok = setrlimit(limit_type, &process_limit);
        if (ok != 0){
            printf("Unable to set the %s limit: %d %s\n", name, ok, strerror(errno));
            return 1;
        } else {
            if (verbose){
                printf("%s limit set to %lu\n", name, limit);
            }
            return 0;
        }
    } else {
        printf("Unable to get the %s limit: %d %s\n", name, ok, strerror(errno));
        return 2;
    }
}

static void set_diet_plan(struct DietPlan const* diet, bool verbose){
    set_limit("address space", RLIMIT_AS, diet->memory_limit, verbose);
    set_limit("data segment", RLIMIT_DATA, diet->data_limit, verbose);
    set_limit("open file", RLIMIT_NOFILE, diet->file_limit, verbose);
}

static int run(struct DietPlan const * diet, bool verbose, int argc, char **argv, char** envp){
    /*
    set_limit("address space", RLIMIT_AS, 20 * MEGABYTE, true);
    set_limit("data segment", RLIMIT_DATA, 1 * MEGABYTE, true);
    set_limit("open file", RLIMIT_NOFILE, 16, true);
    */

    set_diet_plan(diet, verbose);

    int ok = execvpe(argv[0], argv, envp);
    /* Shouldn't get here */
    printf("Unable to execute '%s'. execve returned %d: %s\n", argv[0], ok, strerror(errno));
    return 1;
}

static void show_help(){
    printf("diet: run programs with reduced resources (via rlimit)\n");
    printf("diet <option>... <program> <program-arg> ...\n");
    printf(" --large: set limits to 1GB address space, 1GB data segment, 512 open files. This is the default if no option is specified.\n");
    printf(" --medium: set limits to 512MB address space, 512MB data segment, 256 open files\n");
    printf(" --small: set limits to 128MB address space, 128MB data segment, 64 open files\n");
    printf(" --starving: set limits to 16MB address space, 16MB data segment, 16 open files\n");
    printf(" --memory <size>: set the memory limit to <size>\n");
    printf(" --data <size>: set the data segment limit to <size>\n");
    printf(" --files <size>: set the number of open files limit to <size>\n");
    printf(" --verbose: print what the limits are being set to\n");
    printf(" --help: show this help\n");
    printf("<size> is an integer optionally followed a unit, such as 2k or 3m. 2kb is also ok\n");
    printf("\n");
    printf("Example:\n");
    printf("$ diet --medium /bin/ls -l\n");
}

/* Convert units to bytes
 *   5k = 5 * 1024
 *   5m = 5 * 1024 * 1024
 *   5mb = 5 * 1024 * 1024
 */
static uint64_t convert_size(const char* value, bool* ok){
    const char* digit = "0123456789";
    const char* start = value;
    for (start = value; *start != '\0' && strchr(digit, *start) != NULL; start++);

    if (start == value){
        printf("'%s' is an invalid size\n", value);
        *ok = false;
        return -1;
    }

    char buffer[256];
    bzero(buffer, sizeof(buffer));
    memcpy(buffer, value, start-value);
    buffer[start-value] = 0;

    uint64_t base = atoi(buffer);

    *ok = true;

    if (strcasecmp(start, "k") == 0 ||
        strcasecmp(start, "kb") == 0){
        return base * KILOBYTE;
    }
    if (strcasecmp(start, "m") == 0 ||
        strcasecmp(start, "mb") == 0){
        return base * MEGABYTE;
    }
    if (strcasecmp(start, "g") == 0 ||
        strcasecmp(start, "gb") == 0){
        return base * GIGABYTE;
    }

    return base;
}

int main(int argc, char **argv, char** envp){
    if (argc < 2){
        printf("Error: give a program to execute\n");
        show_help();
        return 1;
    }

    int arg = 1;
    bool verbose = false;
    struct DietPlan plan = DietLarge;
    for (arg = 1; arg < argc; arg++){
        if (strcmp(argv[arg], "--large") == 0){
            plan = DietLarge;
        } else if (strcmp(argv[arg], "--medium") == 0){
            plan = DietMedium;
        } else if (strcmp(argv[arg], "--small") == 0){
            plan = DietSmall;
        } else if (strcmp(argv[arg], "--starving") == 0){
            plan = DietStarving;
        } else if (strcmp(argv[arg], "--verbose") == 0){
            verbose = true;
        } else if (strcmp(argv[arg], "--memory") == 0){
            arg += 1;
            if (arg < argc){
                bool ok = false;
                plan.memory_limit = convert_size(argv[arg], &ok);
                if (!ok){
                    return 1;
                }
            } else {
                printf("Error: expected a size argument after --memory\n");
                return 1;
            }
        } else if (strcmp(argv[arg], "--data") == 0){
            arg += 1;
            if (arg < argc){
                bool ok = false;
                plan.data_limit = convert_size(argv[arg], &ok);
                if (!ok){
                    return 1;
                }
            } else {
                printf("Error: expected a size argument after --data\n");
                return 1;
            }
        } else if (strcmp(argv[arg], "--files") == 0){
            arg += 1;
            if (arg < argc){
                bool ok = false;
                plan.file_limit = convert_size(argv[arg], &ok);
                if (!ok){
                    return 1;
                }
            } else {
                printf("Error: expected a size argument after --files\n");
                return 1;
            }
        } else if (strcmp(argv[arg], "--help") == 0 ||
                   strcmp(argv[arg], "-h") == 0){
            show_help();
            return 0;
        } else {
            if (arg == argc){
                printf("Error: give a program to execute\n");
                show_help();
                return 1;
            }
            run(&plan, verbose, argc - arg, &argv[arg], envp);
        }
    }

    printf("Error: give a program to execute\n");
    show_help();
    return 1;
}
