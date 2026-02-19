#include <fcntl.h> /* To use open, read, write, close */
#include <unistd.h>
#include <stdio.h>

// TODO: preguntar profe error codes
#define FATAL_ERROR_CODE 		-1
#define DIV_ZERO_ERROR_CODE 	-2
#define PARAM_ERROR_CODE 		-3
#define FILE_OPEN_ERROR_CODE 	-4
#define FILE_CLOSE_ERROR_CODE 	-5
#define FILE_WRITE_ERROR_CODE 	-6
#define INCORRECT_PARAM_ERROR   -7
#define FILE_READ_ERROR_CODE    -8
#define HISTORY_NOT_FOUND_ERROR -9

#define false 	0
#define true 	1
#define bool 	int

#define BUFSIZE 64

const char *log_file = "mycalc.log";


// Returns the length of the string `input`, not the size that it occupies in memory
int _strlen(char *input) {
	int out = 0;
	for (int i = 0; input[i] != '\0' ; i++) {
		out += 1;
	}
	return out;
}


// Compares two strings `a` and `b` returns true if all characters are equal
bool _strcmp(char* a, char* b) {
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
void _strcpy(char *to, char *from, int offset) {
	for (int i = offset; i < _strlen(from) + offset; i++) {
		to[i] = from[i - offset];
	}	
}


// Appends the string `rhs` to the string `lhs`. Result is stored in `lhs`.
void _strappend(char *lhs, char *rhs) {
	_strcpy(lhs, rhs, _strlen(lhs));
}


// TODO preguntar profe sobre esto y si debe poner el print usage.
// Ascii to integer, get integer value of ascii.
// Raises error if one of the characters is not a digit
int _atoi(char *input) {
	int out = 0;
	for (int i = 0; i < _strlen(input); i++) {
		char c = input[i];
		if (c > '9' || c < '0'){
			fprintf(stderr, "Tried to convet non digit character to integer.\n");
			_exit(INCORRECT_PARAM_ERROR);
		}
		out += c - '0';
		out *= 10;
	}
	return out / 10;
}


// Returns the number of digits in the input
int digits(int input) {
	int i = 0;
	for (; input > 0; input /= 10) {
		i++;
	}
	return i;
}


// Integer to ascii, writes the ascii value of `input` to `buf`.
void _itoa(int input, char* buf) {
	int digs = digits(input);
	for (int i = digs; input > 0; input /= 10) {
		buf[--i] = (char) (input % 10) + '0';
	}
	buf[digs] = '\0';
}


// TODO: Pregunar profe si esto es error
// Function to print how to use the binary
void print_usage(char *bin_name) {
	printf("Usage (1): %s <num1> <operation (+, -, x, /)> <num2> \n", bin_name);
	printf("Usage (2): %s -b <num operation> \n", bin_name);
}


// Appends a result entry to the log file. 
// Raises errors for when the file is not found, cannot close file or cannot write to file.
void append_file(char* a_s, char op, char* b_s, int res) {
	int fd = open(log_file, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd < 0) {
		perror("Error, could not open file!");
		_exit(FILE_WRITE_ERROR_CODE);
	}

	char buf[BUFSIZE] = "Operation: ";

	char r_s[16];
	_itoa(res, r_s);
	
	char o_s[2];
	o_s[0] = op;
	o_s[1] = '\0';
	
	_strappend(buf, a_s);
	_strappend(buf, " ");
	_strappend(buf, o_s);
	_strappend(buf, " ");
	_strappend(buf, b_s);
	_strappend(buf, " = ");
	_strappend(buf, r_s);
	_strappend(buf, "\n");
	
	ssize_t err1 = write(fd, buf,(size_t) _strlen(buf));
	if (err1 < 0) {
		perror("Error writing to file: ");
		_exit(FILE_WRITE_ERROR_CODE);
	}
	
	int err2 = close(fd);

	if (err2 < 0) {
		perror("Error closing file");
		_exit(FILE_CLOSE_ERROR_CODE);
	}
}


// Routine to handle the main mode of the calculator
void operation_mode(char *argv[]) {
	int a = _atoi(argv[1]);
	int b = _atoi(argv[3]);
	
	if (_strlen(argv[2]) != 1) {
		fprintf(stderr, "Parameter error!, use +, -, x or /\n");
		print_usage(argv[0]);
		_exit(PARAM_ERROR_CODE);
	}
	
	char op = *argv[2];
	int res;

	switch (op) {
		case '+':
			res = a + b;
			break;
		case '-':
			res = a - b;
			break;
		case 'x':
			res = a * b;
			break;
		case '/':
			if (b == 0) {
				fprintf(stderr, "Error: Division by zero\n");
				_exit(DIV_ZERO_ERROR_CODE);
			}
			res = a / b;
			break;
		default:
		// TODO: preguntar profe sobre esto
			fprintf(stderr, "Parameter error!, use +, -, x or /\n");
			_exit(PARAM_ERROR_CODE);
	}

	printf("Operation: %d %c %d = %d\n", a, op, b, res);
	append_file(argv[1], op, argv[3], res);

}


void get_line(int fd, char * buffer){
	char buf[2];
	ssize_t n_read;
	while( (n_read = read(fd, buf, 1)) > 0){
		if (buf[0] == '\n'){
			break;
		}
		_strappend(buffer, buf);
	}
	if (n_read < 0){
		perror("Error reading file");
		_exit(FILE_READ_ERROR_CODE);
	}
}


// TODO: comprobar edge cases
void search_history(int n, char * buffer) {
	int fd = open(log_file, O_RDONLY);
	char buf[1];
	ssize_t n_read;
	int lines_read = 0;
	while( (n_read = read(fd, buf, 1)) > 0){
		if (_strcmp(buf, "\n")){
			lines_read += 1;
		}
		if (lines_read == n-1){
			if (lines_read == 0){
				int err = lseek(fd, -1, SEEK_CUR);
				if (err < 0){
					perror("Couldn't seek back!");
					_exit(FILE_READ_ERROR_CODE);	
				}
			}
			return get_line(fd, buffer);
		}
	}
	if (n_read < 0){
		perror("Error reading file");
		_exit(FILE_READ_ERROR_CODE);
	}
}


// Routine to handle the history mode of the calculator.
void history_mode(char *argv[]) {
	int n = _atoi(argv[2]);
	char buffer[BUFSIZE] = "";
	search_history(n, buffer);
	if (_strcmp(buffer, "")){
		fprintf(stderr, "History entry not found\n");
		_exit(HISTORY_NOT_FOUND_ERROR);
	}else{
		printf("Line %d: %s\n", n, buffer);
	}
}


int main(int argc, char *argv[]) {
	if (argc != 4 && argc != 3){
		char *bin_name = argv[0];
		print_usage(bin_name);
		return 1;
	}

  	switch (argc){
		case 3:
			if (_strcmp(argv[1], "-b")) {
				history_mode(argv);
				break;
			} else {
				print_usage(argv[0]);
				break;
			}
		case 4:
			operation_mode(argv);
			break;
		default:
			fprintf(stderr, "Fatal error!");
			return FATAL_ERROR_CODE;
			break;
	}

	return 0;
}
