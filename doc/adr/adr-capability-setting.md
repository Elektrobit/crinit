# Design Decisions - Choice of how crinit's default capabilities are merged into a task's capability set

## Problem

Beside a _task specific_ capability configuration (defined in a `.crinit` file), there is also _system wide_ valid set of default capabilities (defined in a `.series` file). Eventually, both sets of capabilities need to be merged into a task's effective capability set. This can be accomplished in various ways each of which brings about different levels of complexity and maintainability.

## Influencing factors

- With inheritable capabilities, the Linux kernel natively offers a mechanism to to define and provide an initial (default) set of capabilities for a new process.
- Crinit's system architecture, especially it's execution stack via crinit-launch to start a new task, has implications on utilizing inheritable capabilities.

## Assumptions

None

## Considered Alternatives

### 1) Set inheritable capabilities of crinit to default capabilities

Use crinit's inheritable capabilities to hold default capabilities and provide it to crinit-launch to the task.

_pros_

- This seems to be the natural way by design of the capability concept: Inheritable capabilities are preserved during `execve` and are automatically integrated (via permitted capabilities) into a thread's effective capability set.

_cons_

- Adds overhead to crinit: Even if a task would not configure capabilities, crinit would set its inheritable capabilities according to the default capability set specification.
- Would require crinit to run as root during smoketests or as secondary init process.
- Distributes application logic (set default capabilities with and merge them with effective capabilities) over both crinit and crinit-launch

### 2) Calculate the final set of effective capabilities all together in crinit

This approach is designed to merge default capabilities with both capabilities that shall be set and cleared for a task already in crinit before crinit-launch is started. The resulting effective set of capabilities is then provided to crinit-launch as single CLI parameter.

_pros_

- There is only one new single CLI parameter of crinit-launch to describes the effective set of capabilities of the new task.
- Only few changes necessary to existing execution logic.
- All capabilities related application logic is implemented in crint. Final set of effective capabilities is just provided as a CLI option to crinit-launch

_cons_

- None.

## Decision

Alternative 2 is taken.

### Rationale

### Open Points

### References

- [1] https://man7.org/linux/man-pages/man3/libcap.3.html
