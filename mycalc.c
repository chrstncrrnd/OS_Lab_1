#include <fcntl.h> /* To use open, read, write, close */
#include <unistd.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

// TODO: preguntar profe error codes
#define DIV_ZERO_ERROR_CODE -2
#define FATAL_ERROR_CODE -1
#define PARAM_ERROR_CODE -3


#define false 0
#define true 1
#define bool int

const char *log_file = "mycalc.log";

int _strlen(char *input){
	int out = 0;
	for (int i = 0; input[i] != '\0' ; i++){
		out += 1;
	}
	return out;
}

bool _strcmp(char* a, char* b){
	int len_a = _strlen(a);
	if (len_a != _strlen(b)){
		return false;
	}
	for (int i = 0; i < len_a; i ++){
		if (a[i] != b[i]){
			return false;
		}
	}
	return true;
}



void _strcpy(char *to, char *from, int offset){
	for (int i = offset; i < _strlen(from) + offset; i++){
		to[i] = from[i - offset];
	}	
}


// overwrites lhs
void _strappend(char *lhs, char *rhs){
	_strcpy(lhs, rhs, _strlen(lhs));
}

int _atoi(char *input){
	int out = 0;
	for (int i = 0; i < _strlen(input); i++){
		out += input[i] - '0';
		out *= 10;
	}
	return out / 10;
}

int digits(int input){
	int i = 0;
	for (; input > 0; input /= 10){
		i++;
	}
	return i;
}

void _itoa(int input, char* buf){
	int digs = digits(input);
	for (int i = digs; input > 0; input /= 10){
		buf[--i] = (char) (input % 10) + '0';
	}
	buf[digs] = '\0';
}

// TODO: Pregunar profe si esto es error
void print_usage(char *bin_name){
	printf("Usage (1): %s <num1> <operation (+, -, x, /)> <num2> \n", bin_name);
	printf("Usage (2): %s -b <num operation> \n", bin_name);
}

// TODO: error handling
void append_file(char* a_s, char op, char* b_s, int res){
	int fd = open(log_file, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd < 0){
		perror("Error, could not open file!");
	}


	char buf[512] = "Operation: ";

	char r_s[32];
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
	if (err1 < 0){
		perror("Error writing to file: ");
	}
	
	int err2 = close(fd);
	if (err2 < 0){
		perror("Error closing file: ");
	}
}

void operation_mode(char *argv[]){
	int a = _atoi(argv[1]);
	int b = _atoi(argv[3]);
	
	if (_strlen(argv[2]) != 1){
		fprintf(stderr, "Parameter error!, use +, -, x or /\n");
		print_usage(argv[0]);
		_exit(PARAM_ERROR_CODE);
	}
	
	char op = *argv[2];
	int res;

	switch (op){
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
			if (b == 0){
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

void history_mode(char *argv[]){
	printf("TODO!\n");
	char *a = argv[0];
	printf("a: %s\n", a);
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
