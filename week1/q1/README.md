# Week 1 - Question 1: File Word, Line & Character Statistics

## Problem Statement
Write a program that reads a text file and prints:
- Total number of lines
- Total number of words
- Total number of characters
- The longest line in the file

![Question](./1.png)

---

## 🎯 Three Exam Approaches

### Approach 1: Original Interactive C++ (`q1.cpp`)
Reads filename from user via `cin`.
```cpp
#include <bits/stdc++.h>
using namespace std;

int main() {
    string filename;
    cout << "enter file name:- ";
    cin >> filename;
    
    ifstream file(filename);
    if (!file) {
        cout << "file not found" << endl;
        return 1;
    }
    string line; 
    int totalline = 0, totalwords = 0, totalchars = 0;
    string longestline;
    
    while (getline(file, line)) {
        totalline++;
        totalchars += line.length();
        stringstream ss(line);
        string word;
        while (ss >> word) totalwords++;
        if (line.length() > longestline.length()) longestline = line;
    }
    file.close();
    
    cout << "total number of lines:- " << totalline << endl;
    cout << "total number of words:- " << totalwords << endl;
    cout << "total number of characters:- " << totalchars << endl;
    cout << "longest line:- " << longestline << endl;
    return 0;
}
```

---

### Approach 2 (OS Exam Favorite): Fork-Exec Child via `execlp("wc", ...)` (`q1_cli_fork.c`)

> **Why examiners ask this:**  
> Instead of manually parsing the text file in C/C++, the parent takes the filename from Command-Line Arguments (`argc`, `argv[1]`), forks a child process, and the child delegates the line/word/byte counting to the existing Linux `wc` utility using `execlp()`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child executes Linux wc command to compute line, word, and character counts
        printf("[Child PID %d] Running 'wc' on file: %s\n", getpid(), argv[1]);
        execlp("wc", "wc", argv[1], (char *)NULL);

        // execlp only returns if an error occurs
        perror("execlp failed");
        exit(1);
    }

    // Parent continues and waits for child
    printf("Parent (PID %d): Waiting for child to finish analysis...\n", getpid());
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("Parent: Child finished with exit status %d.\n", WEXITSTATUS(status));
    }

    return 0;
}
```

#### Compile & Run
```bash
gcc q1_cli_fork.c -o q1_cli_fork
./q1_cli_fork sample.txt
```

---

### Approach 3: Pure C Command-Line File Parser (`q1_stats.c`)
If the professor asks for pure C (no C++ headers) taking filename from CLI arguments (`argv[1]`):

```c
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
```

#### Compile & Run
```bash
gcc q1_stats.c -o q1_stats
./q1_stats sample.txt
```
