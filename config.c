#include "config.h"

void read_portlist_from_file( portlist_t *portlist, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char buffer[STRL] = {'\0'};
    portlist->len = 0;
    while (fgets(buffer, STRL, file)) {
        (portlist->data[portlist->len]).number = atoi(strtok(buffer, ":"));
        strcpy((portlist->data[portlist->len]).protocol, strtok(NULL, ":"));
        strcpy((portlist->data[portlist->len]).description, strtok(NULL, ":\n"));
        (portlist->len)++;
    }
    fclose(file);
}