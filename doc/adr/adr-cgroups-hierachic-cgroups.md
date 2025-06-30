# Design Decisions - Do we want to support hierarchical cgroups?

## Problem

Cgroups can be organized in some sort of tree. If so, they can "inherit" some specifications from their parents.

## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* <first>
* <second>



## Assumptions

The following assumptions have been made in the decision process:
* <first>
* <second>



## Considered Alternatives

### 1) Don't support hierarchical cgroups

Don't support hierarchical cgroups.

*pros*
* <first> Reduces complexity

*cons*
* <first> Maybe not capable to support complex scenarios or only very cumbersome.

### 2) Support hierarchical cgroups

Support hierarchical cgroups.

*pros*
* <first> Most flexible approach

*cons*
* <first> More room for errors. Processes may only be in the leave nodes of the cgroup tree, for example.
** But: 

## Decision

Decision 1 "Don't support hierarchical cgroups" is taken

### Rationale

The original ADR requests a "flat style". It states the following pros and cons:

*pros*
* integrator just needs to define cg/ns features he/she wants.
    But only meaningful selected ones.

*cons*
* feature bloat of crinit?

### Open Points

if something was left open after this decision
