/*
 * @Author: Naiqian
 * @Date: 2020-11-09 20:25:42
 * @LastEditTime: 2020-11-09 21:31:57
 * @LastEditors: Naiqian
 * @Description: 
 */
#include "ip.h"

int sendIPPacket(const struct in_addr src, const struct in_addr dest, int proto, const void *buf, int len){
    
}

int setIPPacketReceiveCallback(IPPacketReceiveCallback callback){
    
}

/**
 * @brief Manully add an item to routing table. Useful when talking with real 
 * Linux machines.
 * 
 * @param dest The destination IP prefix.
 * @param mask The subnet mask of the destination IP prefix.
 * @param nextHopMAC MAC address of the next hop.
 * @param device Name of device to send packets on.
 * @return 0 on success, -1 on error
 */
int setRoutingTable(const struct in_addr dest, const struct in_addr mask, const void* nextHopMAC, const char *device){
    
}

int lookForRoute(const struct in_addr, const struct in_addr mask, void* nextHopMAC, const char *device){
    
}