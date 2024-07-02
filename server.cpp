#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <iostream>


const size_t k_max_msg = 4096;



static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void msg(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

static int32_t do_something(int connfd)
{
    char rbuf[64] = {};
    int32_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    printf("reading data %d\n",n);
    if(n==0) return n;
    if (n < 0)
    {
        msg("read() error");
        return n;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";
    return write(connfd, wbuf, strlen(wbuf));
}


static int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        //printf("read full %ld \n",rv);
        buf[6]='\0';
        printf("read full buf before %s \n",buf);
        if (rv <= 0) {
            return -1;  // error, or unexpected EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
        //printf("read full buf after %s \n \n",buf);

    }
    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;  // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t one_request(int connfd) {
    // 4 bytes header
    char rbuf[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    if (err) {
        if (errno == 0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4);  // assume little endian
    //std::cout<<"first length of proto "<<len<<std::endl;
    if (len > k_max_msg) {
        msg("too long");
        return -1;
    }

    // request body
    err = read_full(connfd, &rbuf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    // do something
    rbuf[4 + len] = '\0';
    printf("client says: %s\n", &rbuf[4]);

    //printf("Request ends here \n \n");
    // reply using the same protocol
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);
    return write_all(connfd, wbuf, 4 + len);
}


int main()
{
    /*
        int socket(int <family>, int <type>, int <protocol>);

        SOCKET:
          1.AF_INET is the Internet address family for IPv4.
          2. SOCK_STREAM is the socket type for TCP, the protocol that will be used to transport messages in the network.
          3. The third parameter is usually zero because communication families usually have only one protocol.

     */

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
    }

    // this is needed for most server applications
    /**
     *  setsockopt: int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);


     * 2nd Argument: Level argument ->
     *       1. specifies the protocol level at which the option resides
     *       2. For socket level its SOL_SOCKET
     *       3. for TCP level its IPPROTO_TCP
     *       4. Its defined on the <netinet/in.h> header
     *
     * 3rd Argument option_name: value SO_REUSEADDR
     *      1. Specifies that the rules used in validating addresses supplied to bind() should allow reuse of local addresses, if this is supported by the protocol. This option takes an int value. This is a Boolean option.
     *      2. The SO_REUSEADDR socket option allows a socket to forcibly bind to a port in use by another socket. The second socket calls setsockopt with the optname parameter set to SO_REUSEADDR and the optval parameter set to a boolean value of TRUE before calling bind on the same port as the original socket
     *       3. If is socket running at a address+port combination is closed before fully closing it enters into TIME_WAIT state. If another socket tries to connect at same address+port combination it will reject if this options is not selected.
     *   e
     *
     *
     * 4th Argument option value :
     *   1. The option value is specified as an int. A nonzero value enables the option; 0 disables the option.
     */
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind, this is the syntax that deals with IPv4 addresses
    /*
        sockaddr_in are the basic structures for all syscalls and functions that deal with internet addresses.
    struct sockaddr_in {
   short            sin_family;   // e.g. AF_INET
   unsigned short   sin_port;     // e.g. htons(3490)
   struct in_addr   sin_addr;     // see struct in_addr, below
   char             sin_zero[8];  // zero this if you want to
    };
    */
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234); // ntohs function takes a 16-bit number in TCP/IP network byte order (the AF_INET or AF_INET6 address family) and returns a 16-bit number in host byte order.
    // takes a 32-bit number in TCP/IP network byte order (the AF_INET or AF_INET6 address family) and returns a 32-bit number in host byte order.
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // wildcard address 0.0.0.0
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        die("bind()");
    }

    // listen
    /*  int listen(int __fd, int __n)
        1.Second Argument is backlog argument which is size of the queue, which in our case is SOMAXCONN.
    */
    rv = listen(fd, SOMAXCONN);
    if (rv)
    {
        die("listen()");
    }

    while (true)
    {
        //printf("how many times \n");
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);

        /*
        The accept() function shall extract the first connection on the queue of pending connections, create a new socket with the same socket type protocol and address family as the specified socket, and allocate a new file descriptor for that socket. The file descriptor shall be allocated as described in File Descriptor Allocation.
        */

        /*
            The accept() syscall also returns the peer’s address. The addrlen argument is both the input size and the output size.
            1. Second Argument: Either a null pointer, or a pointer to a sockaddr structure where the address of the connecting socket shall be returned.

        */
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);

        if (connfd < 0)
        {
            continue; // error
        }

         while (true) {
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }


        close(connfd);
    }
}