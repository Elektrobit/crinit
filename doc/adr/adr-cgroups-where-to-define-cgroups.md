# Design Decisions - Where to define cgroups?

## Problem

If we decide to handle cgroups natively in crinit we have to decide where cgroups can be defined.

## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* <first>
* <second>



## Assumptions

The following assumptions have been made in the decision process:
* <first>
* <second>



## Considered Alternatives

### 1) In series file only

Cgroups can only be defined and created in the series file part of the configuration.

*pros*
* All cgroups are defined in one place.
* All cgroups are created before they are used
* If multiple tasks use the same cgroup this approach is the more natural one to use a shared ressource in multiple places.

*cons*
* If a single task want its own very specific cgroup it has to relay on a proper modified series file.

### 2) In task files only

Cgroups can only be defined and created in task files.

*pros*
* Every task creates the cgroups that it needs. This would allow for a nice atomic approach.

*cons*
* If multiple tasks need to use the same cgroup they would have to wait on a setup task or other tasks to which they share no further mutuality.

### 3) Allow cgroup definition in series and task files

Cgroups can be defined and created in series and task files.

*pros*
* Commonly used cgroups can be defined in a common place
* Tasks don't need to wait for otherwise unrelated tasks.
* Tasks can create their own cgroups that are only relevant for themselves.

*cons*
* It's the most complex approach.

## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
