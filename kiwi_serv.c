#include <vws/websocket.h>
#include <unistd.h>

//char uri_string[256];

int main(int argc, const char* argv[])
{
//cstr uri;
char uri_string[256];

// Create connection object
vws_cnx* cnx = vws_cnx_new();

// Set connection timeout to 2 seconds (the default is 10). This applies
// both to connect() and to read operations (i.e. poll()).
vws_socket_set_timeout((vws_socket*)cnx, 5);

// Connect. This will automatically use SSL if "wss" scheme is used.
//cstr uri = "ws://localhost:8181/websocket";

time_t utc_now = time( NULL );
printf(" utc %d \n" , utc_now);


//Complete GET header string is:
sprintf(uri_string,"ws://norsom.proxy.kiwisdr.com:8073/%d/W/F",utc_now);
printf("Header string: %s\n",uri_string);

//cstr uri = "ws://norsom.proxy.kiwisdr.com:8073/%d/W/F",utc_now);
       
if (vws_connect(cnx, uri_string) == false)
    {
    printf("Failed to connect to the WebSocket server\n");
    vws_cnx_free(cnx);
    return 1;
    }

// Can check connection state this way. Should always be true here as we
// just successfully connected.
assert(vws_socket_is_connected((vws_socket*)cnx) == true);

// Enable tracing. This will dump frames to the console in human-readable
// format as they are sent and received.
vws.tracelevel = VT_PROTOCOL;

//The following are the series of command to the KIWISDR to set up a waterfall

// Send a TEXT frame
vws_frame_send_text(cnx, "SET auth t=kiwi p=");
usleep(100000);
vws_frame_send_text(cnx,"SET zoom=4 cf=15000");
usleep(100000);
vws_frame_send_text(cnx,"SET maxdb=0 mindb=-100");
usleep(100000);
vws_frame_send_text(cnx,"SET wf_speed=1");
usleep(100000);
vws_frame_send_text(cnx,"SET wf_comp=0");
usleep(100000);
vws_frame_send_text(cnx,"SET ident_user=Blodwyn");
printf(" Line %d \n",__LINE__);

//LOOPIN

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
        printf(" Received: \n");
        vws_msg_free(reply);
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
