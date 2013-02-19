/* ****************************************************************
 *      File Name: 		receiver.c
 *      Description:     This file is emulated the receiver of stop-and-wait protocol
 *      Execute File:     ./receiver
 *      Author:   			Duan Cong 01091138
 ******************************************************************/
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "protocol.h"

void InitSocket ( void );
void Receiver ( void );
/*************************************************************************
  *      Function:         	InitSocket
  *      Paramters:          NIL
  *      Return:         		Void
  *      Description:         Initial Socket Communication
***************************************************************************/
void InitSocket ( void )
{
       int CliLen;
       if ( (newsockfd = socket ( AF_INET, SOCK_STREAM, 0 )) < 0 )
       {
              perror ( "Server :socket() failed\n" );
              exit (1);
       }
       bzero ( (char *) &Server, sizeof (Server) );
       Server.sin_family = AF_INET;
       Server.sin_addr.s_addr = htonl (INADDR_ANY);
       Server.sin_port = htons (Port);
       if ( bind ( newsockfd, (struct sockaddr *) &Server, sizeof (Server) ) < 0 )
       {
              perror ( "Server :bind() failed\n" );
              exit (1);
       }
       listen ( newsockfd, 1 );
       printf ( "Wait..........\n\n" );
       CliLen = sizeof (Client);
       sockfd = accept ( newsockfd, (struct sockaddr *) &Client, &CliLen );
       printf ( "Receive Client successfully....\n\n" );
}
/*************************************************************************
  *      Function:          	Sender
  *      Paramters:          NIL
  *      Return:          		Void
  *      Description:        Receive frame from Sender, then send frame with ack to sender 
***************************************************************************/
void Receiver ( void )
{
       seq_nr frame_expected;
       frame r, s;
       event_type event;
       int i;
       s.kind = ack;
       s.seq = 0;
       s.ack = 1;
       strcpy ( s.info.data, "" );
       frame_expected = 0;
       for ( i = 0; i < NumOfFrames; i++ )
       {
              wait_for_event (&event);
              if ( event == frame_arrival )
              {
                     from_physical_layer (&r);
                     printf ( "Receive frame (data)  %d ......\n", r.seq );
                     if ( r.seq == frame_expected )
                     {
                           to_network_layer (&r.info);
                           inc (frame_expected);
                     }
                     printf ( "Send ACK ......\n" );
                     to_physical_layer (&s);
                     sleep (1);
              }
              printf ( "\n" );
       }
}
/*************************************************************************
  *      Function:          main
  *      Paramters:        NIL
  *      Return:          	  NIL
  *      Description:      main
***************************************************************************/
main ()
{
       system ( "clear" );
       InitSocket ();
       enable_network_layer ();
       Receiver ();
}