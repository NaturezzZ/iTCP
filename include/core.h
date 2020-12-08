/*
 * @Author: Naiqian
 * @Date: 2020-11-02 22:12:15
 * @LastEditTime: 2020-12-09 03:20:04
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
//#ifndef mac
#include <netpacket/packet.h>
//#endif
#include "tcp.h"
#include "udp.h"
#include <condition_variable>

ipv4_t ptr2ip_t(const uint8_t*);
void ip_t2ptr(ipv4_t, uint8_t*);
uint16_t h2n(uint16_t host);
class iTCP_kernel_t;

class mysocket_t{
public:
    uint8_t* databuf;
    int readp;
    int writep;
    int sizep;

    std::mutex data_mtx;

    int index;
    int sid;
    int role;
    int domain;
    int type;
    int protocol;
    sockaddr *address;
    socklen_t address_len;
    TCB *tcb;

    std::mutex conn_mtx; 
    std::condition_variable conn_cv; 
    bool conn_ready; 

    std::mutex acc_syn_mtx; 
    std::condition_variable acc_syn_cv; 
    bool acc_syn_ready; 

    std::mutex acc_ack_mtx;
    std::condition_variable acc_ack_cv;
    bool acc_ack_ready;

    std::mutex read_mtx;
    std::condition_variable read_cv;
    bool read_ready;

    int myread(void *buf, int len){
        
        int readLen = 0;
        int pastread = readp;
        std::unique_lock<std::mutex> lck(read_mtx);
        //fprintf(stderr, "*****************\n");
        // while(pastread + len > sizep){
        //     fprintf(stderr, "fuck\n");
        //     fprintf(stderr, "%d,%d,%d\n", readp, len, sizep);
        //     data_mtx.lock();
        //     memcpy(buf+readLen, databuf+readp, sizep-readp);
        //     readp += sizep-readp;
        //     readLen += sizep-readp;
        //     data_mtx.unlock();
        //     if(readp + len <= sizep) break;
        //     read_cv.wait(lck);
        // }
        while(pastread + len > sizep){
            read_cv.wait(lck);
        }
        data_mtx.lock();
        memcpy(buf, databuf+readp, len);
        readp += len;
        //readLen += len-readLen;
        data_mtx.unlock();
        return len;
    }

    int mywrite(const void *buf, int len){
        //fprintf(stderr, "mywrite\n");
        data_mtx.lock();
        
        memcpy(databuf+sizep, buf, len);
        //fprintf(stderr, "mywrite,%x\n", *((uint32_t*)databuf));
        //writep += len;
        sizep += len;
        data_mtx.unlock();
        return len;
    }

    mysocket_t() { }
    mysocket_t(const mysocket_t & cp){
        databuf = new uint8_t[1<<20];
        memcpy(databuf, cp.databuf, (1<<20));
        readp = cp.readp;
        writep = cp.writep;
        sizep = cp.sizep;
        index = cp.index;
        sid = cp.sid;
        role = cp.role;
        domain = cp.domain;
        type = cp.type;
        protocol = cp.protocol;
        address = new sockaddr;
        memcpy(address, cp.address, cp.address_len);
        tcb = new TCB;
        memcpy(tcb, cp.tcb, sizeof(TCB));
        //iTCP_kernel = cp.iTCP_kernel;
        conn_ready = cp.conn_ready;
        acc_syn_ready = cp.acc_syn_ready;
    }
    mysocket_t(const mysocket_t* cp){
        databuf = new uint8_t[1<<20];
        memcpy(databuf, cp->databuf, (1<<20));
        readp = cp->readp;
        writep = cp->writep;
        sizep = cp->sizep;
        index = cp->index;
        sid = cp->sid;
        role = cp->role;
        domain = cp->domain;
        type = cp->type;
        protocol = cp->protocol;
        address = new sockaddr;
        address->sa_family = AF_INET;
        memcpy(address->sa_data, cp->address->sa_data, sizeof(tuple4));
        tcb = new TCB;
        memcpy(tcb, cp->tcb, sizeof(TCB));
        //iTCP_kernel = cp.iTCP_kernel;
        conn_ready = cp->conn_ready;
        acc_syn_ready = cp->acc_syn_ready;
        acc_ack_ready = cp->acc_ack_ready;
    }
    mysocket_t(int _index, int _sid, const iTCP_kernel_t& _iTCP_kernel, int _domain, int _type, int _protocol){
        databuf = new uint8_t[1<<20];
        memset(databuf, 0, (1<<20));
        readp = 0;
        writep = 0;
        sizep = 0;
        index = _index; sid = _sid;
        domain = _domain;
        type = _type;
        protocol = _protocol;
        role = -1;
        address = new sockaddr;
        address_len = 0;
        //iTCP_kernel = _iTCP_kernel;
        tcb = new TCB();
        conn_ready = false;
        acc_syn_ready = false;
    }
    int bind(const struct sockaddr *_address, socklen_t _address_len){
        if(role != -1) return -1;
        //if(iTCP_kernel.bindOK(_address, _address_len)){
        role = TCP_SERVER;
        address = new sockaddr();
        address_len = _address_len;
        memcpy(address, _address, _address_len);
        //fprintf(stderr, "checkbind:%d\n", ((tuple4*)(address->sa_data))->srcPort);
        if(type == SOCK_STREAM){
            tcb = new TCB(role, CLOSED);
        }
        return 0;
        //}
        //else{
        //    sprintf(stderr, "[ERROR] Can not bind!\n");
        //    return -1; 
        //}
    }
    int listen(int _sockfd, int backlog=1){
        if(role != TCP_SERVER) return -1;
        if(tcb->get_state() != CLOSED) return -1;
        tcb->set_state(LISTEN);
        return 0;
    }
    int accept(int accfd){
        return 0;
    }
};

class device_t
{
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

typedef int (*connectPacketReceiveCallback)(const void* buf, int len);

using frameReceiveCallback = int (*)(const void *, int, int);
using IPPacketReceiveCallback = int (*) (const void*, int);

class mysocket_t;

class iTCP_kernel_t{
//private:
public:
    BlockingQueue<int> unSolvedSocket;
    std::map<int, BlockingQueue< std::pair<uint8_t*, int> >* > socket_queue;
    std::map<int, mysocket_t*> socket_set; 
    rtTable_t routeTable;
    std::vector<device_t> currDevice;
    frameReceiveCallback eth_callback;
    IPPacketReceiveCallback ip_callback;
    connectPacketReceiveCallback connect_callback; 
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
    int bindOK(const sockaddr * address, const socklen_t address_len){
        return 1;
    }
    ~iTCP_kernel_t(){

    }
    using bqtmp = BlockingQueue<std::pair<uint8_t* ,int>>;
    int socket_create(int _domain, int _type, int _protocol){
        int i = 100;
        for(i = 100; i < 200; i++){
            if(socket_set.find(i) != socket_set.end()) continue;
            mysocket_t *mysocket = new mysocket_t(i, i, (*this), _domain, _type, _protocol);
            socket_set.insert(std::pair<int, mysocket_t*>(i, mysocket));
            bqtmp* bq = new bqtmp(1000);
            socket_queue.insert(std::pair<int, bqtmp* >(i, bq));
            break;
        }
        return i;
    }

    int socket_bind(int sfd, const struct sockaddr* address, socklen_t address_len){
        if(!bindOK(address, address_len)){
            return -1;
        }
        if(socket_set.find(sfd) == socket_set.end()) return -1;
        (*(socket_set.find(sfd))).second->bind(address, address_len);
        return 0;
    }

    int socket_listen(int bfd){
        (*(socket_set.find(bfd))).second->tcb->status = LISTEN;
        return 0;
    }

    // int socket_accept_start(int bfd, sockaddr* address, socklen_t* len){
    //     if(address || len) return -1;
    //     int connfd = 100;
    //     for(connfd = 100; connfd < 200; connfd++){
    //         if(socket_set.find(connfd) != socket_set.end()) continue;
    //         break;
    //     }
    //     mysocket_t conn_socket = mysocket_t((*(socket_set.find(bfd))).second);
    //     conn_socket.index = connfd;
    //     conn_socket.sid = connfd;
    //     conn_socket.tcb->status = LISTEN;
    //     connect_callback()
    //     uint8_t* buf;
    //     int len;
    //     return connfd;
    // }

    // int socket_accept_end(int connfd, uint16_t flag){
        
    // }
};
/*
 domain: ipv4/ipv6
 type: tcp/udp
 protocol: stream or not
 */



#endif