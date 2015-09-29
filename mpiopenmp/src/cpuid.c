#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int mycpuid() {
    /* Get the the current process' stat file from the proc filesystem */
    // man proc => Status information about the process
    FILE* procfile = fopen("/proc/self/stat", "r");
    long to_read = 8192;
    char buffer[to_read];
    int cpu_id=-1;
    int read = fread(buffer, sizeof(char), to_read, procfile);
    if (!read) { 
        printf("error reading /proc/self/stat %d\n", read);
        return -1;
    }
    fclose(procfile);
    // Field with index 38 (zero-based counting) is the one we want
    char* line = strtok(buffer, " ");
    int i;
    for (i = 1; i < 38; i++) { line = strtok(NULL, " "); }   
    line = strtok(NULL, " ");
    cpu_id = atoi(line);
    return cpu_id;
}
