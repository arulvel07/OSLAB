#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("File not found");
        return 1;
    }

    char line[1024];
    char longest_line[1024] = "";
    int lines = 0, words = 0, chars = 0;

    while (fgets(line, sizeof(line), file)) {
        lines++;
        int len = strlen(line);
        chars += len;

        if (len > strlen(longest_line)) {
            strcpy(longest_line, line);
        }

        // Count words
        int in_word = 0;
        for (int i = 0; i < len; i++) {
            if (isspace((unsigned char)line[i])) {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                words++;
            }
        }
    }
    fclose(file);

    printf("Total Lines      : %d\n", lines);
    printf("Total Words      : %d\n", words);
    printf("Total Characters : %d\n", chars);
    printf("Longest Line     : %s", longest_line);
    return 0;
}
