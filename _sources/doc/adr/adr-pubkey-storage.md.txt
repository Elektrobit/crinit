# Architecture Design Record - Storage scheme for public keys

## Problem

In order to support signatures, crinit needs to keep track of one or more public keys. These should be protected from
tampering to ensure authenticity of signatures.

## Influencing Factors

* Multiple trusted vendors of crinit files may exist.

## Assumptions

* Signatures are always used in conjunction with some secure boot scheme, i.e. everything up to the Kernel binary, Device-tree (dtb) and initram-disk can
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
    configs could be considered trustworthy by their presence in the already completely signed rootfs.

### Option 2 - Keys in system keyring

A number of public keys are available via the user keyring (in-kernel key management and retention facility, man 7
keyrings). If a crinit config is signed with one of them, it is considered trusted.

The method to load the keys to the user keyring must be protected by the secure boot chain. The means to do that are
implementation defined, an example would be to enroll them from a signed initramfs.

#### Pros

* trustworthy and tamper-proof in conjunction with secure boot

#### Cons

* hard to add new keys by third party, as e.g. initramfs may need to be altered

### Option 3 - Master key in system keyring, signed keys in rootfs key directory

A root public key is available via the Kernel user keyring. A number of public keys which are signed with the root
private key exist in a known directory of the rootfs. If a crinit config is signed with one of them or the root key,
it is considered trusted.

The method to load the root public key could be from a signed initramfs, as in Option 2.

#### Pros

* trustworthy and tamper-proof in conjunction with secure boot
* reasonably easy to add new third-party keys

#### Cons

* a change of the root key needs not only a new initramfs but also an update to rootfs

## Decision

Option 3 is taken.
