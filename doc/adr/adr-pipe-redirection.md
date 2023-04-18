# Architecture Design Record - Management of named pipes for IO redirection

## Problem

As of version `0.9.0`, crinit supports IO redirection from/to files. With `0.9.2`, this is extended to named pipes or
FIFOs using the `PIPE` keyword. The intention is to simplify connecting `STDOUT/ERR` from one task to `STDIN` of
another. Prior to the addition, the user would have to create the special file tied to the pipe themselves using mkfifo.

For the new functionality, crinit will handle creation of the pipe itself. The remaining question is, if crinit should
somehow manage that file during its lifetime or if one-time creation is enough.

## Influencing Factors

### Assumptions

* presence of a writable temporary file system like `/tmp` or `/run`

## Considered Alternatives

### Option 1 - Pipe creation only

The IO redirection statement shall be of the form

```
IO_REDIRECT = <STREAM> "/path/to/pipe" PIPE [OCTAL_MODE]
```

The pipe's path is to be decided entirely by configuration. In order for two tasks to be connected they need to point
to the same path. If a task finds no existing pipe at the target, it will create one. The created pipe will stay there
until externally deleted.

#### Pros
* simple concept
* least implementation effort required
* no assumptions of target system structure necessary
* pipe is easily externally accessible
    - in case we want to send something to a Crinit task from a process not managed by Crinit

#### Cons
* no centralized location where all pipes reside
* no deletion of pipe files after use suggests a temporary file system
    - But: that is the usual way to do it anyway
* some organisation work offloaded to system integrator

### Option 2 - Pipe file management

Crinit maintains a list of named pipes residing in a central (globally configurable) location. Tasks use named
references, defaulting to their own name.

For a sender and a receiver that may look like
```
# sending task
NAME = sender_task
[...]
IO_REDIRECT = STDOUT PIPE
```

```
# receiving task
NAME = receiver_task
[...]
IO_REDIRECT = STDIN sender_task PIPE
```

Upon first encountering the `sender_task` pipe reference, crinit will create that pipe in the globally configured
FIFODIR (default would be something like `/run/crinit/fifos/`). Optionally crinit might clear that directory on
shutdown, or even if all connected tasks terminate if so configured.

#### Pros
* better managed centralized location
* crinit takes off some organisation work from system integrator
* optional deletion possible

#### Cons
* concept more complex
* implementation work more complex
* pipe is not easily accessible from outside

### Option 3 - Both usable, decided through configuration

For this, we can let Crinit behave as in option 1 if given an absolute path or as in option 2 otherwise.

#### Pros
* pros of options 1 and 2 excluding complexity considerations

#### Cons
* most complex concept
* most complex implementations
* less intuitive for users

## Decision

Option 1 is taken for current milestone `0.9.2`, with possibility to move in the direction of option 2 for later
releases.

## Rationale

Options 2 and 1 share much of their technical groundwork in the low-level process spawning area. While option 1 does not
need much more than that, option 2 can be seen as an extension which needs more higher level logic within crinit. Given
that, option 1 can be seen as a reasonable stepping stone to offer basic functionality now, evaluate feedback, and
extend with aspects of option 2 down the line if required.
