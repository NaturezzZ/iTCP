/*
 * @Author: Naiqian
 * @Date: 2020-11-13 00:42:44
 * @LastEditTime: 2020-11-15 20:22:18
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/src/myArp.cc
 */
#include "myArp.h"

extern opRoutingTable routingTableop;
int sendArp(iTCP_kernel_t &kernel){
    int len = kernel.currDevice.size();
    for(int i = 0; i < len; i++){
        //fprintf(stderr, "????\n");    
        const char * Name = kernel.currDevice[i].Name.c_str();
        //fprintf(stderr, "%s\n", Name);
        int payloadLen = strlen(Name) + 1;
        in_addr src; src.s_addr = kernel.currDevice[i].IP;
        //fprintf(stderr, "%x\n", src.s_addr);
        in_addr broadcastIP; broadcastIP.s_addr = 0xffffffff;
        //in_addr mask; mask.s_addr = 0xffffffff;
        uint8_t*buf = new uint8_t[1];
        int fullSize = 1;
        int ttl = 8;
        sendIPPacket(src, broadcastIP, L3_MYARP, buf, fullSize, ttl);
    }
    return 0;
}
int sendArpBroadCast(iTCP_kernel_t &kernel, const ipv4_t srcIP, int ttl){
    //int len = kernel.currDevice.size();
    //for(int i = 0; i < len; i++){
    //    const char * Name = kernel.currDevice[i].Name.c_str();
    //    int payloadLen = strlen(Name) + 1;
        in_addr src; src.s_addr = srcIP;
        in_addr broadcastIP; broadcastIP.s_addr = 0xffffffff;
        uint8_t*buf = new uint8_t[1];
        int fullSize = 1;
        sendIPPacket(src, broadcastIP, L3_MYARP, buf, fullSize, ttl);
    //}
    return 0;
}

void recvArp(iTCP_kernel_t &kernel, const ipv4_t srcIP, const ipv4_t mask, 
    const deviceID_t ID, const mac_t srcMac, const int ttl){
    
    for(int i = 0; i < kernel.currDevice.size(); i++){
        if(kernel.currDevice[i].IP == srcIP) return;
    }
    //std::cout << "srcIP" << std::hex<< srcIP << std::endl;
    in_addr addr; addr.s_addr = srcIP;
    in_addr mask1; mask1.s_addr = mask;
    std::pair<std::vector<nextHop>, std::vector<int> > ans = routingTableop.myArpLookForRoute(addr, mask1);
    std::vector<nextHop>& nextHopList = ans.first;
    std::vector<int>& ttlList = ans.second;
    if(ttlList.size() == 0){
        //fprintf(stderr, "12121212\n");
        routingTableop.setRoutingTable(addr, mask1, srcMac, kernel.currDevice[ID].Name.c_str(), ttl);
        //fprintf(stderr, "12121212\n");
        sendArpBroadCast(kernel, srcIP, ttl-1);
        return;
    }
    else{
        // here ttlList.size() should be 1
        for(int i = 0; i < ttlList.size(); i++){
            //fprintf(stderr, "12121212\n");
            //if with larger ttl, it receives a myArp packet via a shorter path
            if(ttlList[i] < ttl){
                routingTableop.setRoutingTable(addr, mask1, srcMac, kernel.currDevice[ID].Name.c_str(), ttl);
                //sendArpBroadCast(kernel, srcIP, ttl-1);
            }
        }
        sendArpBroadCast(kernel, srcIP, ttl-1);
    }
    return;
}