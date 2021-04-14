# hangman-game

Hangman game in C++ language using local network, developed in the Computer Networks discipline at UFMG.

## Setup

To run this project, follow the steps below.

Run the command make to generate server and client files:

```bash
make
```

Then run the server file, passing the port parameter, for example:

```bash
./server 5151
```

Next, run the client file in another terminal tab, passing the server address and port, for example:

```bash
./client 127.0.0.1 5151
```
