cat latency-*.txt > latency.txt
cat thp-*.txt > thp.txt

python3 analysis-rocksdb.py latency.txt thp.txt