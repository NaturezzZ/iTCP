/*
 * @Author: Naiqian
 * @Date: 2020-11-02 23:04:53
 * @LastEditTime: 2020-11-15 17:52:01
 * @LastEditors: Naiqian
 * @Description: 
 */
#include "device.h"
#include <bits/stdc++.h>
#include "utils.h"
using namespace std;
extern iTCP_kernel_t iTCP_kernel;
int addDevice(const char* device){
    int ret = -1;
    ifaddrs *ifaddr = NULL;
    // get interface address failed
    if(getifaddrs(&ifaddr) == -1){
        fprintf(stderr, "[ERROR] Can not get interface address ");
        fprintf(stderr, strerror(errno));
        fprintf(stderr, "\n");
        return ret;
    }
    // successfully get interface address
    for(ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
        //cout << ifa->ifa_name << ',' << ifa->ifa_addr->sa_family << endl;
        
        // no device -> add "lo"
        // device -> add device
        bool parse_flag = 0;
        if(!device){
            if(!strcmp(ifa->ifa_name, "lo") && ifa->ifa_addr->sa_family == AF_PACKET) parse_flag = 1;
        }
        else{
            if(!strcmp(ifa->ifa_name, device) && ifa->ifa_addr->sa_family == AF_PACKET) parse_flag = 1;
        }
        if (!parse_flag) continue;
        else{

            struct sockaddr_ll *s = (struct sockaddr_ll*)(ifa->ifa_addr);

            ret = iTCP_kernel.addDeviceKernel(string(device),s->sll_addr);
        }

    }
    if(ret == -1){
        if(!device) fprintf(stderr, "[ERROR] No device name is inputted!\n");
        else fprintf(stderr, "[ERROR] Can not find interface requested!\n");
    }
    return ret;
}

int addAll(){
    int ret = 0;
    ifaddrs *ifaddr = NULL;
    // get interface address failed
    if(getifaddrs(&ifaddr) == -1){
        fprintf(stderr, "[ERROR] Can not get interface address ");
        fprintf(stderr, strerror(errno));
        fprintf(stderr, "\n");
        return ret;
    }

    // successfully get interface address
    for(ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
        if(ifa->ifa_addr->sa_family != AF_PACKET) continue;
        if(ifa->ifa_name[0] != 'v') continue;
        struct sockaddr_ll *s = (struct sockaddr_ll*)(ifa->ifa_addr);
        iTCP_kernel.addDeviceKernel(string(ifa->ifa_name), s->sll_addr);
        std::cerr << "[INFO] Successfully Added Device " << ifa->ifa_name << std::endl;
        ret += 1;
    }
    for(ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
        if(ifa->ifa_addr->sa_family != AF_INET) continue;
        if(ifa->ifa_name[0] != 'v') continue;
        struct sockaddr_in *s = (struct sockaddr_in*)(ifa->ifa_addr);
        in_addr tmp = (s->sin_addr);
        uint8_t* tmpip = new uint8_t[4];
        ip_t2ptr(tmp.s_addr, tmpip);
        iTCP_kernel.addIP(string(ifa->ifa_name), tmpip);
        delete[] tmpip;
        std::cerr << "[INFO] Successfully Added IP of Device " << ifa->ifa_name << std::endl;
    }
    if(ret == 0){
        fprintf(stderr, "[WARNING] Can not find any available interface!\n");
    }
    return ret;    
}

int findDevice(const char* device, mac_t Mac){
    int deviceID = iTCP_kernel.findDeviceKernel(string(device));
    if(deviceID<0){
        fprintf(stderr, "[WARNING] Can not find corresponding device.\n");
        return -1;
    }
    Mac = iTCP_kernel.getMac(string(device));
    return deviceID;
}

