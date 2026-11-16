# MyTop -- Linux System & Process Monitor

A lightweight, terminal-based system monitoring tool written in **C**
for Linux. MyTop displays an overview of system resource usage and a
live list of running processes, in a layout inspired by the Linux `top`
command.

## Project Output

The current application interface displays:

-   Average CPU usage
-   Memory usage (used / total)
-   Swap usage (used / total)
-   Battery percentage and charging status
-   A process table containing PID, user, CPU usage, memory usage, and
    command name
-   Automatic screen refresh every **1 second**
-   `Ctrl + C` to exit the monitor

## Features

-   **Live monitoring:** Refreshes system and process information
    periodically.
-   **System overview:** Shows CPU, RAM, swap, and battery information.
-   **Process listing:** Displays process ID, owner, CPU percentage,
    memory percentage, and command.
-   **Terminal interface:** Presents the information in a clear,
    organized text-based dashboard.
-   **Linux process information:** Designed to obtain process and system
    details from Linux system interfaces such as `/proc`.

## Technologies Used

-   **Language:** C
-   **Platform:** Linux
-   **System interface:** Linux `/proc` filesystem
-   **Display:** Terminal / command-line interface

## Requirements

-   A Linux operating system (for example, Ubuntu)
-   GCC compiler
-   A terminal
-   Any additional libraries required by the source code, if applicable

## Build and Run

1.  Open a terminal in the project directory.

2.  Compile the C source file. For a single-file project named `main.c`,
    use:

    ``` bash
    gcc main.c -o mytop
    ```

3.  Run the program:

    ``` bash
    ./mytop
    ```

> **Note:** If your source file has a different name or the project uses
> multiple C files, update the compile command accordingly. If your code
> uses a library such as `ncurses`, include the required compiler/linker
> flags (for example, `-lncurses`).

## How to Use

1.  Start the program using the run command above.
2.  View the live system summary and process list.
3.  The display refreshes automatically every second.
4.  Press **Ctrl + C** to stop the program and return to the terminal.

## Example Display

``` text
------------------------------------------------------------
CPU Avg                                           2.0%
Memory                                  4.2G / 14.8G
Swap                                     0K / 4.0G
Battery                           75% (Not charging)
------------------------------------------------------------
PID     USER        %CPU     %MEM     COMMAND
13113   user         3.1      1.2     process-name
13282   user         2.1      4.3     process-name
...
------------------------------------------------------------
Refresh: 1 second                         Ctrl + C to exit
```

*The values and process names above are illustrative. Actual output
depends on the computer running the program.*

## Project Objective

The objective of MyTop is to demonstrate how a C program can read Linux
system information, monitor running processes, calculate resource usage,
and present the results in a continuously updating terminal interface.

## Future Improvements

-   Add sorting by CPU usage and memory usage.
-   Display each process state.
-   Add keyboard controls for changing the sort order.
-   Improve handling of processes that start or exit while the monitor
    is running.
-   Add clearer error messages when system information is unavailable.

## Exit

Press **Ctrl + C** to exit the application.

------------------------------------------------------------------------

**Project:** MyTop -- Linux System & Process Monitor\
**Course:** Operating Systems (OS)\
**Language:** C
