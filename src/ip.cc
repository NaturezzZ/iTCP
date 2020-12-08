/*
 * @Author: Naiqian
 * @Date: 2020-11-09 20:25:42
 * @LastEditTime: 2020-12-08 18:52:42
 * @LastEditors: Naiqian
 * @Description: 
 */
#include "ip.h"
#include "utils.h"
#include "packetio.h"
#include "core.h"
#include "include.h"
#include "forward.h"
using namespace std;
extern iTCP_kernel_t iTCP_kernel;

ipv4_t fullMask = 0xffffffff;

opRoutingTable routingTableop;

/*
 * Return packets sent altogether
 */
int sendIPPacket(const struct in_addr src, const struct in_addr dest, int proto, const void *buf, int len, int ttl=32){
    
    uint8_t *fullIPPacket;
    uint32_t totalLength = sizeof(ipHdr_t) + (uint32_t)len;
    fullIPPacket = new uint8_t[totalLength];
    constructIPPacket(fullIPPacket, src, dest, proto, buf, len, ttl);
    //fprintf(stderr, "abcd\n");
    
    struct in_addr mask;
    mask.s_addr = fullMask;
    
    std::vector<nextHop> nHop = routingTableop.lookForRoute(dest, mask);
    int Len = nHop.size();
    if(Len == 0) return -1;
    int ret = 0;
    for(int i = 0; i < Len; i++){
        //std::cout << i << std::endl;
        //fprintf(stderr, "%x\n", *nHop[i].dstMAC);
        if (0 == sendFrame(fullIPPacket, len+sizeof(ipHdr_t), L2_IPV4, nHop[i].dstMAC, nHop[i].srcDevice->ID)) ret += 1;
        else{
            fprintf(stderr, "Can not send frame!\n");
        }
    }
    return 0;
}

/*
 * Return ID of device, -1 for not found.  
 */
int getDeviceID(string &Name){
    int deviceNum = iTCP_kernel.currDevice.size();
    int ret = -1;
    for(int i = 0; i < deviceNum; i++){
        if(iTCP_kernel.currDevice[i].Name == Name) ret = iTCP_kernel.currDevice[i].ID;
    }
    return ret;
}


int constructIPPacket(uint8_t *fullIPPacket, const struct in_addr src, const struct in_addr dest, 
    int proto, const void *buf, int len, int ttl){
    //if(src.s_addr == 0) {std::cerr << "!!!!!" << endl; pause();}
    //construct header
    uint32_t totalLength = sizeof(ipHdr_t) + (uint32_t)len;
    ipHdr_t ipHdr;
    uint8_t row1[4];
    row1[0] = 0x45;
    row1[1] = 0;
    row1[2] = (uint8_t)((totalLength & 0xff00)>>8);
    row1[3] = (uint8_t)(totalLength & 0xff);
    ipHdr.part1 = ptr2ip_t(row1);
    
    ipHdr.part2 = 0;
    ipHdr.ttl = ttl;
    ipHdr.protocol = proto;
    ipHdr.hdrCRC = 0;
    ipHdr.src = src.s_addr;
    ipHdr.dst = dest.s_addr;
    uint8_t *hdrPtr = (uint8_t*)(void*)&ipHdr;
    
    //set up full packet buffer
    memset(fullIPPacket, 0, totalLength);
    
    //copy bytes into the full packet
    memcpy(fullIPPacket, hdrPtr, sizeof(ipHdr_t));
    memcpy(fullIPPacket+sizeof(ipHdr_t), buf, len);

    return 0;
}

int setIPPacketReceiveCallback(IPPacketReceiveCallback callback){
    iTCP_kernel.getIPCallback() = callback;
    return 0;
}


// callback function of layer 2
int recvFrame(const void* buf, int len, int id){
    
    uint8_t * ethHdr = new uint8_t[sizeof(eth_hdr_t)];
    uint8_t * ethPayload = new uint8_t[len - sizeof(eth_hdr_t) - 4];
    uint8_t * ethCheckSum = new uint8_t[4];
    memcpy(ethHdr, buf, sizeof(eth_hdr_t));
    memcpy(ethPayload, buf + sizeof(eth_hdr_t), len - sizeof(eth_hdr_t) - 4);
    memcpy(ethCheckSum, buf + len - 4, 4);
    mac_t ethSrc = ((eth_hdr_t*)ethHdr)->src;
    if(memcmp(ethSrc, iTCP_kernel.currDevice[id].Mac, 6) == 0){
        return -1;
    }
    //mac_t ethSrc = ((eth_hdr_t*)ethHdr)->src;
    mac_t ethDst = ((eth_hdr_t*)ethHdr)->dst;
    ipv4_t ipSrc = ((ipHdr_t*)ethPayload)->src;
    ipv4_t ipDst = ((ipHdr_t*)ethPayload)->dst;
    uint8_t ipTtl = ((ipHdr_t*)ethPayload)->ttl;
    uint8_t * frag = (uint8_t*)&(((ipHdr_t*)ethPayload)->part2);
    frag += 16;
    //if( *((uint16_t*) frag) != 0) return 0;
    uint8_t ipProtocol = ((ipHdr_t*)ethPayload)->protocol;
    uint8_t *ipPayload = (ethPayload+sizeof(ipHdr_t));
    int ipPayloadLen = len - sizeof(eth_hdr_t) - 4 - sizeof(ipHdr_t);
    bool reachDst = 0;
    //cerr << "????" << endl;
    for(int i = 0; i < iTCP_kernel.currDevice.size(); i++){
        ipv4_t deviceIP = iTCP_kernel.currDevice[id].IP;
        if(deviceIP == ipDst) reachDst = 1;
    }
    //cerr << reachDst << "????" << endl;
    if(reachDst == 1){
        //successfully received a packet, ip callback
        fprintf(stderr, "[INFO] %s recieved a destinated packet\n", iTCP_kernel.currDevice[id].Name.c_str());
        //iTCP_kernel.getIPCallback()(NULL, 0);
        iTCP_kernel.getIPCallback()(ethPayload, (int)(len - sizeof(eth_hdr_t) - 4));
        //fprintf(stderr, "adfasd\n");
    }
    else if(ipProtocol == L3_MYARP){
        //myArp routing
        //fprintf(stderr, "[INFO] %s recieved a myArp packet\n", iTCP_kernel.currDevice[id].Name.c_str());
        recvArp(iTCP_kernel, ipSrc, 0xffffffff, id, ethSrc, ipTtl);
    }
    else{
        //forward
        //fprintf(stderr, "[INFO] %s recieved a forwarding packet\n", iTCP_kernel.currDevice[id].Name.c_str());
        forward(ipSrc, ipDst, ipProtocol, ipPayload, ipPayloadLen, ipTtl);
    }
    return 0;
}