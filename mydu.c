#include <fcntl.h> /* To use open, read, write, close */
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>




#define DIR_STR_SIZE 128

#define bool int
#define true 1
#define false 0

#define PARAM_ERROR		-1
#define EXEC_ERROR		-1

typedef struct stat stat_t;


// Name of the binary file to write/read
const char *binary_file = "mydu.bin";


// Returns the length of the string `input`, not the size that it occupies in memory
int _strlen(const char *input) {
	int out = 0;
	for (int i = 0; input[i] != '\0' ; i++) {
		out += 1;
	}
	return out;
}

bool _strcmp(const char* a, const char* b) {
	int len_a = _strlen(a);
	if (len_a != _strlen(b)) {
		return false;
	}
	for (int i = 0; i < len_a; i ++) {
		if (a[i] != b[i]) {
			return false;
		}
	}
	return true;
}

// Copies the contents of the second argument, `from`, to the memory positions of `to` starting from `offset`
void _strcpy(char *to, const char *from, int offset) {
	for (int i = offset; i < _strlen(from) + offset; i++) {
		to[i] = from[i - offset];
	}	
}


// Appends the string `rhs` to the string `lhs`. Result is stored in `lhs`.
void _strappend(char *lhs, const char *rhs) {
	_strcpy(lhs, rhs, _strlen(lhs));
}


// Recursive function that prints out the current directory and 
// calls itself for all subdirectories
int print_directory_rec(const char directory[DIR_STR_SIZE]) {
    DIR *dir = opendir(directory);
    struct dirent *entry;
	if (dir == NULL) {
		perror("Error opening directory");
		_exit(-1);
	}
    int size = 0;

    while((entry = readdir(dir)) != NULL) {
        // 4 for directories
		// 8 for files
        if (entry->d_type == 4) {
            if (_strcmp(entry->d_name, ".") || _strcmp(entry->d_name, "..")) {
                continue;
            }
            char new_dir_str[DIR_STR_SIZE] = "";
            _strcpy(new_dir_str, directory, 0);
            _strappend(new_dir_str, "/");
            _strappend(new_dir_str, entry->d_name);
            size += print_directory_rec(new_dir_str);
        } else if (entry->d_type == 8){
            stat_t buf;
            char fname[DIR_STR_SIZE] = "";
            _strcpy(fname, directory, 0);
            _strappend(fname, "/");
            _strappend(fname, entry->d_name);
            int err = stat(fname, &buf);
            if (err < 0) {
                perror("Error reading file size");
                _exit(-1);
            }
            size += buf.st_size;
        }
    }
    closedir(dir);
    printf("%dK\t%s \n", size/1000, directory);

    return size;
}



void print_bin_content(){}


void print_usage(const char * bin_name){
    printf("Usage 1: %s [<directory>]\n", bin_name);
    printf("Usage 2: %s [-b]\n");
}


int main(int argc, char *argv[]) {
  	switch (argc){
        case 1:
            print_directory_rec(".");
			break;
		case 2:
			if (_strcmp(argv[1], "-b")) {
				print_bin_content();
			} else {
            	print_directory_rec(argv[1]);
			}
            break;
        default:
			print_usage(binary_file);
            //TODO: pregutnar al profe si aqui debe salir con error
            break;
  	}

  	return 0;
}