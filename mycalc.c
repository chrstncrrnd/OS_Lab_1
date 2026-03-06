#include <fcntl.h> /* To use open, read, write, close */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define FATAL_ERROR_CODE 		-1
#define DIV_ZERO_ERROR_CODE 	-1
#define PARAM_ERROR_CODE 		-1
#define FILE_OPEN_ERROR_CODE 	-1
#define FILE_CLOSE_ERROR_CODE 	-1
#define FILE_WRITE_ERROR_CODE 	-1
#define INCORRECT_PARAM_ERROR   -1
#define FILE_READ_ERROR_CODE    -1
#define HISTORY_NOT_FOUND_ERROR -1

#define INT_MAX ((int)(~0U >> 1))
#define INT_MIN (-INT_MAX - 1)

#define false 	0
#define true 	1
#define bool 	int

#define BUFSIZE 64

const char *log_file = "mycalc.log";


// Prints `text` in `stderr`
void eprint(const char* text){
	ssize_t err1 = write(STDERR_FILENO, text,(size_t) strlen(text));
	if (err1 < 0){
		_exit(-1);
	}
}


// Returns the number of digits in the input
int digits(int input) {
	int i = 0;
	for (; input > 0; input /= 10) {
		i++; // TODO preguntar profe sobre esto y si debe poner el print usage.
	}
	return i;
}


// Integer to ascii, writes the ascii value of `input` to `buf`.
void itoa(int input, char* buf) {
	// making int min positive, we get an overflow so we need to handle this case seperately
	if (input == INT_MIN){
		switch(sizeof(int)){
			// - 2** 15 (16 bit integer)
			case (2):
				strcpy(buf, "-32768");
				return;
			// - 2**31 (32 bit integer)
			case (4):
				strcpy(buf, "-2147483648");
				return;
			// - 2**63 (64 bit integer)
			case (8):
				strcpy(buf, "-9223372036854775808");
				return;
			default:
				eprint("No support for minimum integer on this platform");
				_exit(-1);
		}
	}

	// handle edge case of 0
	if (input == 0){
		strcpy(buf, "0");
		return;
	}

	bool negative = false;
	if (input < 0) {
		negative = true;
		input = 0 - input;
	}

	int digs = digits(input);
	for (int i = digs; input > 0; input /= 10) {
		buf[--i] = (char) (input % 10) + '0';
	}
	buf[digs] = '\0';

	if (negative) {
		char negative_buf[BUFSIZE] = "-";
		strcat(negative_buf, buf);
		strcpy(buf, negative_buf);
	}
}


bool is_int(const char* str){
	size_t start = 0;
	if (str[0] == '-'){
		start = 1;
	}
	for (size_t i = start; i < strlen(str); ++i){
		if (str[i] > '9' || str[i] < '0'){
			return false;
		}
	}
	return true;
}


// Safely converts a string to an int using strtol.
// Returns true and stores the result in *out on success.
// Returns false if the string is empty, contains non-numeric characters,
// or the value is outside the range [INT_MIN, INT_MAX].
bool safe_atoi(const char *str, int *out) {
	if (str == NULL || str[0] == '\0') {
		return false;
	}
	char *endptr;
	errno = 0;
	long val = strtol(str, &endptr, 10);
	if (errno == ERANGE) {
		return false;
	}
	// Ensure the entire string was consumed:
	// endptr == str means no digits were found (e.g. "+" or "-" alone);
	// *endptr != '\0' means trailing non-numeric characters (e.g. "42abc").
	if (endptr == str || *endptr != '\0') {
		return false;
	}
	// Ensure the long value fits in an int
	if (val > (long)INT_MAX || val < (long)INT_MIN) {
		return false;
	}
	*out = (int)val;
	return true;
}


// Prints `text` in `stdout`
void print(const char* text){
	ssize_t err1 = write(STDOUT_FILENO, text,(size_t) strlen(text));
	if (err1 < 0){
		_exit(-1);
	}
}


// Adds `a` and `b` and panics if overflow
int add_with_overflow(int a, int b) {
    if (b > 0 && a > INT_MAX - b) {
        eprint("Added with overflow!\n");
		_exit(-1);
    }
    if (b < 0 && a < INT_MIN - b) {
        eprint("Added with underflow!\n");
		_exit(-1);
    }
    return a + b;
}


// Subtracts `b` from `a` and panics if overflow or underflow
int sub_with_overflow(int a, int b) {
    if (b > 0 && a < INT_MIN + b) {
        eprint("Subtracted with underflow!\n");
		_exit(-1);
    }
    if (b < 0 && a > INT_MAX + b) {
        eprint("Subtracted with overflow!\n");
		_exit(-1);
    }
    return a - b;
}


// Multiplies `a` and `b` and panics if overflow
int mul_with_overflow(int a, int b) {
    if (a > 0 && b > 0 && a > INT_MAX / b) {
		eprint("Multiplied with overflow!\n");
		_exit(-1);
	};
    if (a > 0 && b <= 0 && b < INT_MIN / a) {
		eprint("multiplied with underflow!\n");
		_exit(-1);
	}

    if (a < 0 && b > 0 && a < INT_MIN / b) {
		eprint("multiplied with underflow!\n");
		_exit(-1);
	}
    if (a < 0 && b <= 0 && a != 0 && b < INT_MAX / a) {
		eprint("Multiplied with overflow!\n");
		_exit(-1);
	}
    
    return a * b;
}


// Divides `a` by `b` and panics if an overflow has occurred
int div_with_overflow(int a, int b) {
    if (b == 0) {
        eprint("Division by zero!\n");
		_exit(-1);
    }
    if (a == INT_MIN && b == -1) {
        eprint("Divided with overflow!\n");
		_exit(-1);
    }
    return a / b;
}


// Function to print how to use the binary
void print_usage(const char *bin_name) {
	print("Usage (1): ");
	print(bin_name);
	print(" <num1> <operation (+, -, x, /)> <num2> \n");
	print("Usage (2): ");
	print(bin_name);
	print(" -b <num operation>\n");
}


// Appends a result entry to the log file. 
// Raises errors for when the file is not found, cannot close file or cannot write to file.
void append_file(const char* a_s, char op, const char* b_s, const char* r_s) {
	int fd = open(log_file, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd < 0) {
		eprint("Error, could not open file!\n");
		_exit(FILE_WRITE_ERROR_CODE);
	}

	char buf[BUFSIZE] = "Operation: ";

	
	char o_s[2];
	o_s[0] = op;
	o_s[1] = '\0';
	
	strcat(buf, a_s);
	strcat(buf, " ");
	strcat(buf, o_s);
	strcat(buf, " ");
	strcat(buf, b_s);
	strcat(buf, " = ");
	strcat(buf, r_s);
	strcat(buf, "\n");
	
	ssize_t err1 = write(fd, buf,(size_t) strlen(buf));
	if (err1 < 0) {
		eprint("Error writing to log file!\n");
		_exit(FILE_WRITE_ERROR_CODE);
	}
	
	int err2 = close(fd);

	if (err2 < 0) {
		eprint("Error closing log file!\n");
		_exit(FILE_CLOSE_ERROR_CODE);
	}
}


// Routine to handle the main mode of the calculator
void operation_mode(char *argv[]) {
	int a, b;
	if (!safe_atoi(argv[1], &a) || !safe_atoi(argv[3], &b)){
		eprint("Parameter error!, numbers should be integers\n");
		print_usage(argv[0]);
		_exit(-1);
	}
	
	if (strlen(argv[2]) != 1) {
		eprint("Parameter error!, use +, -, x or /\n");
		print_usage(argv[0]);
		_exit(0);
	}
	
	char op = *argv[2];
	int res;

	switch (op) {
		case '+':
			res = add_with_overflow(a, b);
			break;
		case '-':
			res = sub_with_overflow(a, b);
			break;
		case 'x':
			res = mul_with_overflow(a, b);
			break;
		case '/':
			if (b == 0) {
				eprint("Error: Division by zero\n");
				_exit(DIV_ZERO_ERROR_CODE);
			}
			// we cannot overflow with this operation
			res = a / b;
			break;
		default:
		// TODO: preguntar profe sobre esto
			eprint("Parameter error!, use +, -, x or /\n");
			_exit(PARAM_ERROR_CODE);
	}

	char buf[BUFSIZE] = "Operation: ";

	char a_s[16] = "";
	itoa(a, a_s);

	char b_s[16] = "";
	itoa(b, b_s);

	char r_s[16] = "";
	itoa(res, r_s);
	
	char o_s[2];
	o_s[0] = op;
	o_s[1] = '\0';
	
	strcat(buf, a_s);
	strcat(buf, " ");
	strcat(buf, o_s);
	strcat(buf, " ");
	strcat(buf, b_s);
	strcat(buf, " = ");
	strcat(buf, r_s);
	strcat(buf, "\n");
	
	print(buf);
	append_file(a_s, op, b_s, r_s);
}


void get_line(int fd, char * buffer){
	char buf[2];
	ssize_t n_read;
	while( (n_read = read(fd, buf, 1)) > 0){
		if (buf[0] == '\n'){
			break;
		}
		strcat(buffer, buf);
	}
	if (n_read < 0){
		eprint("Error reading file\n");
		_exit(FILE_READ_ERROR_CODE);
	}
}


void search_history(int n, char * buffer) {
	int fd = open(log_file, O_RDONLY);
	char buf[2];
	ssize_t n_read;
	int lines_read = 0;
	while( (n_read = read(fd, buf, 1)) > 0){
		if (strcmp(buf, "\n") == 0){
			lines_read += 1;
		}
		if (lines_read == n-1){
			if (lines_read == 0){
				long err = lseek(fd, -1, SEEK_CUR);
				if (err < 0){
					eprint("Couldn't seek back on log file!\n");
					_exit(FILE_READ_ERROR_CODE);	
				}
			}
			get_line(fd, buffer);
      return;
		}
	}
	if (n_read < 0){
		eprint("Error reading file\n");
		_exit(FILE_READ_ERROR_CODE);
	}
}


// Routine to handle the history mode of the calculator.
void history_mode(char *argv[]) {
	int n;
	if (!safe_atoi(argv[2], &n)) {
		eprint("Parameter error!, history entry should be an integer\n");
		print_usage(argv[0]);
		_exit(-1);
	}
	char buffer[BUFSIZE] = "";
	search_history(n, buffer);
	if (strcmp(buffer, "") == 0){
		eprint("History entry not found\n");
		_exit(HISTORY_NOT_FOUND_ERROR);
	} else {
		print("Line ");
		char n_str[16];

		itoa(n, n_str);
		print(n_str);
		print(": ");
		print(buffer);
		print("\n");
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
			if (strcmp(argv[1], "-b") == 0) {
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
			eprint("Fatal error!\n");
			return FATAL_ERROR_CODE;
			break;
	}

	return 0;
}
