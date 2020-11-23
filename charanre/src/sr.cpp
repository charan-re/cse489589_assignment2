#include "../include/simulator.h"
#include <iostream>
#include <string.h>
#include <vector>
#include <algorithm>

#define A 0
#define B 1
#define T_TIME 20

using namespace std;

int next_seq;
int last_ack;
int expected_seq;
int window_size;
int win_start_index;
int total_packets;
vector <struct pkt> msg_buffer;
vector <struct pkt> rcv_buffer;
vector <float> timer_list;
int t_index;
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

void A_output(struct msg message)
{

	/* receive message from layer 5 make packet 
 * 	if no packet are in transit transmit and start the timer 
 * 	if packets in transit and and window is free send packet to layer 3
 * 	store the time for each packet when it is transmitted*/
	struct pkt new_packet;
	new_packet.seqnum = total_packets;
	new_packet.acknum = total_packets;
	total_packets++;
	strcpy(new_packet.payload, message.data);
	new_packet.checksum = get_checksum(new_packet);
	msg_buffer.push_back(new_packet);
	cout<<"A_out buffering "<<new_packet.seqnum<<" "<<new_packet.acknum<<" "<<new_packet.payload<<" at "<<msg_buffer.size()<<endl;
	cout<<"next_seq "<<next_seq<<" win_start_index "<<win_start_index<<" "<<timer_list.size()<<endl;
	cout<<"msg_buffer_size "<<msg_buffer.size()<<"next seq "<<next_seq<<" win start "<<win_start_index<<endl;
	cout<<"A_out "<<msg_buffer[msg_buffer.size()-1].seqnum<< " "<<msg_buffer[msg_buffer.size()-1].payload<<endl;
	if(next_seq>=win_start_index+window_size){
		cout<<"unacked limit exceeded"<<endl;
	}
	else{
		struct pkt tx_pkt ;
		cout<<"last packet in buffer is "<<msg_buffer[msg_buffer.size()-1].seqnum<< " "<<msg_buffer[msg_buffer.size()-1].payload<<endl;
		for(auto i :msg_buffer){
			if(i.seqnum == next_seq)
				tx_pkt = i;
		}
		//struct pkt tx_pkt = msg_buffer[next_seq-win_start_index];
		//msg_buffer.erase(msg_buffer.begin());
		tolayer3(A,tx_pkt);
		timer_list.push_back(get_sim_time());
		cout<<"A_out txing "<<tx_pkt.seqnum<<" "<<tx_pkt.payload<<" at "<<get_sim_time()<<" buf sizes "<<msg_buffer.size()<<" "<<timer_list.size()<<endl;
		if(win_start_index == next_seq){
			cout<<"starting timer in A_output "<<tx_pkt.seqnum<<" "<<tx_pkt.payload<<endl;
			starttimer(A,T_TIME);
		}
	next_seq++;
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	int w_index=0;
	int checksum = get_checksum(packet);
	/*received ack is out of the window ignore*/
	if(packet.seqnum<last_ack){
		cout<<"duplicate ack"<<endl;
	}
	/*if the received ack is the ack for packet for which timer is running then 
 * 	stop the timer and start timer for next in sequence with relative timeout value 
 * 	delete that packet and correspondig timer index from the buffer, if there are untransmitted packets in buffer and window is not full 
 * 	send the packets to layer3 towards B */ 
	else if((get_checksum(packet) == packet.checksum)&&(packet.acknum == win_start_index)){
                //stoptimer(A);
                cout<<"stoptimer in A_input received successful ack "<<packet.acknum<<" last ack received is "<<last_ack<<endl;
		cout<<"next_seq "<<next_seq<<" win_start_index "<<win_start_index<<" buf_size "<<msg_buffer.size()<<" "<<timer_list.size()<<endl;
                last_ack = packet.acknum;
		cout<<"Received A_in "<<packet.acknum<<" "<<packet.payload<<" at "<<get_sim_time()<<" "<<timer_list.front()<<" "<<timer_list.size()<<endl;
		if(t_index >0 ){
			cout<<"t_index is in middle of buffer "<<t_index<<endl;
                        t_index--;
		}
                else if(t_index ==0 && timer_list.size()==1){
			cout<<"t_index is the start of the buffer"<<endl;
                        stoptimer(A);
                }
                else if(t_index ==0 && timer_list.size()==1){
			cout<<"t_index is the start of the buffer"<<endl;
                        stoptimer(A);
                        starttimer(A,T_TIME-(get_sim_time()-timer_list.front()));

                }
		msg_buffer.erase(msg_buffer.begin());
		timer_list.erase(timer_list.begin());
		if(!msg_buffer.size())
			win_start_index = next_seq;
		else
			win_start_index = last_ack +1;
		if(msg_buffer.size()){
			win_start_index = msg_buffer.front().seqnum;
			cout<<"starting timer for "<<win_start_index<< " "<<msg_buffer.front().payload<<endl;
			for(auto i : msg_buffer){
				if(next_seq-win_start_index >=window_size){
					break;
				}
				if(i.seqnum == next_seq){
					tolayer3(A,i);
					timer_list.push_back(get_sim_time());
					next_seq++;
					if(win_start_index == next_seq){
						cout<<"starting timer in A_output "<<i.seqnum<<" "<<i.payload<<endl;
						starttimer(A,T_TIME);
					}
				}
			}
			//cout<<"A_in starting timer "<<T_TIME-(get_sim_time()-timer_list.front())<<" for "<<msg_buffer.front().payload<<endl;
			//starttimer(A,T_TIME-(get_sim_time()-timer_list.front()));
		}
		else
			win_start_index == next_seq;
		
		cout<<"next_seq "<<next_seq<<" win_start_index "<<win_start_index<<" buf_size "<<msg_buffer.size()<<" "<<timer_list.size()<<endl;
		//win_start_index = last_ack +1;
	}
	/*if the received packet is out if sequence and in acceptable window
 * 	clear the packet from the buffer */
	else if((get_checksum(packet) == packet.checksum)&&(packet.acknum < next_seq)){
                cout<<"not stoptimer in A_input received successful ack "<<packet.acknum<<" last ack "<<last_ack<<" timerlist "<<timer_list.size()<<endl;
		for(auto i : msg_buffer){
			if(i.seqnum == packet.seqnum){
				msg_buffer.erase(msg_buffer.begin()+w_index);
				timer_list.erase(timer_list.begin()+w_index);
				if(t_index>=w_index)
					t_index--;
				break;
			}
			w_index++;
		}
		for(auto i : msg_buffer){
			if(next_seq-win_start_index >=window_size){
				cout<<"window exceeded in A_in "<<next_seq<<" "<<win_start_index<<endl;
				return;
			}
                	if(i.seqnum == next_seq){
                        	tolayer3(A,i);
				timer_list.push_back(get_sim_time());
				next_seq++;
                        }
                        }
                cout<<"exit not stoptimer in A_input received successful ack "<<packet.acknum<<" last ack "<<last_ack<<" timerlist "<<timer_list.size()<<endl;
	}
		
}

/* called when A's timer goes off */
/* retransmit the packet for which timeout occured update the start tie for the packet to current simulator time
 * start timer for the next packet in sequence with a relative timeoutvalue of its starttime*/
void A_timerinterrupt()
{
	cout<<"timer interrupt occured "<<msg_buffer[t_index%timer_list.size()].seqnum<<" "<<msg_buffer[t_index%timer_list.size()].payload<<" t_index "<<t_index<< " "<<win_start_index<<endl;
	tolayer3(A,msg_buffer[t_index%timer_list.size()]);
	timer_list[t_index%timer_list.size()] = get_sim_time();
	//starttimer(A,T_TIME);
	if(timer_list.size()>1){
                t_index = (t_index+1)%timer_list.size();
                cout<<"in retransmit Starting timer for next "<<msg_buffer[t_index%timer_list.size()].seqnum<<" "<<msg_buffer[t_index%timer_list.size()].payload<<endl;
                starttimer(A,T_TIME-(get_sim_time()-timer_list[t_index%timer_list.size()]));
        }
        else{
                cout<<"Starting timer for current "<<msg_buffer[t_index%timer_list.size()].seqnum<<" "<<msg_buffer[t_index%timer_list.size()].payload<<endl;
                starttimer(A,T_TIME);
        }	
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	window_size = getwinsize();
	next_seq = 0;
	total_packets = 0;
	last_ack = -1;
	win_start_index = 0;
//	timer_list = (int *)malloc(sizeof(int)*window_size);
//	memset(timer_list,'\0',sizeof(timer_list));
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
bool compareseq(struct pkt pkt1, struct pkt pkt2) 
{ 
    return (pkt1.seqnum < pkt2.seqnum); 
}

void B_input(struct pkt packet)
{
	int checksum = get_checksum(packet);
	cout<<"B_input "<<checksum<<" "<< packet.checksum<<" "<<packet.seqnum<<" "<<expected_seq<<endl;
	/*check if the packet is not corrupt and received packet is at the start of the receive window
  	deliver the packet packet along with all the received in order packets and move the window
  	*/
	if((checksum == packet.checksum)&&(packet.seqnum == expected_seq)){
		cout<<"in seq match "<<packet.seqnum<<" "<<packet.payload<<endl;
		tolayer5(B,packet.payload);
		packet.acknum = packet.seqnum;
                memset(packet.payload, '\0', sizeof(packet.payload));
                packet.checksum = get_checksum(packet);
                tolayer3(B,packet);
                expected_seq = expected_seq+1;
		while(rcv_buffer.size()){
			if(rcv_buffer.front().seqnum == expected_seq){
				tolayer5(B,rcv_buffer.front().payload);
				expected_seq = rcv_buffer.front().seqnum+1;
				rcv_buffer.erase(rcv_buffer.begin());
			}
			else{
				break;
			}
		}
	}
	/*if the received packet is not the ecpected packet at start of the window and within in acceptable window
 	buffer the message and sort the buffer as per sequence numbers
  	*/
	else if((checksum == packet.checksum)&&(packet.seqnum < expected_seq+window_size)&&(packet.seqnum>expected_seq)){
		cout<<"B_in buffering "<<packet.seqnum<<" "<<packet.payload<<endl;
		rcv_buffer.push_back(packet);
		sort(rcv_buffer.begin(),rcv_buffer.end(),compareseq);
	}
	else if((checksum == packet.checksum)&&(packet.seqnum < expected_seq)&&(packet.seqnum>(expected_seq-window_size))){
		cout<<"B_in sending dup ack for "<<packet.seqnum<<" "<<packet.payload<<endl;
		packet.acknum = packet.seqnum;
                memset(packet.payload, '\0', sizeof(packet.payload));
                packet.checksum = get_checksum(packet);
                tolayer3(B,packet);
	}
	else{
		cout<<"recvd packet "<<packet.seqnum<<" "<<packet.payload<<" is out of acceptable range"<<endl;
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	expected_seq = 0;

}
