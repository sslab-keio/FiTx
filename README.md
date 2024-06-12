# FiTx: Framework for Finger Traceable Bugs in Linux
## Overview
This repository contains the artifacts for USENIX ATC 2024 Paper:

> Keita Suzuki, Kenta Ishiguro, Kenji Kono, "Balancing Analysis Time and Bug Detection: Daily Development-friendly Bug Detection in Linux", In 2024 USENIX Annual Technical Conference (to appear), 2024

FiTx is a framework to generate bug detection checkers for "Finger traceable
bugs". It aims to explores an approach that balances the trade-off of bug
detection capabilities and short analysis time by combining the computationally
less-complex analysis techniques.

## Prerequisites
- A Linux machine (Ubuntu 20.04 recommended)
- Network access
- A decent amount of memory and disk storage to compile Linux kernel with `allyesconfig`
    - RAM: Preferably 32 GB or more
    - Disk storage: At least 100GB
- Software
    - [Optional, Strongly Recommended] Docker and docker-compose to run the experiments
        - Refer to [docker official document](https://docs.docker.com/engine/install/) for installation guide

### Experimental Setup
Our experiments are conducted using the following setup (taken from the paper):

|Setup|Version|
|:---:|:---:|
|OS                  | Ubuntu 20.04                  |
|CPU                 | 16 Core Intel Xeon CPU E5-2620|
|RAM                 | 96 GB (limited to 32 GB)      |
|LLVM                | 10.0.1                        |
|Target Linux Version| v5.15                         |
|Config              | allyesconfig                  |



## Artifact overview
The artifacts are provided as both docker image and source code. We strongly
recommend using the docker image since it deals with installing all the required
packages. For the rest of this document, we assume that you are using the docker
container. If you would like to manually build the setup, please refer to 
[Manual instructions](#manual-instructions).
The container is based on a debian-buster image.

The Dockerfile conducts the following:
1. Install relevant packages including LLVM version 10.0.1
1. Build FiTx and the detectors from source
1. Setup and mount scripts and test directories.
1. Download the experimented Linux kernel version 5.15.

### FiTx Structure

The main part of FiTx is consisted of two parts:
1. The main FiTx framework which provides various libraries to implement bug checkers
2. The bug checkers (detectors) implemented using the FiTx framework.

The structure of the respository and its explanation are as follows:
```
FiTx/
  ├ src - includes all the relevant source files for FiTx and the detectors
    ├ framework - Contains all code related to FiTx.
        ├ core         - Contains common libraries and util classes.
        ├ framework_ir - Contains code related to preserving non-optimized IRs.
        ├ frontend     - Contains code related to analyzing the code.
        ┗ include      - Contains header files.
    ┗ detectors - Contains the codes for example detectors using FiTx.
        ├ all_detector - Contains code for detector to find all available bugs.
        ├ df_detector  - Contains code for double free detectors.
        ├ ... (Ommiting example for other detectors)
        ┗ include      - Contains header files.
  ├ test - includes the test source code to conduct simple functional testing of FiTx
  ┗ scripts - includes experimental scripts as well as some configuration files
```

## Getting Started Guide
### Using Docker Container
Assuming that Docker and docker-compose is installed, run the following command
to start the container:

```
docker-compose up --build -d
```

This will install the relevant packages, build FiTx, and download Linux kernel
with the version used in the experiments within the container.

By default, docker-compose will set the container name as `FiTx`.
If you decide to change the container name, be sure to replace container name
to the one you specified throughout the examples.

### Manual instructions
If you need to manually build your own enviornment to use FiTx, please conduct
the following: 

#### Install LLVM
1. Install LLVM 10.0.1. Follow Getting Started Guilde from [llvm.org](https://llvm.org/docs/GettingStarted.html)
1. To use the scripts, install python3, pip3 and install packages in requirements.txt

#### Building FiTx
FiTx can be built using CMake.

1. Download this repository
1. Run `cmake -S [PATH_TO_FITX]/src -B [PATH_TO_FITX]/build`
1. In build directory, run build by making `make -j${nproc}`

This will build FiTx library inside `[PATH_TO_FITX]/build/framework`,
and generate the detectors in
`[PATH_TO_FITX]/build/detector/[BUG_TYPE]/lib[BUG_TYPE].so`.
For instance, the bug checker for double free will be
`build/detector/df_detector/libDFDetectorMod.so`.

In most cases, using the `all_detector` should be enough, since it will run the
detector for all the implemented checkers.

### Running FiTx detectors

FiTx framework generates a LLVM plugin to be used with `Clang` compiler for
each detector. To use each FiTx detector, simply run the Clang compiler
(build the source) with the following arguments:

```
Clang [Source File] -g -O0 -Xclang -load -XClang [PATH_TO_DETECTOR]

# Example of running `all_detector` on example.c
Clang -g -Xclang -load -XClang FiTx/build/detector/all_detector/libAllDetectorMod.so example.c 
```

If the checker finds a bug, it will generate a log to stderr. Otherwise, the
compilation should complete as it usually does without the checkers.

Please use the script `scripts/analyze.py` as it will handle adding the arguments
to Clang automatically and run the all_detector. The rest of the document will
assume you are using this script.


### Running FiTx with toysized examples
Run the following command to run FiTx on a test source code. By default, tests
are mounted to `/tmp/tests` within the container. Each test is seperated by the
type of bug by directory. Within each bug directory, there are two directories:
`/src`, which stores the testcase sources, and `expected`, which stores the expected
output of each testcase.

The test can be run using the script `/FiTx/scripts/analyze.py test`.
The script will deal with identifying the source files within each directory,
running analysis on them, and printing the analysis log as output if there are
any. Empty log indicates no bug was found.

```
# Script template. By default, container name is FiTx.
docker exec FiTx python3 /FiTx/scripts/analyze.py test PATH_TO_TEST

# Example of running double free tests
docker exec FiTx python3 /FiTx/scripts/analyze.py test /tmp/tests/double_free
```

To check if the log is correctly pointing to a bug (or not pointing to a bug),
please refer to log files in `[PATH_TO_TEST]/expected/[TEST_NAME].log` which
stores the expected log for each testcase (or check the testcase file and
manually confirm the bug). For instance, if you are testing
`/tmp/tests/double_free/src/intra_procedural_df.c`, the expected log will be
`/tmp/tests/double_free/expected/intra_procedural_df.log`.

#### Example running a test
Example code `/tmp/tests/double_free/src/inter_procedural_df.c` looks like the following:

```tests/double_free/src/inter_procedural_df.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME 100

void free_char(char* name) {
  free(name);
}

int main() {
  char *name = (char *) malloc(NAME);

  if (name == NULL)
    return -1;

  free_char(name);
  free(name); // BUG: DoubleFree here
  return 0;
}
```

When running the analysis on this, you should run 

```
# OPTION1:  Run on entire directory
docker exec FiTx python3 /FiTx/scripts/analyze.py test /tmp/tests/double_free

# OPTION2: Run on single testcase
docker exec FiTx python3 /FiTx/scripts/analyze.py test /tmp/tests/double_free/src/inter_procedural_df.c

```

This should show you the following log:
```

Running test on /tmp/tests/double_free/src/inter_procedural_df.c
Found 1 tests
[Running] /tmp/tests/double_free/src/inter_procedural_df.c
[ERROR] /tmp/tests/double_free/src/inter_procedural_df.c:18:3: --- [double free] ---
[ERROR] /tmp/tests/double_free/src/inter_procedural_df.c:18:3: [framework::Value] ValueType: 55 Array Element: -2 ({Type: i8* Field: -1}, )
  [LOG] /tmp/tests/double_free/src/inter_procedural_df.c:17:3: [Transition] init to free
  [LOG] /tmp/tests/double_free/src/inter_procedural_df.c:18:3: [Transition] free to double free
```

If this is printed, it means that the detector is correctly working, and the
bug has been identified.

### Reading the log
Each log generated by FiTx will look like the following:
```
[ERROR] /tmp/tests/double_free/src/intra_procedural_df.c:32:3: --- [double free] ---
[ERROR] /tmp/tests/double_free/src/intra_procedural_df.c:32:3: [framework::Value] ValueType: 55 Array Element: -2 ({Type: i8* Field: -1}, )
  [LOG] /tmp/tests/double_free/src/intra_procedural_df.c:28:5: [Transition] init to free
  [LOG] /tmp/tests/double_free/src/intra_procedural_df.c:32:3: [Transition] free to double free
```
The first line indicates the type of bug that was identified (`[double free]`).
The second line will indicate the type of the value (Refer to later section on details).
The remaining lines with `[LOG]` tag indicate where the state transition occurred
within each source file.

<!-- #### How to read the values -->
<!-- TODO -->

### [Optional] Compiling Linux Kernel
This section is completely optional as compiling Linux kernel will be handled
in later sections when running FiTx on Linux source. The same exact steps
will be conducted in [later](#running-fitx-on-linux) section, in `/FiTx/scripts/analyze.py` script.

Run the following command to make sure the Linux kernel can be compiled within
the container:

> [!WARNING]
> Running the following command will trigger a `allyesconfig` compilation
> and will take a long time to complete

```
docker exec FiTx make CC=clang HOSTCC=clang -j4 LLVM_IAS=0 -C [PATH_TO_LINUX: default is `/linux`]
```

> [!NOTE]
> Please make sure to run `make clean` inside the container after compilation.

## Detailed Instruction
### Running FiTx on Linux
To run FiTx on Linux, run the following command:
```
docker exec FiTx python3 /FiTx/scripts/analyze.py linux
```
This will run the analysis on the downloaded Linux kernel. Please make sure that
the Linux kernel builds are clean (i.e. run `make clean`) before starting this
script. By default, the logs will be stored inside `/tmp/log/[datetime_of_analysis].log`.

### Reproducing the main contribution of the paper

The main results provided in the paper is as follows:

1. Number of warnings: The number of warnings generated are small.
    - As shown in Section 6.1.2 (Table 6(b)), our analysis only generates few
    warnings per source file. 99.9% of the source files only generated at most
    3 warnings per file.
    - In total, our analysis generated 113 warnings (Table 8).
2. Analysis time: The analysis time is not prolonged.
    - FiTx focuses on achieving short analysis time while still finding as
    much bug as possible. As mentioned in Section 6.1.2 (Table 6(a)), 99%ile
    of the source file required less than 0.99 seconds to complete the analysis.
3. Bug Detection Capabilities: FiTx was able to find 13 confirmed bugs.
    - Our analysis was able to find 13 confirmed bugs in Linux kernel. Table 10
    in Section 6.2 shows these results.

The rest of this section will provide a method to reproduce these key results.

#### 1. Number of warnings
To retrive the number of warnings, follow the following steps:
1. run the analysis as shown in [this](#running-fitx-on-linux) section.
2. Run `/FiTx/scripts/count_warning.py`. This script will count the warnings
in the generated log file, and show the results.
3. Alternatively, you could manually count the warning in the log file.

##### Data diffference paper

During Artifact Evaluation, we have found a minor memory bug in the original
code of FiTx. This bug has already been fixed. After re-evaluating with the
fixed version, there are following changes from the paper:

1. The total number of warnings will change to 121, from the prior 113 in the paper
2. The number of true positive does not change with this result
3. The 8 newly generated warnings were false positives, making the false
positive rate slightly higher to 61.2% from the original 58.4%. 

The result should look as follows:

|  | Warnings | TP |
| --- | --- | --- |
| Double Free | 41 | 21 |
| Double Lock | 16 | 7 |
| Double unlock | 13 | 5 |
| Memory leak | 15 | 3 |
| use after free | 31 | 9 |
| reference count | 5 | 2 |
| Total | 121 | 47 |

Other than the number of warnings, no change is made to other results, and
our claim in the paper is also not affected, thus will not be changed.

Please be noted that the data retrieved at the timing of writing the paper and
conducted experiments in our environment is still the same as the paper.

#### 2. Measuring analysis time of each source file
To retrive the analsys time, conduct the following:
1. run the analysis with `--measure` flag. The script will generate a different
log file named `/tmp/log/{datatime}_time.log`. This log file keeps track of all
output related to elapsed time in milliseconds.

```
docker exec FiTx python3 /FiTx/scripts/analyze.py linux --measure
```

2. Run `/FiTx/scripts/count_time.py`. This script will collect the measured
analysis time, and print the 50 Percentile value and 90 Percentile value in ms.
```
docker exec FiTx python3 /FiTx/scripts/count_time.py [MEASURE_LOG_FILE]
```

3. Alternatively, you could manually count the analysis time in the log file.


#### 3. Bugs detection with FiTx
Our analysis found 13 new confirmed bugs in the Linux kernel.
Table 10 in Section 6.2 shows the results of this analysis. To reproduce
this result with the analysis time, conduct the following:

1. For each bug detected source file, run the analysis script with `--file` option.
This will allow to analyze a single target in the Linux Makefile.

For example, if you want to analyze `drivers/platform/chrome/chromeos_laptop.c`,
run the following command:
```
docker exec FiTx python3 /FiTx/scripts/analyze.py linux --file drivers/platform/chrome/chromeos_laptop.o
```

Note that the file will be the target of Makefile in Linux, and not the exact
source file. Hence, in the example above, the target will be the object file
(`chromeos_laptop.o`) and not the source file.

2. With the generated logs, check each warning manually and determine if they
point to a bug.
