import numpy as np
import sys

dat = np.loadtxt(sys.argv[1])
dat = dat[:,2]

avg_cycle = np.average(dat)
avg_ns = avg_cycle / 2.9

min_cycle = np.amin(dat)
min_ns = min_cycle / 2.9

p10_cycle = np.percentile(dat, 10)
p10_ns = p10_cycle / 2.9

p25_cycle = np.percentile(dat, 25)
p25_ns = p25_cycle / 2.9

median_cycle = np.median(dat)
median_ns = median_cycle / 2.9

p90_cycle = np.percentile(dat, 90)
p90_ns = p90_cycle / 2.9

p99_cycle = np.percentile(dat, 99)
p99_ns = p99_cycle / 2.9

max_cycle = np.amax(dat)
max_ns = max_cycle / 2.9

print('-------------------- %15s --------------------' % 'In Cycles')
print('%8s %10s %10s %10s %10s %10s %10s %10s %10s' % ("", "Average", "Min", "10%", "25%", "Median", "90%", "99%", "Max"))
print('%8s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f' % ("Lat(cycles)", avg_cycle, min_cycle, p10_cycle, p25_cycle, median_cycle, p90_cycle, p99_cycle, max_cycle))
print('---------------------------------------------------------')

print('-------------------- %15s --------------------' % 'In Nanoseconds')
print('%8s %10s %10s %10s %10s %10s %10s %10s %10s' % ("", "Average", "Min", "10%", "25%", "Median", "90%", "99%", "Max"))
print('%8s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f' % ("Lat(ns)", avg_ns, min_ns, p10_ns, p25_ns, median_ns, p90_ns, p99_ns, max_ns))
print('---------------------------------------------------------')

print('-------------------- %15s --------------------' % 'In Microseconds')
print('%8s %10s %10s %10s %10s %10s %10s %10s %10s' % ("", "Average", "Min", "10%", "25%", "Median", "90%", "99%", "Max"))
print('%8s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f' % ("Lat(us)", avg_ns / 1000.0, min_ns / 1000.0, p10_ns / 1000.0, p25_ns / 1000.0, median_ns / 1000.0, p90_ns / 1000.0, p99_ns / 1000.0, max_ns / 1000.0))
print('---------------------------------------------------------')

thp = np.loadtxt(sys.argv[2])
print('Rx: %8.4f (Mps), 1: %8.4f (Mps), 2: %8.4f (Mps), Tx: %8.4f (Mps)' % (np.sum(thp[:,0]) / 1000.0, np.sum(thp[:,1]) / 1000.0, np.sum(thp[:,2]) / 1000.0,  np.sum(thp[:,3]) / 1000.0))
