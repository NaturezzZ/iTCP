/*
 * @Author: Naiqian
 * @Date: 2020-11-14 00:02:22
 * @LastEditTime: 2020-12-09 03:51:17
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/src/sender.cc
 */

#include "include.h"
#include "ip.h"
#include "utils.h"
#include "tcp.h"
#include "pthread.h"
#include "socket.h"
extern iTCP_kernel_t iTCP_kernel;
extern opRoutingTable routingTableop;
using namespace std;

void recvPcapPacket(BlockingQueue<recvPcap> &q, int id){
    //fprintf(stderr, "????\n");
    pcap_t *handle;
    char *errbuf;
    errbuf = new char[BUFSIZ];
    handle = pcap_open_live(iTCP_kernel.currDevice[id].Name.c_str(), BUFSIZ, 1, 1000, errbuf);
    while(1){
        struct pcap_pkthdr header;
        const u_char *packet;
        //fprintf(stderr, "????\n");
        packet = pcap_next(handle, &header);
        //fprintf(stderr, "fuck\n");
        recvPcap ret;
        memcpy(ret.packet, packet, header.len);
        ret.len = header.len;
        ret.ID = id;
        q.push(ret);
    }
    pcap_close(handle);
}

void periodSendMyArp(uint32_t interval){
    clock_t start = clock();
    int cnt = 0;
    while(cnt<2){
        cnt +=1;
        routingTableop.dumpRoutingTable();
        sendArp(iTCP_kernel);
        sleep(interval);
    }
}

void processIPPacket(){
    while(1){
        int fd = iTCP_kernel.unSolvedSocket.pop();
        readIPPacket(fd);
    }
    return;
}

void work(){
    sleep(10);
    fprintf(stderr, "[INFO] Start testing\n");
    std::thread processip(processIPPacket);
    processip.detach();

    int fd = __wrap_socket(AF_INET, SOCK_STREAM, 0);
    sockaddr * sock = new sockaddr;
    uint8_t srcipp[4] = {10,100,2,1};
    uint8_t dstipp[4] = {10,100,2,2};
    
    ((tuple4*)(sock->sa_data))->srcIP = ptr2ip_t(srcipp);
    ((tuple4*)(sock->sa_data))->srcPort = h2n(10000);
    ((tuple4*)(sock->sa_data))->dstIP = ptr2ip_t(dstipp);
    ((tuple4*)(sock->sa_data))->dstPort = h2n(10000);

    __wrap_connect(fd, sock, sizeof(sockaddr));

    //cerr << "connect finished "<< (*(iTCP_kernel.socket_set.find(fd))).second->tcb->status << endl;
    uint8_t* writefile = new uint8_t[1001];
    *((uint32_t*)writefile) = 0x12345678UL;
    __wrap_write(fd, writefile, 1001);
    __wrap_write(fd, writefile, 1001);
    __wrap_close(fd);
    return;
}

int main(){
    int deviceNum = addAll();
    setFrameReceiveCallback(recvFrame);
    //IPPacketReceiveCallback
    setIPPacketReceiveCallback(recvIPPacket);
    //setIPPacketReceiveCallback()
    uint8_t ip2[4] = {10, 100, 2, 2};
    uint8_t mask2[4] = {255, 255, 255, 255};
    uint8_t mac2[6] = {0x82,0xf3,0xa8,0x60,0x27,0xfd};
    in_addr ip22; ip22.s_addr = ptr2ip_t(ip2);
    in_addr mask22; mask22.s_addr = ptr2ip_t(mask2);
    const char devicename[20] = "veth2-3";
    routingTableop.setRoutingTable(ip22, mask22, mac2, devicename, 1024);
    BlockingQueue<recvPcap> q(1024);
    
    for(int i = 0; i < deviceNum; i++){
        std::thread t(recvPcapPacket, std::ref(q), i);    
        t.detach();
    }
    std::thread t(periodSendMyArp, (uint32_t)3);
    
    std::thread th_work(work);

    while(1){
        recvPcap tmp = q.pop();
        iTCP_kernel.getEthCallback()(tmp.packet, tmp.len, tmp.ID);
    }
    t.detach();
    th_work.detach();


    return 0;
}