import numpy as np
import sys

dat = np.loadtxt(sys.argv[1])
dat = dat[:,2]
print('%6s %8s %8s %8s %8s %8s %8s %8s' % ("", "Average", "Min", "10%", "25%", "Median", "90%", "99%"))
print('%6s %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f' % ("Lat(ns)", np.average(dat), np.amin(dat), np.percentile(dat, 10), np.percentile(dat, 25), np.median(dat), np.percentile(dat, 90), np.percentile(dat, 99)))

thp = np.loadtxt(sys.argv[2])
print('Rx: %8.4f (Kops), INSERT: %8.4f (Kops), READ: %8.4f (Kops), UPDATE: %8.4f (Kops), SCAN: %8.4f (Kops)' % (np.sum(thp[:,0]), np.sum(thp[:,1]), np.sum(thp[:,2]), np.sum(thp[:,3]), np.sum(thp[:,4])))
