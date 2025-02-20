#include <stdio.h>
#include <stdlib.h>

// EXTRA CREDIT - print the drexel dragon from the readme.md
extern void print_dragon() {
    FILE *fp = fopen("dragon.txt", "r");
    if (fp == NULL) {
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }

    fclose(fp);
}
