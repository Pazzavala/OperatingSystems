#include <fcntl.h>
#include <grp.h>
#include <math.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "minitar.h"

#define NUM_TRAILING_BLOCKS 2
#define MAX_MSG_LEN 512

/*
 * Helper function to compute the checksum of a tar header block
 * Performs a simple sum over all bytes in the header in accordance with POSIX
 * standard for tar file structure.
 */
void compute_checksum(tar_header *header) {
    // Have to initially set header's checksum to "all blanks"
    memset(header->chksum, ' ', 8);
    unsigned sum = 0;
    char *bytes = (char *)header;
    for (int i = 0; i < sizeof(tar_header); i++) {
        sum += bytes[i];
    }
    snprintf(header->chksum, 8, "%07o", sum);
}

/*
 * Populates a tar header block pointed to by 'header' with metadata about
 * the file identified by 'file_name'.
 * Returns 0 on success or -1 if an error occurs
 */
int fill_tar_header(tar_header *header, const char *file_name) {
    memset(header, 0, sizeof(tar_header));
    char err_msg[MAX_MSG_LEN];
    struct stat stat_buf;
    // stat is a system call to inspect file metadata
    if (stat(file_name, &stat_buf) != 0) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to stat file %s", file_name);
        perror(err_msg);
        return -1;
    }

    strncpy(header->name, file_name, 100); // Name of the file, null-terminated string
    snprintf(header->mode, 8, "%07o", stat_buf.st_mode & 07777); // Permissions for file, 0-padded octal

    snprintf(header->uid, 8, "%07o", stat_buf.st_uid); // Owner ID of the file, 0-padded octal
    struct passwd *pwd = getpwuid(stat_buf.st_uid); // Look up name corresponding to owner ID
    if (pwd == NULL) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to look up owner name of file %s", file_name);
        perror(err_msg);
        return -1;
    }
    strncpy(header->uname, pwd->pw_name, 32); // Owner  name of the file, null-terminated string

    snprintf(header->gid, 8, "%07o", stat_buf.st_gid); // Group ID of the file, 0-padded octal
    struct group *grp = getgrgid(stat_buf.st_gid); // Look up name corresponding to group ID
    if (grp == NULL) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to look up group name of file %s", file_name);
        perror(err_msg);
        return -1;
    }
    strncpy(header->gname, grp->gr_name, 32); // Group name of the file, null-terminated string

    snprintf(header->size, 12, "%011o", (unsigned)stat_buf.st_size); // File size, 0-padded octal
    snprintf(header->mtime, 12, "%011o", (unsigned)stat_buf.st_mtime); // Modification time, 0-padded octal
    header->typeflag = REGTYPE; // File type, always regular file in this project
    strncpy(header->magic, MAGIC, 6); // Special, standardized sequence of bytes
    memcpy(header->version, "00", 2); // A bit weird, sidesteps null termination
    snprintf(header->devmajor, 8, "%07o", major(stat_buf.st_dev)); // Major device number, 0-padded octal
    snprintf(header->devminor, 8, "%07o", minor(stat_buf.st_dev)); // Minor device number, 0-padded octal

    compute_checksum(header);
    return 0;
}

/*
 * Removes 'nbytes' bytes from the file identified by 'file_name'
 * Returns 0 upon success, -1 upon error
 * Note: This function uses lower-level I/O syscalls (not stdio), which we'll learn about later
 */
int remove_trailing_bytes(const char *file_name, size_t nbytes) {
    char err_msg[MAX_MSG_LEN];
    // Note: ftruncate does not work with O_APPEND
    int fd = open(file_name, O_WRONLY);
    if (fd == -1) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to open file %s", file_name);
        perror(err_msg);
        return -1;
    }
    //  Seek to end of file - nbytes
    off_t current_pos = lseek(fd, -1 * nbytes, SEEK_END);
    if (current_pos == -1) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to seek in file %s", file_name);
        perror(err_msg);
        close(fd);
        return -1;
    }
    // Remove all contents of file past current position
    if (ftruncate(fd, current_pos) == -1) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to truncate file %s", file_name);
        perror(err_msg);
        close(fd);
        return -1;
    }
    if (close(fd) == -1) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to close file %s", file_name);
        perror(err_msg);
        return -1;
    }
    return 0;
}

int add_header(tar_header * header, const char * file_name, FILE * fw) {
    if(fill_tar_header(header, file_name) == -1)
            return -1;

        fwrite(header, BLOCK_SIZE, 1, fw);

    return 0;
}

int add_footer(FILE * fw) {
    // Adding 2 512 bloccks of 0 byte footers 
    void * footer = malloc(BLOCK_SIZE * NUM_TRAILING_BLOCKS);
    if (footer == NULL)
        return -1;

    memset(footer, 0, BLOCK_SIZE * NUM_TRAILING_BLOCKS);
    fwrite(footer, BLOCK_SIZE, NUM_TRAILING_BLOCKS, fw);

    free(footer);

    return 0;
}

int add_file(tar_header * header, const char * file_name, FILE * fw) {
    //  Open current file to be read
    unsigned int file_size;
    char err_msg[MAX_MSG_LEN];


    FILE * fr = fopen(file_name, "r");
    if (fr == NULL) {
        fclose(fw);
        return -1;
    }

    // Getting the size of current file being read
    sscanf(header->size, "%o", &file_size);

    // Getting amount of blocks to be written and allocating extra space to last block
    int count = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    file_size += ((BLOCK_SIZE * count) - file_size);

    // Getting the data from current file and writing it to archive file
    void * data = malloc(file_size);
    if (data == NULL) {
        fclose(fr);
        fclose(fw);
        return -1;
    }

    memset(data, 0, file_size);
    fread(data, file_size, 1, fr);
    fwrite(data, BLOCK_SIZE, count, fw);
    free(data);

    if (fclose(fr) == -1) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to close file %s", file_name);
        perror(err_msg);
        return -1;
    }

    return 0;
}

int create_archive(const char *archive_name, const file_list_t *files) {
    char err_msg[MAX_MSG_LEN];
    tar_header header;
    node_t * curr = files->head;

    FILE * fw = fopen(archive_name, "w");
    if (fw == NULL)
        return -1;

    while (curr != NULL) {
        const char * file_name = curr->name;

        //  generate a header for each file and write each file’s header
        if(add_header(&header, file_name, fw) == -1) {
            fclose(fw);
            return -1;
        }
        
        // Add current file contents to archive file
        if (add_file(&header, file_name, fw) == -1) {
            fclose(fw);
            return -1;
        }

        curr = curr->next;
    }

    // Generate a footer for archive file
    if (add_footer(fw) == -1) {
        fclose(fw);
        return -1;
    }

    if (fclose(fw) == -1) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to close file %s", archive_name);
        perror(err_msg);
        return -1;
    }
    
    return 0;
}

int append_files_to_archive(const char *archive_name, const file_list_t *files) {
    char err_msg[MAX_MSG_LEN];
    tar_header header;
    node_t * curr = files->head;

    // Make sure that the specified archive file actually exists
    if (access(archive_name, F_OK) == -1) {
            snprintf(err_msg, MAX_MSG_LEN, "File %s does not exist", curr->name);
            perror(err_msg);
            return -1;
    }

    // Remove existing footer
    if (remove_trailing_bytes(archive_name, BLOCK_SIZE * NUM_TRAILING_BLOCKS) == -1) 
        return -1;
    
    FILE * fa = fopen(archive_name, "a");
    if (fa == NULL) 
        return -1;

    // add new representations of the specified member files to the archive file
    while (curr != NULL) {
        const char * file_name = curr->name;

        //  generate a header for each file and write each file’s header
        if(add_header(&header, file_name, fa) == -1) {
            fclose(fa);
            return -1;
        }
        
        // Add current file contents to archive file
        if (add_file(&header, file_name, fa) == -1) {
            fclose(fa);
            return -1;
        }

        curr = curr->next;
    }

    // add a footer to the newly extended archive file
    if (add_footer(fa) == -1) {
        fclose(fa);
        return -1;
    }

    if (fclose(fa) == -1) {
        snprintf(err_msg, MAX_MSG_LEN, "Failed to close file %s", archive_name);
        perror(err_msg);
        return -1;
    }

    return 0;
}

int get_archive_file_list(const char *archive_name, file_list_t *files) {
    // TODO: Not yet implemented
    return 0;
}

int extract_files_from_archive(const char *archive_name) {
    // TODO: Not yet implemented
    return 0;
}

