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


    // Check permisions
    if(access(device, R_OK | W_OK) != 0) {
        fprintf(stderr, "ERROR in access(): %s\n", strerror(errno));
        return 1;
    }

    // Open device file
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd == -1) {
        fprintf(stderr, "ERROR in open(): %s\n", strerror(errno));
        return 1;
    }

    // Check if device is a tty
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

    //
    // Input flags - Turn off input processing
    //
    // convert break to null byte, no CR to NL translation,
    // no NL to CR translation, don't mark parity errors or breaks
    // no input parity check, don't strip high bit off,
    // no XON/XOFF software flow control
    //
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                        INLCR | PARMRK | INPCK | ISTRIP | IXON);

    //
    // Output flags - Turn off output processing
    //
    // no CR to NL translation, no NL to CR-NL translation,
    // no NL to CR translation, no column 0 CR suppression,
    // no Ctrl-D suppression, no fill characters, no case mapping,
    // no local output processing
    //
    // config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
    //                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
    config.c_oflag = 0;

    //
    // No line processing
    //
    // echo off, echo newline off, canonical mode off,
    // extended input processing off, signal chars off
    //
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    //
    // Turn off character processing
    //
    // clear current char size mask, no parity checking,
    // no output processing, 8N1 configuration
    //
    config.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    config.c_cflag |= (CS8 | CREAD | CLOCAL);

    //
    // One input byte is enough to return from read()
    // Inter-character timer off
    //
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    //
    // Communication speed: 9600B
    //
    if(cfsetispeed(&config, B9600) < 0 || cfsetospeed(&config, B9600) < 0) {
        fprintf(stderr, "ERROR in cfsetispeed(): %s\n", strerror(errno));
        return 1;
    }

    // Apply configuration
    if(tcsetattr(fd, TCSAFLUSH, &config) < 0) {
        fprintf(stderr, "ERROR in tcsetattr(): %s\n", strerror(errno));
        return 1;
    }

    // File descriptor polling info
    struct pollfd fds[2];

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = fd;
    fds[1].events = POLLIN;


    // Send test message
    const char* test_message = "This is a test message sent to the UART\n";
    write(fd, test_message, strlen(test_message));

    char c = 0;
    while (c!='q')
    {
        if (poll(fds, 2, -1) == -1){
            fprintf(stderr, "ERROR in poll(): %s\n", strerror(errno));
            break;
        }

        if (fds[0].revents & POLLIN) {
            if (read(STDIN_FILENO, &c, 1) > 0) {
                write(fd, &c, 1);
            }
        }

        if (fds[1].revents & POLLIN) {
            if (read(fd, &c, 1) > 0) {
                write(STDOUT_FILENO, &c, 1);
            }
        }

    }


    // Close device
    close(fd);
}
