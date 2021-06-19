#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/queue.h>
#include <sys/stat.h>

#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

// #include <rte_ring.h>

int x  = 0;

class HD
{
	private:
	// int x;
public:
	// static int x=555;
	static void
	handler_item(int signum)
	{
		// void *msg = nullptr;
		// rte_ring_enqueue(send_ring,msg);
		printf("\nXXX %d\n",x);
		exit(0);
	}
	// rte_ring* send_ring;
	HD()
	{
		x = 555;
		signal(SIGINT, this->handler_item);
	}
};

// rte_ring * send_ring;

int main()
{
	// send_ring = rte_ring_lookup("HELLO");
	// signal(SIGINT,handler_item);
	HD ll;
	while (1)
	{
	}
	return 0;
}