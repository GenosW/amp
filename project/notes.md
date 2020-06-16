# Project 2: Bounded-timestamp register locks

## Algorithms

* Szymanski’s solution (Boleslaw K. Szymanski: A simplesolution to Lamport’s concurrent programming problem with linear wait. ICS 1988: 621-626)
* Jayanti et al.’s solution (Prasad Jayanti, King Tan, Gregory Friedland, Amir Katz: Bounding Lamport’s Bakery Algorithm. SOFSEM 2001: 261-270)
* Aravind’s solution (Alex A. Aravind: Yet Another Simple Solution for the Concurrent Programming Control Problem. IEEE Trans. Parallel Distrib. Syst. 22(6): 1056-1063, 2011)
* Black-white Bakery aka "Taubenfeld" (Gadi Taubenfeld: The Black-White, Bakery Algorithm and Related Bounded-Space, Adaptive, Local-Spinning and FIFO Algorithms. DISC 2004: 56-70)
* Lamport’s Bakery (lecture version or original)

## Tasks

* Implement Taubenfeld, Lamport, and two out of the other three; verify (make plausible) with performance counters
and asertions that time stamps are within bounds.

* Which is better? For baseline performance, compare to the
following locks: pthreads or native C11 locks, simple test-and-set
lock, simple test-and-test-and-set lock

* Challenge: Memory behavior. Ensure that memory (register)
updates become visible in required order!

## 1) Lamport's Bakery

A mutual exclusion with stronger fairness guarantees (firstcome-first-served, FIFO, …)

Idea: Take a ticket that is larger than the ones already in the bakery (or having been served); wait until my ticket is smallest.

## Nebulac (Nebula Compute node)

Architecture:        x86_64
CPU op-mode(s):      32-bit, 64-bit
Byte Order:          Little Endian
Address sizes:       43 bits physical, 48 bits virtual
CPU(s):              64
On-line CPU(s) list: 0-63
Thread(s) per core:  1
Core(s) per socket:  32
Socket(s):           2
NUMA node(s):        8
Vendor ID:           AuthenticAMD
CPU family:          23
Model:               1
Model name:          AMD EPYC 7551 32-Core Processor
Stepping:            2
CPU MHz:             1196.897
CPU max MHz:         2000,0000
CPU min MHz:         1200,0000
BogoMIPS:            3992.33
Virtualization:      AMD-V
L1d cache:           32K
L1i cache:           64K
L2 cache:            512K
L3 cache:            8192K
NUMA node0 CPU(s):   0,8,16,24,32,40,48,56
NUMA node1 CPU(s):   2,10,18,26,34,42,50,58
NUMA node2 CPU(s):   4,12,20,28,36,44,52,60
NUMA node3 CPU(s):   6,14,22,30,38,46,54,62
NUMA node4 CPU(s):   1,9,17,25,33,41,49,57
NUMA node5 CPU(s):   3,11,19,27,35,43,51,59
NUMA node6 CPU(s):   5,13,21,29,37,45,53,61
NUMA node7 CPU(s):   7,15,23,31,39,47,55,63
Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ht syscall nx mmxext fxsr_opt pdpe1gb rdtscp lm constant_tsc rep_good nopl nonstop_tsc cpuid extd_apicid amd_dcm aperfmperf pni pclmulqdq monitor ssse3 fma cx16 sse4_1 sse4_2 movbe popcnt aes xsave avx f16c rdrand lahf_lm cmp_legacy svm extapic cr8_legacy abm sse4a misalignsse 3dnowprefetch osvw skinit wdt tce topoext perfctr_core perfctr_nb bpext perfctr_llc mwaitx cpb hw_pstate sme ssbd sev ibpb vmmcall fsgsbase bmi1 avx2 smep bmi2 rdseed adx smap clflushopt sha_ni xsaveopt xsavec xgetbv1 xsaves clzero irperf xsaveerptr arat npt lbrv svm_lock nrip_save tsc_scale vmcb_clean flushbyasid decodeassists pausefilter pfthreshold avic v_vmsave_vmload vgif overflow_recov succor smca


