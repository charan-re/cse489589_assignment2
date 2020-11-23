#include "../include/simulator.h"
#include <vector>
#include<string.h>
#include <iostream>

using namespace std;

int next_seq;
int last_ack;
int expected_seq;
struct pkt retrans_pkt;
vector<struct msg> msg_buffer;

#define A 0
#define B 1
#define T_TIME 20



/* ******************************************************************
 *
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

/* called from layer 5, passed the data to be sent to other side */

int get_checksum(struct pkt packet){
	cout<<"in get_checksum function"<<endl;
	int checksum=0;
	checksum = checksum+packet.acknum;
	checksum = checksum+packet.seqnum;
	for(auto i : packet.payload)
		checksum = checksum+int(i);
	cout<<"exiting get_checksum"<<endl;
	return checksum;
}

void A_retransmit(struct pkt packet){

	cout<<"in retransmit function"<<endl;
	tolayer3(A, packet);	
	starttimer(A,T_TIME);
	cout<<"exiting retransmit function"<<endl;
}

void tx_buffer_msg(){
	cout<< "in tx_buffer_msg function "<<"next_seq "<<next_seq<<" last_ack "<<last_ack<<endl;
	struct msg new_message;
	struct pkt new_packet;
	if(msg_buffer.size()>0){
	new_message = msg_buffer.front();
	msg_buffer.erase(msg_buffer.begin());
	strcpy(new_packet.payload, new_message.data);
	new_packet.seqnum = (last_ack+1)%2;
	new_packet.acknum = (last_ack+1)%2;
	new_packet.checksum = get_checksum(new_packet);
	retrans_pkt = new_packet;
	starttimer(A,T_TIME);
	tolayer3(A, new_packet);
	next_seq = (next_seq+1)%2;	
	cout<<"in tx buffer received seq and ack "<<new_packet.seqnum<< " and "<<new_packet.acknum<<" msg is "<< new_packet.payload<<endl;
	//next_seq = (next_seq+1)%2;	
	}

}

void A_output(struct msg message)
{
	cout<<"in A_output function "<<"next seq "<<next_seq<<" last_ack "<<last_ack<<endl;
	struct pkt new_packet;
	if(next_seq == last_ack){
		msg_buffer.push_back(message);
	}
	else{
		//if(last_ack = 1)
		new_packet.seqnum = (last_ack+1)%2;
		cout<<"in A_output new_packet.seqnum "<<new_packet.seqnum<<" last_ack "<<last_ack<<endl;
		//else
		//	new_packet.seqnum = 1;
		new_packet.acknum = (last_ack+1)%2;
		cout<<"in A_output new_packet.acknum "<<new_packet.acknum<<" last_ack "<<last_ack<<endl;
		strcpy(new_packet.payload, message.data);
		new_packet.checksum = get_checksum(new_packet);
		retrans_pkt = new_packet;
		starttimer(A,T_TIME);
		tolayer3(A, new_packet);
		cout<<"in A_output received seq and ack "<<new_packet.seqnum<< " and "<<new_packet.acknum<<" msg is "<< new_packet.payload<<endl;
		next_seq = (next_seq+1)%2;	
	}	
	cout<<"exiting A_output"<<endl;
	
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	cout<<"in A_input function last ack is "<<last_ack<< " next_seq "<<next_seq<<endl;
	if((get_checksum(packet) == packet.checksum)&&(last_ack!=packet.acknum)){
		last_ack = (last_ack+1)%2;
		cout<<"in A_input received successful ack "<<packet.acknum<<" last ack received is "<<last_ack<<endl;
		stoptimer(A);
		tx_buffer_msg();
	}
	/*else{
		cout<<"ack corupted requesting retransmission"<<endl;
		A_retransmit(retransmit_buffer);
	}*/
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	cout<<"timer interrupt occured"<<endl;
	A_retransmit(retrans_pkt);
	cout<<"exiting timer interrupt"<<endl;
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	next_seq = 0;
	last_ack = 1;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	cout<<"in B_input function "<<"expected seq is "<< expected_seq<<endl;
	/*struct msg message_recvd;*/
	int checksum = get_checksum(packet);
	if((checksum == packet.checksum)&&packet.seqnum == expected_seq){
		cout<<"in B_input packet received successfully"<<endl;
		/*strcpy(message_recvd.data, packet.payload);*/
		tolayer5(B,packet.payload);
		packet.acknum = packet.seqnum;
		memset(packet.payload, '\0', sizeof(packet.payload));
		packet.checksum = get_checksum(packet);
		tolayer3(B,packet);
		expected_seq = (expected_seq+1)%2;
	}
	else{
		cout<<"unexpected or corrupted packet"<<endl;
		cout<<"in B_input received seq and ack "<<packet.seqnum<< " and "<<packet.acknum<<" msg is "<< packet.payload<<endl;
		packet.acknum = (expected_seq+1)%2;
		packet.seqnum = (expected_seq+1)%2;
		memset(packet.payload, '\0', sizeof(packet.payload));
		packet.checksum = get_checksum(packet);
		tolayer3(B,packet);
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	expected_seq = 0;
}
