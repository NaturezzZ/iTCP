/*
 * @Author: Naiqian
 * @Date: 2020-11-02 22:12:15
 * @LastEditTime: 2020-11-05 00:58:16
 * @LastEditors: Naiqian
 * @Description: 
 */
#ifndef __ITCP_CORE_H
#define __ITCP_CORE_H
#include<bits/stdc++.h>
#include<pcap.h>
#include <ifaddrs.h>
#include "packetio.h"
#include <netpacket/packet.h>
#define ETHER 0
#define IPV4 1
#define IPV6 2
typedef std::string deviceName_t;
typedef uint8_t* mac_t;
typedef uint32_t deviceID_t;
class device_t{
public:
    mac_t Mac;
    deviceName_t Name;
    deviceID_t ID;
    pcap_t *pcapHandler;
    char *pcap_errbuf;
    device_t(deviceID_t deviceID, void* deviceMac, deviceName_t deviceName){
        ID = deviceID;
        Mac = new uint8_t[6];
        memcpy(Mac, deviceMac, 6);
        pcap_errbuf = new char[4096];
        memset(pcap_errbuf, 0, 4096);
        Name = deviceName;
        pcapHandler = pcap_open_live(Name.c_str(), BUFSIZ, 1, 1000, pcap_errbuf);
    }
    device_t(const device_t & cp){

        Mac = new uint8_t[6];
        memcpy(Mac, cp.Mac, 6);
        pcap_errbuf = new char[4096];
        memset(pcap_errbuf, 0, 4096);
        memcpy(pcap_errbuf, cp.pcap_errbuf, 4096);

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
class iTCP_kernel_t{
private:
    std::vector<device_t> currDevice;
    frameReceiveCallback eth_callback;
public:
    frameReceiveCallback &getCallback(){
        return eth_callback;
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
    device_t & getDevice(int ID){
        return currDevice[ID];
    }
    ~iTCP_kernel_t(){

    }
};
#endif