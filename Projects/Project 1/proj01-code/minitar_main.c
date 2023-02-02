#include <stdio.h>
#include <string.h>

#include "file_list.h"
#include "minitar.h"

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: %s -c|a|t|u|x -f ARCHIVE [FILE...]\n", argv[0]);
        return 0;
    }

    file_list_t files;
    file_list_init(&files);
    
    char *archive_name = argv[3];
    char archiveOp = argv[1][1];
    int archive_status, file_status, retVal;

    for (int i = 4; i < argc; i++)
        file_status = file_list_add(&files, argv[i]);

    if (file_status == 1)
        return 1;

    switch(archiveOp) {
        case 'c':   archive_status = create_archive(archive_name, &files);
                    break;
        case 'a':   archive_status = append_files_to_archive(archive_name, &files);
                    break;
        case 't':   archive_status = get_archive_file_list(archive_name, &files);
                    break;
        case 'u':   
                    // Check if archive file contains updating files
                    // if (not_contains)
                    //     printf("Error: One or more of the specified files is not already present in archive");
                    // else
                        archive_status = append_files_to_archive(archive_name, &files);
                    break;
        case 'x':   archive_status = extract_files_from_archive(archive_name);
                    break;
        default: 
                    break;
    }

    if (archive_status == -1) {
        printf("Could not preform operation %s", argv[1]);
        retVal = 1;
    } else retVal = 0;
    
    file_list_clear(&files);

    return retVal;
}
