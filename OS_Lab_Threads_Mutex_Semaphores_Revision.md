# OS Lab Revision — POSIX Threads, Mutexes and Semaphores

**Focus:** Ubuntu-based C lab questions. **Not included:** condition variables (as requested).

> **How to study:** First memorize the syntax, then read each full question and its commented program. Finally try writing the code without looking. For the exam, adapt the code to the exact question; a basic Dining Philosophers solution does **not** satisfy extra fairness/timing requirements automatically.

---

## 1. Headers and compilation

```c
#include <stdio.h>      // printf(), scanf()
#include <stdlib.h>     // rand(), srand()
#include <pthread.h>    // pthreads and mutexes
#include <semaphore.h>  // sem_init(), sem_wait(), sem_post()
#include <unistd.h>     // usleep(), sleep()
#include <time.h>       // time()
#include <sys/time.h>   // gettimeofday(), struct timeval (if needed)
```

Compile on Ubuntu/Linux:

```bash
gcc program.c -o program -pthread
./program
```

**Where does each part go?** Headers at the top; global shared data and locks before functions; worker functions before `main()`; initialization, thread creation, joins, and cleanup in `main()`.

---

## 2. PTHREAD — When and how to use threads

**Use threads when the question asks for multiple workers doing things concurrently:** terminals, printers, bank workers, philosophers, etc.

| Syntax | Meaning | Where to write it |
|---|---|---|
| `pthread_t t;` or `pthread_t t[N];` | Thread handle(s) | Usually in `main()` |
| `void *worker(void *arg)` | Function run by each thread | Outside `main()` |
| `int id = *(int *)arg;` | Read integer ID passed by pointer | Inside worker function |
| `pthread_create(&t[i], NULL, worker, &ids[i]);` | Start a thread | In `main()` |
| `pthread_join(t[i], NULL);` | Wait for thread to finish | In `main()`, after creation |
| `return NULL;` | End worker function | End of worker function |
| `pthread_exit(NULL);` | Explicitly end calling thread (optional) | Inside a thread, if needed |

### Practice question: Create five concurrent workers

**Question:** Write a C program using POSIX threads to create five worker threads. Each worker should print its ID, and the main thread must wait for all workers to finish.

```c
#include <stdio.h>
#include <pthread.h>

#define N 5

// WORKER FUNCTION: executed by each newly created thread.
void *worker(void *arg) {
    int id = *(int *)arg;       // Convert void* to int*, then read ID.
    printf("Worker %d running\n", id);
    return NULL;                // Worker thread finishes.
}

int main() {
    pthread_t threads[N];       // MAIN: handles for all 5 threads.
    int ids[N];                 // MAIN: stable ID for each thread.

    for (int i = 0; i < N; i++) {
        ids[i] = i + 1;        // Set ID BEFORE creating its thread.
        pthread_create(&threads[i], NULL, worker, &ids[i]);
        //                 handle    attributes function argument
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);  // MAIN waits for every worker.
    }

    printf("All workers finished.\n");
    return 0;
}
```

**Remember:** Pass `&ids[i]`, not `&i` (the loop variable changes). Thread execution/print order is not guaranteed. Production code should check the return values of pthread calls.

---

## 3. MUTEX — When and how to protect shared data

**Use a mutex when threads update the same shared variable, array, queue, or account.** A mutex does not automatically know what variable it protects. **You choose** which operations go between lock and unlock, and all threads accessing that data must follow the same rule.

| Syntax | Meaning | Where to write it |
|---|---|---|
| `pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;` | Declare + initialize | Global scope is convenient |
| `pthread_mutex_init(&lock, NULL);` | Alternative runtime initialization | Before threads start (don't double-initialize) |
| `pthread_mutex_lock(&lock);` | Enter protected section | Worker function, just before shared access |
| `pthread_mutex_unlock(&lock);` | Leave protected section | Worker function, after shared access |
| `pthread_mutex_destroy(&lock);` | Clean up | `main()`, after joins |

### Practice question: Concurrent deposits

**Question:** Five threads each deposit ₹100 into one shared bank balance initially ₹1,000. Use a mutex to avoid lost updates. Print the final balance.

```c
#include <stdio.h>
#include <pthread.h>

#define N 5

int balance = 1000;  // GLOBAL: shared by all threads.
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // GLOBAL shared lock.

void *deposit(void *arg) {
    int id = *(int *)arg;

    pthread_mutex_lock(&lock);  // WORKER: lock BEFORE changing balance.
    balance += 100;             // CRITICAL SECTION: shared update.
    printf("Worker %d deposited 100; balance = %d\n", id, balance);
    pthread_mutex_unlock(&lock);// WORKER: unlock AFTER shared update.

    return NULL;
}

int main() {
    pthread_t threads[N];
    int ids[N];

    for (int i = 0; i < N; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, deposit, &ids[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL); // Wait before checking result.

    printf("Final balance = %d\n", balance); // 1500.
    pthread_mutex_destroy(&lock);       // MAIN: cleanup after joins.
    return 0;
}
```

**When there are multiple mutexes (bank transfers):** Acquire needed locks in the same global order (e.g., lower account index first) to prevent circular waiting/deadlock. Release every acquired lock on every path.

---

## 4. SEMAPHORE — When and how to control limited resources

**Use a semaphore when threads must wait for a resource or signal that one is available.** Examples: queue slots, ready jobs, chopsticks, two sauce users.

| Syntax | Meaning | Where to write it |
|---|---|---|
| `sem_t s;` or `sem_t s[N];` | Semaphore variable(s) | Often global |
| `sem_init(&s, 0, 2);` | Initialize with **2 permits**; `0` means threads in same process | `main()`, before thread creation |
| `sem_wait(&s);` | Acquire one permit; wait if none | Worker, **before** using resource |
| `sem_post(&s);` | Release/announce one permit | Worker, **after** using/releasing resource |
| `sem_destroy(&s);` | Clean up | `main()`, after joins |

**Golden rule: `sem_wait` for what you NEED; `sem_post` for what you MAKE AVAILABLE.**

### Producer–Consumer memory table

| Role | First (need) | Then modify queue under mutex | Last (make available) |
|---|---|---|---|
| Producer | `sem_wait(&empty)` | Insert job | `sem_post(&full)` |
| Consumer | `sem_wait(&full)` | Remove job | `sem_post(&empty)` |

**Why `sem_wait` before mutex lock?** If a consumer held the queue mutex while waiting for `full`, a producer could be unable to lock the queue to add a job. The semaphore wait must happen **outside** the queue mutex.

---

## 5. Full practice question — ORIGINAL Priority Producer–Consumer

**Question (from your lab):** A print server has a spooler queue that can hold at most 8 print jobs. Three student terminals are producers, each submitting 10 jobs with random delays; two printers are consumers. Jobs are either normal or priority. A printer must choose a priority job if one is waiting, otherwise a normal job. Never insert into a full queue or remove from an empty queue; protect against simultaneous access. Process 30 jobs, terminate threads cleanly, and log producer, consumer, priority and queue occupancy after operations.

### Where everything goes

- **Global:** `Job`, queue, `count`, `served`, semaphores `empty/full`, queue mutex.
- **`producer()` thread:** generate job → `wait(empty)` → lock → insert → unlock → `post(full)`.
- **`printer()` thread:** `wait(full)` → lock → find first priority → remove/shift → unlock → `post(empty)`.
- **`main()`:** initialize semaphore counts, create 3+2 threads, join, destroy.

### Your original program with explanatory comments

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define SIZE 8
#define TOTAL 30

typedef struct {
    int terminal, job, priority;
} Job;

Job queue[SIZE];             // GLOBAL shared queue.
int count = 0, served = 0;  // GLOBAL shared counters.

sem_t empty, full;          // empty = free slots; full = available jobs.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Protect queue/counters.

void *producer(void *arg) {
    int terminal = *(int *)arg; // Each terminal has a separate ID.

    for (int job = 1; job <= 10; job++) {
        usleep((rand() % 500 + 100) * 1000); // Simulate arrival delay.
        Job j = {terminal, job, rand() % 2}; // priority 0=normal, 1=priority.

        sem_wait(&empty);           // NEED one FREE SLOT; block if queue full.
        pthread_mutex_lock(&mutex);// Lock before changing shared queue/count.

        queue[count++] = j;        // Insert at end; increase occupancy.
        printf("Terminal %d produced Job %d (%s) | Queue: %d/8\n",
               terminal, job, j.priority ? "Priority" : "Normal", count);

        pthread_mutex_unlock(&mutex); // Queue modification complete.
        sem_post(&full);              // MADE one JOB available to printers.
    }
    return NULL;
}

void *printer(void *arg) {
    int printer = *(int *)arg; // Printer 1 or 2.

    while (1) {                // Keep processing until all 30 jobs served.
        sem_wait(&full);       // NEED one available job; wait if none.
        pthread_mutex_lock(&mutex); // Protect queue, count and served.

        if (served == TOTAL) { // Other printer may have taken last job.
            pthread_mutex_unlock(&mutex);
            sem_post(&full);   // Original code's artificial wake-up signal.
            break;
        }

        int index = 0;         // Default: take first (oldest) normal job.
        for (int i = 0; i < count; i++) {
            if (queue[i].priority) { // First priority job takes precedence.
                index = i;
                break;
            }
        }

        Job j = queue[index];  // Save job details before removing it.
        for (int i = index; i < count - 1; i++)
            queue[i] = queue[i + 1]; // Shift later jobs left to fill gap.

        count--;               // One fewer job in queue.
        served++;              // One more job DEQUEUED (not yet delay-finished).
        printf("Printer %d consumed Job %d from Terminal %d (%s) | Queue: %d/8\n",
               printer, j.job, j.terminal,
               j.priority ? "Priority" : "Normal", count);

        int done = (served == TOTAL); // Record last-job status under lock.
        pthread_mutex_unlock(&mutex); // Release shared queue.
        sem_post(&empty);             // MADE one FREE SLOT available.

        usleep((rand() % 1000 + 500) * 1000); // Simulated print duration.

        if (done) {
            sem_post(&full); // Wake other printer so it can exit.
            break;
        }
    }
    return NULL;
}

int main() {
    pthread_t producers[3], printers[2]; // Thread handles.
    int terminals[] = {1, 2, 3};         // Stable IDs for producers.
    int printer_ids[] = {1, 2};          // Stable IDs for printers.

    srand(time(NULL)); // Seed pseudo-random numbers once.

    sem_init(&empty, 0, SIZE); // Initially 8 empty slots.
    sem_init(&full, 0, 0);     // Initially 0 available jobs.

    for (int i = 0; i < 3; i++)
        pthread_create(&producers[i], NULL, producer, &terminals[i]);
    for (int i = 0; i < 2; i++)
        pthread_create(&printers[i], NULL, printer, &printer_ids[i]);

    for (int i = 0; i < 3; i++)
        pthread_join(producers[i], NULL); // Wait for all 30 submissions.
    for (int i = 0; i < 2; i++)
        pthread_join(printers[i], NULL);  // Wait for printers to exit.

    sem_destroy(&empty);      // Cleanup after all threads have exited.
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    printf("\nAll 30 jobs have been served.\n");
    return 0;
}
```

**Important limitations of the original:** The `sem_post(&full)` used to wake the other printer is a **shutdown signal, not a real job**; this is a special-case design, not the normal `full` rule. `served` counts jobs removed from the queue, not necessarily completed printing at that instant. Also, `rand()` is not guaranteed thread-safe on all systems. The code is reproduced to match your lab example; for more general production-quality designs, use explicit shutdown messages/flags and thread-safe randomness.

### Practice: identify the line

1. **Where is queue capacity enforced?** Producer's `sem_wait(&empty)`; `empty` starts at 8.
2. **Where is empty-queue removal prevented?** Printer's `sem_wait(&full)`; `full` starts at 0.
3. **Where is shared queue corruption prevented?** Both functions lock the **same** `mutex` around queue modifications.
4. **Where is priority enforced?** The printer scans for the first `queue[i].priority`.
5. **Why `queue[count++] = j`?** Insert at current end, then increment count.
6. **Why shift the array?** Remove a middle priority job without leaving a gap.
7. **Why create and join threads in `main()`?** Start concurrent workers and wait for all to terminate before cleanup.

---

## 6. Full practice question — SHORT Dining Philosophers

**Question (simplified practice version):** Five philosophers sit around a circular table with five chopsticks. Each philosopher needs their two adjacent chopsticks to eat, and each must eat five times. Write a C program using POSIX threads and semaphores to avoid simultaneous use of a chopstick and prevent deadlock. Print thinking, picking, eating and releasing actions, and terminate after all meals.

### Where everything goes

- **Global:** `N`, `MEALS`, `sem_t chopstick[N]`.
- **`philosopher()` thread:** determine two adjacent chopsticks, acquire in ascending-number order, eat, release, repeat five times.
- **`main()`:** initialize all chopstick semaphores to 1, create N threads, join all, destroy semaphores.

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5
#define MEALS 5

sem_t chopstick[N]; // GLOBAL: one semaphore per physical chopstick.

void *philosopher(void *arg) {
    int id = *(int *)arg;    // Philosopher IDs: 0,1,2,3,4.
    int left = id;           // Left chopstick index.
    int right = (id + 1) % N;// Right index wraps from 4 back to 0.

    // Consistent order breaks circular waiting => prevents deadlock.
    int first = left < right ? left : right;
    int second = left < right ? right : left;

    for (int i = 0; i < MEALS; i++) { // EACH philosopher eats 5 meals.
        printf("Philosopher %d is thinking\n", id + 1);
        usleep(100000);              // Think for 100 ms.

        sem_wait(&chopstick[first]);  // NEED first chopstick; wait if held.
        sem_wait(&chopstick[second]); // NEED second chopstick; wait if held.

        printf("Philosopher %d picked chopsticks\n", id + 1);
        printf("Philosopher %d is eating\n", id + 1);
        usleep(100000);              // Simulate eating for 100 ms.

        sem_post(&chopstick[second]); // RELEASE second chopstick.
        sem_post(&chopstick[first]);  // RELEASE first chopstick.
        printf("Philosopher %d released chopsticks\n", id + 1);
    }
    return NULL;                     // Thread ends after 5 meals.
}

int main() {
    pthread_t threads[N]; // MAIN: thread handles.
    int ids[N];           // MAIN: separate stable philosopher IDs.

    for (int i = 0; i < N; i++)
        sem_init(&chopstick[i], 0, 1); // Every chopstick starts available.

    for (int i = 0; i < N; i++) {
        ids[i] = i;       // Store ID before thread creation.
        pthread_create(&threads[i], NULL, philosopher, &ids[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL); // Wait for all philosophers.

    for (int i = 0; i < N; i++)
        sem_destroy(&chopstick[i]);     // Clean up after joins.

    printf("All philosophers finished eating.\n");
    return 0;
}
```

**Total meals:** `N × MEALS = 5 × 5 = 25`. If the question says 7 philosophers, change `N` to 7 (total 35 meals). The smaller-index-first rule prevents deadlock, but **does not prove starvation freedom**. If the question requires strict fairness, waiting-time statistics, or sauce, the short program must be extended.

### If the question adds a shared sauce pot (at most 2 users)

Add globally:

```c
sem_t sauce;
```

Initialize in `main()` **before creating threads**:

```c
sem_init(&sauce, 0, 2); // At most 2 concurrent sauce users.
```

Inside `philosopher()` **after acquiring BOTH chopsticks, before eating**:

```c
sem_wait(&sauce);       // Acquire sauce permit.
printf("Philosopher %d using sauce\n", id + 1);
usleep(50000);          // Simulate sauce use.
sem_post(&sauce);       // Release sauce permit.
```

Destroy in `main()` **after all joins**:

```c
sem_destroy(&sauce);
```

### Practice: identify the line

1. **Why `right = (id + 1) % N`?** Wrap around the circular table.
2. **Why initialize each chopstick to 1?** Only one philosopher can hold a chopstick at once.
3. **Why smaller index first?** Prevent circular wait/deadlock.
4. **Why two `sem_wait`s?** A philosopher must hold both chopsticks before eating.
5. **Why two `sem_post`s?** Return both chopsticks after eating.
6. **Why loop `MEALS` times?** Every philosopher independently completes 5 meals.
7. **Why `pthread_join` before `sem_destroy`?** No thread may still be using a semaphore when it is destroyed.

---

## 7. Final exam decision guide

| Wording in question | Tool | Exact pattern to remember |
|---|---|---|
| Multiple workers concurrently | Pthreads | `pthread_create` / `pthread_join` |
| Shared balance / queue / counter | Mutex | `lock` → modify shared data → `unlock` |
| Limited resource, e.g. 2 printers | Counting semaphore | `sem_init(..., 2)` → `wait` → use → `post` |
| Bounded producer queue | Two semaphores + mutex | Producer: `wait(empty)` → lock/add/unlock → `post(full)` |
| Consumer of bounded queue | Two semaphores + mutex | Consumer: `wait(full)` → lock/remove/unlock → `post(empty)` |
| Philosopher chopsticks | Semaphore per chopstick | `wait(first)` → `wait(second)` → eat → `post(second)` → `post(first)` |
| Two shared sauce permits | Semaphore | `sem_init(&sauce,0,2)` |
| Several locks may deadlock | Consistent ordering | Acquire smaller-numbered lock first |

### Last-minute memory card

```c
// THREADS
pthread_t t[N];
int ids[N];
void *worker(void *arg) { int id = *(int *)arg; return NULL; }
pthread_create(&t[i], NULL, worker, &ids[i]);
pthread_join(t[i], NULL);

// MUTEX
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_lock(&lock);
// protected shared update
pthread_mutex_unlock(&lock);
pthread_mutex_destroy(&lock);

// SEMAPHORE
sem_t s;
sem_init(&s, 0, 2);
sem_wait(&s); // TAKE/WAIT for a permit
// use resource
sem_post(&s); // RETURN/ANNOUNCE a permit
sem_destroy(&s);

// PRODUCER-CONSUMER
// Producer: wait(empty) -> mutex lock -> ADD -> mutex unlock -> post(full)
// Consumer: wait(full)  -> mutex lock -> REMOVE -> mutex unlock -> post(empty)
```

**Study order:** Syntax → pthread worker example → mutex deposit example → priority Producer–Consumer → basic Dining Philosophers. For any exact exam question, check additional requirements before using a template unchanged.
