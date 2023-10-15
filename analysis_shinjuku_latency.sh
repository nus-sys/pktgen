cat latency-*.txt > latency.txt
cat thp-*.txt > thp.txt

python3 analysis_shinjuku.py latency.txt thp.txt