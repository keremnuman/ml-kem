# Post-Quantum ML-KEM Implementation in C

Minimal implementation of a **post-quantum key encapsulation mechanism (KEM)** in C, based on the **FIPS 203 (ML-KEM)** standard and supporting primitives defined in **FIPS 202**.

## Features
* ML-KEM (Kyber) implementation aligned with FIPS 203
* SHA3 and SHAKE functions based on FIPS 202

## Project Structure
algortihms.h
algortihms.c
fips202.h
fips202.c
main.c

## Build
Use a standard C compiler
```
gcc -o ml-kem main.c fips202.c algorithms.c
```

## Usage
Flow:
1. Key generation
2. Encapsulation (generate shared secret + ciphertext)
3. Decapsulation (recover shared secret)

## Notes
* Side-channel protections (constant-time guarantees, masking, etc.) are not implemented yet.
* Parameter sets follow FIPS 203, but may be simplified for clarity.

## References
* FIPS 203 – Module-Lattice-Based Key-Encapsulation Mechanism Standard
* FIPS 202 – SHA-3 Standard: Permutation-Based Hash and Extendable-Output Functions
* https://github.com/pq-crystals/kyber
* https://csrc.nist.gov/Projects/post-quantum-cryptography/pqc-archive

