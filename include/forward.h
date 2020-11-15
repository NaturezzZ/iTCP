/*
 * @Author: Naiqian
 * @Date: 2020-11-14 00:37:13
 * @LastEditTime: 2020-11-14 15:01:59
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/include/forward.h
 */
#include "utils.h"
#include "ip.h"
#include <bits/stdc++.h>

int forward(const ipv4_t src, const ipv4_t dest, int proto, const void *buf, int len, int ttl);
//int sendIPPacket(const struct in_addr src, const struct in_addr dest, int proto, const void *buf, int len, int ttl=32){

