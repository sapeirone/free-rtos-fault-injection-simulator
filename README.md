# FreeRTOS Fault Injector Simulator

![CMake build process](https://github.com/peiro98/free-rtos-fault-injection-simulator/actions/workflows/cmake.yml/badge.svg)

FreeRTOS Fault Injector Simulator is a C based project that aims at producing statistics on custom Single Event Upsets generated onto the kernel structures of a FreeRTOS instance of execution.  
The user can specify, in a .cvs file, the characteristics of multiple injection campaigns, among which are number of instances to be tested, the kernel structures to be hit and the injection time.  
The program runs all the campaigns and at the end produces a table of results, classifying executions in five categories: Silent, Delay, Error, Hang and Crash executions.  

## Installation

As a first step, clone or download this repository in an empty folder.  
The simulator can be compiled and run both on POSIX and Windows operating systems.  
Be sure to run the following scripts from the root directory of the project.  
On POSIX, the simulator can be compiled by running the [compile_posix.sh](compile_posix.sh) shell script.  
On Windows, the simulator can be compiled by running the [compile_win32.bat](compile_win32.bat) batch script.  

## Usage

When not specifically stated, all terminal/shell commands should be run from the root directory of the project.
The simulator can be run in multiple configurations.

In order to print a list of all the possible injeciton targets in the kernel's structures, use the command:
```bash
./sim.exe --list
```

In order to run a golden execution, step required in order to compute the injection statistics, use the command:
```bash
./sim.exe --golden
```

In order to run a single injection at a specific time and on a specific bit, use the command:
```bash
./sim.exe --run <targetStructureName> <timeInjection> <offsetByte> <offsetBit>
```
This command is also used internally to generate all instances of an injection campaign.

In order to run one or multiple injection campaigns, use the command:
```bash
./sim.exe --campaign <inputFileName>.csv
```
The csv input file supports the insertion of comment lines by prepending a "#" character at the beginning of the line.

## Example
An example of the output produced by small injection campaigns on different targets ([input.csv](input.csv)).

| Target                         |  Time (ns) |   nExecs |   Silent % |    Delay % |    Error % |     Hang % |    Crash % |
|--------------------------------|------------|----------|------------|------------|------------|------------|------------|
| uxTopReadyPriority             |   20000000 |       50 |      0.00% |      4.00% |      0.00% |      4.00% |     92.00% |
| xNumOfOverflows                |   20000000 |       50 |     88.00% |     12.00% |      0.00% |      0.00% |      0.00% |
| xDelayedTaskList2              |   20000000 |       50 |     76.00% |     24.00% |      0.00% |      0.00% |      0.00% |
| xIdleTaskHandle                |   20000000 |       50 |      0.00% |      0.00% |      0.00% |      0.00% |    100.00% |
| xSchedulerRunning              |   20000000 |       50 |     66.00% |     32.00% |      0.00% |      0.00% |      2.00% |
| xTimerTaskHandle               |   20000000 |       50 |     92.00% |      8.00% |      0.00% |      0.00% |      0.00% |
| xTimerQueue                    |   20000000 |       50 |      0.00% |      0.00% |      0.00% |      0.00% |    100.00% |
| xNextTaskUnblockTime           |   20000000 |       50 |     62.00% |      4.00% |      0.00% |     32.00% |      2.00% |
| xTasksWaitingTermination       |   20000000 |       50 |     58.00% |     14.00% |      0.00% |      0.00% |     28.00% |
| uxDeletedTasksWaitingCleanUp   |   20000000 |       50 |      0.00% |      0.00% |      0.00% |      0.00% |    100.00% |
| xActiveTimerList1              |   20000000 |       50 |     62.00% |     24.00% |      0.00% |      0.00% |     14.00% |
| *pxCurrentTimerList            |   20000000 |       50 |     68.00% |     20.00% |      0.00% |      0.00% |     12.00% |
| pxCurrentTCB.pxTopOfStack      |   20000000 |        5 |      0.00% |      0.00% |      0.00% |      0.00% |    100.00% |
| pxCurrentTCB.uxPriority        |   20000000 |        5 |      0.00% |      0.00% |      0.00% |      0.00% |    100.00% |
| pxCurrentTCB.ucNotifyState[2]  |   20000000 |        5 |     20.00% |     80.00% |      0.00% |      0.00% |      0.00% |
| pxReadyTasksLists[0][0]        |   20000000 |        5 |     20.00% |      0.00% |      0.00% |      0.00% |     80.00% |


## Contributing
Contribution to the project is welcome, although the project won't be maintained in the future by the development team.

## Authors and Acknowledgement

The project has been developed by Alessandro Franco, Simone Alberto Peirone and Marzio Vallero and supervised by professor A. Savino.  
The project amounts as a part of the [Computer Engineering Masters Degree](https://didattica.polito.it/pls/portal30/sviluppo.offerta_formativa.corsi?p_sdu_cds=37:18&p_lang=EN) exam [System and Device Programming](https://didattica.polito.it/pls/portal30/gap.pkg_guide.viewGap?p_cod_ins=01NYHOV&p_a_acc=2021&p_header=S&p_lang=EN), taught by professors S. Quer, G. Cabodi and A. Vetrò, during the academic year 2020/2021 at the [Politecnico di Torino](https://www.polito.it/).

## License
For right to use, copyright and warranty of this software, refer to this project's [License](License.md).
