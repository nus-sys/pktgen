# pktgen
Packet generation based on DPDK

## Setup Instructions

1. Fetch the dependencies

```
git submodule init
git submodule update
```

2. Build the dependencies and `pktgen` 

3. Run `pktgen`

```
sudo LD_LIBRARY_PATH=/usr/local/lib/x86_64-linux-gnu ./build/pktgen -l 0 -n 4 -- -c 1 -f 1 -r 1 -l shinjuku -p benchmark/workloada.spec
```