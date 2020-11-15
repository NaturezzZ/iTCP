/*
 * @Author: Naiqian
 * @Date: 2020-11-09 20:25:36
 * @LastEditTime: 2020-11-15 20:28:35
 * @LastEditors: Naiqian
 * @Description: 
 */
/** 
 * @file ip.h
 * @brief Library supporting sending/receiving IP packets encapsulated in an 
 * Ethernet II frame.
 */

#ifndef __ITCP_IP_H
#define __ITCP_IP_H

#include "core.h"
#include "utils.h"
#include <netinet/ip.h>
#include <bits/stdc++.h>
//#include "include.h"

extern iTCP_kernel_t iTCP_kernel;
using addr_t = uint8_t[4];

struct nextHop{
    uint8_t dstMAC[6];
    device_t *srcDevice;
    nextHop(){
        for(int i = 0; i < 6;i++)dstMAC[i] = 255;
        srcDevice = NULL;
    }
    nextHop(const nextHop &cp){
        memcpy(dstMAC, cp.dstMAC, 6);
        srcDevice = cp.srcDevice;
    }
};
/**
 * @brief Send an IP packet to specified host. 
 *
 * @param src Source IP address.
 * @param dest Destination IP address.
 * @param proto Value of `protocol` field in IP header.
 * @param buf pointer to IP payload
 * @param len Length of IP payload
 * @return 0 on success, -1 on error.
 */
int sendIPPacket(const struct in_addr src, const struct in_addr dest, 
    int proto, const void *buf, int len, int ttl);
int constructIPPacket(uint8_t *fullIPPacket, const struct in_addr src, const struct in_addr dest, int proto, const void *buf, int len, int ttl);
int getDeviceID(const struct in_addr dest, const struct in_addr mask, uint8_t* outMAC);
/** 
 * @brief Process an IP packet upon receiving it.
 *
 * @param buf Pointer to the packet.
 * @param len Length of the packet.
 * @return 0 on success, -1 on error.
 * @see addDevice
 */
typedef int (*IPPacketReceiveCallback)(const void* buf, int len);


/**
 * @brief Register a callback function to be called each time an IP packet
 * was received.
 *
 * @param callback The callback function.
 * @return 0 on success, -1 on error.
 * @see IPPacketReceiveCallback
 */
int setIPPacketReceiveCallback(IPPacketReceiveCallback callback);

/**
 * @brief Manully add an item to routing table. Useful when talking with real 
 * Linux machines.
 * Note that setRoutingTable and lookForRoute is inside class routingTableop 
 * thread security.
 * @param dest The destination IP prefix.
 * @param mask The subnet mask of the destination IP prefix.
 * @param nextHopMAC MAC address of the next hop.
 * @param device Name of device to send packets on.
 * @return 0 on success, -1 on error
 * 
 */
/*
int setRoutingTable(const struct in_addr dest, const struct in_addr mask, 
    const void* nextHopMAC, const char *device);
*/
int recvFrame(const void* buf, int len, int id);

class opRoutingTable{
private:
std::mutex rt_m;
public:

void dumpRoutingTable(){
    //return;
    rt_m.lock();
    rtTable_t::iterator it = iTCP_kernel.routeTable.begin();
    fprintf(stderr, "[INFO] Dump Routing Table:\n");
    fprintf(stderr, "---------------------------------------------------------------\n");
    while(it != iTCP_kernel.routeTable.end()){
        uint8_t ip[4]; ip_t2ptr((*it).first.first, ip);
        uint8_t mask[4]; ip_t2ptr((*it).first.second, mask);
        mac_t mac; mac = (*it).second.dstMAC;
        
        fprintf(stderr, "| %03d", ip[0]);
        for(int i = 1; i <4; i++) fprintf(stderr, ".%d", ip[i]);
        fprintf(stderr, " | %03d", mask[0]);
        for(int i = 1; i <4; i++) fprintf(stderr, ".%d", mask[i]);
        fprintf(stderr, " | %02x", mac[0]);
        for(int i = 1; i <6; i++) fprintf(stderr, ":%x", mac[i]);
        fprintf(stderr, " | %s |\n", iTCP_kernel.currDevice[(*it).second.deviceID].Name.c_str());
        
        it++;
    }
    fprintf(stderr, "---------------------------------------------------------------\n");
    rt_m.unlock();
    return;
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
int setRoutingTable(const struct in_addr dest, const struct in_addr mask, const void* nextHopMAC, const char *device, int ttl=1024){
    rt_m.lock();
    //fprintf(stderr, "*******%x\n", dest.s_addr);
    ipv4_t dstIP = dest.s_addr; ipv4_t dstMask = mask.s_addr;
    int deviceID = iTCP_kernel.getDeviceID(std::string(device));
    if(deviceID < 0){
        fprintf(stderr, "can not find device!\n");
        rt_m.unlock();
        return -1;
    }
    rtKey_t rtKey = std::make_pair(dstIP, dstMask);
    rtTable_t &rtTable = iTCP_kernel.routeTable;
    rtTable_t::iterator it = rtTable.find(rtKey);
    if(it != rtTable.end()){
        (*it).second.deviceID = deviceID;
        (*it).second.ttl = ttl;
        memcpy((*it).second.dstMAC, nextHopMAC, 6);
    }
    else{
        rtValue_t rtValue;
        rtValue.deviceID = deviceID;
        memcpy(rtValue.dstMAC, nextHopMAC, 6);
        rtValue.ttl = ttl;
        iTCP_kernel.routeTable.insert(std::pair<rtKey_t, rtValue_t>(rtKey, rtValue));
    }
    rt_m.unlock();
    return 0;
}
int setRoutingTable(const struct in_addr dest, const struct in_addr mask, const void* nextHopMAC, const char *device){
    rt_m.lock();
    int ttl = 1024;
    ipv4_t dstIP = dest.s_addr; ipv4_t dstMask = mask.s_addr;
    int deviceID = iTCP_kernel.getDeviceID(std::string(device));
    if(deviceID < 0){
        fprintf(stderr, "can not find device!\n");
        rt_m.unlock();
        return -1;
    }
    rtKey_t rtKey = std::make_pair(dstIP, dstMask);
    rtTable_t &rtTable = iTCP_kernel.routeTable;
    rtTable_t::iterator it = rtTable.find(rtKey);
    if(it != rtTable.end()){
        (*it).second.deviceID = deviceID;
        (*it).second.ttl = ttl;
        memcpy((*it).second.dstMAC, nextHopMAC, 6);
    }
    else{
        rtValue_t rtValue;
        rtValue.deviceID = deviceID;
        memcpy(rtValue.dstMAC, nextHopMAC, 6);
        rtValue.ttl = ttl;
        iTCP_kernel.routeTable.insert(std::pair<rtKey_t, rtValue_t>(rtKey, rtValue));
    }
    rt_m.unlock();
    return 0;
}

std::vector<nextHop> lookForRoute(const struct in_addr dest, const struct in_addr mask){
    rt_m.lock();
    std::vector<nextHop> ret;
    if(dest.s_addr == 0xffffffff){
        int len = iTCP_kernel.currDevice.size();
        for(int i = 0; i < len; i++){
            nextHop nhop;
            static mac_t bcm = new uint8_t[6];
            for(int j = 0; j < 6; j++) bcm[j] = 0xff;
            memcpy(nhop.dstMAC, bcm, 6);
            deviceID_t deviceID = iTCP_kernel.currDevice[i].ID;
            nhop.srcDevice = &iTCP_kernel.currDevice[deviceID];
            ret.push_back(nhop);
        }
        rt_m.unlock();
        return ret;
    }
    
    rtTable_t &rtTable = iTCP_kernel.routeTable;
    rtTable_t::iterator it = rtTable.begin();
    for(; it != rtTable.end(); it++){
        rtKey_t key = (*it).first;
        rtValue_t value = (*it).second;
        if(dest.s_addr == key.first && mask.s_addr == key.second){
            nextHop nhop;
            memcpy(nhop.dstMAC, value.dstMAC, 6);
            deviceID_t deviceID = value.deviceID;
            nhop.srcDevice = &iTCP_kernel.currDevice[deviceID];
            ret.push_back(nhop);
        }
    }
    /*
     * Default routing: broadcast
     */
    if(ret.size() != 0) {rt_m.unlock(); return ret;}
    else{
        int len = iTCP_kernel.currDevice.size();
        for(int i = 0; i < len; i++){
            nextHop nhop;
            static mac_t bcm = new uint8_t[6];
            for(int j = 0; j < 6; j++) bcm[j] = 0xff;
            memcpy(nhop.dstMAC, bcm, 6);
            deviceID_t deviceID = iTCP_kernel.currDevice[i].ID;
            nhop.srcDevice = &iTCP_kernel.currDevice[deviceID];
            ret.push_back(nhop);
        }
        rt_m.unlock();
        return ret;
    }
}

std::pair<std::vector<nextHop>, std::vector<int> > myArpLookForRoute(const struct in_addr dest, const struct in_addr mask){
    rt_m.lock();
    std::vector<nextHop> ret;
    std::vector<int> ttl;
    rtTable_t &rtTable = iTCP_kernel.routeTable;
    rtTable_t::iterator it = rtTable.begin();
    for(; it != rtTable.end(); it++){
        rtKey_t key = (*it).first;
        rtValue_t value = (*it).second;
        if(dest.s_addr == key.first && mask.s_addr == key.second){
            nextHop nhop;
            memcpy(nhop.dstMAC, value.dstMAC, 6);
            deviceID_t deviceID = value.deviceID;
            nhop.srcDevice = &iTCP_kernel.currDevice[deviceID];
            ret.push_back(nhop);
            ttl.push_back(value.ttl);
        }
    }
    rt_m.unlock();
    return std::make_pair(ret, ttl);
}
};

//std::vector<nextHop> lookForRoute(const struct in_addr, const struct in_addr mask);
#endif