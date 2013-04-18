/* ****************************************************************
 *      File Name:  		protocol.h
 *      Description:      	This file is emulated the Network Layer and Data Link Layer for ISO/OSI protocol
 *      Author:   			Zhou Hao
 ******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "wrapsock.h"

#define MAX_SEQ 6
#define BUFFER_SIZE 190
#define PKT_SIZE 192
#define MAX_FRM_PAYLOAD 100
#define FRM_SIZE 105  //include 1 byte for Error Detection
#define MAX_BYTE_STREAM_SIZE 200
#define BUF_SIZE 18   //This should be replaced
#define ServerIP "127.0.0.1"
#define inc(k) if (k < MAX_SEQ) k = k + 1; else k = 0
typedef enum {frame_arrival, cksum_err, timeout, network_layer_ready} event_type;
typedef unsigned char seq_nr;
typedef unsigned char uchar;
typedef enum { false, true } boolean;
/* --- Define Packet Structure [MAX 192 bytes (fixed size) And 190 for payload] --- */
typedef struct
{
	uchar size;
	uchar eop;
    uchar data[BUFFER_SIZE];
} packet;
/* --- Define frame Structure [MAX 1+1+1+1+100=104 bytes (fixed size)]--- */
typedef struct
{
	uchar size;
	seq_nr seq;//1bytes
	seq_nr ack;//add 1 byte
    uchar eof;
	uchar ed;
	uchar data[MAX_FRM_PAYLOAD];
} frame;

/* --- declaration of Global Variables --- */
FILE *stream;
struct sockaddr_in Server, Client;
int sockfd, newsockfd;
int Port=5000;
uchar pktBuf[BUFFER_SIZE];
uchar pktEOP='0';
uchar pktSize=0;
event_type event;
int NETWORK_LAYER, Index = 0;
int IsFileTransmitting=0;
time_t timer[MAX_SEQ+1];
int frmSend=0;
int frmRtr=0;
int dataFrmRecvCrt=0;
int dataFrmRecvErr=0;
int ACKFrmRecvCrt=0;
int ACKFrmRecvErr=0;
int BlockTimes=0;

/* ----- ARQ Protocol Functions ----- */
void from_network_layer ( packet *p );
void to_network_layer ( packet p );
int from_physical_layer (frame *r );
void to_physical_layer ( frame s );
void start_timer ( seq_nr k );
void stop_timer ( seq_nr k );
void enable_network_layer ( void );
void disable_network_layer ( void );

/* -- Event Types -- */
void wait_for_event_recv( event_type *event )
{
       fd_set rfds, wfds;
       FD_ZERO (&rfds);
       FD_SET ( sockfd, &rfds );
       FD_ZERO (&wfds);
       FD_SET ( sockfd, &wfds );
       if(select( sockfd+1, &rfds, &wfds, NULL, NULL) != -1 )
       {
		   //if(FD_ISSET(sockfd, &rfds))
		   //{
			   //printf ( "Even : frame_arrival\n" );
			   *event = frame_arrival;
			   return;
		   //}
       }
}
void wait_for_event_send(event_type *event )
{
       fd_set rfds, wfds;
       time_t t;
       int i;
       FD_ZERO (&rfds);
       FD_SET ( sockfd, &rfds );
       FD_ZERO (&wfds);
       FD_SET ( sockfd, &wfds );
       time (&t); //current time

       for ( i = 0; i <= MAX_SEQ; i++ )
	   {
          if ( timer[i] != 0 && t - timer[i] > 2)//5 seconds means timeout
          {
              printf ( "Even : TimeOut seq=%d\n",i );
              *event = timeout;
              return;
          }
		}
		if(NETWORK_LAYER==1)
        {
			//printf ( "Even : network_layer_ready\n" );
			*event = network_layer_ready;
			return;
		}
		if(select( sockfd+1, &rfds, NULL, NULL, NULL) != -1 )
        {

		   //if(FD_ISSET(sockfd, &rfds))
		   //{
			   //printf ( "Even : frame_arrival\n" );
			   *event = frame_arrival;
			   return;
		   //}
        }

}
  /* --- Message To Packet --- */
  //use four parameters to produce a packet(pkt), return 1(convert successfully)/0(failed)
  //every command can be encapsulated into one packet, only files need plenty of packets.
int message_to_packet ( const uchar pkt_size, const uchar pkt_eop, const uchar *buffer, packet *pkt )
{
		int returnValue=0;
	    pkt->size=pkt_size;
	    pkt->eop=pkt_eop;
	    bzero(pkt->data,BUFFER_SIZE);
	    memcpy(pkt->data,buffer,pkt_size); //pkt_size VS 190?
		returnValue=1;
		return returnValue; //this return value make no sense (by coder---- Hao Zhou)
}
 /* --- divide Packet into two Pieces--- */
 //each packet generates two data frames
int packet_to_two_buffers_for_frame ( const packet pkt, uchar (*buffer)[MAX_FRM_PAYLOAD], uchar *size, uchar *frmEOF)
{
        frmEOF[0]='0';
        frmEOF[1]='0';
        bzero(buffer[0],MAX_FRM_PAYLOAD);
        bzero(buffer[1],MAX_FRM_PAYLOAD);

		if(pkt.size > MAX_FRM_PAYLOAD-2) //file transmission, one packet for two frames
		{
            if(pkt.eop=='1')
            {
                frmEOF[1]='1';
            }
            buffer[0][0]=pkt.size;
            buffer[0][1]=pkt.eop;
            memcpy(buffer[0]+2,pkt.data,MAX_FRM_PAYLOAD-2);
            memcpy(buffer[1],pkt.data+MAX_FRM_PAYLOAD-2,pkt.size-MAX_FRM_PAYLOAD+2);
            size[0]=MAX_FRM_PAYLOAD;
            size[1]=pkt.size-MAX_FRM_PAYLOAD+2;
            return 2;
		}
		else    //command line, one packet for one frame
		{
		    if(pkt.eop=='1')
            {
                frmEOF[0]='1';
            }
            buffer[0][0]=pkt.size;
            buffer[0][1]=pkt.eop;
            memcpy(buffer[0]+2,pkt.data,pkt.size);
            size[0]=pkt.size+2;
            size[1]=0;
            return 1;
		}
}
void buffer_to_frame(const uchar *str, const uchar size,const uchar seq, const uchar eof, frame *frm)
{
    bzero(frm->data, MAX_FRM_PAYLOAD);
    frm->size=size;
    frm->seq=seq;
    frm->eof=eof;
    frm->ack='\0';//it's useless here(I won't use paggyback)
    frm->ed='\0';
    memcpy(frm->data,str,size);
}
void generate_ACK_frame(const uchar ack, frame *frm)
{
	bzero(frm->data, MAX_FRM_PAYLOAD);
    frm->size='\0';
    frm->seq='\0';
    frm->eof='\0';
    frm->ack=ack;
    frm->ed='\0';
}
/* --- Generate Error Detection Field for a Frame --- */
//return frame string with ED field
int add_ed_to_frame(const frame frm, uchar *streamStr)//streamStr is fixed size:105 bytes
{
        uchar edField='\0';
        int i=0;
        //srand ((int)time (NULL));
        int rdm=0;
        bzero(streamStr,FRM_SIZE);
        edField=edField^frm.size;
        edField=edField^frm.seq;
        edField=edField^frm.ack;
        edField=edField^frm.eof;
        for(i=0; i<frm.size; i++)
        {
            edField=edField^frm.data[i];
        }
        streamStr[0]=frm.size;
        streamStr[1]=frm.seq;
        streamStr[2]=frm.ack;
        streamStr[3]=frm.eof;
        memcpy(streamStr+4,frm.data,frm.size);
        rdm=rand()%100;
        //printf("\t\t\t\trdm=%d \n",rdm);
        if(rdm > 98)
        {
			printf("\t\t\t\trdm=%d \n",rdm);
			edField=edField^1;
		}

        streamStr[FRM_SIZE-1]=edField;
        return 0;
}
int frmString_with_flag_after_stuffing(const uchar *frmStringSrc, uchar *frmStringDes) //ppp byte stuffing:7D->7D5D    7E->7D5E; return length
{
        int i=0; //for loop control
        int j=0; //for index of frmStringDes
        bzero(frmStringDes,MAX_BYTE_STREAM_SIZE);
        frmStringDes[j++]=0X7E; //flag for head

        for(i=0; i<FRM_SIZE;i++)//FRM_SIZE=104
        {
            if(frmStringSrc[i]==0X7D)
            {
                frmStringDes[j++]=0X7D;
                frmStringDes[j++]=0X5D;
            }
            else if( frmStringSrc[i]==0X7E)
            {
                frmStringDes[j++]=0X7D;
                frmStringDes[j++]=0X5E;
            }
            else
            {
                frmStringDes[j++]=frmStringSrc[i];
            }
        }

        frmStringDes[j++]=0X7E; //flag for tail
        return j;
}

int destuffing_of_streamStr(const uchar *frmStringSrc, uchar *frmStringDes)//frmStringSrc with head and tail of '7E' to control the loop
{
        int i=1;
        int j=0;
        while(frmStringSrc[i]!=0X7E)
        {
            if(frmStringSrc[i]==0X7D && frmStringSrc[i+1]==0X5D)
            {
                frmStringDes[j++]=0X7D;
                i=i+2;
            }
            else  if(frmStringSrc[i]==0X7D && frmStringSrc[i+1]==0X5E)
            {
                frmStringDes[j++]=0X7E;
                i=i+2;
            }
            else
            {
                frmStringDes[j++]=frmStringSrc[i++];
            }
        }
        return j;
}

void streamStr_to_frame(const uchar streamStr, frame *frm)
{

}
/* --- Check Error Detection Field of an arrived Frame --- */
int check_sum(const uchar *streamStr, frame *frm)//no error return true, otherwise return false(true for frame_arrival, false for cksum_error)
{
    int i;
    uchar ed='\0';
    bzero(frm->data,MAX_FRM_PAYLOAD);
    frm->size=streamStr[0];
    frm->seq=streamStr[1];
    frm->ack=streamStr[2];
    frm->eof=streamStr[3];
    for(i=0; i<frm->size+4;i++)
    {
        ed=ed^streamStr[i];
        if(i>3)
        {
            frm->data[i-4]=streamStr[i];
        }
    }
    frm->ed=streamStr[FRM_SIZE-1];
    if(ed==streamStr[FRM_SIZE-1])
        return 1;
    else
        return 0;
}
void two_frames_to_packet(const frame oddFrm, const frame evenFrm, packet *pkt)
{
    bzero(pkt->data, BUFFER_SIZE);
    pkt->size=oddFrm.data[0];
    pkt->eop=oddFrm.data[1];
    memcpy(pkt->data,oddFrm.data+2,oddFrm.size-2);
    memcpy(pkt->data+oddFrm.size-2,evenFrm.data,evenFrm.size);
}
void one_frame_to_packet(const frame oddFrm, packet *pkt)
{
    bzero(pkt->data, BUFFER_SIZE);
    pkt->size=oddFrm.data[0];
    pkt->eop=oddFrm.data[1];
    memcpy(pkt->data,oddFrm.data,oddFrm.size);

}
int packet_to_message (uchar *buffer, const packet pkt, uchar *size )
{
		int IsEOP=0;
		IsEOP=pkt.eop;
	    memcpy(buffer,pkt.data,pkt.size);
        *size=pkt.size;
		return IsEOP;
}
/* --- Get a Packet from Network Layer --- */
void Refresh()
{
	bzero(pktBuf,BUFFER_SIZE);
	pktSize = fread(pktBuf,1,BUFFER_SIZE,stream);
	if(pktSize<BUFFER_SIZE)
		pktEOP='1';
	else
		pktEOP='0';

}
void from_network_layer ( packet *p )
{
    if (NETWORK_LAYER)
    {
		Refresh();
		message_to_packet (pktSize, pktEOP, pktBuf, p);
    }
}
/* --- Send a Packet to Network Layer --- */
void to_network_layer ( const packet p )
{
    if (NETWORK_LAYER)
    {
		fwrite(p.data,sizeof(char),p.size,stream);
    }
}

/* --- Get a Frame from Physical Layer --- */
int from_physical_layer ( frame *r ) //return 0 means cksum_error
{
	uchar buffer[MAX_BYTE_STREAM_SIZE];
	uchar frmStr_ed[FRM_SIZE];
	bzero( frmStr_ed, FRM_SIZE );
    bzero( buffer,MAX_BYTE_STREAM_SIZE );
    recv( sockfd, buffer,MAX_BYTE_STREAM_SIZE, 0 );
    destuffing_of_streamStr(buffer,frmStr_ed);//before cksum
    return check_sum(frmStr_ed, r);
}

/* --- Send a Frame to Physical Layer --- */
void to_physical_layer (frame s )
{
	uchar frmStr_ed[FRM_SIZE];
	uchar buffer[MAX_BYTE_STREAM_SIZE];
	bzero( frmStr_ed, FRM_SIZE );
    bzero( buffer, MAX_BYTE_STREAM_SIZE );
	add_ed_to_frame(s, frmStr_ed);
	frmString_with_flag_after_stuffing(frmStr_ed, buffer);
    send (sockfd, buffer, MAX_BYTE_STREAM_SIZE, 0 );
}

/* ---  Start Frame Timer --- */
void start_timer ( seq_nr k )
{
       time_t t;
       timer[k % MAX_SEQ] = time (&t);
}

/* --- Stop Frame Timer --- */
void stop_timer ( seq_nr k )
{
       timer[k % MAX_SEQ] = 0;
}

/* --- Enable Network Layer --- */
void enable_network_layer ( void )
{
       srand (time (NULL));
       NETWORK_LAYER = 1;
}

/* --- Disable Network Layer --- */
void disable_network_layer ( void )
{
       NETWORK_LAYER = 0;
}
/*--- Return 1 if a<=b<=c circularly, 0 otherwise ---*/
int between(seq_nr a, seq_nr b, seq_nr c)
{
	if(((a<=b)&&(b<c)) ||((b<c)&&(c<a)) ||((c<a)&&(a<=b)))
		return 1;
	else
		return 0;
}
void send_data(seq_nr frame_nr, frame buffer[])
{
	frame s;
	s=buffer[frame_nr];
	to_physical_layer(s);
	start_timer(frame_nr);
}
void send_ACK(seq_nr frame_nr)
{
	frame s;
	generate_ACK_frame(frame_nr,&s);
	to_physical_layer(s);
	//start_timer(frame_nr);
}
/*--- operations on Data Link Layer ---*/
void Data_Link_Layer_Recv()
{
	int COUNT=1;
	//time_t t,stopTime;
	seq_nr frame_expected=0;
	seq_nr i=0;
	int frmRecvCount=0;
	frame bufRecvFrm[2];
	packet pkt;
	int IsEveryThingTransmitted=0;
	enable_network_layer();
	while(1)
	{
		wait_for_event_recv(&event);
		switch(event)
		{
			case frame_arrival:
				 if(from_physical_layer(&bufRecvFrm[frmRecvCount])==0) //cksum_err here
				 {

					 dataFrmRecvErr++;
					 printf("CK_ERROR Seq=%d\n",bufRecvFrm[frmRecvCount].seq);
					 break;
				 }
				 else
				 {
					 //frmRecv++;
				 	 dataFrmRecvCrt++;
					 if(bufRecvFrm[frmRecvCount].seq==frame_expected)
					 {
						 frmRecvCount++;
						 if(frmRecvCount==1 && bufRecvFrm[0].eof=='1')
						 {
						 	 one_frame_to_packet(bufRecvFrm[0],&pkt);
						 	 to_network_layer(pkt);
						 	 IsEveryThingTransmitted=1;
						 }

						 if(frmRecvCount==2)
						 {
						 	two_frames_to_packet(bufRecvFrm[0], bufRecvFrm[1], &pkt);
						 	if(pkt.eop=='1')
						 	{
						 		IsEveryThingTransmitted=1;
						 	}
							frmRecvCount=0;
							to_network_layer(pkt);
						 }
						 send_ACK(frame_expected);//send ACK frame
						 frmSend++;
						 //printf("ACK_Send=%d\tRecv_COUNT=%d\n",frame_expected,COUNT++);
						 inc(frame_expected);
					 }
					 else
					 {
						 send_ACK(bufRecvFrm[frmRecvCount].seq);
						 frmRtr++;
						 printf("Re-Send_SEQ=%d\n",bufRecvFrm[frmRecvCount].seq);
					 }
				 }
				 break;
		}
		if(IsEveryThingTransmitted==1)
		{
			//send_ACK(frame_expected);
			break;
		}
	}
	printf("==============================================================\n");
	printf("# of acknowledgments sent(No retransmissions):       \t%d\n",frmSend);
	printf("# of acknowledgments retransmissions sent:           \t%d\n",frmRtr);
	printf("# of data frame received correctly:         	     \t%d\n",dataFrmRecvCrt);
	printf("# of data frame received with errors:                \t%d\n",dataFrmRecvErr);
	printf("==============================================================\n");
}
void Data_Link_Layer_Send()
{
	int COUNT=1;
	seq_nr next_frame_to_send=0;
	seq_nr ACK_expected=0;
	seq_nr nbuffered=0;
	seq_nr i=0;
	int flag=0;
	uchar strBuffer[2][MAX_FRM_PAYLOAD];
	uchar *oddStrBuffer=strBuffer[0];
	uchar *evenStrBuffer=strBuffer[1];
	uchar frmStreamStr[MAX_BYTE_STREAM_SIZE];
	uchar size[2]={0,0};
	uchar frmEOF[2]={'0','0'};
	int frmCount=0;
	uchar lastSeq=MAX_SEQ+1;

	frame r,s,bufFrm[MAX_SEQ+1];
	packet pkt;
	int IsEveryThingTransmitted=0;
	int NoMore_network_layer_ready=0;
	enable_network_layer();
	while(1)
	{
		wait_for_event_send(&event);
		switch(event)
		{
			case network_layer_ready:
				 if(NoMore_network_layer_ready==0)
				 {
					 from_network_layer(&pkt);//divide the packet into 1~2 frame
					 frmCount=packet_to_two_buffers_for_frame(pkt,strBuffer,size,frmEOF);
					 for(i=0; i < frmCount; i++)
					 {
						buffer_to_frame(strBuffer[i], size[i],next_frame_to_send, frmEOF[i], &bufFrm[next_frame_to_send]);
						nbuffered++;
						send_data(next_frame_to_send,bufFrm);
						frmSend++;
						//printf("Seq=%d\tACK_expected=%d\tOUNT=%d, time=%ld\n",next_frame_to_send,ACK_expected,COUNT++,timer[next_frame_to_send]);
						//printf("Seq=%d\tACK_expected=%d\tSend_COUNT=%d\tnbuffered=%d\n",next_frame_to_send,ACK_expected,COUNT++,nbuffered);

						if(frmEOF[i]=='1')
						{
							lastSeq=next_frame_to_send;
							NoMore_network_layer_ready=1;
						}
						inc(next_frame_to_send);
					 }
				 }
				 break;
			case frame_arrival:
				 if(from_physical_layer(&r)==0) //cksum_err here
				 {
					 ACKFrmRecvErr++;
					 //printf("CKsum_error ACK_Recv=%d\n",r.ack);
					 //send_data(r.ack,bufFrm);
					 //frmSend++;
					 break;
				 }
				 else
				 {
					 ACKFrmRecvCrt++;
					 //printf("ACK=%d\n",r.ack);
					 if(r.ack==lastSeq)
					 {
						IsEveryThingTransmitted=1;
						break;
					 }
					 while(between(ACK_expected, r.ack, next_frame_to_send)==1)
					 {
						 nbuffered--;
						 stop_timer(ACK_expected);
						 inc(ACK_expected);
					 }

				 }
				 break;
			//case cksum_err:break;// It is included in frame_arrival event
			case timeout:
				 frmRtr++;
				 if(lastSeq!=MAX_SEQ+1)
					break;
				 //printf("TimeOut Here\nACK_expected=%d\tnbuffered=%d\n",ACK_expected,nbuffered);
				 next_frame_to_send=ACK_expected;
				 for(i=1;i<=nbuffered;i++)
				 {
					 send_data(next_frame_to_send,bufFrm);
					 frmRtr++;
					 //printf("next_frame_to_send=%d\n",next_frame_to_send);
					 inc(next_frame_to_send);
				 }
				 break;
		}
		if(lastSeq!= MAX_SEQ+1)
		{
			//break;
		}
		if(frmRtr>=13)
			break;
		if(nbuffered<MAX_SEQ-1)// ensure there will be two more free spaces in the buffer
		{
			flag=0;
			enable_network_layer();
		}
		else
		{
			disable_network_layer();
			if(flag==0)
			{
				BlockTimes++;
				flag=1;
			}
		}
	}
	printf("==============================================================\n");
	printf("# of data frames sent(No retransmissions):       \t%d\n",frmSend);
	printf("# of retransmissions sent:                       \t%d\n",frmRtr);
	printf("# of acknowledgments received correctly:         \t%d\n",ACKFrmRecvCrt);
	printf("# of acknowledgements received with errors:      \t%d\n",ACKFrmRecvErr);
	printf("# of times DDL blocks due to a full window.:     \t%d\n",BlockTimes);
	printf("==============================================================\n");
}

