import numpy as np
import pyopenephys
import matplotlib.pylab as plt
import os
import quantities as pq
from exana import tracking as tr
import neo

processor = 0

oe_folder_uni = os.path.join(os.getcwd(), 'OE_Data/closed_loop/test_no_pulsepal_uni_2018-01-12_16-33-49')
oe_folder_gauss = os.path.join(os.getcwd(), 'OE_Data/closed_loop/test_no_pulsepal_gauss_2018-01-12_16-23-31')

file_uni = pyopenephys.File(oe_folder_uni)
exp1 = file_uni.experiments[0]
rec1 = exp1.recordings[0]

tracking_uni = rec1.tracking
events_uni = rec1.events

sptr_uni = neo.SpikeTrain(events_uni[processor].times, t_stop=events_uni[processor].times[-1])

file_gauss = pyopenephys.File(oe_folder_gauss)
exp1 = file_gauss.experiments[0]
rec1 = exp1.recordings[0]

tracking_gauss = rec1.tracking
events_gauss = rec1.events

sptr_gauss = neo.SpikeTrain(events_gauss[processor].times, t_stop=events_gauss[processor].times[-1])

fig_uni = plt.figure()
ax1 = fig_uni.add_subplot(1, 2, 1, xlim=[0, 1], ylim=[0, 1], aspect=1)
ax1.grid(False)
x_uni = tracking_uni[0].x * pq.m
y_uni = tracking_uni[0].y * pq.m
t_uni = tracking_uni[0].times# - tracking_uni[0].times[0]

bxl = byl = 1*pq.m

tr.plot_path(x_uni, y_uni, t_uni, sptr=sptr_uni, ax=ax1, box_xlen=bxl, box_ylen=byl)
ax1.set_title('N spikes {}'.format(len(sptr_uni)))
ax2 = fig_uni.add_subplot(1, 2, 2, xlim=[0, 1], ylim=[0, 1], aspect=1,
                      yticks=[])
tr.plot_ratemap(x_uni, y_uni, t_uni, sptr_uni, binsize=0.05*pq.m,
                 mask_unvisited=True, ax=ax2, smoothing=0.04)

ax2.grid(False)



fig_gauss = plt.figure()
ax1 = fig_gauss.add_subplot(1, 2, 1, xlim=[0, 1], ylim=[0, 1], aspect=1)
ax1.grid(False)
x_gauss = tracking_gauss[0].x * pq.m
y_gauss = tracking_gauss[0].y * pq.m
t_gauss = tracking_gauss[0].times# - tracking_gauss[0].times[0]

bxl = byl = 1*pq.m
#
tr.plot_path(x_gauss, y_gauss, t_gauss, sptr=sptr_gauss, ax=ax1, box_xlen=bxl, box_ylen=byl)
ax1.set_title('N spikes {}'.format(len(sptr_gauss)))
ax2 = fig_gauss.add_subplot(1, 2, 2, xlim=[0, 1], ylim=[0, 1], aspect=1,
                      yticks=[])
tr.plot_ratemap(x_gauss, y_gauss, t_gauss, sptr_gauss, binsize=0.05*pq.m,
                 mask_unvisited=True, ax=ax2, smoothing=0.04)

ax2.grid(False)

plt.ion()
plt.show()