#include <fcntl.h> /* To use open, read, write, close */
#include <unistd.h>
#include <dirent.h>
#include <stdio.h> // exclusively for printf and perror
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


#define DIR_STR_SIZE 128


typedef struct stat stat_t;
 

// Name of the binary file to write/read
const char *binary_file = "mydu.bin";

// File SIZE Type
typedef unsigned long long fsize_t;


// encode the byte string into param `ouptut'
void encode_size(fsize_t size, unsigned char output[8]){
    output[0] = (unsigned char)(size >> 56);
    output[1] = (unsigned char)(size >> 48);
    output[2] = (unsigned char)(size >> 40);
    output[3] = (unsigned char)(size >> 32);
    output[4] = (unsigned char)(size >> 24);
    output[5] = (unsigned char)(size >> 16);
    output[6] = (unsigned char)(size >> 8);
    output[7] = (unsigned char)(size);
}


// decode the byte string into a number
fsize_t decode_size(const unsigned char input[8]){
    fsize_t size = 0;
    size |= (fsize_t)input[0] << 56;
    size |= (fsize_t)input[1] << 48;
    size |= (fsize_t)input[2] << 40;
    size |= (fsize_t)input[3] << 32;
    size |= (fsize_t)input[4] << 24;
    size |= (fsize_t)input[5] << 16;
    size |= (fsize_t)input[6] << 8;
    size |= (fsize_t)input[7];
    return size;
}


// function that prints the result and appends it to the binary file
// records in the file are stored as: 8 bytes for size and `DIR_STR_SIZE` bytes for directory name
// this may not be optimally space efficient but it lets us exctract the data very easily
void print_and_append(fsize_t size, const char* directory, const int* fd){
    // print the result 
    printf("%llu\t%s\n", size, directory);
    // we need to use unsigned chars because we just care about the bytes
    unsigned char size_text[8];
    // encode the size into a byte string
    encode_size(size, size_text);

    // write the byte string
    ssize_t err1 = write(*fd, size_text, 8);
    if (err1 < 0){
        perror("Error size to log file!");
        _exit(-1);
    }
    // write the directory.
    ssize_t err2 = write(*fd, directory, DIR_STR_SIZE);
    if (err2 < 0){
        perror("Error writing directory to log file!");
        _exit(-1);
    }
}

// Recursive function that prints out the current directory and 
// calls itself for all subdirectories
fsize_t print_directory_rec(const char directory[DIR_STR_SIZE], const int *out_fd) {
    DIR *dir = opendir(directory);
    struct dirent *entry;
    // we cannot open the directory
	if (dir == NULL) {
		perror("Error opening directory");
		_exit(-1);
	}

    fsize_t size = 0;

    while((entry = readdir(dir)) != NULL) {
        // 4 for directories
		// 8 for files
        if (entry->d_type == 4) {
            // do not read the current or parent directory (since this would result in an infinite loop)
            if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
                continue;
            }
            // append the folder to directory and call self
            char new_dir_str[DIR_STR_SIZE] = "";
            strcpy(new_dir_str, directory);
            strcat(new_dir_str, "/");
            strcat(new_dir_str, entry->d_name);
            size += print_directory_rec(new_dir_str, out_fd);
        } else if (entry->d_type == 8){
            stat_t buf;
            char fname[DIR_STR_SIZE] = "";

            strcpy(fname, directory);
            strcat(fname, "/");
            strcat(fname, entry->d_name);
            int err = stat(fname, &buf);
            // stat the file to obtain file size
            if (err < 0) {
                perror("Error reading file size");
                _exit(-1);
            }
            //printf("Size of file: %s is: %d\n", fname, buf.st_size);
            size += (fsize_t)buf.st_size;
        }
    }
    closedir(dir);
    // there are some discrepancies with du that i believe are because of 
    // the rounding...
    fsize_t kb = (size + 1023) / 1024;
    print_and_append(kb, directory, out_fd);

    return size;
}


// function to handle the `-b` or history functionality of the program
void print_bin_content(){
    // output header
    printf("--- Contents of binary file ---\n");
    // file that we need to read
    int in_fd = open(binary_file, O_RDONLY);
    if (in_fd < 0){
        perror("Couldn't open log file for reading!");
        _exit(-1);
    }

    // define buffers
    char buf[DIR_STR_SIZE];
    unsigned char size_text[8];

    // define errors and size
    ssize_t err_size_read, err_dir_read;
    fsize_t size;

    // while we are reading something
    while ((err_size_read = read(in_fd, size_text, 8)) > 0) {
        // check for errors
        err_dir_read = read(in_fd, buf, DIR_STR_SIZE);
        if (err_dir_read < 0){
            perror("Error reading log file!");
            _exit(-1);
        }
        // decode the size using out function
        size = decode_size(size_text);
        // print using the same format as previously
        printf("%llu\t%s\n", size, buf);
    }
    if (err_size_read < 0){
        perror("Error reading log file!");
        _exit(-1);
    }
    // finally close the file
    close(in_fd);
}



// function to print how to run the executable
void print_usage(const char * bin_name){
    printf("Usage 1: %s [<directory>]\n", bin_name);
    printf("Usage 2: %s [-b]\n", bin_name);
}



// function to actually run the du on `directory`
void run_du(char *dir){
    // string that holds precisely DIR_STR_SIZE chars
    char directory[DIR_STR_SIZE];
    strcpy(directory, dir);
    // open output file
    int out_fd = open(binary_file, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (out_fd < 0){
        perror("Couldn't open output file!");
        _exit(-1);
    }
    // call underlying functionality
    print_directory_rec(directory, &out_fd);

    int err = close(out_fd);
    if (err < 0){
        perror("Error closing output file!");
        _exit(-1);
    }
}

int main(int argc, char *argv[]) {
    // string to hold the directory name
  	switch (argc){
        case 1:
            run_du(".");
			break;
		case 2:
            // strcmp has odd behaviour... (returns 0 if strings are equal)
			if (strcmp(argv[1], "-b") == 0) {
				print_bin_content();
			} else {
                run_du(argv[1]);
			}
            break;
        default:
			print_usage(binary_file);
            //TODO: pregutnar al profe si aqui debe salir con error
            break;
  	}
  	return 0;
}