/*
 * @Author: Naiqian
 * @Date: 2020-11-13 00:42:37
 * @LastEditTime: 2020-11-14 22:30:17
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/include/myArp.h
 */

//#include "include.h"
#ifndef __ITCP_MYARP_H
#define __ITCP_MYARP_H
#include "core.h"
#include <bits/stdc++.h>
#include "ip.h"
#include "utils.h"
#include "ip.h"
/*
 * send self introduction, a broadcast IPV4 packet
 * return 0 if normal
 */
int sendArp(iTCP_kernel_t & kernel);
void recvArp(iTCP_kernel_t &kernel, const ipv4_t srcIP, 
    const ipv4_t mask, const deviceID_t ID, const mac_t srcMac, const int ttl);
#endif