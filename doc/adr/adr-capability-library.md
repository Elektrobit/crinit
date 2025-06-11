# Design Decisions - Choice of helper library to impelement the capability feature

## Problem

Technically, capabilities can be configured via the Linux kernel's syscall interface. But when it comes to provide a user interface (e.g. crinit task configuration file), we require a reliable API to transform human readable capability identifiers into their numerical representation.

## Influencing factors

- Besides aforementioned necessity to transform capability identifiers, libraries also offer convenience APIs to set capabilities in the kernel. This is less error prone than using the low level syscall interface.
- When it comes to choose between different libraries, license agreements play an important role

## Assumptions

None

## Considered Alternatives

### 1) Use ro library at all

All capability related features are implemented via the syscall interface.

_pros_

- No additional license to handle

_cons_

- No human readable configuration interface (capability identifiers from /linux/capabilities.h cannot be used): capabilities have to be provided by their numerical values. This is error prone.
- Conversion between numerical and textual representation of capabilities needs to be implemented and kept in sync with the Linux kernel by ourselves. This is error prone.

### 2) libcap

Library for capability handling. License BSD-3-Clause OR GPL-2.0-only

_pros_

- This library provides all necessary APIs and is especially capable of converting between numerical and textual representation of capabilities.
- There is an easy to use and well documented API available.
- (Compared to libcap-ng) The license is better to handle.

_cons_

- None that actually matter.

### 3) libcap-ng

Alternative library for capability handling. License GPL-2.0-or-later, LGPL-2.1-or-later

_pros_

- The library provides an equally good feature/API set as libcap.

_cons_

- Its license is not really clear, libcap seems to be easier to handle

## Decision

Alternative 2 is taken. Feature set of both libraries is roughly the same. The license decided for libcap.

### Rationale

### Open Points

### References

- https://man7.org/linux/man-pages/man3/libcap.3.html
- https://github.com/stevegrubb/libcap-ng
- https://git.kernel.org/pub/scm/libs/libcap/libcap.git/
