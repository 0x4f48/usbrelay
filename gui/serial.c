/* 
 * Serial programming code copied from the site below: 
 * https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
 * 
 * Copyright (c) 2021, Justin Oh <0x4f48@gmail.com>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
 
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"

int g_serial_fd;

int set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    if (tcgetattr (fd, &tty) != 0)
    {
        debug_msg ("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    //tty.c_iflag |= IGNBRK; 
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);    // ignore modem controls,
                                        // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);  // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    
    /* this is for stop arduino reset upon serial open */
    tty.c_cflag &= ~HUPCL;			// disable hang-up-on-close to avoid reset
    
    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        debug_msg ("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

void set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        debug_msg ("error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        debug_msg ("error %d setting term attributes", errno);
}

int serial_open(char* dev_node)
{
    g_serial_fd = open (dev_node, O_RDWR | O_NOCTTY | O_SYNC);
    if (g_serial_fd < 0)
    {
        debug_msg ("error %d opening %s: %s", errno, dev_node, strerror (errno));
        return -1;
    }
    set_interface_attribs (g_serial_fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (g_serial_fd, 0);                    // set no blocking
    //set_blocking (fd, 1);                           // set blocking
    return 0;
}

void serial_close()
{
    if ( g_serial_fd > 0)
    {
        close(g_serial_fd);
        g_serial_fd = 0;
    }
}

int serial_read(char *msg, size_t size)
{
    return read(g_serial_fd, msg, size);
}

int serial_write(char* msg, int len)
{
    return write(g_serial_fd, msg, len);
}
