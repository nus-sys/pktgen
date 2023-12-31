import numpy as np
import sys

dat = np.loadtxt(sys.argv[1])
dat = dat[:,2]

print('%6s %8s %8s %8s %8s %8s %8s %8s' % ("", "Average", "Min", "10%", "25%", "Median", "90%", "99%"))
print('%6s %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f' % ("Lat(ns)", np.average(dat), np.amin(dat), np.percentile(dat, 10), np.percentile(dat, 25), np.median(dat), np.percentile(dat, 90), np.percentile(dat, 99)))

thp = np.loadtxt(sys.argv[2])
print('RX: %8.4f (Mps), 1: %8.4f (Mps), 2: %8.4f (Mps)' % 
    (np.sum(thp[:,1]), np.sum(thp[:,3]), np.sum(thp[:,5])))

print('%.4f %.4f %.4f' % 
    (np.sum(thp[:,1]), np.sum(thp[:,3]), np.sum(thp[:,5])))
