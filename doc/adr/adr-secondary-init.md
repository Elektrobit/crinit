# ADR crinit as secondary init

[[_TOC_]]

## Problem


[crinit](https://github.com/Elektrobit/crinit) is an init system. Usually it acts in the same role as e.g. systemd or a SysV-style init. In this case crinit receives the pid 1.

But crinit can also be used as secondary init started by another init. In that situation crinit receives a pid greater than 1 (pid!=1).

If crinit is used as a secondary init (pid!=1), some default features of init need to be implemented in a different way. This ADR discusses the options to handle such default features.

The features are:

### zombie reaper
(from `man PR_SET_CHILD_SUBREAPER`)

> When a process becomes orphaned (i.e., its immediate
> parent terminates), then that process will be reparented to init (pid=1) or
> to the nearest still living ancestor subreaper, if defined.
> 
> Subsequently, calls to
> getppid in the orphaned process will now return the PID of the
> subreaper process, and when the orphan terminates, it is the
> subreaper process that will receive a SIGCHLD signal and will be
> able to wait on the process to discover its termination
> status.

If crinit is running as pid=1, handling of such processes must be fulfilled by crinit. If
crinit is running as secondary init (pid!=1), a decision needs to be taken how to
handle orphaned child processes.


### default minimum mounts

Mounting of the important filesystems such as /proc and /sys shall be done by crinit or have already been done by the previous init (pid=1).

Following mounts are deemed as minimal by crinit:

| #  | mount    | mount point | mandatory for crinit             |
|----|----------|-------------|----------------------------------|
| 1  | devtmpfs | /dev/       |                                  |
| 2  | procfs   | /proc/      | yes, for kernel cmd-line reading |
| 3  | sysfs    | /sys/       |                                  |
| 4  | devpts   | /dev/pts/   |                                  |
| 5  | tmpfs    | /run/       | yes, for socket interface        |

If crinit is running as pid=1 these mount operations need to be fulfilled by crinit, if
crinit is running as secondary init (pid!=1) a decision needs on what occasions these filesystems shall be 


## Influencing factors


Debugging and development aids of systems are not addressed here.


## Assumptions


### pid-name spaces

If crinit is started in a pid name space (using flag `CLONE_NEWPID`) it runs under pid=1, therefor it can not detect that it is in fact running as a secondary init. This way crinit will do the same operations for mount and zombie-reaping as if it was started directly from kernel as the primary init.


## mount - Considered alternatives in case crinit is secondary init

### 1.1) no mounts if pid!=1

crinit does not do the minimal mounts if it detects a pid!=1, it expects mandatory mounts to be done by primary init.

*pros*

- current behavior, no changes needed
- double mounting and subsequent error messages are avoided
- older integrations will not experience a breaking change

*cons*

- if operated in a mount-namespace (but not a pid-namespace) other tools (e.g. crinit tasks) need to do the mounts


### 1.2) mount if not found

crinit tries to detect and mount the filesystems in case of unsuccessful detection.

*pros*

- the minimal filesystem are always there
- double mounting and subsequent error messages are avoided

*cons*

- needs careful design to not cause any race-condition etc.
- older integrations might experience a breaking change


### 1.3) mount if configured

The crinit configuration is extended to define the requested behavior.
Default is to mount the filesystems, if pid=1, otherwise mount is skipped.
Mandatory mounts are done anyhow.

| option        | description            |
|---------------|------------------------|
| sys-mounts    | mounts the minimal fs  |
| no-sys-mounts | does not do any mounts |

The configuration is done via commandline argument. Configuration via kernel-commandline is not needed, as default fits to pid=1. Configuration via config-file is not yet considered.


*pros*

- all needs can be covered

*cons*

- implementation/documentation effort
- older integrations might experience a significant breaking change




## zombie reaper - Considered alternatives in case crinit is secondary init


### 2.1) reap all terminating child processes (zombies), if pid=1

If crinit detects to have pid=1, it acknowledges the termination of each if its child processes using 'wait()'.
If crinit has a pid different from 1 (pid!=1) it does nothing.

*pros*

- implements the required activity of init
- simple
- works fine as overall primary init
- works fine as primary init in a pid-namespace
- older integrations will not experience a breaking change

*cons*

- as secondary init, orphaned processes will be re-parented to an init above crinit. Hence an ancestor of crinit will need to care about processes that were orphaned in the process hierarchy of crinit.


### 2.2) request use of pid-namespace

The integrator using crinit is requested to always start crinit in a new pid-namespace to force it to pid=1.

*pros*

- no code changes needed, see above alternative
- older integrations will not experience a breaking change

*cons*

- doc changes needed
- making crinit dependent on kernel feature is not ideal
- debugging might become more complicated
- obstructs view of system "above" the secondary init


### 2.3) crinit to become CHILD_SUBREAPER if pid!=1

If crinit detects pid=1 normal handling of terminated child processes is committed.
If crinit detects pid!=1, it sets the PR_SET_CHILD_SUBREAPER flag, see above.

*pros*

- limited changes
- no pid namespace needed, but still possible

*cons*

- the flag is set unconditionally
- older integrations might experience a breaking change


### 2.4) configure crinit to become CHILD_SUBREAPER if pid!=1

As before, but the flag PR_SET_CHILD_SUBREAPER is only set in case it is configured.
The configuration defaults to not setting the flag.
If crinit detects being pid=1 it always handles terminating children.

| option             | description                                    |
|--------------------|------------------------------------------------|
| child-subreaper    | Make crinit register itself as child subreaper |
| no-child-subreaper | Inverse of above                               |

The configuration is done via commandline argument. Configuration via kernel-commandline is not needed, as default fits to pid=1. Configuration via config-file is not yet considered.

*pros*

- all options possible

*cons*

- more implementation/documentation effort









## Decision
After checking for completeness of alternatives and taking all pros and cons to account following decision was taken:

For mount:
*X*

For Zombie-Reaping
*Y*

*Hint: no further rationale for the decision is given here! If you feel the need to add a rationale here consider to add it as pros and cons to the above lists.*


## Open Point

Following points are identified as open after the decision was taken.

*none*
