/*
 * @Author: Naiqian
 * @Date: 2020-11-02 23:05:13
 * @LastEditTime: 2020-11-14 02:18:20
 * @LastEditors: Naiqian
 * @Description: 
 */
#include "core.h"
#include <bits/stdc++.h>
//#include <map>
iTCP_kernel_t iTCP_kernel;
ipv4_t ptr2ip_t(const uint8_t* ptr){
    ipv4_t ret = *((const ipv4_t*) ptr);
    return ret;
}
void ip_t2ptr(ipv4_t ip, uint8_t* ptr){
    *((ipv4_t*)ptr) = ip;
    return;
}
std::map<rtKey_t, rtKey_t> x;