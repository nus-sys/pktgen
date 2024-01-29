# Yahoo! Cloud System Benchmark
# Workload A: Update heavy workload
#   Application example: Session store recording recent actions
#                        
#   Read/update ratio: 50/50
#   Default data size: 1 KB records (10 fields, 100 bytes each, plus key)
#   Request distribution: zipfian
src_mac=b8:ce:f6:2a:30:ec
dst_mac=b8:ce:f6:2a:3f:9c

src_ip=10.0.0.4
dst_ip=10.0.0.3

dport=1234

workload=rocksdb

recordcount=1000000
operationcount=10000000

fieldlength=256

readproportion=1
updateproportion=0
scanproportion=0
insertproportion=0

requestdistribution=zipfian

payload_len=64