#include <stdio.h>
#include <time.h>
#include "../include/parser.h"

int main(int argc, char* argv[]){

    if (!(argc == 2)){
        fprintf(stderr, "Usage: %s <file.xml>\n", argv[0]);
        return 1;
    }

    struct timespec t0, t1;

    XML_MAP(map, MAX_ENTRIES);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    if (parse(argv[1], &map) != XML_OK) {
        fprintf(stderr, "Parse failed\n");
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Parse time: %ld ns\n", (t1.tv_sec - t0.tv_sec) * 1000000000 + (t1.tv_nsec - t0.tv_nsec));

    for (int i = 0; i < map.count; i++)
        printf("%s = %s\n", map.entries[i].key, map.entries[i].value);

    return 0;
}
