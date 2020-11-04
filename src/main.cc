/*
 * @Author: Naiqian
 * @Date: 2020-11-04 17:07:54
 * @LastEditTime: 2020-11-05 01:00:34
 * @LastEditors: Naiqian
 * @Description: 
 */
#include "core.h"
#include "device.h"
#include "packetio.h"
//#include "utils.h"
extern iTCP_kernel_t iTCP_kernel;

using namespace std;

int main(){
    addDevice("veth0-3");
    uint8_t *tmp = new uint8_t[10];
    uint8_t *dst = new uint8_t[6];
    constructFrame(tmp, 10, 0, dst, 0);
    return 0;
}
