# Architecture Design Record - Choice of signature algorithm for signed configurations

## Problem

Crinit shall support signed (task/global) configurations. Choosing the right signature algorithm is critical for
security and long-term usability.

## Influencing Factors

* We only take a look at algorithms recommended by the BSI in Technical Guideline TR&ndash;02102&ndash;1 [^1]

## Assumptions

* The terms RSASSA-PSS and RSA-PSS are used interchangeably in literature and functionally describe the same thing

## Considered Alternatives

### Option 1 - RSA(SSA)-PSS

Combines classical RSA signatures with PSS (Probabilistic Signature Scheme). The addition of PSS made it possible to
mathematically prove that the cryptographic strength of the resulting signature is tightly related to the strength of
the underlying RSA scheme. [^3]

#### Pros

* well-understood and widely used
* widely implemented in libraries
* secure, given large enough key length and hash function
* also approved by NIST as of FIPS 186-5 [^4]

#### Cons

* RSA is considered computationally expensive
  - *But*: RSA is only applied to the hash value, so performance of hash function is likely dominant

### Option 2 - DSA

An algorithm distinct from RSA published in 1991 by NIST for use in US governmental agencies. Usage is otherwise
similar to RSA-PSS (public/private key cryptography with an underlying hash function).

#### Pros

* widely implemented

#### Cons

* not considered secure enough anymore by NIST as of the recently released FIPS 186-5. [^4]
* speed similar to RSA with current key lengths
  - *But*: see above

### Option 3 - ECDSA

A further development of the original DSA algorithm using elliptic-curve cryptography, enabling the use of shorter key
lengths for equivalent security.

#### Pros

* widely implemented in libraries
* considered secure, given large enough key length, hash function, and a suitable elliptic curve function
* also approved by NIST as of FIPS 186-5 [^4]
* theoretically faster than RSA due to shorter necessary key length
  - *But:* As stated above, speed of cryptography not really a concern, also practical implementation may differ due to
    hardware acceleration.

#### Cons

* not quite as widely used as RSA
* political concerns have been voiced in the wake of the NSA scandal which revealed a backdoor in an elliptic curve
  random number generator (`Dual_EC_DRBG`), casting general (but as of yet unproven) doubts on the elliptic curve
  functions published by NIST
  - e.g. SSH has moved towards the ed25519 algorithm using a non-NIST elliptic curve due to this

### Option 4 - Merkle signatures

Merkle signatures are different mathematically from the other options. They only rely on the cryptographic security of
a hash function and a random number generator but no asymmetric mathematical problem.

#### Pros

* Secure against quantum computer attacks [^1]

#### Cons

* Public keys can only be securely used a limited number of times. The number of possible uses is proportional with the
  computational effort required to generate the key. [^1]
* Not widely used
* cutting-edge, implementation in crypto libraries still very new if present at all [^2]
* Not (yet) mentioned in the NIST DSS. [^4]

## Decision

Option 1 is taken.

**NOTE:** The BSI recommends at least SHA256 and (for usage "beyond 2023") an RSA key length of at least 3000 bits
          (meaning RSA-4096 should be used in practice). The padding function and salt shall be chosen according to
          the current PKCS standard.

## References

[^1]: *BSI TR&ndash;02102&ndash;1: Cryptographic Mechanisms: Recommendations and Key Lengths* (version 2023-01).
      Federal Office for Information Security (BSI).
      [[PDF Link]](https://www.bsi.bund.de/SharedDocs/Downloads/EN/BSI/Publications/TechGuidelines/TG02102/BSI-TR-02102-1.pdf?__blob=publicationFile&v=6)
[^2]: Butin, Denis, Julian Wälde, and Johannes Buchmann. 2017. “Post-Quantum Authentication in OpenSSL with Hash-Based
      Signatures.” In *2017 Tenth International Conference on Mobile Computing and Ubiquitous Network (ICMU)*, 1–6.
      [[PDF Link]](http://www.amphawa.eu/data/icmu-paper.pdf)
[^3]: Bellare, Mihir, and Phillip Rogaway. 1996. “The Exact Security of Digital Signatures &ndash; How to Sign with RSA
      and Rabin.” In *Advances in Cryptology — EUROCRYPT ’96*, edited by Ueli Maurer, 399–416.
      Berlin, Heidelberg: Springer Berlin Heidelberg. [[PDF Link]](https://www.cs.ucdavis.edu/~rogaway/papers/exact.pdf)
[^4]: *FIPS 186-5: Digital Signature Standard (DSS)*. 2023. National Institute of Standards and Technology (NIST).
      [[PDF Link]](https://doi.org/10.6028/NIST.FIPS.186-5)
