//Arulvel V
//CS24i1027
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char *argv[]){
    if (argc != 4) {
        printf("Usage:- %s num1 operator num2\n", argv[0]);
        return 1;
    }
    double n1 = atof(argv[1]);
    double n2 = atof(argv[3]);
    char op = argv[2][0];
    double r;
    
    switch (op) {
        case '+':
            r = n1 + n2;
            break;
        case '-':
            r = n1 - n2;
            break;
        case '*':
            r = n1 * n2;
            break;
        case '/':
            if (n2 == 0) {
                fprintf(stderr, "Division by zero\n");
                return 1;
            }
            r = n1 / n2;
            break;
        default:
            printf("Invalid operator\n");
            return 1;
    }


    printf("%.2f %c %.2f = %.2f\n", n1, op, n2, r);
    return 0;
}
