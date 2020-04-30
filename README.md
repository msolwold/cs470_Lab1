# CS470 Lab1

## Introduction
This README is for a shell written in C that provides access to basic system functions as well as a few custom commands described below.
A manual is accessible in the shell via "help", "man", or "manual".

## Requirements
This shell is meant to be ran on a \textbf{macOS} system.\\
There are no other requirements.

## Custom Commands
Custom commands can be used in the form '[command] [OPTION]'.
eg. 'prompt -help'.

- prompt - Changes the prompt preceding the carrot \\
- cpuinfo - Provides information related to the systems CPU \\
- meminfo - Provides information related to the systems memory \\
- exit - Exit the shell

### prompt

prompt [OPTION]
Changes the prompt preceding the carrot.
Default Prompt : 'cwushell'
Valid Options : single words
If no option is given, the default prompt will be set

### cpuinfo

* cpuinfo -$<$switch$>$
* Provides information related the systems CPU
* Valid switches :
  * -c  CPU Clock Speed
  * -t  CPU Type 
  * -n  Number of cores

### meminfo

* meminfo -$<$switch$>$
* Provides information related the systems memory
* Valid switches :
  * -t  Total system RAM
  * -u  Amount of RAM in use
  * -c  Size of L2 cache / core

### exit

* exit [n]
* Exits the shell with the return code given ([n])
* If no code is given, the exit code from the previous program will be used

## Return / Exit Codes

This program uses fork() to execute the programs called by the user. 
It uses the following exit codes to determine the result of the program.

### Prompt
* 14 - default prompt
* 15 - user provided

### General
* 1 - Successful
* 2 - Unsuccessful (Error)
* 3 - Unsuccessful (Invalid Input)

# Known Issues
* Slightly inaccurate memory reporting with 'meminfo -u'
The link below expands on the issue of using host\_statistics() to gather memory information. There is a discrepancy in how many pages are returned from the function call and the number of actual pages. This appears to be the fault of mach's implementation of memory reporting. None the less, I found the post below interesting.
[StackOverflow](https://stackoverflow.com/questions/14789672/why-does-host-statistics64-return-inconsistent-results)

# Example Output

