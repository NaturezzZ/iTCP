#include "device.h"
#include <bits/stdc++.h>
#include "utils.h"
using namespace std;

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
        cout << ifa->ifa_name << ',' << ifa->ifa_addr->sa_family << endl;
        
        // no device -> add "lo"
        // device -> add device
        bool parse_flag = 0;
        if(!device){
            if(!strcmp(ifa->ifa_name, "lo") && ifa->ifa_addr->sa_family == AF_PACKET) parse_flag = 1;
        }
        else{
            if(!strcmp(ifa->ifa_name, device) && ifa->ifa_addr->sa_family == AF_PACKET) parse_flag = 1;
        }
        //cout << ifa->ifa_addr << endl;
        if (!parse_flag) continue;
        else{
            ret = iTCP_kernel.addDevice(string(device));
        }
    }
    //cerr << AF_INET << ',' << AF_INET6 << ',' <<AF_PACKET << endl;
    return ret;
}

int findDevice(const char* device){

}
int main(){
    addDevice("eth1");
    return 0;
}
