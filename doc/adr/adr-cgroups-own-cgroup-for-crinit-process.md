# Design Decisions - Should the crinit process itself be placed in a cgroup on its own?

## Problem

The crinit process could remain in the "real" root cgroup (that's the parent of the crinit create root cgroup) or it could be moved to a cgroup on its own.

## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* Systemstability must be key. A killed crinit will result in a kernel panic!



## Assumptions

The following assumptions have been made in the decision process:
* <first>
* <second>



## Considered Alternatives

### 1) Leave crinit in "real" root cgroup

The crinit process does not get moved into another cgroup.

*pros*
* No resource limits for the crinit process
* No direct competition with its children

*cons*
* Considering safety related environments it might be more appropriate to have crinit in a restricted cgroup, too

### 2) Move crinit as soon as possible to its own cgroup under the created crinit root cgroup

Create a cgroup under the crinit root cgroup and move the crinit process there.

*pros*
* Might be more appropriate if used in a safety related environment

*cons*
* That cgroup must be configurable to have the most possible benefit
* It must be prevented that the crinit process will be killed (e.g. bei OOM killer) if other processes use to much resources.


## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
