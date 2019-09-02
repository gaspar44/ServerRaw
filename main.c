/*
 * main.c
 *
 *  Created on: 5 ago. 2019
 *      Author: gaspar
 */

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/tcp.h>
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

void printRemainingData(unsigned char *buffer,unsigned short iphdrlen,ssize_t numberOfBytesReaded) {
	unsigned char * data = (buffer + sizeof(struct ethhdr) + iphdrlen + sizeof(struct tcphdr));
	int remaining_data = numberOfBytesReaded - (iphdrlen + sizeof(struct ethhdr) + sizeof(struct tcphdr));
	printf("\nData\n");

	for(int i = 0;i < remaining_data; i++){
		if(i != 0 && i%16 == 0)
			printf("\n");
		printf("%d ",data[i]);
	}
}


void printAsTCPHeader(unsigned char *buffer,unsigned short iphdrlen,ssize_t numberOfBytesReaded) {
	struct tcphdr *tcp = (struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));

	printf("TCP Header\n");
	printf("\t|-Source Port: %d\n",ntohs(tcp->source));
	printf("\t|-Destination Port: %d\n",ntohs(tcp->dest));
	printf("\t|-Sequence Numbers: %d\n",ntohl(tcp->seq));
	printf("\t|-CheckSum: %d\n",ntohl(tcp->check));

	printRemainingData(buffer,iphdrlen,numberOfBytesReaded);

}

void printIPHeader(unsigned char *buffer,ssize_t numberOfBytesReaded){
	struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
	unsigned short iphdrlen = ip->ihl*4;
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

	printAsTCPHeader(buffer,iphdrlen,numberOfBytesReaded);
}

void printData(unsigned char *buffer,ssize_t numberOfBytesReaded) {
	printEthernetHeader(buffer);
	printIPHeader(buffer,numberOfBytesReaded);
}


int main(int argc, char **argv) {
	if (argc > 1 && !strcmp(argv[1],"false")) {
		loopCondition = 0;
	}

	/* The use of ETH_P_ALL is for all types of packets
	 * ETH_P_IP for only IP packets
	 */
	signal(SIGINT, endLoop);
	int rawSocket = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));

	if(rawSocket<0) {
		printf("error in socket\n");
		return 1;
	}

	unsigned char *buffer = (unsigned char *) calloc(0,NUMBER_OF_PORTS);//(NUMBER_OF_PORTS); // aquí se recibe por todos los puertos
	struct sockaddr socketAddress;
	int socketAddressLen = sizeof(socketAddress);

	do {
		ssize_t numberOfBytesReaded = recvfrom(rawSocket,buffer,65536,0,&socketAddress,(socklen_t*) &socketAddressLen);

		if ((numberOfBytesReaded < 0) && (loopCondition) ) {
			printf("error al leer del buffer");
			return 1;
		}

		printData(buffer,numberOfBytesReaded);
		sleep(1);
	} while(loopCondition);

	close(rawSocket);

	return 0;
}
