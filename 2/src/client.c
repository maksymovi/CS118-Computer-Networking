#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netdb.h>

// =====================================

#define RTO 50000        /* timeout in microseconds */
#define HDR_SIZE 12      /* header size*/
#define PKT_SIZE 524     /* total packet size */
#define PAYLOAD_SIZE 512 /* PKT_SIZE - HDR_SIZE */
#define WND_SIZE 10      /* window size*/
#define MAX_SEQN 25601   /* number of sequence numbers [0-25600] */
#define FIN_WAIT 2       /* seconds to wait after receiving FIN*/

// Packet Structure: Described in Section 2.1.1 of the spec. DO NOT CHANGE!
struct packet
{
    unsigned short seqnum;
    unsigned short acknum;
    char syn;
    char fin;
    char ack;
    char dupack;
    unsigned int length;
    char payload[PAYLOAD_SIZE];
};

// Printing Functions: Call them on receiving/sending/packet timeout according
// Section 2.6 of the spec. The content is already conformant with the spec,
// no need to change. Only call them at correct times.
void printRecv(struct packet *pkt)
{
    printf("RECV %d %d%s%s%s\n", pkt->seqnum, pkt->acknum, pkt->syn ? " SYN" : "", pkt->fin ? " FIN" : "", (pkt->ack || pkt->dupack) ? " ACK" : "");
}

void printSend(struct packet *pkt, int resend)
{
    if (resend)
        printf("RESEND %d %d%s%s%s\n", pkt->seqnum, pkt->acknum, pkt->syn ? " SYN" : "", pkt->fin ? " FIN" : "", pkt->ack ? " ACK" : "");
    else
        printf("SEND %d %d%s%s%s%s\n", pkt->seqnum, pkt->acknum, pkt->syn ? " SYN" : "", pkt->fin ? " FIN" : "", pkt->ack ? " ACK" : "", pkt->dupack ? " DUP-ACK" : "");
}

void printTimeout(struct packet *pkt)
{
    printf("TIMEOUT %d\n", pkt->seqnum);
}

// Building a packet by filling the header and contents.
// This function is provided to you and you can use it directly
void buildPkt(struct packet *pkt, unsigned short seqnum, unsigned short acknum, char syn, char fin, char ack, char dupack, unsigned int length, const char *payload)
{
    pkt->seqnum = seqnum;
    pkt->acknum = acknum;
    pkt->syn = syn;
    pkt->fin = fin;
    pkt->ack = ack;
    pkt->dupack = dupack;
    pkt->length = length;
    memcpy(pkt->payload, payload, length);
}

// =====================================

double setTimer()
{
    struct timeval e;
    gettimeofday(&e, NULL);
    return (double)e.tv_sec + (double)e.tv_usec / 1000000 + (double)RTO / 1000000;
}

double setFinTimer()
{
    struct timeval e;
    gettimeofday(&e, NULL);
    return (double)e.tv_sec + (double)e.tv_usec / 1000000 + (double)FIN_WAIT;
}

int isTimeout(double end)
{
    struct timeval s;
    gettimeofday(&s, NULL);
    double start = (double)s.tv_sec + (double)s.tv_usec / 1000000;
    return ((end - start) < 0.0);
}

// =====================================

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        perror("ERROR: incorrect number of arguments\n");
        exit(1);
    }

    struct in_addr servIP;
    if (inet_aton(argv[1], &servIP) == 0)
    {
        struct hostent *host_entry;
        host_entry = gethostbyname(argv[1]);
        if (host_entry == NULL)
        {
            perror("ERROR: IP address not in standard dot notation\n");
            exit(1);
        }
        servIP = *((struct in_addr *)host_entry->h_addr_list[0]);
    }

    unsigned int servPort = atoi(argv[2]);

    FILE *fp = fopen(argv[3], "r");
    if (fp == NULL)
    {
        perror("ERROR: File not found\n");
        exit(1);
    }

    // =====================================
    // Socket Setup

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr = servIP;
    servaddr.sin_port = htons(servPort);
    memset(servaddr.sin_zero, '\0', sizeof(servaddr.sin_zero));

    int servaddrlen = sizeof(servaddr);

    // NOTE: We set the socket as non-blocking so that we can poll it until
    //       timeout instead of getting stuck. This way is not particularly
    //       efficient in real programs but considered acceptable in this
    //       project.
    //       Optionally, you could also consider adding a timeout to the socket
    //       using setsockopt with SO_RCVTIMEO instead.
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // =====================================
    // Establish Connection: This procedure is provided to you directly and is
    // already working.
    // Note: The third step (ACK) in three way handshake is sent along with the
    // first piece of along file data thus is further below

    struct packet synpkt, synackpkt;

    unsigned short seqNum = rand() % MAX_SEQN;
    buildPkt(&synpkt, seqNum, 0, 1, 0, 0, 0, 0, NULL);

    printSend(&synpkt, 0);
    sendto(sockfd, &synpkt, PKT_SIZE, 0, (struct sockaddr *)&servaddr, servaddrlen);
    double timer = setTimer();
    int n;

    while (1)
    {
        while (1)
        {
            n = recvfrom(sockfd, &synackpkt, PKT_SIZE, 0, (struct sockaddr *)&servaddr, (socklen_t *)&servaddrlen);

            if (n > 0)
                break;
            else if (isTimeout(timer))
            {
                printTimeout(&synpkt);
                printSend(&synpkt, 1);
                sendto(sockfd, &synpkt, PKT_SIZE, 0, (struct sockaddr *)&servaddr, servaddrlen);
                timer = setTimer();
            }
        }

        printRecv(&synackpkt);
        if ((synackpkt.ack || synackpkt.dupack) && synackpkt.syn && synackpkt.acknum == (seqNum + 1) % MAX_SEQN)
        {
            seqNum = synackpkt.acknum;
            break;
        }
    }

    // =====================================
    // FILE READING VARIABLES

    char buf[PAYLOAD_SIZE];
    size_t m;

    // =====================================
    // CIRCULAR BUFFER VARIABLES

    struct packet ackpkt;
    struct packet pkts[WND_SIZE];
    int s; //start window
    int e; //next empty slot, if s == e, then we have used all our slots
    //int full = 0;

    // =====================================
    // Send First Packet (ACK containing payload)
    //special packet due to ack, has to be sent individually, also initiates our window algorithm

    m = fread(buf, 1, PAYLOAD_SIZE, fp);

    buildPkt(&pkts[0], seqNum, (synackpkt.seqnum + 1) % MAX_SEQN, 0, 0, 1, 0, m, buf);
    printSend(&pkts[0], 0);
    sendto(sockfd, &pkts[0], PKT_SIZE, 0, (struct sockaddr *)&servaddr, servaddrlen);
    timer = setTimer();
    buildPkt(&pkts[0], seqNum, (synackpkt.seqnum + 1) % MAX_SEQN, 0, 0, 0, 1, m, buf);

    e = 1;                            //this initiates the windowing algorithm
    seqNum = (seqNum + m) % MAX_SEQN; //increase seqnum
    short clientAckNum = (synackpkt.seqnum + 1) % MAX_SEQN;

    //wait for first packet ACK, and send all other packets
    while (1)
    {
        while (s != e && m != 0) //start window is not equal to
        {
            //we build and send one packet per every slot that opens up
            if ((m = fread(buf, 1, PAYLOAD_SIZE, fp)) != 0) //we have data to send
            {
                buildPkt(&pkts[e], seqNum, clientAckNum, 0, 0, 0, 0, m, buf);
                printSend(&pkts[e], 0);
                sendto(sockfd, &pkts[e], PKT_SIZE, 0, (struct sockaddr *)&servaddr, servaddrlen);

                seqNum = (seqNum + m) % MAX_SEQN; //increment seqNum for next loop
                e = (e + 1) % WND_SIZE;           //we decrease window size by 1
                timer = setTimer();               //maybe put this in a better place, set the timer once all packets are transmitted
            }
            //if the if fails, then we have transmitted the entire file and exit the loop
            //maybe there is a better way to handle this but here we are
        }
        //while we have no more data to send, wait for acks.
        n = recvfrom(sockfd, &ackpkt, PKT_SIZE, 0, (struct sockaddr *)&servaddr, (socklen_t *)&servaddrlen);
        if (n > 0) //polling this way is a bit inefficient but doing this for now
        {          //we have an ack, confirm which packet it is part of.
            printRecv(&ackpkt);

            //here we find the right ack for the packet
            int j = s; //s contains the oldest unacked packet, e contains the most recently sent packet
            do         //will probably be the ack for the first packet, do while because condition is weird
            {
                //in theory I can just look at the seqNum of the next packet to get the right acknum, but its clearer this way.
                if ((pkts[j].seqnum + pkts[j].length) % MAX_SEQN == ackpkt.acknum)
                {
                    //j is the maximally ack'd packet, advance s and break
                    s = (j + 1) % WND_SIZE;
                    //fprintf(stderr, "s is now %d, e is now %d\n", s, e);
                    if (m == 0 && s == e)
                    {
                        //all packets sent and acked, exit here
                        //nested loop break
                        goto FINALIZE;
                    }
                    break;
                }
                j = (j + 1) % WND_SIZE;
            } while (j != e); //if this loop fails, then the recieved ack was invalid
        }
        //check timeout here
        if (isTimeout(timer))
        {
            printTimeout(&pkts[s]);
            //retransmit all unacked packets
            int j = s;
            do
            {
                printSend(&pkts[j], 1);
                sendto(sockfd, &pkts[j], PKT_SIZE, 0, (struct sockaddr *)&servaddr, servaddrlen);
                timer = setTimer(); //maybe move to after for loop
                j = (j + 1) % WND_SIZE;
            } while (j != e);
        }
    }
FINALIZE:
    // *** End of your client implementation ***
    fclose(fp);

    // =====================================
    // Connection Teardown: This procedure is provided to you directly and is
    // already working.

    struct packet finpkt, recvpkt;
    buildPkt(&finpkt, ackpkt.acknum, 0, 0, 1, 0, 0, 0, NULL);
    buildPkt(&ackpkt, (ackpkt.acknum + 1) % MAX_SEQN, (ackpkt.seqnum + 1) % MAX_SEQN, 0, 0, 1, 0, 0, NULL);

    printSend(&finpkt, 0);
    sendto(sockfd, &finpkt, PKT_SIZE, 0, (struct sockaddr *)&servaddr, servaddrlen);
    timer = setTimer();
    int timerOn = 1;

    double finTimer;
    int finTimerOn = 0;

    while (1)
    {
        while (1)
        {
            n = recvfrom(sockfd, &recvpkt, PKT_SIZE, 0, (struct sockaddr *)&servaddr, (socklen_t *)&servaddrlen);

            if (n > 0)
                break;
            if (timerOn && isTimeout(timer))
            {
                printTimeout(&finpkt);
                printSend(&finpkt, 1);
                if (finTimerOn)
                    timerOn = 0;
                else
                    sendto(sockfd, &finpkt, PKT_SIZE, 0, (struct sockaddr *)&servaddr, servaddrlen);
                timer = setTimer();
            }
            if (finTimerOn && isTimeout(finTimer))
            {
                close(sockfd);
                if (!timerOn)
                    exit(0);
            }
        }
        printRecv(&recvpkt);
        if ((recvpkt.ack || recvpkt.dupack) && recvpkt.acknum == (finpkt.seqnum + 1) % MAX_SEQN)
        {
            timerOn = 0;
        }
        else if (recvpkt.fin && (recvpkt.seqnum + 1) % MAX_SEQN == ackpkt.acknum)
        {
            printSend(&ackpkt, 0);
            sendto(sockfd, &ackpkt, PKT_SIZE, 0, (struct sockaddr *)&servaddr, servaddrlen);
            finTimer = setFinTimer();
            finTimerOn = 1;
            buildPkt(&ackpkt, ackpkt.seqnum, ackpkt.acknum, 0, 0, 0, 1, 0, NULL);
        }
    }
}
