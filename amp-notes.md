# AMP

## Exercises

Download exercises from link on homepage.

Exercises are from book version 2.

Homework 1:

    - Done: 6, 7, 8, 9, 10, 11, 12, 13, 15, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 46 from the Herlihy/Shavit book.

    - Deadline: 02.04.2020, 23:59

    - Submitted: 02.04.2020, 20:00

Homework 2:

    - Done: 21, 22, 23, 24, 27, 32, 52 (hint: see 58), 53, 54, 59, 65(hint: see 52) from the Herlihy/Shavit book.

    - Deadline: 09.04.2020, 23:30

    - Submitted: 09.04.2020, 20:00

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
