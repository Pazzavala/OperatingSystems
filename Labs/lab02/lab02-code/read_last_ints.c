#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * Read the last integers from a binary file
 *   'num_ints': The number of integers to read
 *   'file_name': The name of the file to read from
 * Returns 0 on success and -1 on error
 */

int read_last_ints(const char *file_name, int num_ints) {
    // TODO Not yet implemented
    FILE * fr = fopen(file_name, "r");

    if (fr == NULL) {
        printf("Fail to open file: %s\n", file_name);
        return 1;
    }

    // int size = file_size(file_name);
    int * nums_read = malloc(sizeof(int) * (num_ints));
    memset(nums_read, 0,num_ints );

    int nbytes;
    // int move = (sizeof(int) * (num_ints) * -1);

    // fseek(fr, move, SEEK_END);
    int i = 0;
    while ((nbytes = fread(nums_read, 1, num_ints, fr)) > 0) {
            // fwrite(nums_read, sizeof(int), 1, stdout);
            printf("%d", nums_read[i++]);
    }

    free(nums_read);

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
