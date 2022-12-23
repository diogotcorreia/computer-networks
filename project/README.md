# Projeto RC 2022/2023 - Grupo 44

## Compiling

The project can be compiled by running `make` in this directory.
Since this is a C++ project, it might take a while, depending on the machine.
This project uses C++17.

Once compiled, two binaries, `player` and `GS` will be placed in this directory.

## Running the player

The options available for the `player` executable can be seen by running:

```
./player -h
```

Once the player is running, it will prompt the user for a command.
The list of commands is shown on startup and can be shown again at any time by
typing `help` or `?` into the prompt.

In addition to the requested commands, we've implemented the `kill PLID` command,
which allows us to terminate a game of a given player, mostly for debugging
purposes. It sends the `QUT` protocol message.

All commands work as per the specification, with highlight to the `hint` command,
which allows cancelling an on-going download.

State is not saved between sessions.
The player tries to quit the current game before exiting, even when receiving
a SIGINT or SIGTERM signal.

### Available constants

These constants, defined in `src/common/constants.hpp`, might be changed for testing,
as requested by the teachers:

- `UDP_TIMEOUT_SECONDS`: The amount of seconds between giving up on receiving a
  UDP reponse.
- `UDP_RESEND_TRIES`: The amount of times to try to send a UDP packet before
  giving up.
- `TCP_READ_TIMEOUT_SECONDS`: The read timeout for TCP connections. If the connected
  server does not write within this time period, the client closes the connection.
- `TCP_WRITE_TIMEOUT_SECONDS`: The write timeout for TCP connections. If the connected
  server does not ack within this time period, the client closes the connection.

## Running the server

The options available for the `GS` executable can be seen by running:

```
./GS -h
```

We've added an extra option, `-r` that enabled random word selection.
By default, words are selected sequentially, as requested by the teachers.

The server persists data between sessions in the `.gamedata` folder, so while
testing it might make sense to delete the folder after each test.
The files stored in this folder are in binary format and can be inspected with
a program like `hexdump`.

The server is extremely resilient to all kinds of incorrect input and errors,
attempting to recover from them when possible.
When that is not possible, it exists gracefully.

The server also handles the SIGINT signal (CTRL + C), waiting for existing TCP
connections to end. The user can press CTRL + C again to forcefully exit the
server.

We've decided to use threads for concurrency. By default, the server supports
up to 50 simultaneous TCP connections, but this can be ajusted by the
`TCP_WORKER_POOL_SIZE` variable in `src/common/constants.hpp`.
We use mutexes to synchronize access to shared variables.

### Available constants

These constants, defined in `src/common/constants.hpp`, might be changed for testing:

- `TCP_WORKER_POOL_SIZE`: The number of threads handling TCP connections, that is,
  the maximum number of current TCP connections supported.
- `TCP_READ_TIMEOUT_SECONDS`: The read timeout for TCP connections. If the connected
  client does not write within this time period, the server closes the connection.
- `TCP_WRITE_TIMEOUT_SECONDS`: The write timeout for TCP connections. If the connected
  client does not ack within this time period, the server closes the connection.
