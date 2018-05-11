import numpy as np
import pyopenephys
import matplotlib.pylab as plt
import os
import quantities as pq
from exana import tracking as tr
import neo


oe_folder_uni = os.path.join(os.getcwd(), 'OE_Data/closed_loop/closed_loop_uniform_2018-02-09_23-05-54')
# oe_folder_gauss = os.path.join(os.getcwd(), 'OE_Data/closed_loop/closed_loop_gaussi_2018-02-09_22-30-33')
oe_folder_gauss = os.path.join(os.getcwd(), 'OE_Data/closed_loop/tests/closed_loop_gauss_2018-02-10_20-56-42')


source='fpga'

file_uni = pyopenephys.File(oe_folder_uni)
exp1 = file_uni.experiments[0]
rec1 = exp1.recordings[0]

tracking_uni = rec1.tracking
events_uni = rec1.events
for ev in events_uni:
    if ev.processor == 'Rhythm_FPGA':
        ev_fpga_uni = ev.times[::2]
    else:
        ev_stim_uni = ev.times

if source=='fpga':
    sptr_uni = neo.SpikeTrain(ev_fpga_uni, t_stop=ev_fpga_uni[-1])
else:
    sptr_uni = neo.SpikeTrain(ev_stim_uni, t_stop=ev_stim_uni[-1])

file_gauss = pyopenephys.File(oe_folder_gauss)
exp1 = file_gauss.experiments[0]
rec1 = exp1.recordings[0]

tracking_gauss = rec1.tracking
events_gauss = rec1.events
for ev in events_gauss:
    if ev.processor == 'Rhythm_FPGA':
        ev_fpga_gauss = ev.times[::2]
    else:
        ev_stim_gauss = ev.times

if source=='fpga':
    sptr_gauss = neo.SpikeTrain(ev_stim_gauss, t_stop=ev_fpga_gauss[-1])
else:
    sptr_gauss = neo.SpikeTrain(ev_stim_gauss, t_stop=ev_fpga_gauss[-1])

x_uni = tracking_uni[0].x * pq.m
y_uni = tracking_uni[0].y * pq.m
t_uni = tracking_uni[0].times

x_gauss = tracking_gauss[0].x[1:] * pq.m
y_gauss = tracking_gauss[0].y[1:] * pq.m
t_gauss = tracking_gauss[0].times[1:]

rate_map_uni = tr.spatial_rate_map(x_uni, y_uni, t_uni, sptr_uni, binsize=0.05*pq.m, smoothing=0.02, convolve=True,
                                   mask_unvisited=False)
rate_map_gauss = tr.spatial_rate_map(x_gauss, y_gauss, t_gauss, sptr_gauss, binsize=0.05*pq.m, smoothing=0.02, convolve=True,
                                     mask_unvisited=False)

bxl = byl = 1*pq.m
ms=5

fig_tot, axes = plt.subplots(2,2, figsize=(15,15))
tr.plot_path(x_uni, y_uni, t_uni, sptr=sptr_uni, ax=axes[0][0], box_xlen=bxl,
             box_ylen=byl, markersize=ms,
             origin='upper')
axes[0][0].grid(False)
axes[0][0].tick_params(axis='both', which='both', bottom='off', top='off', left='off', right='off',
                       labelbottom='off', labelleft='off')
axes[0][0].set_title('N spikes {}'.format(len(sptr_uni)))
axes[0][0].axis('off')
axes[0][0].axis('equal')

im = axes[0][1].imshow(rate_map_uni, origin='upper',
                       extent=(0, 1, 0, 1), cmap='jet')
axes[0][1].grid(False)
axes[0][1].tick_params(axis='both', which='both', bottom='off', top='off', left='off', right='off',
                       labelbottom='off', labelleft='off')
axes[0][1].axis('off')

axes[0][1].set_title('%.2f Hz' % np.nanmax(rate_map_uni))

tr.plot_path(x_gauss, y_gauss, t_gauss, sptr=sptr_gauss, ax=axes[1][0], box_xlen=bxl,
             box_ylen=byl, markersize=ms,
             origin='upper')
axes[1][0].grid(False)
axes[1][0].tick_params(axis='both', which='both', bottom='off', top='off', left='off', right='off',
                       labelbottom='off', labelleft='off')
axes[1][0].set_title('N spikes {}'.format(len(sptr_gauss)))
axes[1][0].axis('off')
axes[1][0].axis('equal')

im = axes[1][1].imshow(rate_map_gauss, origin='upper',
                       extent=(0, 1, 0, 1), cmap='jet')
axes[1][1].grid(False)
axes[1][1].tick_params(axis='both', which='both', bottom='off', top='off', left='off', right='off',
                       labelbottom='off', labelleft='off')
axes[1][1].axis('off')
axes[1][1].set_title('%.2f Hz' % np.nanmax(rate_map_gauss))

fig_tot.tight_layout()


# #TEST
# import os
# import pyopenephys
# import pylab as plt
#
# oe_folder = '/home/alessio/Documents/Codes/open-ephys/plugins/tracking-plugins/Tests/closed_loop_gauss_2018-02-09_16-47-28'
#
# file = pyopenephys.File(oe_folder)
# exp1 = file.experiments[0]
# rec1 = exp1.recordings[0]
#
# tracking= rec1.tracking
#
# plt.plot(tracking[0].times)


plt.ion()
plt.show()