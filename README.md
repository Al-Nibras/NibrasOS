# NibrasOS
Operating System Micro Kernel designed for IA-32 Arch.

If you're not familiar with Kernel Development, please close this page and stop wasting your time!

TO BE FIXED:
* Process VAS creation
* TSS data gets lost when executing fork() or sys_fork()
* Reclaiming of usermode pocesses' VM pages

TO BE ADDED:
* More drivers (Networking, USB, SSD using DMAC)
* Device Manager
* FileSystems (primary: HFS, exFAT)
* Shells (at least bash, sh, csh)
* Power Managment
* IPC (basically: MP and Signalling)
* More C-stdlib functionalities
* Floating-point (SSE*)
* VM mmap
* VM86

TO BE STABLIZED:
* Process VAS security
* fork() syscall
* Kheap Manager
* Overall Structure (Higher-Half Kernel VM)
* Scheduling Algorithm: very basic RR is currently in use, the official algorithm used in NibrasOS is not within the source code, it is a new Scheduling Alg. that would be released once its scientific paper gets published

TO BE OPTIMIZED:
* Multiprocessing synchronization
* memcpy (using XMMs)
