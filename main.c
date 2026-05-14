#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>


int main(int argc, char** argv) {
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <device_route> (Ej: /dev/ttyUSB0)\n", argv[0]);
        return 1;
    }
    const char *device = argv[1];


    // Verify read and write permissions
    if(access(device, R_OK | W_OK) != 0) {
        fprintf(stderr, "ERROR in access(): %s\n", strerror(errno));
        return 1;
    }

    // Open device in non-blocking mode, bypassing terminal control restrictions
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd == -1) {
        fprintf(stderr, "ERROR in open(): %s\n", strerror(errno));
        return 1;
    }

    // Check if provided path is a valid terminal/serial device
    if(!isatty(fd)) {
        fprintf(stderr, "ERROR in isatty(): %s\n", strerror(errno));
        return 1;
    }

    // Get current config
    struct termios config;
    if(tcgetattr(fd, &config) < 0) {
        fprintf(stderr, "ERROR in tcgetattr(): %s\n", strerror(errno));
        return 1;
    }

    // Raw mode: Disable input processing to prevent byte modifications
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                        INLCR | PARMRK | INPCK | ISTRIP | IXON);


    // Disable output processing
    config.c_oflag = 0;

    // Disable local line processing
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    // Hardware parameters [8N1]: 8 data bits, no parity, 1 stop bit
    // Ignore modem control lines and enable the receiver.
    config.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    config.c_cflag |= (CS8 | CREAD | CLOCAL);

    // One input byte is enough to return from read()
    // Inter-character timer off
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    // Set standard communication baud rate
    if(cfsetispeed(&config, B9600) < 0 || cfsetospeed(&config, B9600) < 0) {
        fprintf(stderr, "ERROR in cfsetispeed(): %s\n", strerror(errno));
        return 1;
    }

    // Apply configuration
    if(tcsetattr(fd, TCSAFLUSH, &config) < 0) {
        fprintf(stderr, "ERROR in tcsetattr(): %s\n", strerror(errno));
        return 1;
    }

    // Set up polling for bidirectional I/O without blocking the execution thread
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = fd;
    fds[1].events = POLLIN;

    const char* test_message = "This is a test message sent to the UART\n";
    write(fd, test_message, strlen(test_message));

    char c = 0;
    while (c!='q')
    {
        if (poll(fds, 2, -1) == -1){
            fprintf(stderr, "ERROR in poll(): %s\n", strerror(errno));
            break;
        }

        // Forward stdin to the serial device
        if (fds[0].revents & POLLIN) {
            if (read(STDIN_FILENO, &c, 1) > 0) {
                write(fd, &c, 1);
            }
        }

        // Forward serial device output to stdout
        if (fds[1].revents & POLLIN) {
            if (read(fd, &c, 1) > 0) {
                write(STDOUT_FILENO, &c, 1);
            }
        }

    }

    close(fd);
    return 0;
}
