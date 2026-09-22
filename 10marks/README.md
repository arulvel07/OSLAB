# 10-Mark Potential Exam Questions & Complete Solutions

> **Target Question:** Question 1 (10 Marks)  
> **Allocated Time:** ~45–50 minutes  
> **Key Skills Tested:** Process creation (`fork`), process topologies, process replacement (`exec` family), error status decoding (`waitpid`), unidirectional and bidirectional anonymous pipes (`pipe`, `read`, `write`, `close`).

---

## Priority Ranking of 10-Mark Topics

| Priority | Topic / Concept | Probability | Why Examiners Choose It |
| :---: | :--- | :---: | :--- |
| **P1** | **Bidirectional Pipe IPC (Request-Response Protocol)** | **90%** | Tests descriptor lifecycle, closing all 4 unused ends, synchronous communication, and error reporting. |
| **P2** | **Fork-Exec Controller & Worker Orchestration** | **85%** | Tests dynamic argument passing, executing external programs concurrently, and decoding child exit codes (`WEXITSTATUS`). |
| **P3** | **Deterministic Process Hierarchy & Lineage** | **75%** | Tests conditional branching logic with `fork()`, PID/PPID lineage verification, and barrier cleanup. |
| **P4** | **Zombie & Orphan Process Demonstration** | **65%** | Tests fundamental OS process table states: process reaping vs init adoption without `sleep()`. |
| **P5** | **Unidirectional Pipe Data Stream & EOF Handling** | **60%** | Tests half-duplex byte stream transmission, buffer parsing, and EOF detection on pipe close. |

---

## Question 1 (Priority 1): Bidirectional Pipe String Transformation

### Problem Statement
Write a C program where a parent process communicates with a child process using **two anonymous pipes** (`p2c` for parent-to-child and `c2p` for child-to-parent):
1. The parent prompts the user to enter a string and writes it to the child through `p2c`.
2. The child reads the string, counts the total number of vowels and consonants, reverses the string in-place, and sends both the counts and the reversed string back to the parent through `c2p`.
3. The parent reads the response, prints the statistics and the reversed string, and reaps the child process using `wait()`.
4. **Mandatory:** Ensure all unused pipe ends are closed in both processes to prevent deadlocks.

### Theoretical Concept & Pipe Rules
* Pipes are unidirectional byte streams. For full two-way communication, **two separate pipes are required**:
  * Pipe 1 (`p2c`): Parent writes to `p2c[1]`, Child reads from `p2c[0]`.
  * Pipe 2 (`c2p`): Child writes to `c2p[1]`, Parent reads from `c2p[0]`.
* **Deadlock Trap:** If the parent or child forgets to close unused write ends, `read()` will block indefinitely waiting for EOF rather than terminating.

### Solution Code
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main() {
    int p2c[2]; // Parent -> Child
    int c2p[2]; // Child -> Parent

    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // ================= CHILD PROCESS =================
        close(p2c[1]); // Close unused write end of p2c
        close(c2p[0]); // Close unused read end of c2p

        char str[BUFFER_SIZE];
        int bytes_read = read(p2c[0], str, sizeof(str) - 1);
        close(p2c[0]); // Finished reading from parent

        if (bytes_read <= 0) {
            exit(1);
        }
        str[bytes_read] = '\0';

        // Strip trailing newline if present
        str[strcspn(str, "\r\n")] = '\0';

        int vowels = 0, consonants = 0;
        int len = strlen(str);

        for (int i = 0; i < len; i++) {
            char ch = tolower((unsigned char)str[i]);
            if (ch >= 'a' && ch <= 'z') {
                if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u')
                    vowels++;
                else
                    consonants++;
            }
        }

        // Reverse the string in-place
        for (int i = 0, j = len - 1; i < j; i++, j--) {
            char temp = str[i];
            str[i] = str[j];
            str[j] = temp;
        }

        // Send stats and reversed string back to parent
        write(c2p[1], &vowels, sizeof(int));
        write(c2p[1], &consonants, sizeof(int));
        write(c2p[1], str, len + 1);
        close(c2p[1]); // Finished writing

        exit(0);
    } else {
        // ================= PARENT PROCESS =================
        close(p2c[0]); // Close unused read end of p2c
        close(c2p[1]); // Close unused write end of c2p

        char input[BUFFER_SIZE];
        printf("Parent: Enter a sentence: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            strcpy(input, "Operating Systems");
        }

        // Send input string to child
        write(p2c[1], input, strlen(input) + 1);
        close(p2c[1]); // Signal EOF to child

        int vowels, consonants;
        char reversed_str[BUFFER_SIZE];

        read(c2p[0], &vowels, sizeof(int));
        read(c2p[0], &consonants, sizeof(int));
        read(c2p[0], reversed_str, sizeof(reversed_str));
        close(c2p[0]); // Finished reading

        wait(NULL); // Reap child process

        printf("\n--- Parent Received Results ---\n");
        printf("Vowel Count     : %d\n", vowels);
        printf("Consonant Count : %d\n", consonants);
        printf("Reversed String : %s\n", reversed_str);
    }

    return 0;
}
```

### Compilation & Verification
```bash
gcc q1_bidirectional_pipe.c -o q1_bidirectional_pipe
./q1_bidirectional_pipe
```

---

## Question 2 (Priority 2): Fork-Exec Controller & Exit Status Decoding

### Problem Statement
Write a program `controller.c` that runs multiple system commands in parallel:
1. The program receives commands from command-line arguments (e.g. `./controller "ls -l" "date" "whoami"`).
2. For each command, the controller forks a child process. The child parses the command string into tokens and executes it using `execvp()`.
3. The parent **must not wait after each fork**; it spawns all children first to achieve concurrency.
4. After launching all commands, the parent waits for every child using `waitpid()`, checks whether each child terminated normally via `WIFEXITED()`, and prints its PID along with its exit status code (`WEXITSTATUS()`).

### Theoretical Concept
* **`fork()` + `execvp()`:** `fork()` creates a clone of the process; `execvp()` overwrites the clone's text and data segment with the target program.
* **Exit Status Macros:**
  * `WIFEXITED(status)`: Returns true if child exited via `exit()` or `return`.
  * `WEXITSTATUS(status)`: Returns the integer return code ($0$ usually indicates success).

### Solution Code
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_ARGS 16

void execute_command(char *cmd_str) {
    char *args[MAX_ARGS];
    int idx = 0;

    // Tokenize command string by whitespace
    char *token = strtok(cmd_str, " ");
    while (token != NULL && idx < MAX_ARGS - 1) {
        args[idx++] = token;
        token = strtok(NULL, " ");
    }
    args[idx] = NULL; // Must be NULL-terminated array for execvp

    if (idx == 0) exit(0);

    execvp(args[0], args);
    perror("execvp failed");
    exit(127); // Standard failure code for command not found
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s \"cmd1\" \"cmd2\" ...\n", argv[0]);
        printf("Example: %s \"date\" \"uname -r\" \"whoami\"\n", argv[0]);
        return 1;
    }

    int num_cmds = argc - 1;
    pid_t pids[num_cmds];

    printf("Parent (PID %d): Launching %d commands concurrently...\n\n", getpid(), num_cmds);

    // Launch all children concurrently
    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pids[i] == 0) {
            // Child: execute command
            printf("[Child PID %d] Executing: %s\n", getpid(), argv[i + 1]);
            execute_command(argv[i + 1]);
        }
    }

    // Barrier: Wait for each specific child and decode exit status
    printf("\nParent waiting for children to finish...\n");
    for (int i = 0; i < num_cmds; i++) {
        int status;
        pid_t terminated_pid = waitpid(pids[i], &status, 0);

        if (WIFEXITED(status)) {
            printf("Child PID %d terminated normally with exit code: %d\n", 
                   terminated_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child PID %d killed by signal: %d\n", 
                   terminated_pid, WTERMSIG(status));
        }
    }

    printf("\nAll commands finished successfully.\n");
    return 0;
}
```

### Compilation & Verification
```bash
gcc controller.c -o controller
./controller "date" "uname -s" "echo Hello OS Lab"
```

---

## Question 3 (Priority 3): Deterministic Process Tree with Lineage Output

### Problem Statement
Write a C program that builds the following precise hierarchy:
* Original Parent ($P$) forks two children: $C_1$ and $C_2$.
* Child $C_1$ forks two grandchildren: $G_1$ and $G_2$.
* Child $C_2$ does not fork any processes.
* Every process must print:
  `[Role] PID: <own_pid>, Parent PID: <ppid>`
* The parent process must wait for all children to prevent zombie processes.

```
       Parent (P)
        /      \
     C1         C2
    /  \
   G1   G2
```

### Solution Code
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("[Root Parent] PID: %d, PPID: %d\n", getpid(), getppid());

    pid_t c1 = fork();

    if (c1 == 0) {
        // Inside Child 1 (C1)
        printf("[Child C1]    PID: %d, Parent PPID: %d\n", getpid(), getppid());

        pid_t g1 = fork();
        if (g1 == 0) {
            // Grandchild 1 (G1)
            printf("[Grandchild G1] PID: %d, Parent PPID: %d\n", getpid(), getppid());
            exit(0);
        }

        pid_t g2 = fork();
        if (g2 == 0) {
            // Grandchild 2 (G2)
            printf("[Grandchild G2] PID: %d, Parent PPID: %d\n", getpid(), getppid());
            exit(0);
        }

        // C1 waits for both grandchildren
        wait(NULL);
        wait(NULL);
        exit(0);
    }

    pid_t c2 = fork();
    if (c2 == 0) {
        // Inside Child 2 (C2) - Does not fork
        printf("[Child C2]    PID: %d, Parent PPID: %d\n", getpid(), getppid());
        exit(0);
    }

    // Root parent waits for both direct children (C1 and C2)
    wait(NULL);
    wait(NULL);

    printf("[Root Parent] All children have exited cleanly.\n");
    return 0;
}
```

### Compilation & Verification
```bash
gcc tree.c -o tree
./tree
```

---

## Question 4 (Priority 4): Zombie and Orphan Process Demonstration

### Problem Statement
Write two separate routines (or a combined program) demonstrating:
1. **Part A (Zombie Process):** The child prints its PID and exits immediately. The parent sleeps for 8 seconds without calling `wait()`. The child becomes a zombie (`state Z`). After sleeping, parent reaps it with `wait()`.
2. **Part B (Orphan Process):** The parent forks a child and exits immediately. The child busy-waits without `sleep()` until `getppid()` changes, proving it was adopted by `init`/`systemd`.

### Solution Code
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void demonstrate_zombie() {
    printf("=== Part A: Zombie Process ===\n");
    pid_t pid = fork();

    if (pid == 0) {
        printf("Child (PID %d): Exiting immediately.\n", getpid());
        exit(0);
    } else {
        printf("Parent (PID %d): Child PID is %d.\n", getpid(), pid);
        printf("Parent sleeping for 6 seconds without wait()...\n");
        printf("Run 'ps -o pid,ppid,state,cmd' in another terminal to see 'Z' state.\n");
        sleep(6);
        wait(NULL);
        printf("Parent: Reaped zombie child via wait().\n");
    }
}

void demonstrate_orphan() {
    printf("\n=== Part B: Orphan Process ===\n");
    pid_t pid = fork();

    if (pid > 0) {
        printf("Parent (PID %d): Terminating immediately to make child an orphan.\n", getpid());
        exit(0);
    } else {
        pid_t original_parent = getppid();
        printf("Child (PID %d): Initial Parent PPID is %d.\n", getpid(), original_parent);

        // Busy wait until parent terminates and PPID changes
        while (getppid() == original_parent) {
            // Busy wait - NO sleep()
        }

        printf("Child: Parent died! Adopted by new parent PPID: %d (init/systemd)\n", getppid());
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "orphan") == 0) {
        demonstrate_orphan();
    } else {
        demonstrate_zombie();
    }
    return 0;
}
```

---

## Question 5 (Priority 5): Producer-Consumer Array Stream via Unidirectional Pipe

### Problem Statement
Write a program where a parent process sends an array of $N$ integers to its child through a pipe. The child reads integers from the pipe until EOF, computes the running sum, average, and maximum value, and displays them on screen.

### Solution Code
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pfd[2];
    if (pipe(pfd) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child reads from pfd[0]
        close(pfd[1]); // Close unused write end

        int val;
        long long sum = 0;
        int count = 0;
        int max_val = -1e9;

        // read() returns 0 when parent closes write end (EOF)
        while (read(pfd[0], &val, sizeof(int)) > 0) {
            sum += val;
            count++;
            if (val > max_val) max_val = val;
        }
        close(pfd[0]);

        if (count > 0) {
            printf("\n--- Child Processing Report ---\n");
            printf("Elements Processed : %d\n", count);
            printf("Sum                : %lld\n", sum);
            printf("Average            : %.2f\n", (double)sum / count);
            printf("Maximum Element    : %d\n", max_val);
        } else {
            printf("Child: No data received.\n");
        }
        exit(0);
    } else {
        // Parent writes to pfd[1]
        close(pfd[0]); // Close unused read end

        int data[] = {14, 28, 57, 92, 33, 85, 41, 66};
        int n = sizeof(data) / sizeof(data[0]);

        printf("Parent: Sending %d elements across pipe...\n", n);
        for (int i = 0; i < n; i++) {
            write(pfd[1], &data[i], sizeof(int));
        }

        // CRITICAL: Close write end so child receives EOF
        close(pfd[1]);

        wait(NULL); // Reap child
    }

    return 0;
}
```
