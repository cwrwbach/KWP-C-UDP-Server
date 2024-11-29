//This uses library: https://github.com/vrtql/websockets.git
//Make with: gcc kiwi_serv.c -lvws -o serv

#include <vws/websocket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <time.h>

#define PORT_1 11366
#define MAX_IN_BUF 256
#define FFT_PAK_LEN 1024

//sockets
struct sockaddr_in servaddr_1, cliaddr_1;
socklen_t cliLen_1;
int sockfd_1;

int finito(char * msg)
    {
    printf(" %s \n",msg);
    return -1;
    }

int do_network_setup()
{
int rx_msg_buffer[256];
char *hello = "ZXP server"; 
cliLen_1 = sizeof(struct sockaddr_in);
socklen_t len;
      
// Creating socket file descriptor 
if ( (sockfd_1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
	finito("Socket creation failed /n"); 
  
//memset(&servaddr_1, 0, sizeof(servaddr_1)); 
memset(&cliaddr_1, 0, sizeof(cliaddr_1)); 
      
// Fill server information 
servaddr_1.sin_family    = AF_INET; // IPv4 
servaddr_1.sin_addr.s_addr = INADDR_ANY; 
servaddr_1.sin_port = htons(PORT_1); 
      
// Bind the socket 
if ( bind(sockfd_1,(const struct sockaddr *)&servaddr_1, sizeof(servaddr_1)) < 0 ) 
    finito("Bind failed\n"); 
  
len = sizeof(cliaddr_1);

printf(" Bound, waiting for incoming \n");
recvfrom(sockfd_1, & rx_msg_buffer, MAX_IN_BUF, 0, ( struct sockaddr *) &cliaddr_1, 
                &len); 
printf(" Got a caller, sending ack \n");

sendto(sockfd_1, (const char *)hello, strlen(hello), 0 , (const struct sockaddr *) &cliaddr_1,len); 

printf("Hello ACK message sent to client.\n");  
return 0;
}

//========

int main(int argc, const char* argv[])
{
//cstr uri;
char uri_string[256];
int debug;
int watch_dog;
// Create connection object
vws_cnx* cnx = vws_cnx_new();

int8_t xfer_buf[1040];

for(int i = 0; i< 1024;i++)
    xfer_buf[i] = i/4;

do_network_setup();

// Set connection timeout to 2 seconds (the default is 10). This applies
// both to connect() and to read operations (i.e. poll()).
vws_socket_set_timeout((vws_socket*)cnx, 5);

// Connect. This will automatically use SSL if "wss" scheme is used.
//cstr uri = "ws://localhost:8181/websocket";

time_t utc_now = time( NULL );
printf(" utc %d \n" , utc_now);

//Complete 'GET' header string is:
sprintf(uri_string,"ws://norsom.proxy.kiwisdr.com:8073/%d/W/F",utc_now);
printf("Header string: %s\n",uri_string);

if (vws_connect(cnx, uri_string) == false)
    {
    printf("Failed to connect to the WebSocket server\n");
    vws_cnx_free(cnx);
    return 1;
    }

// Can check connection state this way. 
assert(vws_socket_is_connected((vws_socket*)cnx) == true);

// Enable tracing. This will dump frames to the console in human-readable
// format as they are sent and received.
//vws.tracelevel = VT_PROTOCOL;

//The following are the series of command to the KIWISDR to set up a waterfall

// Send a TEXT frame
vws_frame_send_text(cnx, "SET auth t=kiwi p=");
usleep(100000);
vws_frame_send_text(cnx,"SET zoom=8 cf=14100");
usleep(100000);
vws_frame_send_text(cnx,"SET maxdb=0 mindb=-100");
usleep(100000);
vws_frame_send_text(cnx,"SET wf_speed=2");
usleep(100000);
vws_frame_send_text(cnx,"SET wf_comp=0");
usleep(100000);
vws_frame_send_text(cnx,"SET ident_user=Lowa Wather");
printf(" Line %d \n",__LINE__);

//LOOPIN
debug = 0;
watch_dog=0;

while(1)
    {
    // Receive websocket message
    vws_msg* reply = vws_msg_recv(cnx);

    if (reply == NULL)
        {
        printf(" No Message  recd. Line: %d \n",__LINE__);
        // There was no message received and it resulted in timeout
        }
    else
        {
        // Free message
        printf(" Received: %d \n",debug++);
        if(watch_dog++ > 30)
            {
            watch_dog = 0;
            vws_frame_send_text(cnx,"SET keepalive");
            }

        for(int i = 0; i< 1024;i++)
            {
            xfer_buf[i] = reply->data->data[i]; //signed dB
            }
        vws_msg_free(reply);

        sendto(sockfd_1, &xfer_buf, FFT_PAK_LEN , 0, (struct sockaddr *) &	cliaddr_1, sizeof(cliaddr_1));
        }
     } //while(1)
 
// Send a BINARY message
vws_msg_send_binary(cnx, (ucstr)"Hello, world!", 14);

// Receive websocket message
vws_msg* reply = vws_msg_recv(cnx);
printf(" Line %d \n",__LINE__);

if (reply == NULL)
    {
    printf(" No Message  recd. Line: %d \n",__LINE__);
    // There was no message received and it resulted in timeout
    }
    else
    {
    // Free message
    vws_msg_free(reply);
    printf(" Line %d \n",__LINE__);
    }
        
printf(" Line %d \n",__LINE__);
vws_disconnect(cnx);
return 0;
}
