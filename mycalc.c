#include <fcntl.h> /* To use open, read, write, close */
#include <unistd.h>
#include <stdio.h>

#include <stdlib.h>

// TODO: preguntar profe error codes
#define DEV_ZERO_ERROR_CODE -2
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

int _atoi(char *input){
	int out = 0;
	for (int i = 0; i < _strlen(input); i++){
		out += input[i] - '0';
		out *= 10;
	}
	return out / 10;
}

// TODO: Pregunar profe si esto es error
void print_usage(char *bin_name){
	printf("Usage (1): %s <num1> <operation (+ | - | x | /)> <num2> \n", bin_name);
	printf("Usage (2): %s -b <num operation> \n", bin_name);
}

void operation_mode(char *argv[]){
	int a = _atoi(argv[1]);
	int b = _atoi(argv[3]);
	// if (sizeof(argv[3]) != sizeof(char)){
	// 	fprintf(stderr, "Parameter error!, use +, -, x or /");
	// 	print_usage(argv[0]);
	// }
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
				_exit(DEV_ZERO_ERROR_CODE);
			}
			res = a / b;
			break;
		default:
		// TODO: preguntar profe sobre esto
			fprintf(stderr, "Parameter error!, use +, -, x or /\n");
			_exit(PARAM_ERROR_CODE);
	}


	printf("Operation: %d %c %d = %d\n", a, op, b, res);

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
