/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

#include "Stud.h"
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

/*  Online resources */
// Colored font: http://web.theurbanpenguin.com/adding-color-to-your-output-from-c/

/*      Function declarations       */

struct pkt create_packet(int *seq, int *ack, struct msg *message);
bool is_corrupted(struct pkt packet);
int get_checksum(struct pkt packet);
void printResult(int type, char *message, char *data);
float calcRTT(float estRTT, clock_t sampRTT);
float calc_devRTT(float sampRTT);
float calc_TimeoutInterval(clock_t sampRTT);

/*      Global variables    */
int SEQ;
bool A_IS_BUSY;
int ACK;
struct pkt a_packet;
struct pkt b_packet;

clock_t sampleRTT;
float estimatedRTT;
float devRTT;

/*      Constants       */
#define A_SENDER 0
#define B_RECEIVER 1
#define BUFF_SIZE 100
#define RTT_BASE 0.125 //Lower value to quickly react to changes on network - Higher, slower react/change
#define RTT_START_VAL 10 //high start value in order to work with slow connections from start


/*  Calculates est RTT based on the sample RTT, RTT BASE and current est RTT    */
float calcRTT(float estRTT, clock_t sampRTT)
{
    float sample = (double)sampRTT / CLOCKS_PER_SEC;
    return (((1 - RTT_BASE)*estRTT) + (RTT_BASE * sample));
}

float calc_devRTT(float sampRTT)
{
    float variation = 0.25;
    float result = (((1-variation) * devRTT) + (variation * abs(sampleRTT - estimatedRTT)));
    return result;
}

float calc_TimeoutInterval(clock_t sampRTT)
{
    float sample = ((float)sampRTT / CLOCKS_PER_SEC);
    return estimatedRTT + 4*(calc_devRTT(sample));
}

/* called from layer 5, passed the data to be sent to other side */
void A_output( struct msg message)
{
    if(!A_IS_BUSY)
    {
        A_IS_BUSY = true;
        printResult(true, "A_output: Sending packet: ", message.data);
        a_packet = create_packet(&SEQ, NULL, &message);
        

        tolayer3(A_SENDER, a_packet);
        starttimer(A_SENDER, estimatedRTT);
        sampleRTT = clock();
    }else
    {
        printResult(false, "A_output dropped package: ", message.data);
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
    int checksum = (packet.acknum + packet.seqnum);
    int i = 0;
    
    for(i = 0; i<sizeof(packet.payload); i++)
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
    static char output[BUFF_SIZE]; 
    memset(output, 0, BUFF_SIZE);

    strcpy(output, message);
    if(type){
        printf("\033[0;32m"); //Sets font color to green
    }else printf("\033[0;31m"); //Sets font color to red
    
    if(data != NULL) strcat(output, data);

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
        
        A_IS_BUSY = false;

        sampleRTT = clock() - sampleRTT;
        estimatedRTT = calcRTT(estimatedRTT, sampleRTT);
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
    
    sampleRTT = clock() - sampleRTT; //retakes sample RTT one last time before
    starttimer(A_SENDER, calc_TimeoutInterval(sampleRTT));
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
    /*  Initiates a 0-message in order to fill the payload of the paket with certain values. */
    ACK = 1; // 
    struct msg nullMessage;
    for(int i = 0; i<sizeof(struct msg); i++)
    {
        nullMessage.data[i] = 0;
    }
    b_packet = create_packet(NULL, &ACK, &nullMessage);

    /*  Restores ACK to correct init_value */
    ACK = 0;
}


/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    printf("A_init running\n");
    A_IS_BUSY = false;
    SEQ = 0;

    estimatedRTT = RTT_START_VAL; // Inits est RTT for first round without inputs
    devRTT = RTT_START_VAL;
}



/*      Not Needed      */
/* Note that with simplex transfer from a-to-B, there is no B_output() */
void B_output(struct msg message)  /* need be completed only for extra credit */
{}


/* called when B's timer goes off */
void B_timerinterrupt()
{}