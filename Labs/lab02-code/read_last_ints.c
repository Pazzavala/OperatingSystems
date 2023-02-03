#include <stdio.h>
#include <stdlib.h>

/*
 * Read the last integers from a binary file
 *   'num_ints': The number of integers to read
 *   'file_name': The name of the file to read from
 * Returns 0 on success and -1 on error
 */

int read_last_ints(const char *file_name, int num_ints) {
    // TODO Not yet implemented
    char err_msg[255];
    FILE * fr = fopen(file_name, "r");

    if (fr == NULL) {
        printf("Fail to open file: %s\n", file_name);
        return 1;
    }

    int * num = malloc(sizeof(int));
    int nbytes;
    int move = (sizeof(int) * num_ints * -1);

    if (fseek(fr, move, SEEK_END) == -1) {
        snprintf(err_msg, 255, "Failed to fseek() in file %s", file_name);
        perror(err_msg);
        fclose(fr);
        return -1;
    }

    while ((nbytes = fread(num, sizeof(int), 1, fr)) > 0) {
            printf("%d\n", *num);
    }

    free(num);

    if (fclose(fr) != 0) {
        printf("Failed to close file\n");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <file_name> <num_ints>\n", argv[0]);
        return 1;
    }

    const char *file_name = argv[1];
    int num_ints = atoi(argv[2]);
    if (read_last_ints(file_name, num_ints) != 0) {
        printf("Failed to read last %d ints from file %s\n", num_ints, file_name);
        return 1;
    }
    return 0;
}
