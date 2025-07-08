# cgroups implementation suggestion

## Problem

Implement the cgroup support in crinit. Make it configurable. If enabled, crinit must do the following steps:
* mount the cgroup virtual filesystem
* create a cgroup directly under the root cgroup that may change every available controller
* Place every every created cgroup below the crinit cgroup created in the previous step

If a task does not have a cgroup configured, place it in a cgroup anyways.

In the series file the crinit root cgroup has to be configured (if activated).

In the series file crinit global cgroups can be configured. This can be handy to group several tasks in the same cgroup.

Task specific cgroups are configured in the task file. There can only be one cgroup configured in a task file.

Do not support a cgroup hierachy aside from the crinit root cgroup.

The configuration in the series files could be like this:

```
CGROUP_ROOT_NAME = crinit.cg
CGROUP_ROOT_PARAMS = "memory.max=50M"   # array like, one key-value-pair per line

CGROUP_GLOBAL_NAME = first_custom_cgroup  # array like, one name per line
CGROUP_GLOBAL_PARAMS = "first_custom_cgroup: memory.max=25M"  # array like, one key-value-pair per line, prefixed by a valid and existing custom cgroup name followed by a ':'
```

As of July 2025 the parser does not support keywords with a variable part. That's why something like "CGROUP_GLOBAL_PARAMS[first_custom_cgroup]" is currently not possible. It would require a severe change in the parser to support a variable part. That is no problem because the value part on the right side of the '=' sign is completly private to the relevant handler function.

The configuration in a task file is very similar:
```
CGROUP_NAME = group.name
CGROUP_PARAMS = "key=value"     # array like, one key-value-pair per line
```

The key part of the parameters is the filename in the cgroup filesystem that is responsible for that parameter. The value needs to be the exact value that will be written into the file.


## Suggestion to first implementation steps (minimum viable system)

* make crinit mount the cgroup virtual filesystem
* alter the root cgroup configuration to "export" every available controller
* expand the task file parser to understand "CGROUP_NAME" and "CGROUP_PARAMS" and store their values in the task structure
* expand crinit-launcher to check if a requested cgroup exists and create it if not
* expand crinit-launcher to join a requested target cgroup (if any) by writing its PID into the relevant file in the cgroup virtual filesystem *before* starting the target process (this ensures the process's complete live is under the desired cgroup control)


## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* Should we make the crinit root cgroup configurable? Yes, of course. That way we can avoid name clashes and reduce the amount of available resources to child cgroups.
* Should we have common cgroup for unconfigured tasks? Or should we create a cgroup for every unconfigured task? See adr/adr-cgroups-common-cgroup-for-unconfigured-tasks.md
* Should we place the cgroup process itself in a cgroup below its crinit root cgroup? How would this interact with e.g. the OOM killer if the crinit root cgroup runs out of memory? How can we prevent the crinit process from beeing killed under any circumstances? See adr/adr-cgroups-own-cgroup-for-crinit-process.md
