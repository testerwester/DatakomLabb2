/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

#include "Stud.h"
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/*      Function declarations       */

struct pkt create_packet(int *seq, int *ack, struct msg *message);
bool is_corrupted(struct pkt packet);
int get_checksum(struct pkt packet);
void printResult(int type, char *message, char *data);

/*      Global variables    */
int SEQ;
bool A_IS_BUSY;
int ACK;
struct pkt a_packet;
struct pkt b_packet;

/*      Constants       */
#define A_SENDER 0
#define B_RECEIVER 1


/* called from layer 5, passed the data to be sent to other side */
void A_output( struct msg message)
{
    if(!A_IS_BUSY)
    {
        A_IS_BUSY = true;
        a_packet = create_packet(&SEQ, NULL, &message);
        printResult(true, "A_output: Sending packet: ", message.data);

        tolayer3(A_SENDER, a_packet);
        starttimer(A_SENDER, 20);
    }
}

/* Dynamically creates packet for a and b side. Where any input var can be NULL in order to set a generic message there */
struct pkt create_packet(int *seq, int *ack, struct msg *message)
{
    struct pkt packet;
    if(seq != NULL) packet.seqnum = *seq; else packet.seqnum = 10;
    if(ack != NULL) packet.acknum = *ack; else packet.acknum = 10;
    
    if(message != NULL) strcpy(packet.payload, message->data);
    packet.checksum = get_checksum(packet);
    return packet;
}

/*  Calculates checksum and returns value as an int */
int get_checksum(struct pkt packet)
{
    int checksum = 0;
    int i = 0;
    checksum = (packet.acknum + packet.seqnum);
    
    for(i = 0; i<20; i++)
    {
        checksum += packet.payload[i];
    }
    return checksum;
}

/*  Compares a pkt's checksum and its value checksum, returns true if they correspond    */
bool is_corrupted(struct pkt packet)
{
    if(packet.checksum == get_checksum(packet)) return 0;
    else return 1;
}

/*  For terminal output - 
If type == true, green text. 
If type == false, red text
Data can be NUll in order to only use var message   */
void printResult(int type, char *message, char *data)
{
    static char output[100]; 
    memset(output, 0, 100);

    strcpy(output, message);
    if(type){
        printf("\033[0;32m"); //Sets font color to green
    }else printf("\033[0;31m"); //Sets font color to red
    
    if(data != NULL)
    {
        strcat(output, data);
    }
    printf("%s\n", output);
    

    printf("\033[0m"); //resets font color
}



/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    if(packet.acknum == SEQ && !is_corrupted(packet))
    {
        //ACK accepted from b_side
        stoptimer(A_SENDER);
        printResult(true, "A_input: Ack accepted", NULL);
        SEQ = (++SEQ % 2); //Increments SEQ :)
        printf("SEQ is now: %i\n", SEQ);
        
        A_IS_BUSY = false;
    }
    else if(is_corrupted(packet))
    {
        printResult(false, "A_input: recieved corrupt packet", NULL);
    }
    else
    {
        printResult(false, "A_input: recieved wrong ACK", NULL); 
    }
}


/* called when A's timer goes off */
void A_timerinterrupt()
{
    printResult(true, "A_interrupt: Resending packet: ", a_packet.payload);
    tolayer3(A_SENDER, a_packet);
    starttimer(A_SENDER, 20);
}  


/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    if(packet.seqnum == ACK && !is_corrupted(packet))
    {
        printResult(true, "B_input: recieved packet: ", packet.payload);
        b_packet = create_packet(NULL, &ACK, NULL);
        tolayer5(B_RECEIVER, packet.payload);
        tolayer3(B_RECEIVER, b_packet);
        ACK = (++ACK % 2);
        printf("ACK IS now: %i\n", ACK);

    }else if(is_corrupted(packet))
    {
        printResult(false, "B_input: recieved corrupt packet", NULL);
        tolayer3(B_RECEIVER, b_packet);
    } else
    {
        printResult(false, "B_input: recieved wrong packet", NULL);
        tolayer3(B_RECEIVER, b_packet);
    }
}



/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    printf("B_init running\n");
    /* Sets ack to 1 so that first ACK will work for either correct or not correct seq value*/
    ACK = 1; // 
    b_packet = create_packet(NULL, &ACK, NULL);

    /*  Restores ACK to correct init_value */
    ACK = 0;

    for(int i = 0; i<sizeof(struct msg); i++)
    {
        b_packet.payload[i] = 0;
    }
}


/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    printf("A_init running\n");
    A_IS_BUSY = false;
    SEQ = 0;
}



/*      Not Needed      */
/* Note that with simplex transfer from a-to-B, there is no B_output() */
void B_output(struct msg message)  /* need be completed only for extra credit */
{}


/* called when B's timer goes off */
void B_timerinterrupt()
{}