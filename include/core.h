/*
 * @Author: Naiqian
 * @Date: 2020-11-02 22:12:15
 * @LastEditTime: 2020-11-14 02:02:29
 * @LastEditors: Naiqian
 * @Description: 
 */
#ifndef __ITCP_CORE_H
#define __ITCP_CORE_H
#include<bits/stdc++.h>
//#include<map>
//#include "include.h"
//#include "ip.h"
#include "utils.h"
#include<pcap.h>
#include <ifaddrs.h>
#include "packetio.h"
#include <netpacket/packet.h>


ipv4_t ptr2ip_t(const uint8_t*);
void ip_t2ptr(ipv4_t, uint8_t*);

class device_t{
public:
    mac_t Mac;
    ipv4_t IP;
    deviceName_t Name;
    deviceID_t ID;
    pcap_t *pcapHandler;
    char *pcap_errbuf;
    device_t(deviceID_t deviceID, void* deviceMac, deviceName_t deviceName){
        ID = deviceID;
        Mac = new uint8_t[6];
        memcpy(Mac, deviceMac, 6);
        IP = 0;
        pcap_errbuf = new char[PCAP_ERRBUF_SIZE];
        memset(pcap_errbuf, 0, PCAP_ERRBUF_SIZE);
        Name = deviceName;
        pcapHandler = pcap_open_live(Name.c_str(), BUFSIZ, 1, 1000, pcap_errbuf);
    }
    device_t(const device_t & cp){

        Mac = new uint8_t[6];
        memcpy(Mac, cp.Mac, 6);
        IP = cp.IP;
        pcap_errbuf = new char[PCAP_ERRBUF_SIZE];
        memset(pcap_errbuf, 0, PCAP_ERRBUF_SIZE);
        memcpy(pcap_errbuf, cp.pcap_errbuf, PCAP_ERRBUF_SIZE);

        Name = cp.Name;
        ID = cp.ID;
        pcapHandler = pcap_open_live(Name.c_str(), BUFSIZ, 1, 1000, pcap_errbuf);
    }
    int sendFrame(const uint8_t *buf, int len){
        return pcap_sendpacket(pcapHandler, buf, len);
    }
    ~device_t(){
        if(Mac) delete[] Mac;
        if(pcap_errbuf) delete[] pcap_errbuf;
        if(pcapHandler) pcap_close(pcapHandler);
    }
};
using frameReceiveCallback = int (*)(const void *, int, int);
using IPPacketReceiveCallback = int (*) (const void*, int);
class iTCP_kernel_t{
//private:
public:
    rtTable_t routeTable;
    std::vector<device_t> currDevice;
    frameReceiveCallback eth_callback;
    IPPacketReceiveCallback ip_callback;
//public:
    frameReceiveCallback &getEthCallback(){
        return eth_callback;
    }
    IPPacketReceiveCallback &getIPCallback(){
        return ip_callback;
    }
    iTCP_kernel_t(){
    }
    iTCP_kernel_t & get(){
        return *this;
    }
    int addDeviceKernel(deviceName_t deviceName, const mac_t currMac){
        int len = currDevice.size();
        int index;
        bool flag_device = 0;
        int selected_index = -1;
        for(index = 0; index < len; index++){
            if(deviceName == currDevice[index].Name) flag_device = 1, selected_index = index;
        }
        if(!flag_device){
            selected_index = len;
            currDevice.push_back(device_t(selected_index, currMac, deviceName));
        }
        return selected_index;
    }
    int addIP(deviceName_t deviceName, const uint8_t* currIP){
        int len = currDevice.size();
        for(int index = 0; index < len; index++){
            if(deviceName == currDevice[index].Name){
                ipv4_t IP = ptr2ip_t(currIP);
                //fprintf(stderr, "aaaa%x\n", IP);
                currDevice[index].IP = IP;
            }
        }
    }
    int findDeviceKernel(deviceName_t deviceName){
        int len = currDevice.size();
        int result = -1;
        int index = 0;
        for(index = 0; index < len; index++){
            if(deviceName == currDevice[index].Name) result = currDevice[index].ID;
        }
        return result;
    }
    mac_t getMac(deviceName_t deviceName){
        int len = currDevice.size();
        static mac_t result = NULL;
        int index = 0;
        for(index = 0; index < len; index++){
            if(deviceName == currDevice[index].Name) result = currDevice[index].Mac;
        }
        return result;
    }
    ipv4_t getIP(deviceName_t deviceName){
        int len = currDevice.size();
        for(int i = 0; i < len; i++){
            if(deviceName == currDevice[i].Name){
                return currDevice[i].IP;
            }
        }
        return 0;
    }
    int getDeviceID(deviceName_t str){
        int len = currDevice.size();
        int ret = -1;
        for(int i = 0; i < len; i++){
            if(str == currDevice[i].Name) ret = currDevice[i].ID;
        }
        return ret;
    }
    device_t & getDevice(int x){
        return currDevice[x];
    }
    ~iTCP_kernel_t(){

    }
};
#endif