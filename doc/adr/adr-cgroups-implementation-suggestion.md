# Design Decisions - cgroups design suggestions

## Problem

Implement the cgroup support in crinit. Make it configurable. If enabled, create a cgroup directly under the root cgroup that may change every available controller. Below that cgroup place every created cgroup.

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

While the CGROUP_GLOBAL_PARAMS may not be the niciest approach, in my understanding it needs the least modification in the existing parser.

The configuration in a task file is very similar:
```
CGROUP_NAME = group.name
CGROUP_PARAMS = "key=value"     # array like, one key-value-pair per line
```

The key part of the parameters is the filename in the cgroup filesystem that is responsible for that parameter. The value needs to be the exact value that will be written into the file.


## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* Should we make the crinit root cgroup configurable?
* Should we have common cgroup for unconfigured tasks? Or should we create a cgroup for every unconfigured task?
* Should we place the cgroup process itself in a cgroup below its crinit root cgroup? How would this interact with e.g. the OOM killer if the crinit root cgroup runs out of memory? How can we prevent the crinit process from beeing killed under any circumstances?



## Assumptions

The following assumptions have been made in the decision process:
* <first>
* <second>



## Considered Alternatives

### 1) <alt 1>

Description of this alternative.

*pros*
* <first>
** But: <if applicable react to the above with a but...>
* <second>
** But: 

*cons*
* <first>
** But: <if applicable react to the above with a but...>
* <second>
** But: 

### 2) <alt 2>

Description of this alternative.

*pros*
* <first>
** But: <if applicable react to the above with a but...>
* <second>
** But: 

*cons*
* <first>
** But: <if applicable react to the above with a but...>
* <second>
** But: 

### 3) <alt 3>

Description of this alternative.

*pros*
* <first>
** But: <if applicable react to the above with a but...>
* <second>
** But: 

*cons*
* <first>
** But: <if applicable react to the above with a but...>
* <second>
** But: 

## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
