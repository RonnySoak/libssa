# libssa

## Introduction

libssa is a library for SIMD accelerated optimal sequence alignments. This is project is currently developed as part of my master's thesis.

Its aim is to provide functionality for aligning query sequences against a sequence database. The main focus is here on:
* optimal global sequence alignments, using the Needleman-Wunsch algorithm
* optimal local sequence alignments, using the Smith-Waterman algorithm
* acceleration of the alignments using multiple threads and SIMD intructions
* a small public API

A main part of my thesis is to show the benefits of the AVX2 intructionset, for the speedup of optimal sequence alignments. [AVX2](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions) is similar to [SSE](https://en.wikipedia.org/wiki/Streaming_SIMD_Extensions) a set of intructions offering SIMD (Single instruction, multiple data) processing, on modern CPUs.

In the past, it had been shown, that a significant speedup can be gained using the SSE (TODO cite). AVX2 implements wider registers: 256 bit instead of 128 bit, with SSE. The hypothesis is, that it gets twice as fast, using AVX2 instead of SSE.


## Limitations

- exit on an error
- still work in progress


## libssa license and third party licenses

The libssa code is licensed under the GNU Affero General Public License version 3.

libssa includes code (the Needleman Wunsch SSE2 implementation) derived from VSEARCH [VSEARCH](https://github.com/torognes/vsearch) by Torbjørn Rognes et al.


## References

* Rognes T (2011)
**Faster Smith-Waterman database searches by inter-sequence SIMD parallelisation.**
*BMC Bioinformatics*, 12: 221.
doi:[10.1186/1471-2105-12-221](http://dx.doi.org/10.1186/1471-2105-12-221)

* Rognes T (2014)
**VSEARCH**
doi:[10.5281/zenodo.14860](http://dx.doi.org/10.5281/zenodo.14860)