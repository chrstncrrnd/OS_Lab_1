#include <fcntl.h> /* To use open, read, write, close */
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


//#define DEBUG
#define DIR_STR_SIZE 128

#define bool int
#define true 1
#define false 0

#define PARAM_ERROR		-1
#define EXEC_ERROR		-1

typedef struct stat stat_t;


// Name of the binary file to write/read
const char *binary_file = "mydu.bin";

// Recursive function that prints out the current directory and 
// calls itself for all subdirectories
long long print_directory_rec(const char directory[DIR_STR_SIZE]) {
    DIR *dir = opendir(directory);
    struct dirent *entry;
	if (dir == NULL) {
		perror("Error opening directory");
		_exit(-1);
	}

    long long size = 0;

    while((entry = readdir(dir)) != NULL) {
        // 4 for directories
		// 8 for files
        if (entry->d_type == 4) {
            if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
                continue;
            }
            char new_dir_str[DIR_STR_SIZE] = "";
            strcpy(new_dir_str, directory);
            strcat(new_dir_str, "/");
            strcat(new_dir_str, entry->d_name);
            size += print_directory_rec(new_dir_str);
        } else if (entry->d_type == 8){
            stat_t buf;
            char fname[DIR_STR_SIZE] = "";
            strcpy(fname, directory);
            strcat(fname, "/");
            strcat(fname, entry->d_name);
            int err = stat(fname, &buf);
            if (err < 0) {
                perror("Error reading file size");
                _exit(-1);
            }
            //printf("Size of file: %s is: %d\n", fname, buf.st_size);
            size += (long long)buf.st_size;
        }
    }
    closedir(dir);
    long long kb = (size + 1023) / 1024;
    printf("%lld\t%s\n", kb, directory);

    return size;
}



void print_bin_content(){}


void print_usage(const char * bin_name){
    printf("Usage 1: %s [<directory>]\n", bin_name);
    printf("Usage 2: %s [-b]\n", bin_name);
}


int main(int argc, char *argv[]) {
    char dirname[DIR_STR_SIZE] = "";
  	switch (argc){
        case 1:
            strcpy(dirname, ".");
            print_directory_rec(dirname);
			break;
		case 2:
			if (strcmp(argv[1], "-b") == 0) {
				print_bin_content();
			} else {
                strcpy(dirname, argv[1]);
            	print_directory_rec(dirname);
			}
            break;
        default:
			print_usage(binary_file);
            //TODO: pregutnar al profe si aqui debe salir con error
            break;
  	}
  	return 0;
}