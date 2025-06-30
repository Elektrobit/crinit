# Design Decisions - Decide between manual cgroup support and libcgroup

## Problem

To implement cgroup support in crinit you could write to the virtual cgroup file system or you could use libcgroups and especially its tools. According to the cgroup tutorial found at https://labs.iximiuz.com/tutorials/controlling-process-resources-with-cgroups there is no difference for the parameter part of the calls. For example: to set a cgroup which limits the CPU usage to 50 % of the available CPU time the command to set this via cgroupfs would look like this:
```echo "50000 100000" > /sys/fs/cgroup/hog_pen/cpu.max```
If using libcgroup tools the same can be achieved with this command:
```cgset -r cpu.max="50000 100000" hog_pen```
The relevant parts ("50000 100000" and "hog_pen") are the same.

If you write invalid data to the virtual file system you receive an error. At least I observed that behaviour when writing to the cpu.max virtual file without allowing the sub-cgroup to change that parameter in the parent cgroup (in this case the root cgroup).

There is however a deamon supplied by libcgroup that could put processes into the configured cgroup automatically, depending on the user and group IDs. It is uncertain if this feature is relevant for the given use case.
If the daemon is not used and only "cgcreate" and "cgexec" would be used to create cgroups and start processes via cgroupexec the same set of parameters have to be stored in the crinit configuration as for the manual approach.

In any case the virtual cgroup file system has to be mounted.


## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* <first>
* <second>



## Assumptions

The following assumptions have been made in the decision process:
* <first>
* <second>



## Considered Alternatives

### 1) Implement cgroup support manually

Use functions like "write()" to write directly to the relevant files in the virtual file system. Use the existing crinit-launcher to start a new process in the correct target cgroup.

*pros*
* No further dependencies

*cons*
* None

### 2) Use libcgroup tools

Use cgcreate and cgexec.

*pros*
* None compared to the manual approach.

*cons*
* External dependency
* Possible license problem?

### 3) Use libcgroup daemon

Use the in libcgroup included daemon to move processes to cgroups

*pros*
* If used as only means to configure cgroups it would require the least change in crinit

*cons*
* It seems that the usage of the daemon would require that each process is started in a own user context. While this approach might work for daemons it seems quite arkward for other purposes.

## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
