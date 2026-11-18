# Older xv6-x86 — Quick Revision Notes

## 1. Key files

| File | Purpose |
|---|---|
| `proc.h` | `struct proc` fields (`ctime`, `priority`, `tickets`) |
| `proc.c` | `ptable`, `allocproc()`, `fork()`, `scheduler()`, kernel helpers |
| `defs.h` | Declarations for kernel helpers implemented in `proc.c` |
| `sysproc.c` | `sys_*` system-call handlers |
| `syscall.h` | `SYS_*` numbers (choose unused numbers) |
| `syscall.c` | `extern int sys_*(void)` and dispatch table entries |
| `user.h` | User-visible syscall prototypes |
| `usys.S` | `SYSCALL(name)` wrappers |
| `trap.c` | Timer-triggered `yield()` for preemptive scheduling |
| `Makefile` | `_mytest\` in `UPROGS`, `CPUS=1` when testing |

## 2. Process table (`proc.c`)

```c
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;
```

```c
struct proc *p;
acquire(&ptable.lock);
for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
  if(p->state == RUNNABLE) {
    // inspect process
  }
}
release(&ptable.lock);
```

- `p->state`, `p->pid`, `p->priority`, etc. access a particular process.
- `ptable` is defined in `proc.c`; expose a kernel helper through `defs.h` if another kernel file needs to scan it.
- Do not hold `ptable.lock` around arbitrary blocking operations.

### `proccount()` example

```c
int proccount(void) {
  struct proc *p;
  int count = 0;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state != UNUSED) count++;
  release(&ptable.lock);
  return count;
}
```

Add `int proccount(void);` to `defs.h`; in `sysproc.c`, `int sys_proccount(void) { return proccount(); }`. Register the syscall in the usual files if users must call it. Note: counting non-UNUSED slots includes ZOMBIE; check question's definition of active.

## 3. Adding a syscall (older xv6-x86)

Example `settickets(int n)`:

1. `syscall.h`: `#define SYS_settickets 22` **only if 22 is unused**.
2. `syscall.c`: `extern int sys_settickets(void);` and `[SYS_settickets] sys_settickets,` in `syscalls[]`.
3. `sysproc.c`:

```c
int sys_settickets(void) {
  int n;
  if(argint(0, &n) < 0) return -1;
  return settickets(n);
}
```

4. `defs.h`: `int settickets(int);` (if helper is implemented in `proc.c`).
5. `user.h`: `int settickets(int);`.
6. `usys.S`: `SYSCALL(settickets)`.
7. `Makefile`: add `_mytest\` to `UPROGS`.

`argint(0, &n)` reads the first integer syscall argument. `myproc()` returns the current process pointer.

## 4. Adding process fields

In `proc.h`, inside `struct proc`:

```c
uint ctime;    // creation order for FCFS
int priority;  // smaller value = higher priority
int tickets;   // lottery weight
```

In `proc.c`, inside `allocproc()` after PID allocation:

```c
p->ctime = nextctime++;
p->priority = 10;
p->tickets = 10;
```

Declare `static uint nextctime = 0;` at file scope in `proc.c`. The increment occurs while `ptable.lock` is held by `allocproc()`. This is a creation **sequence**, not elapsed timer ticks. If exam demands timer ticks, use a safe tick-sampling design without unsafe lock nesting.

In `fork()` optionally inherit from parent:

```c
np->priority = curproc->priority;
np->tickets = curproc->tickets;
```

Only inherit if the problem specifies it. Child's `ctime` should remain its own creation order.

## 5. Scheduler skeleton (common to all three)

The selection block below changes for each policy. Keep `ptable.lock` held while scanning and switching, as in stock xv6.

```c
void scheduler(void) {
  struct proc *p, *best;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;) {
    sti();
    acquire(&ptable.lock);
    best = 0;

    // POLICY-SPECIFIC SELECTION GOES HERE

    if(best != 0) {
      p = best;
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&(c->scheduler), p->context);
      switchkvm();
      c->proc = 0;
    }
    release(&ptable.lock);
  }
}
```

### FCFS (non-preemptive)

Selection block:

```c
for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
  if(p->state != RUNNABLE) continue;
  if(best == 0 || p->ctime < best->ctime)
    best = p;
}
```

In `trap.c`, disable only timer-triggered `yield()` (not the timer interrupt itself):

```c
// if(myproc() && myproc()->state == RUNNING &&
//    tf->trapno == T_IRQ0+IRQ_TIMER)
//   yield();
```

FCFS means earliest creation order among RUNNABLE processes; when a process sleeps and wakes, this simple version retains original creation order. Disabling timer preemption does not prevent a process from sleeping, exiting, or voluntarily yielding.

### Preemptive priority scheduling

Selection block:

```c
for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
  if(p->state != RUNNABLE) continue;
  if(best == 0 || p->priority < best->priority)
    best = p;
}
```

Restore the timer yield in `trap.c`:

```c
if(myproc() && myproc()->state == RUNNING &&
   tf->trapno == T_IRQ0+IRQ_TIMER)
  yield();
```

Lower numeric priority = higher precedence **for this example**. A higher-priority process is selected at the next scheduling opportunity, not necessarily instantly. Equal priorities may be unfair with a first-match-only selector.

A safe setter helper in `proc.c`:

```c
int setpriority(int n) {
  if(n < 1 || n > 20) return -1;
  acquire(&ptable.lock);
  myproc()->priority = n;
  release(&ptable.lock);
  return 0;
}
```

Declare in `defs.h` and call from `sys_setpriority()` in `sysproc.c` using `argint(0, &n)`; register syscall normally.

### Lottery scheduling

- `tickets > 0` for all eligible RUNNABLE processes.
- Sum tickets, draw winner from `[0, total-1]`, find cumulative range.
- Keep timer yield enabled for repeated lotteries.

Simple educational pseudorandom generator at file scope in `proc.c` (not cryptographic):

```c
static uint seed = 12345;
static uint random(void) {
  seed = seed * 1664525 + 1013904223;
  return seed;
}
```

Use this **instead of** the policy-specific selection block:

```c
int total = 0;
uint winner, cumulative = 0;
for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  if(p->state == RUNNABLE && p->tickets > 0)
    total += p->tickets;

if(total > 0) {
  winner = random() % (uint)total;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->state != RUNNABLE || p->tickets <= 0) continue;
    cumulative += p->tickets;
    if(cumulative > winner) {
      best = p;
      break;
    }
  }
}
```

Example: P1=10 tickets, P2=30, P3=60; total=100. Winner 0–9 => P1, 10–39 => P2, 40–99 => P3. These are **probabilities**, not guaranteed completion order.

`settickets()` helper in `proc.c`:

```c
int settickets(int n) {
  if(n < 1 || n > 1000) return -1;
  acquire(&ptable.lock);
  myproc()->tickets = n;
  release(&ptable.lock);
  return 0;
}
```

For lab use, `CPUS=1` and bounded tickets simplify arithmetic and random-state safety.

## 6. `mytest.c` for priority scheduling

Requires `setpriority(int)` syscall. This demonstrates changing priorities but **does not guarantee** print/finish order because children may run before siblings are created.

```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  int priorities[3] = {10, 2, 5};
  for(int i = 0; i < 3; i++) {
    int pid = fork();
    if(pid < 0) { printf(1, "fork failed\n"); break; }
    if(pid == 0) {
      setpriority(priorities[i]);
      printf(1, "PID %d priority %d START\n", getpid(), priorities[i]);
      volatile int x = 0;
      for(int j = 0; j < 100000000; j++) x++;
      printf(1, "PID %d priority %d END\n", getpid(), priorities[i]);
      exit();
    }
  }
  for(int i = 0; i < 3; i++) wait();
  exit();
}
```

## 7. `mytest.c` for lottery scheduling

Requires `settickets(int)` syscall. Run repeatedly; higher ticket count usually gets more CPU share over many scheduling decisions, but completion order is not proof of proportional share.

```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  int tickets[3] = {10, 30, 60};
  for(int i = 0; i < 3; i++) {
    int pid = fork();
    if(pid < 0) { printf(1, "fork failed\n"); break; }
    if(pid == 0) {
      settickets(tickets[i]);
      printf(1, "PID %d tickets %d\n", getpid(), tickets[i]);
      volatile int x = 0;
      for(int j = 0; j < 100000000; j++) x++;
      printf(1, "PID %d finished\n", getpid());
      exit();
    }
  }
  for(int i = 0; i < 3; i++) wait();
  exit();
}
```

To **measure** fairness properly, add a per-process scheduling counter incremented each time it is chosen and a syscall to read counts, then compare over many rounds.

## 8. Run / test

In `Makefile`, under `UPROGS` add:

```makefile
_mytest\
```

Then from host terminal:

```bash
make qemu-nox CPUS=1
```

Inside xv6:

```text
$ mytest
```

Do not run FCFS and priority/lottery scheduler code simultaneously: choose **one** selection policy at a time. Restore `trap.c` timer `yield()` for preemptive priority and lottery.

## 9. Ultra-fast comparison

| Policy | Selection | `trap.c` timer yield | Main process field |
|---|---|---|---|
| FCFS | Smallest `ctime` | Disable | `uint ctime` |
| Preemptive priority | Smallest `priority` | Enable | `int priority` |
| Lottery | Random ticket interval | Enable | `int tickets` |

## 10. Ubuntu final revision reminder

- `sem_init(&sem, 0, value)` for threads; `sem_init(&sem, 1, value)` for processes **only when semaphore is in shared memory**.
- Producer–Consumer: producer `wait(empty)`, lock, add item, unlock, `post(full)`; consumer `wait(full)`, lock, remove item, unlock, `post(empty)`.
- Shared memory: `shmget` → `shmat` → use → `shmdt` → `shmctl(IPC_RMID)`.
- Review pthread create/join, mutex lock/unlock, process-shared mutex attributes, `fork()`, pipes, and semaphore initialization.

**Exam note:** Code assumes standard older xv6-x86 layout and a single CPU for demonstrations. Confirm unused syscall numbers and your lab's exact scheduler requirements.
