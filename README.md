# FreeRTOS Fault Injector Simulator

FreeRTOS Fault Injector Simulator is a C based project that aims at producing statistics on custom Single Event Upsets generated onto the kernel structures of a FreeRTOS instance of execution.  
The user can specify, in a .cvs file, the characteristics of multiple injection campaigns, among which are number of instances to be tested, the kernel structures to be hit and the injection time.  
The program runs all the campaigns and at the end produces a table of results, classifying executions in five categories: Silent, Delay, Error, Hang and Crash executions.  

## Installation

As a first step, clone or download this repository in an empty folder.  
The simulator can be compiled and run both on POSIX and Windows operating systems.  
Be sure to run the following scripts from the root direcory of the project.  
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
This command is also used internally to generate all instances of an injeciton campaign.

In order to run one or multiple injeciton campaigns, use the command:
```bash
./sim.exe --campaign <inputFileName>.csv
```
The csv input file supports the insertion of comment lines by prepending a "#" character at the beginning of the line.

## Contributing
Contribution to the project is welcome, although the project won't be maintained in the future by the development team.

## Authors and Acknowledgement

The project has been developed by Alessandro Franco, Simone Alberto Peirone and Marzio Vallero and supervised by professor A. Savino.  
The project amounts as a part of the [Computer Engineering Masters Degree](https://didattica.polito.it/pls/portal30/sviluppo.offerta_formativa.corsi?p_sdu_cds=37:18&p_lang=EN) exam [System and Device Programming](https://didattica.polito.it/pls/portal30/gap.pkg_guide.viewGap?p_cod_ins=01NYHOV&p_a_acc=2021&p_header=S&p_lang=EN), taught by professors S. Quer, G. Cabodi and A. Vetrò, during the academic year 2020/2021 at the [Politecnico di Torino](https://www.polito.it/).

## License
For right to use, copyright and warranty of this software, refer to this project's [License](License.md).
