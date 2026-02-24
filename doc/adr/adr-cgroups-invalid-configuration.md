# Design Decisions - How to handle an invalid cgroup configuration

## Problem

It shall be possible to run crinit without cgroup configuration. But if there is one, what is the proper way of handling an error that occures while starting a task with a cgroup configuration? (For example an invalid cgroup configuration.)

In principle it is a decision between a "best-effort-approach" and a strict approach where cgroups are considered vital for the security and stability of the whole system.


## Considered Alternatives

### 1) Try to start the task without cgroup

If the start in a cgroup fails, try to start the process again without giving a cgroup.
Write an error message to the log.

*pros*
* At least all necessary processes are started. In some use-cases that may be the lesser evil.
** But: There is a considerable risk of having a process that locks the computer up.

*cons*
* Just an error message in the log is easy to miss. Crash early and hard.

### 2) Don't start the task / stop the boot (if cgroup creation fails)

If the start of a task inside a cgroup fails, set state to "error", log it and do not attempt to start it without cgroup. But try to start other tasks.
If the creation of a cgroup fails, stop the boot process.

*pros*
* Fail early, fail hard. Best possible solution to notice the problem early.
* Simple, no need to think of error handling

*cons*
* A faulty cgroup configuration will stop the system from booting

### 3) Don't start the task / continue to boot (if cgroup creation fails)

If the start of a task inside a cgroup fails, set state to "error", log it and do not attempt to start it without cgroup. But try to start other tasks.
If the creation of a cgroup fails, continue to boot. This will inevitable lead to failing tasks that want to use the faulty cgroup.

*pros*
* There is a chance to get a somewhat usable system at least without ignoring the errors altogether.
* Current behavior on failing task start

*cons*
* Considering that this will most likely happen during development we would "waste" time on a system boot try that already is doomed.

## Decision

A mix between 2 and 3 was taken. If creation of the crinit root cgroup or a global cgroup fails, the boot process is stopped. If a cgroup configured in a task file fails to be created, the task is set to failed but other tasks are processed as normally.

### Rationale

### Open Points

None.
    
