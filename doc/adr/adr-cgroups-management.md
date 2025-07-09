# Design Decisions - Cgroup management

## Problem

To use cgroups in crinit it is necessary to create and configure them and to assign the tasks to the correct cgroup. For cgroup management there is basically the virtual cgroup file system available, which can directly be used with basic file manipulation tools. As alternative a third party library like libcgroups could be used and especially its tools. This document compares current available options.


According to the cgroup tutorial found at https://labs.iximiuz.com/tutorials/controlling-process-resources-with-cgroups there is no difference for the parameter part of the calls. For example, to set a cgroup which limits the CPU usage to 50% of the available CPU time, the command to set this via cgroupfs would look like this:
```echo "50000 100000" > /sys/fs/cgroup/hog_pen/cpu.max```
If using libcgroup tools the same can be achieved with this command:
```cgset -r cpu.max="50000 100000" hog_pen```
The relevant parts ("50000 100000" and "hog_pen") are the same.

If you write invalid data to the virtual file system you receive an error. At least that behaviour was observed when writing to the cpu.max virtual file without allowing the sub-cgroup to change that parameter in the parent cgroup (in this case the root cgroup).

There is however a daemon supplied by libcgroup that could put processes into the configured cgroup automatically, depending on the user and group IDs. It is uncertain if this feature is relevant for the given use case.
If the daemon is not used and only "cgcreate" and "cgexec" would be used to create cgroups and start processes via cgroupexec the same set of parameters have to be stored in the crinit configuration as for the manual approach.

In any case the virtual cgroup file system has to be mounted.


## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* A task has to be started in the correct cgroup so it does not run unrestricted for any amount of time
* The selected approach shall have minimal as possible effect on the startup behavior.



## Assumptions

The following assumptions have been made in the decision process:
* The cgroup filesystem is available when crinit starts the tasks
* Only cgroupV2 is relevant



## Considered Alternatives

### 1) Implement cgroup support manually

Change the crinit parser to support parsing cgroup configurations (e.g. in series file
and / or task files) and interact with the cgroup filesystem directly.
Use functions like "write()" to write directly to the relevant files in the
virtual file system. Use the existing crinit-launcher to start a new process in
the correct target cgroup. This should be done by moving the crinit-launcher instance to the cgroup by writing its PID to cgroups.procs before starting the target process.

*pros*
* No further dependencies, basic syscalls like open, and write are enough
* Most control on when and how the process started, the process can be run straight in the appropriate cgroup (using crinit-launch)
* Homogenous handling of different configuration options. Crinit supports starting
tasks as a different user, too. This would be a similar use-case (in the meaning of process
restrictions) and could be implemented there quite naturally.
* Creation of a cgroup is a mere directory creation
* Assigning a process to a cgroup is just a write to a virtual file

*cons*
* None compared to the other options. Usage of libcgroup tools would require the same amount of configuration data to be stored, error detection and handling would be similar if not better.

### 2) Use libcgroup tools

There are tools in libcgroup to manage cgroups. This could be seamlessly
integrated in the existing COMMAND parser.
The libcgroup library can be found here: https://github.com/libcgroup/libcgroup
According to the github page its license is LGPL-2.1.
Use cgcreate and cgexec.

*pros*
* No changes in crinit itself, no risk of breaking something.

*cons*
* External dependency
* Possible license problem?
* Need of another intermediate binary to start a binary
* Usage of cgexec would be inconsistent in comparision with other supported process restriction mechanisms (e.g. user / group support)

### 3) Use libcgroup daemon

Use the in libcgroup included daemon to move processes to cgroups

*pros*
* If used as only means to configure cgroups it would require the least change in crinit

*cons*
* It seems that the usage of the daemon would require that each process is started in a own user context. While this approach might work for daemons it seems quite awkward for other purposes.

### 4) Use clone3() and alike

Use syscalls like clone3() and setns() etc. to manage processes with cgroups. Those syscalls can have a target cgroup as a parameter. With this parameter clone3() will start the new process directly in the new cgroup, for example.

*pros*
* none

*cons*
* crinit use posix_spawn to create tasks
* This option does not solve the problem to create cgroups

## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
