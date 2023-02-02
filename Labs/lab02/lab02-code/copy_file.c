#include <stdio.h>
#define BUF_SIZE 4096

/*
 * Copy the contents of one file into another file
 *      source_file: Name of the source file to copy from
 *      dest_file: Name of the destination file to copy to
 *                 the destination file is overwritten if it already exists
 *      Returns 0 on success and -1 on error
 */
long int file_size(const char *source_file) {
    char err_msg[255];
    long int file_size;

    FILE * fr = fopen(source_file, "r");
    
    if (fr == NULL) {
        printf("Fail to open file: %s\n", source_file);
        return -1;
    }

    if (fseek(fr, 0, SEEK_END) == -1) {
        snprintf(err_msg, 255, "Failed to fseek() in file %s", source_file);
        perror(err_msg);
        fclose(fr);
        return -1;
    }

    file_size = ftell(fr);

    if (fseek(fr, 0, SEEK_SET) == -1) {
        snprintf(err_msg, 255, "Failed to fseek() in file %s", source_file);
        perror(err_msg);
        fclose(fr);
        return -1;
    }

    if (fclose(fr) != 0) {
        printf("Failed to close file\n");
        return -1;
    }

    return file_size;
}

int copy_file(const char *source_file, const char *dest_file) {
    // TODO Not yet implemented
    char buff[BUF_SIZE];
    long int size;
    
    if ((size = file_size(source_file)) == -1) {
        return -1;
    } 

    // Reading contents of source file
    FILE * fr = fopen(source_file, "r");
    if (fr == NULL) {
        printf("Fail to open file: %s\n", source_file);
        return -1;
    }

    fread(buff, size, 1, fr);

    if (fclose(fr) != 0) {
        printf("Failed to close file: %s\n", source_file);
        return -1;
    }

    // writing contents of source file
    FILE * fw = fopen(dest_file, "w");
    if (fw == NULL) {
        fclose(fr);
        printf("Fail to open file: %s\n", dest_file);
        return -1;
    }

    fwrite(buff, size, 1, fw);
    
    if (fclose(fw) != 0) {
        printf("Failed to close file: %s\n", dest_file);
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <source> <dest>\n", argv[0]);
        return 1;
    }

    // copy_file already prints out any errors
    if (copy_file(argv[1], argv[2]) != 0) {
        return 1;
    }
    return 0;
}
