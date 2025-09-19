# Design Decisions - One cgroup per unconfigured task or a common cgroup for all unconfigured tasks

## Problem

If cgroup support is enabled in crinit and a task does not have a cgroup configured, in which cgroup should this process be placed?

## Considered Alternatives

### 1) Create a common cgroup for all unconfigured tasks

This common cgroup will get all tasks that do not have a cgroup configured in their task file.

*pros*
* The system can reserve specific resources for unconfigured tasks and there is no guesswork what would be a sensible default for a task specific cgroup

*cons*
* That common cgroup must be configurable in the series file
* Completely unrelated tasks start competing directly with each other in a more direct way than with no cgroups at all

### 2) Create a different cgroup for each unconfigured task

For each task that does not have its cgroup configured a new cgroup will be created before the process is started. The name could be generated from the e.g. the task name.

*pros*
* There is no direct competition to other unrelated tasks

*cons*
* It can be quite hard to guess sensible defaults for the resource available for that cgroup if it shall not be unrestricted.

### 3) Place the unconfigured tasks directly under the "real" root cgroup

The "real" root cgroup (that's the parent of the crinit created root cgroup) can have sub-cgroups and processes at the same time. Therefore it would be possible to have unconfigured tasks in that cgroup. That would be the default behaviour.

*pros*
* The least effort on crinit side

*cons*
* The unconfigured tasks would be quite high-ranked to compete for resources. That might get problematic in case of using crinit in a safety related context.

## Decision

Alternative 42 is taken.

### Rationale

### Open Points

if something was left open after this decision
