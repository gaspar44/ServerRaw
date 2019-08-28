/*
 * main.c
 *
 *  Created on: 5 ago. 2019
 *      Author: gaspar
 */

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <signal.h>

const int NUMBER_OF_PORTS = 65536; // Real numbers 2¹⁶
int loopCondition = 1;


void endLoop() {
	loopCondition = 0;
	printf("Exit reading\n");
	return;
}

void printEthernetHeader(unsigned char *buffer) {
	struct ethhdr *eth = (struct ethhdr *)(buffer);
	printf("\nEthernet Header\n");
	printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
	printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
	printf("\t|-Protocol : %d\n",eth->h_proto);
}

//void printTCPHeader(unsigned char *buffer) {
//	unsigned short iphdrlen;
//	struct tcphdr udp=(struct udphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
//}

void printIPHeader(unsigned char *buffer){
	struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
	struct sockaddr_in source,destination;

	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = ip->saddr;
	memset(&destination, 0, sizeof(destination));
	destination.sin_addr.s_addr = ip->daddr;

	printf("IP Header\n");
	printf("\t|-Source IP : %s\n", inet_ntoa(source.sin_addr));
	printf("\t|-Destination IP %s\n",inet_ntoa(destination.sin_addr));
	printf("\t|-Internet Header Length : %d DWORDS or %d Bytes\n",(unsigned int)ip->ihl,((unsigned int)(ip->ihl))*4);
	printf("\t|-IP version: %d\n",(unsigned int) ip->version);
	printf("\t|-Type Of Service : %d\n",(unsigned int)ip->tos);
	printf("\t|-Total Length of package : %d Bytes\n",ntohs(ip->tot_len));
	printf("\t|-Header Checksum : %d\n",ntohs(ip->check));
	printf("\t|-Identification : %d\n",ntohs(ip->id));
	printf("\t|-Time To Live : %d\n",(unsigned int)ip->ttl);

//	printTCPHeader(buffer);
}

void printData(unsigned char *buffer) {
	printEthernetHeader(buffer);
	printIPHeader(buffer);
}


int main(int argc, char **argv) {
	/* The use of ETH_P_ALL is for all types of packets
	 * ETH_P_IP for only IP packets
	 */
	signal(SIGINT, endLoop);
	int rawSocket = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));

	if(rawSocket<0) {
		printf("error in socket\n");
		return 1;
	}

	unsigned char *buffer = (unsigned char *) malloc(NUMBER_OF_PORTS); // aquí se recibe por todos los puertos
	memset(buffer,0,NUMBER_OF_PORTS);
	struct sockaddr socketAddress;
	int socketAddressLen = sizeof(socketAddress);

	while(loopCondition) {
		ssize_t numberOfBytesReaded = recvfrom(rawSocket,buffer,65536,0,&socketAddress,(socklen_t*) &socketAddressLen);

		if ((numberOfBytesReaded < 0) && (loopCondition) ) {
			printf("error al leer del buffer");
			return 1;
		}

		printData(buffer);
		sleep(1);
	}

	close(rawSocket);

	return 0;
}
