# Architecture Design Record - Storage scheme for public keys

## Problem

In order to support signatures, crinit needs to keep track of one or more public keys. These should be protected from
tampering to ensure authenticity of signatures.

## Influencing Factors

* Multiple trusted vendors of crinit files may exist.

## Assumptions

* Signatures are always used in conjunction with some secure boot scheme, i.e. everything up to the Kernel binary can
  be considered trusted by crinit.

## Considered Alternatives

### Option 1 - Key directory in rootfs

A number of plain public keys exist in a known directory of the rootfs. If a crinit config is signed with one of them,
it is considered trusted.

#### Pros

* easy implementation
* easy to add new keys by third party

#### Cons

* only trustworthy if rootfs itself is trusted, i.e. signed as a whole
  - this mostly defeats the purpose of having individually signed files in the first place as even individually unsigned
    configs could be consideed trustworthy by their presence in the rootfs.

### Option 2 - Keys in system keyring

A number of public keys are available via the system keyring. If a crinit config is signed with one of them, it is
considered trusted.

The method with which to load the keys to the system keyring must be protected by the secure boot chain. The means with
which to do that are implementation defined, possible examples could be:

* compilation into a signed Kernel
* Kernel support for a hardware security module containing the key
* a trusted Kernel module

#### Pros

* trustworthy and tamper-proof in conjunction with secure boot

#### Cons

* hard to add new keys by third party

### Option 3 - Master key in system keyring, signed keys in rootfs key directory

A master public key is available via the Kernel system keyring. A number of public keys which are signed with the master
private key exist in a known directory of the rootfs. If a crinit config is signed with one of them or the master key,
it is considered trusted.

The method with which to load the master key to the system keyring must be protected by the secure boot chain. The means
with which to do that are implementation defined, possible examples could be:

* compilation into a signed Kernel
* Kernel support for a hardware security module containing the key
* a trusted Kernel module

#### Pros

* trustworthy and tamper-proof in conjunction with secure boot
* reasonably easy to add new third-party keys

#### Cons

* a change of master key needs not only a new Kernel but also an update to rootfs

## Decision

Option 3 is taken.
