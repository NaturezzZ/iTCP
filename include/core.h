#ifndef __ITCP_CORE_H
#define __ITCP_CORE_H
#include<bits/stdc++.h>
#define ETHER 0
#define IPV4 1
#define IPV6 2
typedef std::string device_t;
class iTCP_kernel_t{
private:
    std::vector<device_t> currDevice;
public:
    iTCP_kernel_t(){

    }
    int addDevice(device_t deviceName){
        int len = currDevice.size();
        int index;
        bool flag_device = 0;
        for(index = 0; index < len; index++){
            if(deviceName == currDevice[index]) flag_device = 1;
        }
        if(!flag_device){
            currDevice.push_back(deviceName);
        }
        return len;
    }
    ~iTCP_kernel_t(){

    }
};
#endif