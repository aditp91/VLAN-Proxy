#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h> 

#define numBytes 1000

/*	The strategy to construct the VLAN will be for your team to create a proxy program that runs on each machine. 
 *	The proxy will use 2 Ethernet devices to implement the VLAN. One  ethernet device, called eth0, will be 
 *	connected to the Internet. The second device is a virtual device called a tap (for network tap). The tap
 *	operates on layer 2 packets and allows your proxy to send and receive packets on the local machine. 
 */

/* Version Changes
 * 
 * 0.1
 * - changed name to cs352proxy
 * - added arg checking for number of args in main
 * 
 * 0.11
 * - added version notes
 * - added checker for client in main
 * - moved code from main into createTCP
 * - created empty methods: createTap, incomingPackets
 * 
 * 0.12
 * - changed some stuff in client TCP
 * 
 * 0.13
 * - implemented server stuff in main
 * 
 * 0.14
 * - moved server stuff to new method serverTCP
 * 
 * 0.15
 * - implemented tap device intitiation (maybe handling?)
 * 
 * */

/* Create a TCP socket on the port as defined in command line */

/* allocate_tunnel:
 * open a tun or tap device and returns the file
 * descriptor to read/write back to the caller
 */
int allocate_tunnel(char *dev, int flags) {
	int fd, error;
	struct ifreq ifr;
	char *device_name = "/dev/net/tun";
	if( (fd = open(device_name , O_RDWR)) < 0 ) {
		perror("error opening /dev/net/tun");
		return fd;
	}
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = flags;
	if (*dev) {
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}
	if( (error = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
		perror("ioctl on tap failed");
		close(fd);
		return error;
	}
	strcpy(dev, ifr.ifr_name);
	return fd;
}

void serverTCP(int port, char* localInterface)
{
	puts("im in the server funciton");
	// implement server
	char buffer[5000]; //to read data from client
	int sd, sd_current; //, cc, fromlen, tolen;
	unsigned int addrlen;
	struct sockaddr_in sin;
	struct sockaddr_in pin;

	//get socket
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("not able to open a socket");
		exit(1);
	}

	//socket structure
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = port;

	//bind socket to the given port
	if(bind(sd, (struct sockaddr*) &sin, sizeof(sin)) == -1){
		perror("not able to bind");
		exit(1);
	}
	
	//listen on the port
	if(listen(sd,5) == -1){ //not able to listen
		perror("not able to listen");
		exit(1);
	}

	//wait for clients to connect
	addrlen = sizeof(pin);
	if((sd_current = accept(sd, (struct sockaddr*) &pin, &addrlen)) == -1){
		perror("not able to accept");
		exit(1);
	}

	//print out client name and port
	printf("client %s, port %d\n", inet_ntoa(pin.sin_addr), ntohs(pin.sin_port));

	/* [create a thread to] handle virtual tap device */
	/* CHANGE THIS TO THREADING because it's not threaded as of now */
	int tap_fd;
	char* if_name= "tap1";

	if ( (tap_fd = allocate_tunnel(if_name, IFF_TAP | IFF_NO_PI)) < 0 ) {
		perror("Opening tap interface failed! \n");
		exit(1);
	}

	//receive msg from client
	if(recv(sd_current, buffer, sizeof(buffer), 0) == -1){
		perror("not able to receive");
		exit(1);
	}

	//send confirmation of msg receive
	if(send(sd_current, buffer, strlen(buffer), 0) == -1){
		perror("not able to send");
		exit(1);
	}

	//close socket
	close(sd_current);
	close(sd);
}

void clientTCP(int port, char* host, char* localInterface)
{
	puts("im in the client funciton");
	
	char hostname[100];
	char dir[5000];
	int	sd;
	struct sockaddr_in pin;
	struct hostent *hp;

	strcpy(hostname,host);

	/* go find out about the desired host machine */
	if ((hp = gethostbyname(hostname)) == 0) {
		perror("gethostbyname");
		exit(1);
	}

	/* fill in the socket structure with host information */
	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_port = port;

	/* grab an Internet domain socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* connect to PORT on HOST */
	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
		perror("connect");
		exit(1);
	}

	/* [create a thread to] handle virtual tap device */
	/* CHANGE THIS TO THREADING because it's not threaded as of now */
	int tap_fd;
	char* if_name= "tap2";

	if ( (tap_fd = allocate_tunnel(if_name, IFF_TAP | IFF_NO_PI)) < 0 ) {
		perror("Opening tap interface failed! \n");
		exit(1);
	}

	/* send a message to the server PORT on machine HOST */
	if (send(sd, host, strlen(host), 0) == -1) {
		perror("send");
		exit(1);
	}

	/* wait for a message to come back from the server */
	if (recv(sd, dir, 8192, 0) == -1) {
		perror("recv");
		exit(1);
	}

	/* spew-out the results and bail out of here! */
	printf("%s\n", dir);

	close(sd);
}

/* handle virtual tap device */
void handleTap()
{

}

/* create a thread to handle incoming packets on the TCP socket */
void incomingPackets()
{

}

int main(int argc, char *argv[])
{
	/* Read in command line arguments */
	int 	i;
	int 	port;
	int		client;
	char*	host;
	char* 	localInterface;

	for(i = 0; i < argc; i++)
		printf("arg %d: %s\n", i, argv[i]);

	/* Arg check */
	if(argc > 4 || argc < 3){
		puts("Incorrect number of arguments");
		return 0;
	}

	/* Find out if client or host and get appropriate args */
	if(argc == 3){ //it's a server, listen on the port
		port = atoi(argv[1]);
		localInterface = argv[2];
		client = 0;

		serverTCP(port, localInterface);
	}
	else{// it's a client, connect to the server
		host = argv[1];
		port = atoi(argv[2]);
		localInterface = argv[3];
		client = 1;

		clientTCP(port, host, localInterface);
	}

	return 0;
}
