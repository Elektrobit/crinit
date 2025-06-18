# Design Decisions - How to handle an invalid cgroup configuration

## Problem

What should we do if starting a process in a given cgroup fails or a cgroup cannot be
created?

## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* <first>
* <second>



## Assumptions

The following assumptions have been made in the decision process:
* <first>
* <second>



## Considered Alternatives

### 1) Try to start the task without cgroup

If the start in a cgroup fails, try to start the process again without giving a cgroup.
Write an error message to the log.

*pros*
* <first> At least all necessary processes are started. In some use-cases that may be the lesser evil.
** But: There is a considerable risk of having a process that locks the computer up.

*cons*
* <first> Just an error message in the log is easy to miss. Crash early and hard.

### 2) Don't start the task / stop the boot (if cgroup creation fails)

If the start of a task inside a cgroup fails, set state to "error", log it and do not attempt to start it without cgroup. But try to start other tasks.
If the creation of a cgroup fails, stop the boot process.

*pros*
* <first> Fail early, fail hard. Best possible solution to notice the problem early.

*cons*
* <first> The system has a considerable chance to become unusable.

### 3) Don't start the task / continue to boot (if cgroup creation fails)

If the start of a task inside a cgroup fails, set state to "error", log it and do not attempt to start it without cgroup. But try to start other tasks.
If the creation of a cgroup fails, continue to boot. This will inevitable lead to failing tasks that want to use the faulty cgroup.

*pros*
* <first> There is a chance to get a somewhat usable system at least without ignoring the errors alltogether.

*cons*
* <first> Considering that this will most likely happen during developement we would "waste" time on a system boot try that already is doomed.

## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
