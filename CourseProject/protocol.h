/* ****************************************************************
 *      File Name:  		protocol.h
 *      Description:        This file is emulated the Network Layer and Data Link Layer for ISO/OSI protocol
 *      Author:   			Zhou Hao
 ******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Common.h"
#include "wrapsock.h"

#define MAX_SEQ 6
#define BUFFER_SIZE 190
#define PKT_SIZE 192
#define MAX_FRM_PAYLOAD 100
#define FRM_SIZE 105  //include 1 byte for Error Detection
#define MAX_BYTE_STREAM_SIZE 200
#define BUF_SIZE 18   //This should be replaced
#define inc(k) if (k < MAX_SEQ) k = k + 1; else k = 0
typedef enum {frame_arrival, cksum_err, timeout, network_layer_ready} event_type;
typedef unsigned char seq_nr;
typedef enum { false, true } boolean;
/* --- Define Packet Structure [MAX 192 bytes (fixed size) And 190 for payload] --- */
typedef struct
{
	uchar size;
	uchar eop;
    uchar data[BUFFER_SIZE];
} packet;
/* --- Define frame Structure [MAX 1+1+1+1+1+100=105 bytes (fixed size)]--- */
typedef struct
{
	uchar size;
	seq_nr seq;//1byte
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
time_t timer[MAX_SEQ];
uchar _inputStr[MAXLEN_CMD];
uchar *inputStr=_inputStr;
/***********Global variables for statistics***********/
int frmSend=0;
int frmRecv=0;
int frmErr=0;

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

       for ( i = 0; i < MAX_SEQ; i++ )
	   {
          if ( timer[i] != 0 && t - timer[i] > 2)//5 seconds means timeout
          {
              //printf ( "Even : TimeOut\n" );
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

		   if(FD_ISSET(sockfd, &rfds))
		   {
			   //printf ( "Even : frame_arrival\n" );
			   *event = frame_arrival;
			   return;
		   }
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
        streamStr[FRM_SIZE-1]=edField;
        return 0;
}

 //ppp byte stuffing:7D->7D5D    7E->7D5E; return length
 //[PPP Stuffing is used here]
int frmString_with_flag_after_stuffing(const uchar *frmStringSrc, uchar *frmStringDes)
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

//frmStringSrc with head and tail of '7E' to control the loop
int destuffing_of_streamStr(const uchar *frmStringSrc, uchar *frmStringDes)
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

/* --- Check Error Detection Field of an arrived Frame --- */
//no error return true, otherwise return false(true for frame_arrival, false for cksum_error)
int check_sum(const uchar *streamStr, frame *frm)
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
    memcpy(pkt->data,oddFrm.data+2,oddFrm.size-2);

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
    Recv( sockfd, buffer,MAX_BYTE_STREAM_SIZE, 0 );
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
    Send(sockfd, buffer, MAX_BYTE_STREAM_SIZE, 0 );
}
void to_physical_layer_sock (frame s , int sock)
{
	uchar frmStr_ed[FRM_SIZE];
	uchar buffer[MAX_BYTE_STREAM_SIZE];
	bzero( frmStr_ed, FRM_SIZE );
    bzero( buffer, MAX_BYTE_STREAM_SIZE );
	add_ed_to_frame(s, frmStr_ed);
	frmString_with_flag_after_stuffing(frmStr_ed, buffer);
    Send(sock, buffer, MAX_BYTE_STREAM_SIZE, 0 );
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
					 printf("CK_ERROR\n");
					 break;
				 }
				 else
				 {
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
						 printf("ACK=%d\tCOUNT=%d\n",frame_expected,COUNT++);
						 inc(frame_expected);
					 }
					 else
					 {
						 send_ACK(bufRecvFrm[frmRecvCount].seq);
					 }
				 }
				 break;
			//case cksum_err:break;// It is included in frame_arrival event
		}
		if(IsEveryThingTransmitted==1)
		{
			//send_ACK(frame_expected);
			break;
		}
	}
}
void Data_Link_Layer_Send()
{
	int COUNT=1;
	int times=1;
	seq_nr next_frame_to_send=0;
	seq_nr ACK_expected=0;
	seq_nr nbuffered=0;
	seq_nr i=0;

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
		//printf("times=%d\n",times++);
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
						//printf("nbuffered=%d\n",nbuffered);
						send_data(next_frame_to_send,bufFrm);
						printf("Seq=%d\tCOUNT=%d\n",next_frame_to_send,COUNT++);

						if(frmEOF[i]=='1')
						{
							lastSeq=next_frame_to_send;
							NoMore_network_layer_ready=1;
							if(nbuffered==1)
							{
								IsEveryThingTransmitted=1;
								break;
							}
							//IsEveryThingTransmitted=1;
						}
						inc(next_frame_to_send);
					 }
				 }
				 break;
			case frame_arrival:
				 if(from_physical_layer(&r)==0) //cksum_err here
				 {
					printf("CKsum_error\n");
					break;
				 }
				 else
				 {

					 //if(r.ack==lastSeq)
					 if(lastSeq!=MAX_SEQ+1)
					 {
						 for(i=0;i<5;i++)
						 {
							 if(r.ack==lastSeq)
							 {
								IsEveryThingTransmitted=1;
								break;
							 }

							 inc(r.ack);
						 }
						 if(IsEveryThingTransmitted==1)
						 {
							 //printf(" ack=%d  next_frame_to_send=%d  nbuffered=%d  laseSeq=%d\n",r.ack,next_frame_to_send,nbuffered,lastSeq);
							 break;
						 }

					 }
					 while(between(ACK_expected, r.ack, next_frame_to_send)==1)
					 {
						 nbuffered--;
						 //printf("\tnbuffered=%d\n",nbuffered);
						 stop_timer(ACK_expected);
						 //printf("ACK=%d\n",ACK_expected);
						 inc(ACK_expected);
					 }
					 //printf(" ack=%d  next_frame_to_send=%d  nbuffered=%d  laseSeq=%d\n",r.ack,next_frame_to_send,nbuffered,lastSeq);
				 }
				 break;
			//case cksum_err:break;// It is included in frame_arrival event
			case timeout:
				 printf("TimeOut Here\nACK_expected=%d\tnbuffered=%d\n",ACK_expected,nbuffered);
				 next_frame_to_send=ACK_expected;
				 for(i=1;i<=nbuffered;i++)
				 {
					 send_data(next_frame_to_send,bufFrm);
					 printf("TimeOUT=%d\n",i);
					 inc(next_frame_to_send);
				 }
				 break;
		}
		if(IsEveryThingTransmitted==1)
		{
			break;
		}

		if(nbuffered<MAX_SEQ-1)// ensure there will be two more free spaces in the buffer
			enable_network_layer();
		else
			disable_network_layer();
	}
	printf("\nOUTSIDE\n");
}
void send_message(const uchar *msg,const uchar msg_size,const uchar msg_eof, int socket)
{
    int number=0;
    packet pkt;
    int i=0;
	uchar strBuffer[2][MAX_FRM_PAYLOAD];
	uchar frmStreamStr[MAX_BYTE_STREAM_SIZE];
	uchar size[2]={0,0};
	uchar frmEOF[2]={'0','0'};
    uchar streamStr[FRM_SIZE];
    uchar frmSeq=0;
    frame frm;
    message_to_packet(msg_size,msg_eof,msg,&pkt);
    number=packet_to_two_buffers_for_frame(pkt,strBuffer,size,frmEOF);
    for(i=0; i < number; i++)
    {

        buffer_to_frame(strBuffer[i], size[i],frmSeq, frmEOF[i], &frm);
        inc(frmSeq);
        to_physical_layer(frm);
        frmSend++;

    }
}
void recv_message(uchar *msg, int socket)
{
    frame frm;
    bzero(msg,BUFFER_SIZE);
    from_physical_layer(&frm);
    frmRecv++;
    strncpy(msg,frm.data+2,MAX_FRM_PAYLOAD-2);
}
int recv_file(uchar *msg,uchar *msg_size, int socket)
{
    uchar buffer[MAX_BYTE_STREAM_SIZE];
	int length = 0;
    int frmCounter=0;
    int count=1;
    frame frm[2];
    packet pkt;
    //uchar str[FRM_SIZE];
    bzero(buffer,MAX_BYTE_STREAM_SIZE);
    bzero(msg,BUFFER_SIZE);
    from_physical_layer(&frm[0]);
    frmRecv++;
    if(frm[0].eof=='1')
    {
        one_frame_to_packet(frm[0],&pkt);
        memcpy(msg, pkt.data,pkt.size);
        *msg_size=pkt.size;
        return 0;
    }
    else
    {
        from_physical_layer(&frm[1]);
        frmRecv++;
        two_frames_to_packet(frm[0], frm[1], &pkt);
        memcpy(msg, pkt.data,pkt.size);
        *msg_size=pkt.size;
        if(pkt.eop=='1')
            return 0;
        else
            return 1;
    }


}
