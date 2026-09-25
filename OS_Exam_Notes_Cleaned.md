# Operating Systems — Exam Notes
## Cleaned & Structured Version of My Handwritten Notes

> **Scope:** This document reorganizes the handwritten notes into a cleaner, easier-to-revise format.  
> I have **not introduced new topics**. I removed material that is outside the exam portion listed in the notes and clarified wording/formulas where the handwritten version was difficult to read.

---

# 1. Processes

## Process

A **process** is a program in execution.

A process has its own execution state and resources.

### Important process states

```text
        admitted
           ↓
         READY
           ↓
        RUNNING
        ↙     ↘
    waiting   terminated
      ↓
   WAITING
      ↓
    READY
```

- **Ready:** waiting for CPU.
- **Running:** currently executing on CPU.
- **Waiting/Blocked:** waiting for some event, such as I/O.
- **Terminated:** execution has finished.

## PCB — Process Control Block

The OS maintains information about each process in its **PCB**.

Important information includes:

- Process identification: **PID, PPID**
- Process state
- CPU/register information
- Scheduling-related information
- Other information required to manage the process

---

# 2. `fork()`, `exec()`, `wait()`

## `fork()`

`fork()` creates a new process.

After `fork()`:

```text
           fork()
          /      \
     Parent       Child
```

The parent and child continue execution from the point after `fork()`.

### Return value

```c
pid = fork();
```

- `pid > 0` → executing process is the **parent**; value is the child's PID.
- `pid == 0` → executing process is the **child**.
- `pid < 0` → `fork()` failed.

### Important idea

The program does **not** split before `fork()`.

The process duplication happens **at `fork()`**, and from that point both processes continue.

---

## `exec()`

`exec()` is used to **replace the current process program/image with another program**.

Important:

> `exec()` does **not** create a new process.

The PID remains associated with the same process, but the program being executed changes.

Example idea:

```c
fork();

if (pid == 0) {
    execl(...);
}
```

The child can replace its program using `exec()`.

### `exec` family

- `execl()`
- `execlp()`
- `execv()`
- `execvp()`

The important distinction is:

- `l` → arguments supplied as a **list**
- `v` → arguments supplied as a **vector/array**
- `p` → searches for the executable using `PATH`

---

## `wait()`

`wait()` allows a parent to wait for one of its children to terminate.

Basic idea:

```text
Parent
  |
 wait()
  |
  ↓
waits for child
  |
Child terminates
  |
  ↓
Parent continues
```

`wait()` is also important for preventing a terminated child from remaining as a zombie.

---

# 3. Zombie and Orphan Processes

## Zombie

A child has:

1. finished execution, but
2. the parent has not yet collected its termination status.

So the child remains as a **zombie entry** in the process table.

```text
Child terminates
      ↓
Zombie
      ↓
Parent calls wait()
      ↓
OS collects termination information
```

### Key point

**Zombie = child is already terminated, but its parent has not collected its status.**

---

## Orphan

An orphan occurs when:

```text
Child is still running
        ↓
Parent terminates first
        ↓
Child becomes orphan
```

The OS adopts/reassigns the orphan to a special system process.

### Easy difference

| Zombie | Orphan |
|---|---|
| Child has already terminated | Child is still running |
| Parent has not collected status | Parent terminated first |
| `wait()` can collect the child's status | Child gets adopted |

---

# 4. Scheduling

## Why scheduling?

When multiple processes are ready, the OS must decide:

> **Which process gets the CPU next?**

The **CPU scheduler / short-term scheduler** makes this decision.

---

# 5. Scheduling Queues

## Job Queue

Contains processes entering the system / waiting to be admitted.

## Ready Queue

Contains processes that are:

> **Ready to run and waiting for CPU.**

## Device / I/O Queue

Contains processes waiting for an I/O device/event.

## Suspended / Swapped Queue

Contains processes that have been suspended/swapped out and are waiting to return.

### Basic idea

```text
          Job Queue
              ↓
         Ready Queue
              ↓
             CPU
              ↓
        I/O / Waiting
              ↓
         Ready Queue
```

---

# 6. Types of Schedulers

## Long-Term Scheduler

- Decides which processes are admitted into the system/ready queue.
- Controls the degree of multiprogramming.

## Short-Term Scheduler

- Selects a process from the **ready queue**.
- Gives it the CPU.
- Runs very frequently.

## Medium-Term Scheduler

- Associated with **swapping/suspending processes**.
- Can remove processes from memory and later bring them back.

### Memory trick

```text
Long-term    → admit
Short-term   → CPU
Medium-term  → swap/suspend
```

---

# 7. Context Switching

A **context switch** occurs when the CPU changes from one process to another.

Basic sequence:

```text
Interrupt / scheduling event
        ↓
Save current process state
        ↓
Select another process
        ↓
Restore its state
        ↓
Run the new process
```

### Important

Context switching is **overhead**.

During the switch, the CPU is spending time saving/restoring state rather than doing useful application work.

---

# 8. Preemptive vs Non-Preemptive Scheduling

## Preemptive

The OS can take the CPU away from a running process.

Example:

```text
P1 running
   ↓
time quantum expires
   ↓
P1 removed from CPU
   ↓
another process runs
```

## Non-Preemptive

Once a process gets the CPU, it keeps it until it:

- terminates, or
- voluntarily gives up the CPU / blocks.

### Memory trick

> **Preemptive = OS can take CPU away.**

> **Non-preemptive = process keeps CPU until it gives it up.**

---

# 9. Scheduling Criteria and Formulas

## Arrival Time — AT

Time at which the process enters the system/ready queue.

## Burst Time — BT

Amount of CPU time required by the process.

## Completion Time — CT

Time at which the process finishes execution.

## Turnaround Time — TAT

Total time spent by the process in the system.

\[
TAT = CT - AT
\]

## Waiting Time — WT

Time spent waiting in the ready queue.

\[
WT = TAT - BT
\]

## Response Time — RT

Time from arrival until the process gets the CPU **for the first time**.

\[
RT = \text{First CPU start time} - AT
\]

### Very important distinction

```text
Response Time
    ↓
How long until I get CPU for the FIRST time?

Waiting Time
    ↓
How long did I spend waiting in the ready queue overall?

Turnaround Time
    ↓
How long from arrival until completion?
```

---

## CPU Utilization

Measures how much of the available CPU time is spent doing useful CPU work.

The notes express it as:

\[
CPU\ Utilization =
\frac{CPU\ busy\ time}{Total\ time}\times100
\]

---

## Throughput

Number of processes completed per unit time.

\[
Throughput =
\frac{\text{Number of completed processes}}
{\text{Total time}}
\]

---

# 10. FCFS — First Come, First Served

Processes are executed in **arrival order**.

Example:

```text
P1 → P2 → P3
```

if that is their arrival order.

### Important

- FCFS is **non-preemptive**.
- A long process at the front can make shorter processes wait.

### Convoy Effect

A long process can hold the CPU while many short processes wait behind it.

```text
Long process
     ↓
Short process
     ↓
Short process
     ↓
Short process
```

This creates a **convoy effect**.

### Exam trap

> FCFS is based on **arrival order**, not shortest burst time.

---

# 11. SJF — Shortest Job First

Choose the process with the **shortest burst time**.

### Non-preemptive SJF

Once selected, the process continues until completion/blocking.

### Main idea

```text
Choose smallest BT
        ↓
Run it
        ↓
Choose next shortest
```

SJF can give low average waiting time, but a long process may have to wait.

---

# 12. SRTF — Shortest Remaining Time First

SRTF is the **preemptive version of SJF**.

Instead of comparing total burst time, compare:

> **Remaining CPU time.**

### Important twist

If a new process arrives whose remaining time is shorter than the currently running process:

```text
Current process
      ↓
preempt it
      ↓
new shorter process runs
```

### Memory

```text
SJF  → shortest job
SRTF → shortest remaining job
```

---

# 13. Priority Scheduling

Each process is assigned a priority.

The scheduler chooses according to priority.

It can be:

- Preemptive
- Non-preemptive

### Starvation

A low-priority process may wait for a very long time if higher-priority processes keep getting selected.

### Aging

**Aging** gradually increases the priority of a waiting process.

Purpose:

> Prevent starvation.

```text
Waiting longer
     ↓
Priority increases
     ↓
Eventually gets CPU
```

---

# 14. Round Robin

Round Robin is designed for time-sharing systems.

Each process receives a fixed **time quantum**.

Example:

```text
Ready Queue → circular queue
```

If the process finishes before its quantum:

```text
process terminates
```

If its quantum expires before completion:

```text
context switch
      ↓
process goes to back of ready queue
```

### Important

Round Robin is **preemptive**.

### If time quantum is too large

It starts behaving more like FCFS.

### If time quantum is very small

There can be many context switches and therefore more overhead.

---

# 15. HRRN — Highest Response Ratio Next

HRRN selects the process with the highest:

\[
Response\ Ratio =
\frac{Waiting\ Time + Burst\ Time}{Burst\ Time}
\]

or:

\[
RR = 1 + \frac{WT}{BT}
\]

### Important

HRRN uses:

- Waiting time
- Burst time

As a process waits longer, its response ratio increases.

### Purpose

It helps reduce starvation compared with simply always selecting short jobs.

---

# 16. Multilevel Queue Scheduling

The ready queue is divided into multiple separate queues.

Example:

```text
Queue 0 → System processes
Queue 1 → Interactive processes
Queue 2 → Batch processes
```

Each queue can use its own scheduling algorithm.

For example:

```text
Q0 → Round Robin
Q1 → FCFS
```

### Important property

Processes are **permanently assigned** to a queue.

They do not normally move between queues.

### Between-queue scheduling

The OS also needs a method to decide which queue gets the CPU.

Example:

```text
Higher-priority queue
        ↓
Lower-priority queue
```

### Problem

A lower-priority queue can suffer from starvation if higher-priority queues are continuously busy.

---

# 17. Multilevel Feedback Queue — MLFQ

MLFQ is similar to Multilevel Queue, but processes **can move between queues**.

The movement is based on process behavior/feedback.

General idea:

```text
High priority
    ↓
short CPU burst / interactive
    ↓
stay high

Long CPU usage
    ↓
move to lower queue
```

The purpose is to give good response to interactive/short jobs while still allowing longer jobs to run.

### Main difference

| Multilevel Queue | MLFQ |
|---|---|
| Fixed queue assignment | Processes can move |
| No feedback-based movement | Uses feedback |
| Separate queues | Separate queues |

---

# 18. IPC — Interprocess Communication

Processes need IPC when they need to communicate.

The notes cover:

1. **Shared Memory**
2. **Pipes**
3. **Shared Memory in Linux**

---

# 19. Shared Memory

A shared memory region is created so that multiple processes can access the same memory.

### Advantages

- Very fast.
- Processes can read/write shared data directly.

### Important limitation

Shared memory is **error-prone** if synchronization is not used.

Why?

Because multiple processes can access and modify the same data.

That can cause a **race condition**.

---

# 20. Shared Memory in Linux — System V

Basic API flow:

```text
shmget()
   ↓
shmat()
   ↓
use shared memory
   ↓
shmdt()
   ↓
shmctl(... IPC_RMID ...)
```

## `shmget()`

Creates/gets a shared memory segment.

```c
shmget(key, size, flags);
```

Returns a **shmid**.

## `shmat()`

Attaches the shared memory segment to the calling process's address space.

```c
shmat(shmid, addr, flags);
```

## `shmdt()`

Detaches the shared memory segment from the process.

```c
shmdt(...);
```

## `shmctl()`

Used for control operations such as removing the shared memory segment.

Typical removal:

```c
shmctl(shmid, IPC_RMID, NULL);
```

### Remember the flow

> **Get → Attach → Use → Detach → Remove**

---

# 21. Pipes

A pipe provides communication between associated processes.

According to the notes:

- Pipe is **unidirectional**.
- `fd[0]` → read end.
- `fd[1]` → write end.
- For two-way communication, use **two pipes**.

Typical creation:

```c
int fd[2];
pipe(fd);
```

After `fork()`, parent and child inherit the pipe file descriptors.

### Important

Close unnecessary pipe ends.

For example, if a process only reads:

```c
close(fd[1]);
```

If it only writes:

```c
close(fd[0]);
```

---

# 22. Important Pipe Behavior

## Reader waiting

If the pipe is empty but a writer still has the write end open:

```text
read()
   ↓
blocks/waits
```

The reader waits for data.

## EOF

When all write ends are closed and no more data is available:

```text
read()
   ↓
returns 0
```

This represents **EOF**.

### Important distinction

EOF is **not** the same as receiving a `'\0'` byte.

---

# 23. Synchronization

## Race Condition

A race condition occurs when multiple processes access/manipulate shared data and the result depends on the order/timing of their operations.

Example:

```c
counter++;
counter--;
```

Even if the expected final value seems obvious, the operations are made of multiple machine-level steps.

A context switch can happen between those steps.

Therefore the final result can become incorrect.

### Core idea

> **Same shared data + concurrent access + result depends on timing/order = race condition.**

---

# 24. Critical Section

A **critical section** is the part of a program where shared data/resource is accessed or modified.

Only the required code should be protected.

Basic structure:

```text
Entry
  ↓
Critical Section
  ↓
Exit
  ↓
Remainder Section
```

---

# 25. Three Critical-Section Requirements

## 1. Mutual Exclusion

At most **one process** can be inside the critical section at a time.

```text
P1 → CS
P2 → WAIT
```

## 2. Progress

If no process is currently in the critical section, a process that wants to enter should not be delayed unnecessarily.

## 3. Bounded Waiting / No Starvation

There must be a limit on how many times other processes can enter while a particular process is waiting.

### Memory

```text
Mutual Exclusion → only one
Progress → don't unnecessarily delay
Bounded Waiting → don't wait forever
```

---

# 26. Locks and Unlocks

```c
lock(L);

/* critical section */

unlock(L);
```

### `lock(L)`

Acquire exclusive access to lock `L`.

### `unlock(L)`

Release the lock so another process can access the critical section.

### Important

The lock-acquisition operation itself needs to be implemented safely/atomically.

---

# 27. Disabling Interrupts

One simple way to prevent a context switch on a **single CPU** is to disable interrupts during the critical section.

```text
disable interrupts
        ↓
critical section
        ↓
enable interrupts
```

### Limitations

- Requires privilege.
- User processes generally cannot disable interrupts.
- Not suitable for multicore systems.

---

# 28. Software Solution — Attempt 1: Turn Variable

A shared `turn` variable determines whose turn it is.

Example idea:

```c
while (turn == 2);

/* critical section */

turn = 2;
```

### Achieves

- Mutual exclusion ✅

### Problems

- Busy waiting
- Strict alternation
- A process may have to wait even when the other process does not want to enter

Therefore:

- Progress ❌
- Bounded waiting ❌ according to the comparison in the notes

### Memory

> **Attempt 1 = strict alternation.**

---

# 29. Software Solution — Attempt 2: Two Flags

Each process has its own flag.

Example:

```c
while (p2_inside == true);

p1_inside = true;

/* critical section */

p1_inside = false;
```

P2 uses the symmetric version.

### Main problem

The check and setting of the flag are separate operations.

Possible execution:

```text
P1 checks P2 flag → false
        ↓
context switch
        ↓
P2 checks P1 flag → false
        ↓
P2 sets its flag → true
        ↓
context switch
        ↓
P1 sets its flag → true
```

Now both can enter the critical section.

Therefore:

> **Attempt 2 does NOT guarantee mutual exclusion.**

The key problem is:

> **The flag is announced only after breaking out of the `while` loop.**

---

# 30. Software Solution — Attempt 3

Here the process announces its intention **before** checking the other process.

Example:

```c
p1_wants_to_enter = true;

while (p2_wants_to_enter == true);

/* critical section */

p1_wants_to_enter = false;
```

### Advantage

Mutual exclusion is achieved.

### Problem

Both processes can announce that they want to enter:

```text
P1 wants = true
P2 wants = true
```

Then:

```text
P1 waits for P2 = false
P2 waits for P1 = false
```

Both wait forever.

This is a **deadlock**.

Therefore:

- Mutual exclusion ✅
- Progress ❌
- Bounded waiting ❌

### Memory

> **Attempt 3 fixes Attempt 2's race, but creates deadlock.**

---

# 31. Peterson's Solution

Peterson's solution is for **two processes**.

It uses:

- Two intention flags
- A `favored` / `turn` variable

Example idea:

```c
p1_wants_to_enter = true;
favored = 2;

while (p2_wants_to_enter && favored == 2);

/* critical section */

p1_wants_to_enter = false;
```

P2 uses the symmetric version.

## Why `favored`?

It breaks the tie when both processes want to enter at the same time.

Important rule:

> **The process that sets `favored` last loses the tie.**

### Peterson solves

- Mutual exclusion ✅
- Progress ✅
- Bounded waiting ✅

### Important limitation

Classical Peterson's solution is for **two processes**.

---

# 32. Bakery Algorithm

Bakery is used for synchronization among **N processes**.

Idea:

> Processes take numbered tickets, like customers in a bakery.

The process with the smallest non-zero ticket gets priority.

---

# 33. Simplified Bakery

`num[i]` stores the ticket number of process `i`.

Basic idea:

```c
num[i] = MAX(num[0], ..., num[N-1]) + 1;

for (p = 0; p < N; p++) {
    while (num[p] != 0 && num[p] < num[i]);
}
```

After the critical section:

```c
num[i] = 0;
```

### Entry rule

A process can enter when every other process either:

- has ticket `0`, or
- has a ticket larger than its own.

Therefore:

> **Smallest non-zero ticket gets priority.**

---

# 34. Why the Simplified Bakery Doorway Must Be Atomic

The ticket-selection step is called the **doorway**.

If two processes choose a ticket simultaneously without the doorway being atomic, both can see the same maximum and choose the same ticket.

Example:

```text
Current maximum = 1

P4 sees MAX = 1
P5 sees MAX = 1

P4 chooses 2
P5 chooses 2
```

The simplified version therefore requires the doorway to be atomic.

---

# 35. Original Bakery — `choosing[]`

The original Bakery algorithm introduces:

```c
choosing[i] = true;

num[i] = MAX(num[]) + 1;

choosing[i] = false;
```

Then another process checks:

```c
while (choosing[p]);
```

### Purpose of `choosing[]`

`choosing[]` tells other processes:

> **"I am currently choosing my ticket. Do not compare my ticket yet."**

It is different from tie-breaking.

### Three things to remember

```text
choosing[i]
    ↓
Am I still choosing my ticket?

num[i]
    ↓
What ticket did I get?

(num[i], i)
    ↓
Who wins if ticket numbers are equal?
```

If two processes have the same ticket:

```text
(ticket, PID)
```

is compared lexicographically.

Example:

```text
(2,4) < (2,5)
```

so the process with the smaller ID gets priority when the ticket numbers are equal.

---

# 36. Bakery Summary

| Solution | Processes | Mutual Exclusion | Progress | Bounded Waiting | Main issue |
|---|---:|---:|---:|---:|---|
| Attempt 1 — Turn | 2 | ✅ | ❌ | ❌ | Strict alternation |
| Attempt 2 — Two Flags | 2 | ❌ | — | ❌ | Race condition |
| Attempt 3 — Want Flags | 2 | ✅ | ❌ | ❌ | Deadlock |
| Peterson | 2 | ✅ | ✅ | ✅ | Busy waiting |
| Bakery | N | ✅ | ✅ | ✅ | Busy waiting |

---

# 37. Busy Waiting / Spinning

### Busy waiting

A process continuously checks a condition in a loop instead of sleeping.

Example:

```c
while (lock == 1);
```

The CPU keeps executing the loop.

### Active waiting

The process remains active on the CPU instead of sleeping/yielding.

### Advantage

- Very short waits can be handled quickly.
- Avoids sleep/wakeup or context-switch overhead.

### Disadvantage

- CPU time is wasted while waiting.

### Use

Good for:

- Short critical sections
- Situations where the lock is expected to be released quickly

Bad for:

- Long/unpredictable waits

### Memory

> **Short wait → spinning can be useful.**

> **Long wait → blocking/mutex is better.**

---

# 38. Hardware Support — Test-and-Set

The ordinary scheme:

```c
while (lock != 0);

lock = 1;
```

is not safe because:

```text
check lock
     ↓
context switch
     ↓
another process checks
     ↓
both can set lock
```

So we need the operation to be **atomic**.

---

## Test-and-Set

Conceptually:

```c
int test_and_set(int *L) {
    int prev = *L;
    *L = 1;
    return prev;
}
```

The **entire operation is atomic**.

### If lock = 0

```text
old value = 0
lock becomes 1
returns 0
```

The process gets the lock.

### If lock = 1

```text
old value = 1
lock remains 1
returns 1
```

The process keeps waiting.

Typical usage:

```c
while (test_and_set(&lock) == 1);

/* critical section */

lock = 0;
```

### Memory trick

> **Test-and-Set = return the old value + set the lock.**

---

# 39. Spinlock

A spinlock uses busy waiting to acquire a lock.

Basic idea:

```text
Process 1 gets lock
        ↓
enters CS

Process 2 tries
        ↓
lock unavailable
        ↓
keeps spinning

Process 1 releases
        ↓
Process 2 gets lock
```

### Important

Spinlock is useful when the critical section is **very short**.

It is inefficient when the waiting time is long.

---

# 40. Mutex

A mutex improves on busy waiting by allowing the waiting process to **sleep/block**.

Basic idea:

```text
Lock available?
      ↓
    YES → enter CS

    NO
      ↓
   sleep
      ↓
lock released
      ↓
   wake up
```

### Spinlock vs Mutex

| Spinlock | Mutex |
|---|---|
| Busy waits | Blocks/sleeps |
| Uses CPU while waiting | Does not continuously use CPU while waiting |
| Good for short waits | Better for longer waits |
| Avoids sleep/wakeup overhead | Has sleep/wakeup/context-switch overhead |

### Memory

> **Spinlock = keep checking.**

> **Mutex = sleep until available.**

---

# 41. Thundering Herd Problem

Suppose many processes are sleeping while waiting for the same lock/event.

When the event occurs:

```text
          wakeup()
             ↓
P1  P2  P3  P4  P5  P6
 \   \   \   \   \   \
        all wake up
             ↓
    many context switches
             ↓
      only one succeeds
             ↓
     others sleep again
```

This is the **Thundering Herd Problem**.

### Problems

- Many processes wake unnecessarily.
- Large number of context switches.
- CPU overhead.
- Can contribute to starvation.

---

# 42. Solution to Thundering Herd

Instead of waking everyone:

```text
Waiting processes
       ↓
     Queue
       ↓
Wake only the first waiting process
```

### Basic idea

- Put waiting processes into a queue before blocking.
- When the critical section is released, wake only the appropriate/first waiting process.

### Memory

```text
Spinlock
   ↓
busy waiting

Mutex
   ↓
sleep/wakeup

Many wake together
   ↓
Thundering Herd

Queue + wake one
   ↓
reduce unnecessary wakeups
```

---

# 43. Ultra-Quick Revision Sheet

## Process

```text
fork() → creates child
exec() → replaces current program
wait() → parent waits/collects child
```

## Zombie vs Orphan

```text
Zombie → child finished, parent hasn't collected status
Orphan → parent finished, child still running
```

## Scheduling

```text
Long-term  → admission
Short-term → CPU selection
Medium-term → suspension/swapping
```

## Context Switch

```text
save old state → select process → restore state
```

## Formulas

```text
TAT = CT - AT
WT  = TAT - BT
RT  = First CPU start - AT

Throughput = completed processes / total time
```

## Algorithms

```text
FCFS  → arrival order
SJF   → shortest BT
SRTF  → shortest remaining BT
RR    → time quantum
HRRN  → (WT + BT) / BT
MLQ   → fixed queues
MLFQ  → processes can move between queues
```

## Critical Section

```text
Mutual Exclusion
Progress
Bounded Waiting
```

## Software Solutions

```text
Attempt 1 → strict alternation
Attempt 2 → check then set → race
Attempt 3 → announce then check → deadlock
Peterson  → flags + favored/turn
Bakery    → tickets
```

## Hardware / Locks

```text
Test-and-Set → atomic old-value + set
Spinlock     → busy wait
Mutex        → sleep/block
Thundering Herd → wake many unnecessarily
```

## Bakery

```text
choosing[] → still choosing ticket?
num[]      → ticket number
(num, PID) → tie-breaking
```
