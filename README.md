##PROPOSAL

MPI’s most common use case is in High-Performance Computing (HPC). As many HPC tasks are time-critical (e.g. weather prediction), small optimizations can be extremely valuable. 

The MPI specification — and presumedly most implementations — allow MPI clusters to be distributed across cores within a machine, machines with a network (LAN), and networks within other networks (WAN). This means that there can be some degree of variability in the speeds of the links between processes. Also, It is usually the case that different MPI nodes will communicate with each other at different frequencies. Some might be coupled together with data dependencies, while others remain completely independent. Plainly put, the ones that talk a lot should be closer together (paired by faster links) to improve performance.


###EXAMPLE

Three processes A,B,C

Two computers: computer1, computer2

Initially computer1 runs A,B and computer2 runs C.

The profiler discovers that processes running on computer1 can communicate faster than processes running separately on computer1 and computer2. During runtime, it discovers that A and C communicate far more frequently than A and B. C is migrated (swapped with B) to computer1 during runtime. Then A and B start communicating more frequently, and the reverse occurs.


###MILESTONES

1. System profiling

Implement a mechanism to learn the relative delays associated with all of the connections in an MPI system. This will involve sending a series of test messages between all pairs, and measuring delay. 

2. Program profiling

Implement a mechanism to discover the frequencies with which pairs of nodes communicate in a given MPI program. This will involve adding some logic to MPI_Send that will keep track of how often nodes communicate.

3. Dynamic process migration (sans I/O)

Implement light-weight process migration. Most current process migration methods involve writing checkpoint files to a file system, modifying them, and then restarting the system according to the modified checkpoints. Dealing with I\O makes that method of migration impractical for use during runtime. The process migration that I implement will make a series of strong assumptions about the way that processes run in an MPI environment to allow process migration to occur during runtime, via the MPI API as opposed to with file I/O.

4. Integration

Use the profile of the system and the program, and the process migration framework to create an MPI environment where a program can change its configuration dynamically to optimize the latencies of its interprocess communications.



