Astra Scheduler Logging:
------------------------
Astra Scheduler maintains all the tasks in various run and idle queues.
This logging app takes a snapshot of the tasks in the queue at a fixed periodic rate and their associated scheduling parameters tracked in tracelog registers.
Since Tracelog register operations have negligible overheads, this logging would not have any scheduler impact.
The parameters that are being currently tracked are : CPU Core Id, Task ID, Task type (Real Time/ Non Real Time), cumulative CPU Percentage, Task priority and load.
As of this writing, the scheduler deposits this data into tracelog registers every 200 ms.

Host Utility "astra_top":
-------------------------
Online Mode:
Host (Linux) side utility “astra_top” uses the tracelog driver to read the tracelog device file for data deposited by the scheduler.
It then parses this data, averages out CPU percentage and presents the information in tabular format (just like top on Linux).

Offline Mode (File Input):
The utility also supports offline file mode where user can provide a file input with tracelog dumps and utility would average out a set of entries and present the data on output terminal.

Build astra_top( for 64-bit platform):
--------------------------------------
plat 97271 B0
export TZ_ARCH=Arm64
cd astra/linux/utils/astra_top
make

Build astra_top( for 32-bit platform):
--------------------------------------
plat 97445 D1
export TZ_ARCH=Arm32
cd astra/linux/utils/astra_top
make

Run astra_top:
--------------
- Load bl31.bin, astra.bin and kernel on the board.
- mount -o nolock <path> /mnt/nfs
- cd /mnt/nfs/<view path>/obj<TZ_ARCH>/astra/linux/bcm_astra/
- insmod bcm_astra.ko
- open a telnet session to board
- cd /mnt/nfs/<view path>/obj<TZ_ARCH>/astra/linux/utils/astra_top/
- astra_top

astra_top User Options(Explanation followed by Snapshot):
---------------------------------------------------------
1. “astra_top” utility gives options to filter out tasks based on CPU Core Id, task ID and task type, specify output refresh duration, offline file and output style.
Any of the task filtering options can be combined with duration, offline file and style option. However, only one option between core ID and task ID can be given at a time.
-h option lists the options as shown below.
===============================================================================================================
     Usage:astra_top [-option] [argument]
    Option:-h  |  --help                  show help information
           -c  |  --core <coresFlag>,     monitor selected CPU core/s,
                                          coresFlag = decimal number by setting bit/s at position = core ID/s
           -t  |  --task <taskID>,        monitor only one task with id = taskID
           -f  |  --filtertask <RT/NRT>,  monitor real time tasks if RT, monitor non real time tasks if NRT
           -d  |  --duration <duration>,  log entries after every duration(in secs)
           -i  |  --infile <fileName>,    input file with TraceLog entries
           -s  |  --scroll,               output in scrolling style
================================================================================================================

2. astra_top option for specific Core:
With “-c” option followed by cores bitmask value, user can specify core/s to be monitored. To select a core set bit at position = cpu_core_id. The resulting decimal number is given with -c as a bitmask value.
Ex: coresFlag = 1 to select core 0,
    coresFlag = (11) in binary = 3 to select cores 0 and 1
By default all cores are selected.
Output is grouped according to CPU Core ID. Each group has CPU Core ID along with total number of tasks on that core as its heading. Table fields are:
-       Task ID   : ID of active task on that core.
-       Priority  : Valid for CFS (Non real time) tasks. Lists the priority of the CFS task. For real time tasks RT is displayed.
-       Load      : Valid for RT tasks. Lists the max load (MHz) requirement of the RT task.
-       CPU %     : Average CPU consumption of the task in the last between two refreshes.
-       Status    : State of the task = 1 (for “Ready”) or 0 (for “Wait”).
===========================================================================
# astra_top -c 5
CPU 0 : Total number of tasks: 10
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 0       50       NA     41.375     1
 1       50       NA     0          0
 5       50       NA     0          0
 9       RT       50     3.1        1
 10      RT       100    7.122      1
 11      RT       200    14.45      1
 14      10       NA     8.36       1
 15      10       NA     8.03417    1
 16      10       NA     8.12222    1
 17      10       NA     8.33571    1

CPU 2 : Total number of tasks: 2
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 3       50       NA     74.13      1
 6       RT       200    25.4667    1
===========================================================================

3. astra_top option for specific Task:
With “-t” option followed by task id, user can monitor a specific task.
===========================================================================
# astra_top -t 10
Task ID: 10
 ------ ------ ---------- ------
 CPU ID Load   CPU %      Status
 ------ ------ ---------- ------
 0      100    7.11714    1
===========================================================================

4. astra_top option for specific Task type (RT or NRT):
With filtering option “-f” followed by RT/NRT, user can choose to display either RT(Real Time) or NRT(Non Real Time) tasks. Combined with “-c” option can list down RT/NRT tasks on specific core.
Below is the dump of NRT tasks on cores 0 and 1.
===========================================================================
# astra_top -c 3 -f NRT
CPU 0 : Total number of tasks: 10
 ------- -------- ---------- ------
 Task ID Priority CPU %      Status
 ------- -------- ---------- ------
 0       50       40.9909    1
 1       50       0          0
 5       50       0          0
 14      10       8.10875    1
 15      10       8.25       1
 16      10       8.24375    1
 17      10       8.42875    1

CPU 1 : Total number of tasks: 4
 ------- -------- ---------- ------
 Task ID Priority CPU %      Status
 ------- -------- ---------- ------
 2       50       74.7375    1
===========================================================================

5. astra_top option to specify duration between two refreshes:
With option -d followed by duration in secs, user can choose the interval between two refreshes. Default duaration is 1 sec.
Below is the dump of task on all cores with refresh interval as 2 secs.
===========================================================================
# astra_top -d 2
CPU 0 : Total number of tasks: 10
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 0       50       NA     41.11      1
 1       50       NA     0          0
 5       50       NA     0          0
 9       RT       50     3.9        1
 10      RT       100    7.21167    1
 11      RT       200    14.2614    1
 14      10       NA     8          1
 15      10       NA     8          1
 16      10       NA     8          1
 17      10       NA     8          1

CPU 1 : Total number of tasks: 4
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 2       50       NA     74.3257    1
 7       RT       50     3.14286    1
 8       RT       100    7.11571    1
 12      RT       200    14.4125    1

CPU 2 : Total number of tasks: 2
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 3       50       NA     75.1122    1
 6       RT       200    25.3778    1

CPU 3 : Total number of tasks: 2
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 4       50       NA     75.2471    1
 13      RT       300    25.1286    1
===========================================================================

6. Offline or File input option:
With “-i” option followed by file name, user can give an offline file with Tracelog entries and see the parsed Task details on output terminal.
Below is the dump of tasks on cores 0 and 1 from offline file 'task_dump_1s.txt'.
===========================================================================
# astra_top -c 3 -i task_dump_1s.txt
CPU 0 : Total number of tasks: 7
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 9       RT       50     3.14       0
 10      RT       100    6.1        0
 11      RT       200    13.3       0
 14      10       NA     7.3        0
 15      10       NA     7.3        0
 16      10       NA     8          0
 17      10       NA     7.3        0

CPU 1 : Total number of tasks: 3
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 6       RT       200    13.1       0
 7       RT       50     3.5        0
 8       RT       100    6.13       0
===========================================================================

7. Astra_top option to specify output display style:
If -s option is specified the dump in each refresh cycle is printed one after the other instead of starting from the same line in every refresh cycle(like top). Default is top like output.
# astra_top -s -c 2
Below is the dump of all tasks on core 1 in a scrolling fashion
===========================================================================
CPU 1 : Total number of tasks: 4
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 2       50       NA     73.7429    1
 7       RT       50     3.21429    1
 8       RT       100    6.97286    1
 12      RT       200    14.0457    1

CPU 1 : Total number of tasks: 3
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 2       50       NA     81.5       1
 8       RT       100    7.12       1
 12      RT       200    14.3       1

CPU 1 : Total number of tasks: 4
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 2       50       NA     75.6067    1
 7       RT       50     3.35       1
 8       RT       100    6.912      1
 12      RT       200    14.115     1

CPU 1 : Total number of tasks: 4
 ------- -------- ------ ---------- ------
 Task ID Priority Load   CPU %      Status
 ------- -------- ------ ---------- ------
 2       50       NA     74.7156    1
 7       RT       50     3.2875     1
 8       RT       100    6.912      1
 12      RT       200    14.115     1
===========================================================================
