/* ****************************************************************
 *      File Name:  		sender.c
 *      Description:      This file is emulated the sender of stop-and-wait protocol
 *      Execute File:      ./sender
 *      Author:   			 Duan Cong 01091138
 ******************************************************************/
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "protocol.h"
//#define MAX_SEQ 1
typedef enum { frame_arrival, chksum_err, timeout } event_type;
void InitSocket ( void );
void Sender ( void );
/*************************************************************************
  *      Function:          InitSocket
  *      Paramters:        NIL
  *      Return:          	  Void
  *      Description:      Initial Socket Communication
***************************************************************************/
void InitSocket ( void )
{
       printf ( "Input the # of frames : " );
       scanf ( "%d", &NumOfFrames );
       printf ( "Input Port Num : " );
       scanf ( "%d", &Port );
       printf ( "\n" );
       if ( (sockfd = socket ( AF_INET, SOCK_STREAM, 0 )) < 0 )
       {
              perror ( "Client : socket() failed!\n" );
              exit (1);
       }
       bzero ( &Client, sizeof (Client) );
       Client.sin_family = AF_INET;
       Client.sin_addr.s_addr = inet_addr ( ServerIP );
       Client.sin_port = htons (Port);
       printf ( "Waiting...\n\n" );
       if ( connect ( sockfd, (struct sockaddr *) &Client, sizeof (Client) ) < 0 )
       {
              perror ( "Client : Cannot connect to Server\n" );
              exit (1);
       }
       printf ( "Client connects to the Server Successfully.....\n\n" );
}
 
/*************************************************************************
  *      Function:           	Sender
  *      Paramters:          NIL
  *      Return:           	Void
  *      Description:         Send frame to Receiver, then receive frame form Receiver    ***************************************************************************/
void Sender ( void )
{
       seq_nr next_frame_to_send;
       frame s, r;
       packet buff;
       event_type event;
       int i;
       next_frame_to_send = 0;
       from_network_layer (&buff);
       s.kind = data;
       s.ack = 0;
 
       for ( i = 0; i < NumOfFrames; i++ )
       {
              s.info = buff;
              s.seq = next_frame_to_send;
 
              printf ( "Send frame (data) %d ......\n", s.seq );
              to_physical_layer (&s);
              if ( i == NumOfFrames - 1 )
                     break;
              start_timer (s.seq);
              sleep (1);
              wait_for_event (&event);
              if ( event == frame_arrival )
              {
                     printf ( "Receive frame (ACK) %d ......\n\n", s.seq );
                     from_physical_layer (&r);
                     from_network_layer (&buff);
                     inc (next_frame_to_send);
              }
       }
       wait_for_event (&event);
       if ( event == frame_arrival )
              printf ( "Receive frame (ACK) %d ......\n\n", s.seq );
}
/*************************************************************************
  *      Function:           main
  *      Paramters:             NIL
  *      Return:           NIL
  *      Description:             Main
***************************************************************************/
main ()
{
       system ( "clear" );
       InitSocket ();
       enable_network_layer ();
       Sender ();
}