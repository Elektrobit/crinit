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
* Homogenous handling of different configuration options. Crinit supports starting
tasks as a different user, too. This would be a similar use-case (in the meaning of process
restrictions) and could be implemented there quite naturally.

*cons*
* This would require quite some changes in the series and task parseres. The task parser is considered rather easy as it can be handled completely with the existing meachanisms. The series parser would either need new meachnisms to parse different global cgroups or the configuration would deviate from the current scheme. See the implementation suggestion ADR for more information.

### 2) Use the tools of libcgroup

There are tools in libcgroup to manage cgroups and start processes. This could be seamlessly
integrated in the existing COMMAND parser.
The libcgroup library can be found here: https://github.com/libcgroup/libcgroup
According to the github page its license is LGPL-2.1.

*pros*
* No changes in crinit itself, no risk of breaking something.

*cons*
* This would be inconsistent in comparision with other supported process restriction mechanisms (e.g. user / group support).

### 3) Use clone3() and alike

Use syscalls like clone3() and setns() etc. to manage processes with cgroups. Those syscalls can have a target cgroup as a parameter. With this parameter clone3() will start the new process directly in the new cgroup, for example. They do not create cgroups, though.

*pros*
* None.

*cons*
* Not a real solution, because it does not cover the creation of cgroups.

## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
