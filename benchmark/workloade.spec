src_mac=3c:fd:fe:55:45:3a
dst_mac=3c:fd:fe:55:f8:a2

src_ip=10.10.1.2
dst_ip=10.10.1.1

dport=1234

workload=rocksdb

fieldlength=32
readproportion=0.5
updateproportion=0
scanproportion=0.5
insertproportion=0

maxscanlength=200

recordcount=1000000
operationcount=1000000

payload_len=512