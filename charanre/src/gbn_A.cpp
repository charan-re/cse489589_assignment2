#include "../include/simulator.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>

using namespace std;
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
int nextseqnum,maxseqnum,base;
float timeout;
int winsize, expected_ack;
struct pkt retrans_pkt;
vector <struct msg> buffer;
vector <float> timelist;

int checksum(struct pkt packet)
{
	int i,sum=0;
	sum=sum+packet.seqnum+packet.acknum;
	for(i=0;i<20;i++)
	{
		sum=sum+int(packet.payload[i]);
	}	
return sum;
}

void retransmit(struct pkt packet){

        cout<<"in retransmit function"<<endl;
        tolayer3(0, packet);
	timelist.push_back(get_sim_time());
	 if(base==nextseqnum){
                        if(nextseqnum>1)
                        {
                                float t1=timelist[nextseqnum];
                                float t2=timelist[nextseqnum-1];
                                timeout=timeout-(t1-t2);
                        }

                        starttimer(0,timeout);
                        cout<<"Timer started"<<endl;}

        cout<<"exiting retransmit function"<<endl;
}

void buffer_transmit(){
        cout<< "in tx_buffer_msg function "<<"nextseqnum "<<nextseqnum<<" expected_ack "<<expected_ack<<endl;
        struct msg new_message;
        struct pkt send_packet;
        if((buffer.size()>0) && (nextseqnum<base+winsize)){
        new_message = buffer.front();
        buffer.erase(buffer.begin());
        strcpy(send_packet.payload, new_message.data);
        send_packet.seqnum = nextseqnum;
        send_packet.acknum = expected_ack;
        send_packet.checksum = checksum(send_packet);
        retrans_pkt = send_packet;
        tolayer3(0, send_packet);
	timelist.push_back(get_sim_time());
	printf("Sent tolayer 3\n");
                expected_ack=nextseqnum;
                if(base==nextseqnum){
			if(nextseqnum>1)
			{
				float t1=timelist[nextseqnum];
				float t2=timelist[nextseqnum-1];
				timeout=timeout-(t1-t2);
			}
                        starttimer(0,timeout);
			cout<<"Tiner started"<<endl;}
        nextseqnum=nextseqnum+1;

        cout<<"in buffer received seq and ack "<<send_packet.seqnum<< " and "<<send_packet.acknum<<" msg is "<< send_packet.payload<<endl;}
}


/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	struct pkt send_pkt;
	if(nextseqnum<base+winsize)
	{
//		struct pkt send_pkt;
		int i;
		cout<<"received message at a output"<<message.data<<endl;
		send_pkt.seqnum=nextseqnum;
		send_pkt.acknum=expected_ack;
		memcpy(send_pkt.payload,message.data,20);
//		strcpy(send_pkt.payload,message.data);
		printf("After copy %s\n",send_pkt.payload);
		send_pkt.checksum=checksum(send_pkt);
		retrans_pkt=send_pkt;
    		tolayer3(0, send_pkt);
		timelist.push_back(get_sim_time());
		printf("Sent tolayer 3\n");
		expected_ack=nextseqnum;
    		if(base==nextseqnum){
			if(nextseqnum>1)
                        {
                                float t1=timelist[nextseqnum];
                                float t2=timelist[nextseqnum-1];
                                timeout=timeout-(t1-t2);
                        }

    			starttimer(0,timeout);
			cout<<"Timer started"<<endl;}
    		nextseqnum=nextseqnum+1;
		cout<<"next seq num"<<nextseqnum<<endl;
   	}
	else if(nextseqnum>=base+winsize)
        {
                buffer.push_back(message);
        }

/*	else
	{
		cout<<"push back of messages"<<endl;
		buffer.push_back(message);
		cout<<"Done"<<endl;
	}*/
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	int check_sum = packet.acknum+packet.seqnum;
	cout<<"Inside a input"<<endl;
	if ( (packet.acknum=expected_ack)  && (packet.checksum==check_sum))
	{
		base=packet.acknum+1;
		cout<<"base "<<base<<endl;
		if(nextseqnum>base+winsize)
			buffer_transmit();
		if(base== nextseqnum ){
			stoptimer(0);
			cout<<"timer stopped"<<endl;}
//		else
//			starttimer(0,timeout);
	}
	else 
	{
		if(get_sim_time()==timeout)
			retransmit(retrans_pkt);
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	retransmit(retrans_pkt);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	timeout=15;
	winsize= getwinsize();
   	base=1;
   	nextseqnum=1;
	expected_ack=1;
   	maxseqnum=winsize+1;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */
struct pkt ackpkt;
int expected_seqno;
void makeackpt(int seq)
{
	ackpkt.acknum=seq;
 	ackpkt.seqnum=seq;
	ackpkt.checksum=ackpkt.acknum+ackpkt.seqnum;
	memset(ackpkt.payload,'\0',sizeof(ackpkt.payload));
}
/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	cout<<"Inside b input"<<endl;
	cout<<"Expected seq num"<<expected_seqno<<"packet seq num"<<packet.seqnum<<"received"<<packet.payload<<endl;
	if((expected_seqno==packet.seqnum) && (packet.checksum==checksum(packet)))
	{
			cout<<"crt seq num received"<<endl;
			makeackpt(packet.acknum);
			tolayer5(1, packet.payload);
			cout<<"sending ack"<<endl;
			tolayer3(1,ackpkt);
			cout<<"sent ack"<<endl;
		    	expected_seqno++;
	}
	else
	{
	//	tolayer3(1,ackpkt);
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	expected_seqno = 1;
}
