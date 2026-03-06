#include <fcntl.h> /* To use open, read, write, close */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
  // exit if error
	if (err1 < 0){
		_exit(-1);
	}
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

  //negative case
	bool negative = false;
	if (input < 0) {
		negative = true;
		input = 0 - input;
	}

	int digs = digits(input);
  // loop over al ldigits
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

// function to see if a string is an integer (i.e. no letters or non digits in string)
bool is_int(const char* str){
	size_t start = 0;
	if (str[0] == '-'){
		start = 1;
	}
  // loop over each character in string
	for (size_t i = start; i < strlen(str); ++i){
    // make sure that they are digits
		if (str[i] > '9' || str[i] < '0'){
			return false;
		}
	}
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

	// string to hold the operation
	char o_s[2];
	o_s[0] = op;
	o_s[1] = '\0';


  // generating the string that we will write to the file
	strcat(buf, a_s);
	strcat(buf, " ");
	strcat(buf, o_s);
	strcat(buf, " ");
	strcat(buf, b_s);
	strcat(buf, " = ");
	strcat(buf, r_s);
	strcat(buf, "\n");

  // write it to the file
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
  // make sure that both arguments are integer strings
	if (is_int(argv[1]) == 0 || is_int(argv[3]) == 0){
		eprint("Parameter error!, numbers should be integers\n");
		print_usage(argv[0]);
		_exit(-1);
	}
  // convert arguments to ints
	int a = atoi(argv[1]);
	int b = atoi(argv[3]);
	
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
			eprint("Parameter error!, use +, -, x or /\n");
			_exit(PARAM_ERROR_CODE);
	}

	char buf[BUFSIZE] = "Operation: ";
  // LHS string
	char a_s[16] = "";
	itoa(a, a_s);
  // RHS string
	char b_s[16] = "";
	itoa(b, b_s);
  // result string
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

// get the next line from where we are pointing to in the file fd
void get_line(int fd, char * buffer){
	char buf[2];
	ssize_t n_read;
  // check that we aren't going to reach the end of the file
	while( (n_read = read(fd, buf, 1)) > 0){
    // break if we have a newline (reached the end of the line)
		if (buf[0] == '\n'){
			break;
		}
    // append to buffer what we have just read
		strcat(buffer, buf);
	}
	if (n_read < 0){
		eprint("Error reading file\n");
		_exit(FILE_READ_ERROR_CODE);
	}
}


// function to search the file mycalc.log
void search_history(int n, char * buffer) {
  // open the log file
	int fd = open(log_file, O_RDONLY);
  // we are reading one character at a time, so we need a buffer of two elements
  // because of null-terminated strings
	char buf[2];
  // we keep track of how many bytes we have read
	ssize_t n_read;
  // how many lines have we read
	int lines_read = 0;
  // loop while we are still reading from the file
	while( (n_read = read(fd, buf, 1)) > 0){
		if (strcmp(buf, "\n") == 0){
      // increment lines_read when we read a new line character
			lines_read += 1;
		}
    // the next line is the one we want
		if (lines_read == n-1){
      // edge case for n = 1
			if (lines_read == 0){
        // since we pushed forward the pointer at the beginning of the while loop,
        // we need to push it back one
				long err = lseek(fd, -1, SEEK_CUR);
				if (err < 0){
					eprint("Couldn't seek back on log file!\n");
					_exit(FILE_READ_ERROR_CODE);	
				}
			}
      // get the current line and exit
			get_line(fd, buffer);
      return;
		}
	}

  // we havent read any file
	if (n_read < 0){
		eprint("Error reading file\n");
		_exit(FILE_READ_ERROR_CODE);
	}
}


// Routine to handle the history mode of the calculator.
void history_mode(char *argv[]) {
	int n = atoi(argv[2]);
	char buffer[BUFSIZE] = "";
	search_history(n, buffer);
  // if we have not found the entry
	if (strcmp(buffer, "") == 0){
		eprint("History entry not found\n");
		_exit(HISTORY_NOT_FOUND_ERROR);
	} else {
    // print out the result
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
  // check we have a valid number of arguments
	if (argc != 4 && argc != 3){
		char *bin_name = argv[0];
		print_usage(bin_name);
		return 1;
	}
  // based on the number of arguments, what do we do
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
