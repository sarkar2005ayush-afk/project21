# MyTop -- Linux System & Process Monitor

A lightweight, terminal-based system monitoring tool written in **C**
for Linux. MyTop shows an overview of system resource usage, per-core CPU
bars and a live list of running processes, in a layout inspired by the
Linux `top` command. All data is read directly from the `/proc` and `/sys`
filesystems -- no external libraries are needed.

## Project Output

The application interface displays:

-   Uptime (hh:mm:ss) and idle percentage
-   Number of CPU cores
-   Average CPU usage and usage of every core, drawn as coloured bars in
    the top-right corner of the screen
-   Memory usage (used / total, in GB)
-   Swap usage (used / total, in GB)
-   Battery status, percentage and level
-   A process table with PID, user, priority, nice value, virtual memory,
    resident memory, state, CPU %, running time and program name
-   Automatic screen refresh every **1 second**
-   `Ctrl + C` to exit the monitor

## Features

-   **Live monitoring:** Clears and redraws the screen every second.
-   **System overview:** Uptime, idle %, cores, RAM, swap and battery.
-   **Core-wise CPU usage:** One bar per core plus an average bar; the
    busiest core is highlighted in red.
-   **Process listing:** Every numbered folder in `/proc` is read and shown
    as one row. Usernames are looked up from the user id.
-   **Colour coding:**
    -   Idle %: green (> 80%), white (> 20%), red (otherwise)
    -   Battery: blinking red when below 20% and discharging
    -   Negative nice values in red; memory shown in G / T when large
-   **Linux process information:** All details come from the files below.

| Information        | Source                                              |
| ------------------ | --------------------------------------------------- |
| Uptime / idle time | `/proc/uptime`                                      |
| RAM and swap       | `/proc/meminfo`                                     |
| Number of cores    | `/proc/cpuinfo` (`siblings` line)                   |
| CPU usage per core | `/proc/stat`                                        |
| Battery            | `/sys/class/power_supply/BAT1/{capacity,status,capacity_level}` |
| Process details    | `/proc/<pid>/stat`, `/proc/<pid>/status`, `/proc/<pid>/loginuid` |

## Technologies Used

-   **Language:** C
-   **Platform:** Linux
-   **System interface:** Linux `/proc` and `/sys` filesystems
-   **Display:** Terminal using ANSI escape codes (no ncurses)

## Requirements

-   A Linux operating system (for example, Ubuntu)
-   GCC compiler
-   A terminal that supports ANSI colours and UTF-8 (the CPU bars use the
    block character `▒`)
-   A terminal at least **90 columns wide**, because the CPU bars are drawn
    starting at column 88 and move left for each core
-   The `clear` command (the screen is cleared with `system("clear")`)

## Build and Run

1.  Open a terminal in the project directory.

2.  Compile the source file:

    ``` bash
    gcc top_simple.c -o mytop
    ```

3.  Run the program:

    ``` bash
    ./mytop
    ```

> **Note:** If your source file has a different name, change the compile
> command accordingly. No extra linker flags are required.

## How to Use

1.  Start the program using the run command above.
2.  View the live system summary and process list.
3.  The display refreshes automatically every second (change
    `refresh_rate` in the source to use a different interval).
4.  Press **Ctrl + C** to stop the program and return to the terminal.

## Example Display

``` text
--------------------------------------------TOP PROGRAM--------------------------------------------


UpTime          : 02:14:37
Idle            : 88%
Cores           : 4
Memory used     : 4.21/14.80 GB
Swap used       : 0.00/4.00 GB
Bat             : Discharging - 75%
                  (Normal)


PID     USER    PRI     NI      VIR(mb) RES(mb) STATE CPU%      TIME+           PROGRAM
1       root    20      0       165.23  11.40   S     0.05      02:14:37        (systemd)
13113   user    20      0       2.10G   312.55  S     3.10      01:02:10        (firefox)
13282   user    20      0       845.12  95.30   R     2.10      00:45:03        (code)
...
```

*The values and process names above are illustrative. Actual output
depends on the computer running the program. The CPU bars are drawn in the
top-right corner and are not shown in this text example.*

### Process table columns

| Column    | Meaning                                              |
| --------- | ---------------------------------------------------- |
| PID       | Process id                                           |
| USER      | Owner of the process (user id if the name is unknown)|
| PRI       | Scheduling priority                                  |
| NI        | Nice value (-20 high priority to 19 low priority)    |
| VIR(mb)   | Virtual memory (`VmSize`); shown in G / T if large   |
| RES(mb)   | Resident memory (`VmRSS`); shown in G / T if large   |
| STATE     | Process state (R, S, D, Z, ...)                      |
| CPU%      | CPU time used divided by the time since the process started |
| TIME+     | Time since the process started (hh:mm:ss)            |
| PROGRAM   | Name of the process                                  |

## Known Limitations

-   Processes without `VmSize` / `VmRSS` lines (kernel threads and zombie
    processes) are not listed.
-   CPU % is an average over the whole life of the process, not the usage
    in the last second.
-   Battery information is read from `BAT1`; computers whose battery has
    another name (for example `BAT0`) will show an empty status and `-1%`.
-   The screen flickers slightly because it is cleared every refresh.

## Project Objective

The objective of MyTop is to demonstrate how a C program can read Linux
system information, monitor running processes, calculate resource usage,
and present the results in a continuously updating terminal interface.

## Future Improvements

-   Show system information using `uname` (kernel name, release, version,
    machine, ...).
-   Show the load average from `/proc/loadavg`.
-   Detect and list zombie processes (`Z` in `/proc/<pid>/status`).
-   Show whether the power adapter is plugged in (`ACAD/uevent`).
-   Add sorting by CPU usage and memory usage, with keyboard controls to
    change the sort order.
-   Show a process count (total / running / sleeping / zombie).
-   Hide the cursor while running and restore it on exit.
-   Add clearer error messages when system information is unavailable.

## Exit

Press **Ctrl + C** to exit the application.

---

**Project:** MyTop -- Linux System & Process Monitor  
**Course:** Operating Systems (OS)  
**Language:** C
