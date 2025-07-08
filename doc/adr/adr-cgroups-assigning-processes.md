# Design Decisions - Assigning processes to cgroups?

## Problem

There are some options on how to assign a process to a cgroup. This shall list the current options and select the most appropriate one.

## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* How to ensure the task is affected by the cgroup from the beginning and there
  is no gap in which the task could run with unlimited resources.
* The selected approach shall have minimal as possible effect on the startup behavior.



## Assumptions

The following assumptions have been made in the decision process:
* A process started by crinit get assigned only once to a cgroup, no live migration of processes to other cgroups


## Considered Alternatives

### 1) Manually interact with cgroups via cgroup filesystem

Crinit will search, open and write the pids by itself into the correct cgroup.procs file.

*pros*
* simple as writing the PID into cgroup.procs

*cons*
* Error handling could be more complex

### 2) Use the tools of libcgroup

There are tools in libcgroup to start processes. This could be seamlessly
integrated in the existing COMMAND parser.
The libcgroup library can be found here: https://github.com/libcgroup/libcgroup
According to the github page its license is LGPL-2.1.

*pros*
* No changes in crinit itself, no risk of breaking something.

*cons*
* This would be inconsistent in comparision with other supported process restriction mechanisms (e.g. user / group support).

### 3) Use clone3() and alike

Use syscalls like clone3() and setns() etc. to manage processes with cgroups. Those syscalls can have a target cgroup as a parameter. With this parameter clone3() will start the new process directly in the new cgroup, for example.

*pros*
* process started right away in the correct cgroup

*cons*
* crinit use posix_spawn to create tasks
* there is still the need to create the cgroups with other means

## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
