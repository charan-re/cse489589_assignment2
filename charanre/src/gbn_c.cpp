#include "../include/simulator.h"
#include <vector>
#include <string.h>
#include <iostream>

#define A 0
#define B 1
#define T_TIME 15

using namespace std;

int B_expected_seq;
int A_next_seq;
int A_last_ack;
int window_size;
int win_start_index;
struct pkt retrans_buf[1000];
int *timer_list;
vector <struct msg> msg_buffer;
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

/* called from layer 5, passed the data to be sent to other side */

int get_checksum(struct pkt packet){
        int checksum=0;
        checksum = checksum+packet.acknum;
        checksum = checksum+packet.seqnum;
        for(auto i : packet.payload)
                checksum = checksum+int(i);
        return checksum;
}

void A_retransmit(struct pkt packet){

        cout<<"in retransmit function "<<packet.seqnum<<" "<<packet.acknum<<" "<<packet.payload<<endl;
        tolayer3(A, packet);
	timer_list[A_next_seq%window_size] = get_sim_time();
        starttimer(A,T_TIME);
        cout<<"exiting retransmit function"<<endl;
}

void tx_buffer_msg(){
        cout<< "in tx_buffer_msg function "<<"A_next_seq "<<A_next_seq<<" last_ack "<<A_last_ack<<endl;
        struct msg new_message;
        struct pkt new_packet;
        if(msg_buffer.size()>0){
                new_message = msg_buffer.front();
                msg_buffer.erase(msg_buffer.begin());
                strcpy(new_packet.payload, new_message.data);
                new_packet.seqnum = A_next_seq;
                new_packet.acknum = A_next_seq;
                new_packet.checksum = get_checksum(new_packet);
                retrans_buf[A_next_seq%window_size].seqnum = new_packet.seqnum;
                retrans_buf[A_next_seq%window_size].acknum = new_packet.acknum;
                strcpy(retrans_buf[A_next_seq%window_size].payload,new_packet.payload);
		cout<<"pushing in buffer to retrans buf at "<<A_next_seq%window_size<<endl;
		cout<<"buffered "<<retrans_buf[A_next_seq%window_size].seqnum<<" "<<retrans_buf[A_next_seq%window_size].acknum<<" "<<retrans_buf[A_next_seq%window_size].payload<<endl;
		timer_list[A_next_seq%window_size] = get_sim_time();
                tolayer3(A, new_packet);
                cout<<"in tx_buffer received seq and ack "<<new_packet.seqnum<< " and "<<new_packet.acknum<<" msg is "<< new_packet.payload<<endl;
                A_next_seq = A_next_seq+1;
	}
}

void A_output(struct msg message)
{
	cout<<"in A_output function "<<"next seq "<<A_next_seq<<" last_ack "<<A_last_ack<<endl;
        struct pkt new_packet;
        if(A_next_seq-win_start_index >= window_size){
                msg_buffer.push_back(message);
        }
	else{
		new_packet.seqnum = A_next_seq;
		new_packet.acknum = A_next_seq;
                strcpy(new_packet.payload, message.data);
                new_packet.checksum = get_checksum(new_packet);
                //retrans_pkt = new_packet;
                cout<<"copying to retrans buf "<<(A_next_seq)%(window_size)<<" sim time "<<get_sim_time()<<" winsize "<<window_size<<endl;
                retrans_buf[A_next_seq%window_size] = new_packet;
		cout<<"rtx_buf is "<<retrans_buf[A_next_seq%window_size].seqnum<<" "<<retrans_buf[A_next_seq%window_size].acknum<<" "<<retrans_buf[A_next_seq%window_size].payload<<endl;
		timer_list[A_next_seq%window_size] = get_sim_time();
		if(win_start_index == A_next_seq){
			cout<<"starting timer in A_output for seq "<<A_next_seq<<endl;
                	starttimer(A,T_TIME);
		}
                tolayer3(A, new_packet);
                cout<<"in A_output received seq and ack "<<new_packet.seqnum<< " and "<<new_packet.acknum<<" msg is "<< new_packet.payload<<endl;
                A_next_seq = A_next_seq+1;
	}
	
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	int time_gap;
	cout<<"in A_input function last ack is "<<A_last_ack<< " next_seq "<<A_next_seq<<endl;
        if((get_checksum(packet) == packet.checksum)&&(packet.acknum<=(A_last_ack+window_size))){
                A_last_ack = packet.acknum;
                cout<<"stoptimer in A_input received successful ack "<<packet.acknum<<" last ack received is "<<A_last_ack<<endl;
                stoptimer(A);
        
	time_gap= timer_list[win_start_index%window_size]-timer_list[(A_last_ack+1)%window_size];
	cout<<"timegep "<<time_gap<<"(A_last_ack+1)%window_size "<<(A_last_ack+1)%window_size<<" win_start_index%window_size "<<win_start_index%window_size<<endl;
	cout<<time_gap<<"(A_last_ack+1)%window_size "<<timer_list[(A_last_ack+1)%window_size]<<" win_start_index%window_size "<<timer_list[win_start_index%window_size]<<endl;

	if(A_next_seq>A_last_ack+1){
		cout<<"starting timer for packet at "<< A_last_ack+1<<endl;
		starttimer(A,T_TIME+time_gap);
	}
	win_start_index = A_last_ack+1;
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
	cout<<"timer interrupt occured "<<win_start_index<<endl;
	cout<<win_start_index<<" "<<retrans_buf[win_start_index].seqnum<<" "<<retrans_buf[win_start_index].acknum<<" "<<retrans_buf[win_start_index].payload<<endl;
	cout<<A_last_ack+1<<" "<<retrans_buf[A_last_ack+1].seqnum<<" "<<retrans_buf[A_last_ack+1].acknum<<" "<<retrans_buf[A_last_ack+1].payload<<endl;
        A_retransmit(retrans_buf[(win_start_index)%(window_size)]);
        cout<<"exiting timer interrupt"<<endl;

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	A_next_seq = 0;
	A_last_ack = -1;
	win_start_index = 0;
	window_size = getwinsize();
//	retrans_buf = (struct pkt *)malloc(sizeof(struct pkt)*window_size);
	timer_list = (int *)malloc(sizeof(int)*window_size);
	memset(retrans_buf,'\0',sizeof(retrans_buf));
	memset(timer_list,'\0',sizeof(timer_list));
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	cout<<"in B_input function "<<"expected seq is "<< B_expected_seq<<endl;
        /*struct msg message_recvd;*/
        int checksum = get_checksum(packet);
        if((checksum == packet.checksum)&&packet.seqnum == B_expected_seq){
                cout<<"in B_input packet received successfully "<<packet.seqnum<<" " <<packet.payload<<endl;
                /*strcpy(message_recvd.data, packet.payload);*/
                tolayer5(B,packet.payload);
                packet.acknum = packet.seqnum;
                memset(packet.payload, '\0', sizeof(packet.payload));
                packet.checksum = get_checksum(packet);
                tolayer3(B,packet);
                B_expected_seq = B_expected_seq+1;
        }
	else if((checksum == packet.checksum)&&packet.seqnum < B_expected_seq){
                packet.acknum = packet.seqnum;
                memset(packet.payload, '\0', sizeof(packet.payload));
                packet.checksum = get_checksum(packet);
                tolayer3(B,packet);

	}
        else{
                cout<<"unexpected or corrupted packet"<<endl;
                cout<<"in B_input received seq and ack "<<packet.seqnum<< " and "<<packet.acknum<<" msg is "<< packet.payload<<endl;
                /*packet.acknum = B_expected_seq;
                packet.seqnum = B_expected_seq;
                memset(packet.payload, '\0', sizeof(packet.payload));
                packet.checksum = get_checksum(packet);
                tolayer3(B,packet);*/
        }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	B_expected_seq = 0;
}
