# ft_ping

A custom implementation of the `ping` command in C, designed to send ICMP ECHO_REQUEST packets to network hosts and display round-trip time statistics.

## Description

`ft_ping` is a network utility that sends ICMP (Internet Control Message Protocol) echo request packets to a specified host and waits for echo reply packets. It provides detailed statistics about packet loss, round-trip times, and network connectivity.

This implementation is based on the `inetutils-2.0` ping program and supports IPv4 addresses and hostnames.

## Features

- **ICMP Echo Request/Reply**: Sends ICMP echo requests and processes replies
- **IPv4 Support**: Works with IPv4 addresses and hostnames
- **Statistics**: Tracks sent/received packets, min/max/average round-trip times
- **Verbose Mode**: Optional detailed output for debugging
- **TTL Configuration**: Configurable Time-To-Live hop limit
- **Signal Handling**: Graceful shutdown with Ctrl+C
- **Checksum Verification**: Validates ICMP packet integrity
- **Timeout Handling**: Manages packet timeouts and retransmissions

## Prerequisites

- GCC compiler
- Make
- Root privileges (required for raw ICMP sockets)

## Building

### Standard Build

```bash
make
```
## Usage

### Basic Usage

```bash
sudo ./ft_ping [OPTIONS] HOST
```

**Note**: Root privileges are required due to the use of raw ICMP sockets.

### Examples

```bash
# Ping a host by IP address
sudo ./ft_ping 8.8.8.8

# Ping a host by hostname
sudo ./ft_ping google.com

# Enable verbose output
sudo ./ft_ping -v 8.8.8.8

# Set custom TTL
sudo ./ft_ping --ttl 64 8.8.8.8
```

### Command Line Options

- `-v, --verbose` - Produce verbose output
- `--ttl HOP-LIMIT` - Set TTL hop limit
- `HOST` - Target host (IP address or hostname)

## Output Format

The program displays:
- Echo reply messages with sequence numbers, TTL, and round-trip time
- Error messages for unreachable hosts or time exceeded
- Final statistics including:
  - Packets transmitted/received
  - Packet loss percentage
  - Round-trip time statistics (min/avg/max/mdev)

Example output:
```
PING 8.8.8.8 (8.8.8.8) 84(112) bytes of data.
64 bytes from 8.8.8.8: icmp_seq=1 ttl=113 time=12.345 ms
64 bytes from 8.8.8.8: icmp_seq=2 ttl=113 time=11.234 ms
^C
--- 8.8.8.8 ping statistics ---
2 packets transmitted, 2 received, 0% packet loss, time 1001ms
rtt min/avg/max/mdev = 11.234/11.789/12.345/0.555 ms
```

## Project Structure

```
ft_ping/
├── inc/
│   └── ft_ping.h          # Header file with declarations and constants
├── src/
│   ├── main.c             # Main entry point and argument parsing
│   ├── ft_ping.c          # Core ping functionality
│   ├── signal.c           # Signal handling
│   ├── stats.c            # Statistics calculation and display
│   └── utils.c            # Utility functions
├── Makefile               # Build configuration
├── Dockerfile             # Docker container setup
└── README.md              # This file
```

## Technical Details

### Packet Structure

- **Total Packet Size**: 84 bytes
- **IP Header**: 20 bytes
- **ICMP Header**: 8 bytes
- **Payload**: 56 bytes (including timestamp and data chunk)

### Key Components

- **Raw Socket**: Uses `SOCK_RAW` with `IPPROTO_ICMP` for direct ICMP access
- **Checksum Calculation**: Implements RFC 1071 checksum algorithm
- **Timeout Management**: Uses `select()` for non-blocking I/O with timeouts
- **Signal Handling**: Graceful shutdown on SIGINT (Ctrl+C)

### Error Handling

- Permission errors for non-root users
- Network unreachable errors
- Time-to-live exceeded errors
- Checksum validation errors

## Docker Support

The project includes Docker support for development and testing:

```bash
# Build and run container
make container

# Access container shell
make shell

# Remove container
make rm_container
```

## Limitations

- IPv4 only (no IPv6 support)
- Requires root privileges
- Limited to ICMP echo request/reply (no other ICMP types)
- No support for advanced ping features like flood mode or pattern matching

## License

This project is part of the 42 school curriculum and is intended for educational purposes.

## Contributing

This is an educational project. For learning purposes, feel free to study the code and understand how ICMP ping works at a low level. 