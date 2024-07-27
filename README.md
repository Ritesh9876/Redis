# Redis

Commands:


g++ -Wall -Wextra -O2 -g client.cpp -o client

g++ -Wall -Wextra -O2 -g server.cpp hashtable.cpp thread_pool.cpp zset.cpp heap.cpp avl.cpp  -o server


Commands can you try:
./client set k 399
./client get k
./client del k
./client zadd zset 1 n1
./client zadd zset 2 n2
./client zadd zset 1.1 n1
./client zscore zset n1
./client zquery zset 1 "" 0 10
./client zrem zset adsf
./client zrem zset n1
./client zquery zset 1 "" 0 10

THEORY :->

Links:

SetSockopt:
https://learn.microsoft.com/en-us/windows/win32/winsock/using-so-reuseaddr-and-so-exclusiveaddruse
https://pubs.opengroup.org/onlinepubs/009696799/functions/setsockopt.html

BEST EXPLANATION for SO_REUSEADDR
https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ


Accept() sysCall:

/*
        The accept() function shall extract the first connection on the queue of pending connections, create a new socket with the same socket type protocol and address family as the specified socket, and allocate a new file descriptor for that socket. The file descriptor shall be allocated as described in File Descriptor Allocation.
        */

        /*
            The accept() syscall also returns the peer’s address. The addrlen argument is both the input size and the output size.
            1. Second Argument: Either a null pointer, or a pointer to a sockaddr structure where the address of the connecting socket shall be returned.

        */
https://pubs.opengroup.org/onlinepubs/9699919799/functions/accept.html
https://www.geeksforgeeks.org/accept-system-call/


Read() sysCall:
https://man7.org/linux/man-pages/man2/read.2.html
https://man7.org/linux/man-pages/man2/read.2.html#NOTES





fcntl():
/*
#define F_GETFL		3	/* Get file status flags.  */
#define F_SETFL		4	/* Set file status flags.  */
*/
https://www.scottklement.com/rpg/socktut/fcntlapi.html
https://man7.org/linux/man-pages/man2/fcntl.2.html


Level triggered VS Edge triggered:
https://copyconstruct.medium.com/nonblocking-i-o-99948ad7c957

select(), poll(), epoll():
https://devarea.com/linux-io-multiplexing-select-vs-poll-vs-epoll/

epoll():
https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642
https://man7.org/linux/man-pages/man2/epoll_ctl.2.html


Separate Chaining hashtable:   
https://www.geeksforgeeks.org/separate-chaining-collision-handling-technique-in-hashing/



Pointers: 

htab->tab[pos] -> [Node1] -> [Node2] -> [Node3] -> NULL

Understanding Connections :->

from:
from is a pointer to the location where the address of Node1 is stored. Let's say the address of htab->tab[pos] is 0x100.
Example: from = &htab->tab[pos]; means from holds 0x100.


*from:
*from is the address stored at from, which is the address of Node1. Let's say the address of Node1 is 0x200.
Example: *from = htab->tab[pos]; means *from holds 0x200.


**from:
**from is the actual Node1 data located at the address 0x200.
Example: **from dereferences 0x200 to access the Node1 structure.