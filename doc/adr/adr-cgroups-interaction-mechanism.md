# Design Decisions - Mechanism to interact with cgroups V2

## Problem

How do we want to implement cgroups support?

## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* <first>
* <second>



## Assumptions

The following assumptions have been made in the decision process:
* <first>
* <second>



## Considered Alternatives

### 1) Manually interact with cgroups via cgroup filesystem

Change the crinit parser to support parsing cgroup configurations (e.g. in series file
and / or task files) and interact with the cgroup filesystem directly.

*pros*
* <first> Homogenous handling of different configuration options. Crinit supports starting
tasks as a different user, too. This would be a similar use-case (in the meaning of process
restrictions).

*cons*
* <first> This could lead to a quite drastic change in the configuration parser with all 
the consequences.

### 2) Use the tools of libcgroup

There are tools in libcgroup to manage cgroups and start processes. This could be seamlessly
integrated in the existing COMMAND parser.

*pros*
* <first> No changes in crinit itself, no risk of breaking something.

*cons*
* <first> This would be inconsistent in comparision with other supported process restriction mechanisms (e.g. user / group support).

### 3) Use open3() and alike

Use syscalls like open3() and setns() etc. to manage processes with cgroups.

*pros*
* <first> None.

*cons*
* <first> Not a real solution, because it does not cover the creation of cgroups.

## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
