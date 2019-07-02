
# DAMPI

### Design

MPI’s most common use case is in High-Performance Computing (HPC). As many HPC tasks are time-critical (e.g. weather prediction), small optimizations can be extremely valuable. 

The MPI specification — and presumedly most implementations — allow MPI clusters to be distributed across cores within a machine, machines within a network (LAN), and networks within other networks (WAN). This means that there can be some degree of variability in the speeds of the links between processes. Also, It is usually the case that different MPI nodes will communicate with each other at different frequencies. Some might be coupled together with data dependencies, while others remain completely independent. Plainly put, the ones that talk a lot should be closer together (paired by faster links) to improve performance.

DAMPI is a library for MPI applications that allows them to dynamically reconfigure themselves based on the communication delays in the links between compute nodes. DAMPI starts by profiling the system that it will run on, running a series of ping-pong trials to measure communication delays between nodes. For 3 nodes, the profiling system might output something like this:
```
0 <---> 1: 0.000101
0 <---> 2: 0.000046
1 <---> 2: 0.000008
```
After it has found latencies between nodes, it determines which node is the best-connected node (the one with the least total delay across its edges). This node is be used to store some centralized runtime information, such as inter-node communication frequencies, and some information about migration requests (more on this in a second). All nodes in the cluster update the information held by the best-connected node via an MPI window that is setup at startup. For instance, a node will increment a counter in a frequency array on the best-connected node every time it performs a send operation.

Nodes use the information stored at the best-connected node regularly to determine whether or not they should perform a migration. At regular intervals nodes enter a function (`DAMPI_Airlock`) where they make this decision. Every assignment of ranks to processes has a value, defined as:
 
![config value](sum.png)

Nodes compute this value for all configurations that could be acheived with a single migration. If it detects a configuration that has a higher value than the current one, it will post a migration request at the best-connected node. When all the other nodes enter the migration-checking zone (`DAMPI_Airlock`), they see that another node has requested a swap and act accordingly (described below).

At startup, all nodes register which function they will be running during their lifetime in the MPI pool. They also indicate the sizes of their 'suitcases' which hold all of a rank's state that needs to be carried across processes. During a migration, all nodes update their rank->process mapping so that they know where each rank is running (for the purpose of sends and recvs). The migrating nodes exchange their 'suitcase', and start running their migrating partner's function.

In order to send and recv, nodes use thir rank->process mapping. For example, if rank 0 wants to send a message to rank 1, it needs to first look up where rank 1 is running before it can perform the send. This part is best explained by the code:

 ```
 int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
  if (info->proc != dest) {
    int inc = 5; 
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->freqwin);
    MPI_Accumulate(&inc, 1, MPI_INT, info->bnode, eoffset(dest, info->rank), 1, MPI_INT, MPI_SUM, info->freqwin);
    MPI_Win_unlock(info->bnode, info->freqwin);
  }
  return MPI_Send(buf, count, datatype, info->rankprocs[dest], tag, comm);            
} 
 ```
The rank updates the frequency of its communication with `dest` at the best-connected node, and then calls `MPI_Send` with `rankprocs[dest]`, which tells it where dest is running in the cluster.

To test the system, I implemented a prime sieve both with and without the DAMPI library. I immediately realised that profiling the system during the runtime of an MPI application is pointless since the system is unlikely to change during every execution. So I separated the delay profiling system out - its cost can be amortized across all executions on the same system.

### Results

Obviously, since DAMPI adds some synchronization of processes and an exponential-time algorithm to check for migrations, there are significant overheads. Without DAMPI, finding 5 primes on a single machine with tiny latencies took an average of `0.154s`, while it took `0.190s` with DAMPI. 

I added some artificial latencies to simulate a situation where the MPI communication is distributed across links with variable-sized (and larger) delays. With latencies randomly distributed between 0 and 1 second, generating 10 primes took `22.064s` on average without DAMPI, and `18.628s` on average with DAMPI. Obviously, the more time-intensive the application, and the more delayed the links, the more advantageous migrations become. For 11 primes, the sieve without DAMPI took on average `44.311s` and the one with DAMPI took `27.856s`. 

There are obviously unlucky cases where the non-migratable application configures itself nearly perfectly. In the case of the prime sieve and presumably many other applications, the communication patterns between nodes is likely to remain constant even if the actual data does not (e.g. The generator node always sends messages to rank 1 to filter new numbers). I decided to also create an example where the optimal configuration is determined in one run of the application, and is used by subsequent runs. To find 11 numbers as above, this static optimally configured verison took `20.957s`. 

There are also cases (which may not be that realistic) where some edges are very slowly. An example might be two or more clusters spread out across continents. To simulate this situation, I made a few edges have a 1 second delay, while leaving all of the rest without artificial delays. To generate 12 primes, the sans-DAMPI implementation took `23.378s`, the dynamic DAMPI implementation took `2.501s` and the static one took `0.296s`.


### Improvements

There are a few improvements that could be made to DAMPI. 

Firstly, one of the biggest time drains that DAMPI introduces is in the synchronization of processes during the calls to `DAMPI_Airlock`. A method to allow migrations to occur with less process synchronization might be advantageous. At one point in the implementation of DAMPI, nodes were called to the airlock whenever a migration was requested. This required adding timeouts to sends and receives and would complicate library usage, so I dropped it (`commit 491afd966e6af74b04a898f84c90ae94fd624c1e`), but it might have in fact been the way to go as far as performance is concerned.

Secondly, a smarter method of checking for migrations would likely yield a performance improvement. The current method checks every possible configuration at each step, which is exponential in the number of nodes when all the node's operations are considered. Some sort of heuristic alternative could work better - for instance, where possible, putting the two most frequently communicating nodes on the fastest link, the second most frequently communicating nodes on the second fastest, and so on.



### PROPOSAL

MPI’s most common use case is in High-Performance Computing (HPC). As many HPC tasks are time-critical (e.g. weather prediction), small optimizations can be extremely valuable. 

The MPI specification — and presumedly most implementations — allow MPI clusters to be distributed across cores within a machine, machines with a network (LAN), and networks within other networks (WAN). This means that there can be some degree of variability in the speeds of the links between processes. Also, It is usually the case that different MPI nodes will communicate with each other at different frequencies. Some might be coupled together with data dependencies, while others remain completely independent. Plainly put, the ones that talk a lot should be closer together (paired by faster links) to improve performance.


### EXAMPLE

Three processes A,B,C

Two computers: computer1, computer2

Initially computer1 runs A,B and computer2 runs C.

The profiler discovers that processes running on computer1 can communicate faster than processes running separately on computer1 and computer2. During runtime, it discovers that A and C communicate far more frequently than A and B. C is migrated (swapped with B) to computer1 during runtime. Then A and B start communicating more frequently, and the reverse occurs.


### MILESTONES

1. System profiling

Implement a mechanism to learn the relative delays associated with all of the connections in an MPI system. This will involve sending a series of test messages between all pairs, and measuring delay. 

2. Program profiling

Implement a mechanism to discover the frequencies with which pairs of nodes communicate in a given MPI program. This will involve adding some logic to MPI_Send that will keep track of how often nodes communicate.

3. Dynamic process migration (sans I/O)

Implement light-weight process migration. Most current process migration methods involve writing checkpoint files to a file system, modifying them, and then restarting the system according to the modified checkpoints. Dealing with I\O makes that method of migration impractical for use during runtime. The process migration that I implement will make a series of strong assumptions about the way that processes run in an MPI environment to allow process migration to occur during runtime, via the MPI API as opposed to with file I/O.

4. Integration

Use the profile of the system and the program, and the process migration framework to create an MPI environment where a program can change its configuration dynamically to optimize the latencies of its interprocess communications.



