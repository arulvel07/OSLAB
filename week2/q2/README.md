# Week 2 - Question 2: Asynchronous File Backup via `fork()` and `execlp()`

## Problem Statement
A system administrator wants a utility that copies (backs up) a file to a backup location. Instead of writing the copy logic manually, the program should use the existing Linux command internally — but run it as a child process, so the parent process can continue doing other work (like logging) while the copy happens.

![Question](./Screenshot%202026-09-22%20135939.png)

---

## 💻 Top Linux Commands for the `exec` Family (Exam Cheat Sheet)

### 🔑 The 2 Golden Rules of `exec` Syntax
1. **The command name is written twice:**  
   The 1st parameter is the binary to find, and the 2nd parameter is `argv[0]` (the program's name by convention).
2. **The argument list MUST terminate with `NULL` (or `(char *)NULL`):**  
   If you omit `NULL`, `exec` will read uninitialized memory and crash with a Segmentation Fault!

---

### Quick Syntax for Common Exam Commands

| Command | Typical Exam Goal | Exact `execlp` Syntax |
| :--- | :--- | :--- |
| **`cp`** | Copy source to destination | `execlp("cp", "cp", "src.txt", "dst.txt", (char *)NULL);` |
| **`ls`** | Long listing of files | `execlp("ls", "ls", "-l", (char *)NULL);` |
| **`wc`** | Count lines in a file | `execlp("wc", "wc", "-l", "file.txt", (char *)NULL);` |
| **`grep`** | Search pattern in a file | `execlp("grep", "grep", "ERROR", "log.txt", (char *)NULL);` |
| **`cat`** | Display contents of a file | `execlp("cat", "cat", "file.txt", (char *)NULL);` |
| **`head`** | Display first 5 lines | `execlp("head", "head", "-n", "5", "file.txt", (char *)NULL);` |
| **`date`** | Print current timestamp | `execlp("date", "date", (char *)NULL);` |
| **`whoami`** | Print current user | `execlp("whoami", "whoami", (char *)NULL);` |
| **`./custom`** | Run another compiled program | `execl("./calc_worker", "calc_worker", "10", "+", "5", (char *)NULL);` |

---

### 🧠 `execlp` vs `execvp`: When to Use Which?

* **`execlp()` (`l` = list, `p` = PATH lookup):**  
  Use when arguments are **fixed and known at compile time**:
  ```c
  execlp("cp", "cp", argv[1], argv[2], (char *)NULL);
  ```
* **`execvp()` (`v` = vector/array, `p` = PATH lookup):**  
  Use when arguments are **dynamic** (e.g., tokenized from user input string):
  ```c
  char *args[] = {"ls", "-l", "/home", NULL}; // Array MUST end with NULL
  execvp(args[0], args);
  ```

---

## Solution Code (`copy.c`)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <backup_destination>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process: execute Linux cp command 
        execlp("cp", "cp", argv[1], argv[2], (char *)NULL);

        // execlp() returns ONLY if an error occurs
        perror("execlp");
        exit(1);
    }

    // Parent process continues doing other work
    printf("Backup started. Child PID: %d\n", pid);
    printf("Parent process can continue logging/other work...\n");

    // Wait for the backup to finish and inspect exit status
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        printf("Backup completed successfully.\n");
    else
        printf("Backup failed.\n");

    return 0;
}
```

---

## How to Compile & Run
```bash
gcc copy.c -o copy
./copy source_file.txt backup_file.txt
```

---

## Output Screenshot
![Output](./OUTPUT)
