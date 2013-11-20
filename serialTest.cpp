#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define portName "/dev/ttyUSB0"

using namespace std;




int set_interface_attribs (int fd, int speed, int parity) {
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                perror("error: unknown");
                //error_message ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // ignore break signal
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        //tty.c_cc[VMIN]  = 0;            // read doesn't block
        //tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0) {
                perror("error from tcsetattr - ");
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block) {
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        perror("error from tggetattr");
        return;
    }
    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
    if (tcsetattr (fd, TCSANOW, &tty) != 0)
            perror("error setting term attributes");
}

int readPort(int fd, char* buf) {
    char tmpBuf[100];
    char tmp[1];
    char* idx = buf;
    int idxNew = 0;
    if (fd < 0) {
        perror("error: could not open port - ");
        //error_message("error %d opening %s: %s", errno, portName, strerror (errno));
        
        return fd;
    }
    //while(1){
        while(read(fd, tmp, 1)) printf("%c",tmp[0]);  // read up to 100 characters if ready to read
        //printf("\n");
        //if(idx != buf) {
        //    if(*(idx-1) == '\n') return(1);
       // }
    //}
    //if (n < 0)
    //    fputs("read failed!\n", stderr);
    return(0);
}

int main(int argc, char** argv) {
    int fd = open(portName, O_RDWR | O_NOCTTY | O_SYNC);
    char buf[100];
    set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (fd, 0);                // set no blocking

    while(1) {
        int data = readPort(fd,buf);
        //if(data == 0) printf("no data to read\n");
        //else {
            //printf("There is %d data on the port for me to read\n",data);
        //    buf[data] = 0;
            //printf(":%s:%d\n",buf,data); 
        //}
    }
    return(0);
}
