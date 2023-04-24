# Architecture Design Record - INCLUDE file options

## Problem

Crinit needs support for include files in task configuration to provide templates to third-party developers. The main
requirement is to define base sets of dependencies but pre-defining other settings may be possible.

An example might look similar to this:

**Include file** (`server.crincl`)
```
DEPENDS = @provided:network sql-db:spawn other-base-dependency:wait
RESPAWN = YES

ENV_SET = BASE_ENV_VARIABLE "foo"
```

**Task configuration** (`httpd.crinit`)
```
NAME = httpd
INCLUDE = server.crincl

COMMAND[] = /path/to/httpd --some-options

RESPAWN = NO
DEPENDS = added-dep:wait
ENV_SET = ADDED_ENV_VARIABLE "bar"
ENV_SET = COMPOUND_VAR "${BASE_ENV_VARIABLE} ${ADDED_ENV_VARIABLE} baz"
```

This leads to the question of which options to allow in an include file and what the append/override policy is for them.
The example suggests to merge/append some variables (`DEPENDS`, `ENV_SET`) but override others (`RESPAWN`) but this may
not be the best solution in all cases.

## Influencing Factors

* ease of use / intuitively usable
* the design shall not allow circumvention of security 

### Assumptions

## Considered Alternatives

### Option 1 - All options possible, pre-set policy

We have a pre-set append/override policy, like "Options get appended to where possible. If the option is single value,
it will be overriden. If multipled include files are given, append and override appends in the order they are INCLUDEd.
All options valid in task configurations are also valid in `INCLUDE` files.

#### Pros
* simple concept
* least implementation effort
* low documentation effort

#### Cons
* limited functionality
* overriding of multi-value options impossible
* can become chaotic especially if more than one include file is involved.
* some options can be problematic in an include file
    - Example: `COMMAND[]` can be appended to. If include files are used to prepend commands to downstream task files,
               the configuration effort becomes high and behavior may seem obtuse to the downstream user.

### Option 2 - All options possible with task-configurable subset

Like option 1 but we allow tasks to optionally define a list of option imports from the INCLUDE file. This could look
like the following:
```
INCLUDE = some-include.crincl DEPENDS,RESPAWN
```

In this case only  the `DEPENDS` and `RESPAWN` options are imported from the include file. Append/override afterwards
works as in option 1. If no import list is given, it will work the same as option 1 by default.

#### Pros
* simple extension of Option 1
* big improvement in usability
* users can rely on pre-sets but also have full control over what they import

#### Cons
* effective usage of imports requires more system knowledge than plain option 1
* changes (esp. additions) in include files may not propagate downstream if a task does not import the new option
* some options can be problematic in an include file (see above)

### Option 3 - Limited number of options possible with task-configurable subset

Like option 2 but not all options are allowed in an include file. We shall maintain a documented list of all options,
indicating if they are include-safe and what their append/override policy is.

#### Pros
* improves on option 2 by filtering out options, thereby limiting configuration complexity
* maintaining this table in documentation is a good idea anyway as configuration options become more numerous/complex
* otherwise same as option 2

#### Cons
* added documentation effort
  * But that is needed anyhow and some advice on how to use this feature is nice to have
* otherwise as option 2

## Decision

Option 3 is taken.

## Rationale

Option 1 disqualifies itself because having options being appended to without possibility to exclude/override them may
prohibit advanced usage. Options 3 has a leg up on option 2 as it encourages intended usage. Option 2 would offer the
most functional possibilities but not all of them will lead to a cleanly configured system.
