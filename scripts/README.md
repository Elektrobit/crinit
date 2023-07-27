# Crinit-related scripts

This subfolder contains scripts related to Crinit for system setup or maintenance.

Currently, scripts for generating signature keys and for signing data with them are provided. These can be used to
generate signatures for task and global configuration files as well as generating the root key and signed downstream
keys.

While crinit internally uses mbedTLS as a cryptography library, the scripts depend on OpenSSL's user-space interface.

# Script usage information

## `crinit-genkeys.sh`

```
Usage: crinit-genkeys.sh [-h/--help] [-k/--key-file <KEY_FILE>] [-o/--output <OUTPUT_FILE>]
  If no other arguments are given, will generate an RSA-4096 private key. Alternatively using '-k', you can obtain
  the public key for the given private key.
    -h/--help
        - Show this help.
    -k/--key-file <KEY_FILE>
        - Generate a public key from the given private key. Use '-' for Standard Input. Default: Generate a private key.
    -o/--output <OUTPUT_FILE>
        - The filename of the output key. Default: write to Standard Output
```

## `crinit-sign.sh`

```
Usage: crinit-sign.sh [-h/--help] [-k/--key-file <KEY_FILE>] [-o/--output <OUTPUT_FILE>] [<INPUT_FILE>]
  Will sign given input data with an RSA-PSS signature from an RSA-4096 private key using a SHA-256 hash.
    -h/--help
        - Show this help.
    -k/--key-file <KEY_FILE>
        - Use the given private key to sign. Must have been created using crinit-genkeys.sh. (Mandatory)
    -o/--output <OUTPUT_FILE>
        - Write signature to OUTPUT_FILE. Default: Standard Output
    <INPUT_FILE>
        - Positional argument. The input data to sign. Default: Standard Input
```
# Usage examples

## Generate a root key pair

First generate the private root key.

```
$ crinit-genkeys.sh -o crinit-root.key
```

Then generate the public root key from it. This must be the key placed in the system keyring prior to crinit's
execution, e.g. by using an HSM or by compiling it into the Kernel.

```
$ crinit-genkeys.sh -k crinit-root.key -o crinit-root.pub
```

## Generate downstream keys signed by the root key

Assuming the presence of a root key (`crinit-root.key`) we can generate downstream signed keys which do not have to
reside in the Kernel keyring but can be supplied externally.

First we generate a key pair as above.

```
$ crinit-genkeys.sh -o vendor.key
$ crinit-genkeys.sh -k vendor.pub -o vendor.pub
```

Then we sign the public key with our root key which can then be placed e.g. on disk in the target system.

```
$ crinit-sign.sh -k crinit-root.key -o vendor.sig vendor.pub
```

The key generation step could for example be done by a third-party vendor wishing to supply crinit task configurations
for its software. The upstream system integrator would do the signing of their public key. From then on the third-party
vendor can supply trusted configuration files.

## Sign configuration files

The signing of configuration files can be done either with the root key or with downstream signed keys. The command is
the same as above.

```
$ crinit-sign.sh -k some-key.key -o task.sig task.crinit
```

# Note on the key type used in `crinit-genkeys.sh`

In recent versions, OpenSSL can also generate special `rsa-pss` type keys instead of the more general `rsa` type. The
difference between them lies in metadata. While `rsa` keys have no limitations on their usage, `rsa-pss` keys may
contain flags and settings restricting them to pre-configured signature usage only. These are respected by OpenSSL
tooling.

While this seems more appropriate to the use-case, [a bug in OpenSSL](https://github.com/openssl/openssl/issues/17167)
prevents `rsa-pss` keys from passing the OpenSSL RSA key check command. The script `crinit-sign.sh` uses that to make
sure the given key is a valid 4096-bit RSA key. The bug was fixed on March 22, 2022 but the fix has not yet made its
way downstream to current (July 2023) stable distributions.
