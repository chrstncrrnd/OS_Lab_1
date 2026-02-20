#include <fcntl.h> /* To use open, read, write, close */
#include <unistd.h>

// ups
#define FATAL_ERROR_CODE 		-1
#define DIV_ZERO_ERROR_CODE 	-1
#define PARAM_ERROR_CODE 		-1
#define FILE_OPEN_ERROR_CODE 	-1
#define FILE_CLOSE_ERROR_CODE 	-1
#define FILE_WRITE_ERROR_CODE 	-1
#define INCORRECT_PARAM_ERROR   -1
#define FILE_READ_ERROR_CODE    -1
#define HISTORY_NOT_FOUND_ERROR -1

#define false 	0
#define true 	1
#define bool 	int

#define BUFSIZE 64

const char *log_file = "mycalc.log";



// Returns the length of the string `input`, not the size that it occupies in memory
int my_strlen(const char *input) {
	int out = 0;
	for (int i = 0; input[i] != '\0' ; i++) {
		out += 1;
	}
	return out;
}


// Compares two strings `a` and `b` returns true if all characters are equal
bool my_strcmp(const char* a, const char* b) {
	int len_a = my_strlen(a);
	if (len_a != my_strlen(b)) {
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
void my_strcpy(char *to, const char *from, int offset) {
	for (int i = offset; i < my_strlen(from) + offset; i++) {
		to[i] = from[i - offset];
	}	
}


// Appends the string `rhs` to the string `lhs`. Result is stored in `lhs`.
void my_strappend(char *lhs, const char *rhs) {
	my_strcpy(lhs, rhs, my_strlen(lhs));
}
void eprint(char* text){
	ssize_t err1 = write(STDERR_FILENO, text,(size_t) my_strlen(text));
	if (err1 < 0){
		_exit(-1);
	}
}


// TODO preguntar profe sobre esto y si debe poner el print usage.
// Ascii to integer, get integer value of ascii.
// Raises error if one of the characters is not a digit
int my_atoi(const char *input) {
	int out = 0;
	for (int i = 0; i < my_strlen(input); i++) {
		char c = input[i];
		if (c > '9' || c < '0'){
			eprint("Tried to convet non digit character to integer.\n");
			_exit(0);
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
void my_itoa(int input, char* buf) {
	int digs = digits(input);
	for (int i = digs; input > 0; input /= 10) {
		buf[--i] = (char) (input % 10) + '0';
	}
	buf[digs] = '\0';
}

void print(char* text){
	ssize_t err1 = write(STDOUT_FILENO, text,(size_t) my_strlen(text));
	if (err1 < 0){
		_exit(-1);
	}
}



// TODO: Pregunar profe si esto es error
// Function to print how to use the binary
void print_usage(const char *bin_name) {
	char usage1[128] = "Usage (1): ";
	my_strappend(usage1, bin_name);
	my_strappend(usage1, " <num1> <operation (+, -, x, /)> <num2> \n");
	print(usage1);
	char usage2[128] = "Usage (2): ";
	my_strappend(usage2, bin_name);
	my_strappend(usage2, " -b <num operation>\n");
	print(usage2);
	
}


// Appends a result entry to the log file. 
// Raises errors for when the file is not found, cannot close file or cannot write to file.
void append_file(const char* a_s, char op, const char* b_s, int res) {
	int fd = open(log_file, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd < 0) {
		perror("Error, could not open file!");
		_exit(FILE_WRITE_ERROR_CODE);
	}

	char buf[BUFSIZE] = "Operation: ";

	char r_s[16];
	my_itoa(res, r_s);
	
	char o_s[2];
	o_s[0] = op;
	o_s[1] = '\0';
	
	my_strappend(buf, a_s);
	my_strappend(buf, " ");
	my_strappend(buf, o_s);
	my_strappend(buf, " ");
	my_strappend(buf, b_s);
	my_strappend(buf, " = ");
	my_strappend(buf, r_s);
	my_strappend(buf, "\n");
	
	ssize_t err1 = write(fd, buf,(size_t) my_strlen(buf));
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
	int a = my_atoi(argv[1]);
	int b = my_atoi(argv[3]);
	
	if (my_strlen(argv[2]) != 1) {
		//fprintf(stderr, "Parameter error!, use +, -, x or /\n");
		print_usage(argv[0]);
		_exit(0);
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
				//fprintf(stderr, "Error: Division by zero\n");
				_exit(DIV_ZERO_ERROR_CODE);
			}
			res = a / b;
			break;
		default:
		// TODO: preguntar profe sobre esto
			//fprintf(stderr, "Parameter error!, use +, -, x or /\n");
			_exit(PARAM_ERROR_CODE);
	}

	//printf("Operation: %d %c %d = %d\n", a, op, b, res);
	append_file(argv[1], op, argv[3], res);

}


void get_line(int fd, char * buffer){
	char buf[2];
	ssize_t n_read;
	while( (n_read = read(fd, buf, 1)) > 0){
		if (buf[0] == '\n'){
			break;
		}
		my_strappend(buffer, buf);
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
		if (my_strcmp(buf, "\n")){
			lines_read += 1;
		}
		if (lines_read == n-1){
			if (lines_read == 0){
				long err = lseek(fd, -1, SEEK_CUR);
				if (err < 0){
					perror("Couldn't seek back!");
					_exit(FILE_READ_ERROR_CODE);	
				}
			}
			get_line(fd, buffer);
      return;
		}
	}
	if (n_read < 0){
		perror("Error reading file");
		_exit(FILE_READ_ERROR_CODE);
	}
}


// Routine to handle the history mode of the calculator.
void history_mode(char *argv[]) {
	int n = my_atoi(argv[2]);
	char buffer[BUFSIZE] = "";
	search_history(n, buffer);
	if (my_strcmp(buffer, "")){
		//fprintf(stderr, "History entry not found\n");
		_exit(HISTORY_NOT_FOUND_ERROR);
	}else{
		//printf("Line %d: %s\n", n, buffer);
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
			if (my_strcmp(argv[1], "-b")) {
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
			//fprintf(stderr, "Fatal error!");
			return FATAL_ERROR_CODE;
			break;
	}

	return 0;
}
