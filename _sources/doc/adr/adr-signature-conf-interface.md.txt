# Architecture Design Record - Configuration interface for signature handling

## Problem

It should be configurable if crinit shall use/expect signed configurations. How that is done will have implications on
usability and security. We should find a secure way with reasonable usability.

## Influencing Factors

### Assumptions

* a secure boot scheme is used

## Considered Alternatives

### Option 1 - Configuration at build-time

The signature functionality can be activated or deactivated at build-time using compiler definitions.

#### Pros

* simple implementation

#### Cons

* Causes problems with packaging. We would need to maintain and install two different crinit packages depending if we
  want to use signatures or not.
* testing more complicated if multiple binaries exist

### Option 2 - Configured in global configuration

Crinit's global configuration would be always verified and offer a runtime configuration to activate or deactivate
signatures for task configurations.

#### Pros

* simple implementation
* uses already present configuration parser

#### Cons

* impossible to have an unverified global configuration

### Option 3 - command line argument

Crinit will accept command line arguments to activate or deactivate signatures.

#### Pros

* flexible
* easy to parse with getopt
* extensible

#### Cons

* unusual for an init system
* awkward to use from bootloader perspective

### Option 4 - parse Kernel command line

Parse Kernel command line with arguments like `crinit.signatures={yes|no}` and similar to configure signature.

#### Pros

* implicitly secure if secure boot chain is used
* reasonably easy to parse with re2c
* easy to use from bootloader perspective
* extensible

#### Cons

* completely new parser code required

## Decision

Option 4 is taken.
