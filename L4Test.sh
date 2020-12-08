###
 # @Author: Naiqian
 # @Date: 2020-12-09 04:00:42
 # @LastEditTime: 2020-12-09 04:13:23
 # @LastEditors: Naiqian
 # @Description: 
 # @FilePath: /iTCP/L4Test.sh
### 

#!/bin/bash

make clean
make test_l4 -j8
echo ""
echo "/*****************************************************************************/"
echo "[Note] Please open two terminals and "
echo "[Note] [In Terminal 1] sudo ./execNS ns3 ./recver.out"
echo "[Note] [In Terminal 2] sudo ./execNS ns2 ./sender.out"
echo "[Note] Please make sure that recver.out is executed earlier than sender.out"
echo "/*****************************************************************************/"
