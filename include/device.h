/*
 * @Author: Naiqian
 * @Date: 2020-11-02 19:02:34
 * @LastEditTime: 2020-11-14 01:23:05
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/include/device.h
 */
/** 
 * @file device.h
 * @brief Library supporting network device management.
 */
#ifndef __ITCP_DEVICE_H
#define __ITCP_DEVICE_H
#include "core.h"
/**
 * Add a device to the library for sending/receiving packets. 
 *
 * @param device Name of network device to send/receive packet on.
 * @return A non-negative _device-ID_ on success, -1 on error.
 */
int addDevice(const char* device);
int addAll();
/**
 * Find a device added by `addDevice`.
 *
 * @param device Name of the network device.
 * @return A non-negative _device-ID_ on success, -1 if no such device 
 * was found.
 */
int findDevice(const char* device);
#endif