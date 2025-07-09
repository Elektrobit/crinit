# Design Decisions - Where to define cgroups?

## Problem

If we decide to handle cgroups natively in crinit we have to decide where cgroups can be defined.

## Influencing factors

The following constraints have to be taken into account for a suitable solution:
* Respect the capabilities of Crinit config files. (Key/Value-Pairs and lists)



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
* Increase complexity of configuration parser, considering the nature of crinit series files the cgroup identifier and properties assignments must be somehow encode in the key/value pairs.
### 2) In task files only

Cgroups can only be defined and created in task files.

*pros*
* Every task creates the cgroups that it needs. This would allow for a nice atomic approach.
* Easy to map the additional options to new config options (key/value pairs)
*cons*
* If multiple tasks need to use the same cgroup they would have to wait on a setup task or other tasks to which they share no further mutuality.
* Probably not intuitive and error prone, leading to the question "which task file create is creating this cgroup?"
### 3) Allow cgroup definition in series and task files

Cgroups can be defined and created in series and task files.

*pros*
* Commonly used cgroups can be defined in a common place
* Tasks don't need to wait for otherwise unrelated tasks.
* Tasks can create their own cgroups that are only relevant for themselves.

*cons*
* It's the most complex approach.

### 4) In separate cgroup configuration files

A completely separate configuration file structure is introduced. Under the crinit sysconfig directory there could be a sub-directory where *.crinit-cg files are stored that contain the cgroup configuration.

*pros*
* none

*cons*
* A separate parser is needed
* The configuration of task-specific cgroups gets unintuitive
* A format for those configuration files has to be defined

## Decision

Alternative 3 is taken.

### Rationale

### Open Points

if something was left open after this decision
