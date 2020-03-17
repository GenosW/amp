# AMP

## Exercises

Download exercises from link on homepage.

Exercises are from book version 2

## Lectures

### 1. Lecture

- Absolute speed up: SU(p)

    best speedup possible ... SU(p)= p, otherwise we found better seq alg

    in general: linear speed up is already good

- Relative speed up: $T_1(n)/T_p(n)$

- How to measure time? Worst case vs average case...

- Be careful what is presented in papers and such! (How is time measured, which type of rel/abs speed up...)

### 2. Lecture

Tseq(n) ... "work" -> number of operations for task

Tp(n) = O(Tseq(n)/p) ... linear speed up

mandatory fraction that has to be sequential and grows with problem --> Amdahl's law

similiar, but sequetnial part does not grow --> NOT Amdahl's law

Beware modern processor architectures:

- NUMA
- Write buffer
- ...

==> modern processors (often) do not provide sequential consistency
