# Network Port Scanner

A multithreaded C application that scans a subnet for live hosts and open ports.

## Features

- Scans a given subnet (e.g., `192.168.1.0/24`) to detect live hosts
- Checks for open TCP ports from a configurable list
- Multithreaded execution for fast scanning
- Outputs IP addresses and port descriptions of reachable services

## Requirements

- GCC compiler
- POSIX threads (`pthread`)
- Standard C library
- Optional: `valgrind` for memory leak checking

## Build

```bash
make
