# UART Communication Utility

A simple C program to test UART communication on Linux systems. It uses the `termios` API to configure the serial port and `poll()` for non-blocking, bidirectional data transfer between the terminal and the device. 

This project was created for the RISC-V ACT Framework Enablement and M-Mode Firmware Validation Mentorship coding challenge.

## Prerequisites

To compile and run the C program, you need:
* A Linux environment
* `gcc` 
* Read/write permissions for your target serial device (e.g., `/dev/ttyUSB0`). 

To run the simulation script, you also need:
* `tmux`
* `socat`

## How to Build

Compile the source code using `gcc`:

```bash
gcc main.c -o uart_test
```

## How to run (Hardware)
If you have a physical board connected, run the executable and pass the device path as an argument:
```bash
./uart_test /dev/ttyUSB0
```
Once running, the program will send a test message to the board. Then, everything you send trough standard input will be transmitted to the device, and everything received will be printed on screen.

## How to run (Simulation)
If you don't have physical hardware, you can use the provided ```test_uart.sh``` script to simulate a serial connection.

The script:
- Compiles the program.
- Uses ```socat``` to create two connected virtual serial ports (```/tmp/ttyV0``` and ```/tmp/ttyV1```). Anything written to one port comes out the other. 
- Opens a ```tmux``` session with a hidden window keeping the connection alive in the background, and a visible window split into two panes.
- One pane runs cat ```/tmp/ttyV1``` to act as the receiving device, while the other pane runs the C program on ```/tmp/ttyV0```.

## Expected behaviour
When the tmux session opens, you should immediately see the following text appear in the monitoring pane:
```
This is a test message sent to the UART
```

If you type anything in the program's pane and press enter, the characters will be forwarded through the virtual serial connection and appear instantly in the monitoring pane.

To end the test, press ```q``` in the program pane to stop the execution, then close the terminal panes or kill the tmux session by pressing ```ctrl+b```` typing ```kill-session```, and pressing enter.
