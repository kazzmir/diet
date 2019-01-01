#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>

typedef char bool;
static const int true = 1;
static const int false = 0;
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

static int run(struct DietPlan const * diet, int argc, char **argv){
    /*
    set_limit("address space", RLIMIT_AS, 20 * MEGABYTE, true);
    set_limit("data segment", RLIMIT_DATA, 1 * MEGABYTE, true);
    set_limit("open file", RLIMIT_NOFILE, 16, true);
    */

    set_diet_plan(diet, true);

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

    struct DietPlan const * const plan = &DietStarving;

    return run(plan, argc - 1, &argv[1]);
}
