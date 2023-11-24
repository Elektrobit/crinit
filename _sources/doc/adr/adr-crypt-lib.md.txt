# Architecture Design Record - Choice of cryptographic library for config signatures

## Problem

Crinit shall support signed (task/global) configurations. Having chosen the algorithm (RSA-PSS) in
[another ADR](./adr-signature-algorithm.md), we also need to choose the right cryptographic library implementation to
suit our needs.

## Influencing Factors

* The cryptographic library to use needs to offer the needed functionality for RSA-PSS signature verification with
  `SHA256` and `RSA-4096` (see [relevant ADR](./adr-signature-algorithm.md)).

## Assumptions

* The terms RSASSA-PSS and RSA-PSS are used interchangeably in literature and functionally describe the same thing

## Considered Alternatives

### Option 1 - mbedTLS

Minimalistic cryptographic library targeted at embedded systems.

#### Pros

* widely used in embedded devices on different OSes.
* active development, regular releases
* good documentation, available in a modern hyoertext format [^7]
* large amount of tests
  - also organical part of their github merge process via their CI [^8]
* permissive license (Apache License 2.0)
* modular and small in size, making static linking feasible if desired

#### Cons

* no FIPS 140 certification effort up to now

### Option 2 - OpenSSL (version 3.0.0 or later)

Feature-packed cryptographic library targeting a diverse range of systems. Has the longest development history among the
options.

#### Pros

* very widely used
* active development, regular releases
* good documentation available as a huge set of manpages [^6]
* extensive amount of tests
* permissive license (Apache License 2.0, since v3.0.0)
* some versions are FIPS 140 certified (with reduced functionality), currently 3.0.8

#### Cons

* relatively large size of library
  - *But:* Usually present as a shared library in most systems anyway due to other dependencies.
* large and complex codebase
  - Evidenced by performance regressions in v3.0.0 which have, so far, not been solved.[^1][^2]

### Option 3 - wolfSSL

Cryptographic library with corporate backing targeted at embedded systems. The company with the same name provides paid
support and differently licensed editions of the library.

#### Pros

* small size
* active development, regular releases
* OpenSSL-compatible API wraps exist
* good documentation [^4]
* FIPS 140 certified versions exist
  - *But:* The current certificates are older than those of OpenSSL. Also, while the FIPS-module is free, the website
           informing about FIPS-compliance does suggest that this is an area where paid support might be needed. [^3]

#### Cons

* freely-available version released under non-permissive license (GPL-2.0)
* not as widely used
* [test suite seems relatively lean](https://github.com/wolfSSL/wolfssl/tree/master/tests)
  - [at least API tests are all inside a single >2MB source file](https://github.com/wolfSSL/wolfssl/blob/master/tests/api.c)

### Option 4 - GnuTLS

Originally a GNU replacement project for OpenSSL whose license was deemed incompatible with the GPL (before v3.0.0). Has
since been split off from the main GNU project.

#### Pros

* permissive license (LGPL 2.1)
* good documentation [^5]
* extensive test suite
* has a FIPS 140 mode which aims to be compliant
  - *But:* the actual certification seems to exist as part of Red Hat Enterprise Linux

#### Cons

* not as widely used as the other options
  - *But:* Some high-profile projects, such as GNOME
* development seems less active than other projects
* while example code for RSA-PSS exists, the library seems mostly targeted at secure stream communication

## Decision

Option 1 is taken.

*NOTE:* We should make sure to wrap the library calls in a separate C file and provide high-level functions to the
        rest of crinit. This would make it easy to swap out the chosen library against another (e.g. for FIPS com-
        pliance). If we build this source file as a shared object, the swap could even be done without a complete
        rebuild.

## References

[^1]: [*OpenSSL Issue 16791*](https://github.com/openssl/openssl/issues/16791)
[^2]: [*OpenSSL Issue 17064*](https://github.com/openssl/openssl/issues/17064)
[^3]: [*WOLFCRYPT FIPS 140-2 and FIPS 140-3*](https://www.wolfssl.com/license/fips/)
[^4]: [*wolfSSL documentation*](https://www.wolfssl.com/docs/)
[^5]: [*GnuTLS documentation*](https://gnutls.org/manual/gnutls.html)
[^6]: [*OpenSSL documentation*](https://www.openssl.org/docs/manpages.html)
[^7]: [*mbedTLS documentation*](https://mbed-tls.readthedocs.io/en/latest/)
[^8]: [*mbedTLS CI*](https://github.com/Mbed-TLS/mbedtls-test)
