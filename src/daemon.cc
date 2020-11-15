/*
 * @Author: Naiqian
 * @Date: 2020-11-14 00:02:22
 * @LastEditTime: 2020-11-15 21:20:42
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/src/Daemon.cc
 */

#include "include.h"
#include "ip.h"
#include "utils.h"
#include "pthread.h"
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
    while(1){
        routingTableop.dumpRoutingTable();
        sendArp(iTCP_kernel);
        sleep(interval);
    }
}

int main(){
    int deviceNum = addAll();
    setFrameReceiveCallback(recvFrame);
    //setIPPacketReceiveCallback()
    BlockingQueue<recvPcap> q(1024);
    //fprintf(stderr, "%d\n", deviceNum);
    for(int i = 0; i < deviceNum; i++){
        //fprintf(stderr, "%d\n", deviceNum);
        std::thread t(recvPcapPacket, std::ref(q), i);
        t.detach();
    }
    std::thread t(periodSendMyArp, (uint32_t)5);
    t.detach();
    while(1){
        recvPcap tmp = q.pop();
        iTCP_kernel.getEthCallback()(tmp.packet, tmp.len, tmp.ID);
    }
    return 0;
}