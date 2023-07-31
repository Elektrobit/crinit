# Architecture Design Record - Storage scheme for file signatures

## Problem

As Crinit shall support configuration file signatures, we need to decide how to store them.

## Assumption

As described in [Storage scheme for public keys](adr-pubkey-storage.md) the master key to verify is stored in the kernel key ring and further keys (signed/verified by the master key) are stored in the filesystem.

The master key may be identified by a keyID configurable via the Kernel command line. All trusted keys including the master key are used to verify the signatures described here. There is no keyID used to identify the key needed to verify a particular signature.

Reason why this does not consume too many cpu cycles:
- the hash (SHA256) of the data to be verified is calculated only once
- all data is in RAM (key in keyring and hash mentioned above)
- far less than 100 keys are expected to be loaded to the key ring

## Considered Alternatives

### Option 1 - Within the configuration file

The signature is stored as ASCII plaintext (using uuencode or similar) and appended to the file it is signing.

#### Pros

* easy and clear association between signature and file

#### Cons

* file is necessarily changed after being signed, verification slightly more complex
* may look confusing to users

### Option 2 - Separate file associated by name

The signature is stored in either ASCII or binary format and placed in its own file. A config file `some_task.crinit`
would have a corresponding `some_task.sig` signature file. The signature file should be placed in the same path as the
configuration file.

#### Pros

* easy and clear association between signature and file
* easy verification
* the configuration file is not touched in the process
* meshes well with existing configuration "style"

#### Cons

* it may not be immediately apparent to the user that a given file has a signature

### Option 3 - Separate file associated by configurable path

Similar to option 2, but the configuration file contains a path to the signature file which may be arbitrarily named,
like `SIGNATURE = /path/to/signature.sig`.

#### Pros

* flexible
* easy to see if a given configuration has a linked signature

#### Cons

* configuration file needs to be changed during signing
* more complex verification
* difference to configuration "style" of rest of crinit

## Decision

Option 2 is taken.
