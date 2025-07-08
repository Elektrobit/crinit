# Design Decisions - Do we want to support hierarchical cgroups?

## Problem

Cgroups can be organized in some sort of tree. If so, they can "inherit" some specifications from their parents. The inheritance process is described here: https://docs.kernel.org/admin-guide/cgroup-v2.html#top-down-constraint. It is no inheritance like e.g. a C++ programmer would expect it.
It seems that the way the resources are restricted or distributed is depending on the kind of resource. See here for more details: https://docs.kernel.org/admin-guide/cgroup-v2.html#resource-distribution-models

## Influencing factors

The following constraints have to be taken into account for a suitable solution:

* New tasks can be added during runtime, hence it is possible that also a new
  cgroup is added during runtime. As processes can only be assigned to
  cgroup-leafs, adding new cgroups and processes down in the hierarchy can lead
  to complex cgroup/process reassignment tasks.



## Assumptions

The following assumptions have been made in the decision process:
* cgroups are created on startup
* cgroups can be created on runtime
* cgroups never disappear without restart




## Considered Alternatives

### 1) Don't support hierarchical cgroups

Don't support hierarchical cgroups.

*pros*
* Reduces complexity in the parser and the implementation in crinit
* Easy to add new cgroups on runtime

*cons*
* Maybe not capable to support complex scenarios or only very cumbersome. So the user would be restricted in some scenarios.


### 2) Support hierarchical cgroups

Support hierarchical cgroups.

*pros*
* Most flexible approach

*cons*
* More room for errors. Processes may only be in the leave nodes of the cgroup tree, for example.
* Complex to deal with new cgroups on runtime

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
