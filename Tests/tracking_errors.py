import numpy as np
import pyopenephys
import matplotlib.pylab as plt
import os
import quantities as pq
import seaborn as sns
from plotting_convention import *


fs = 30
fs_tick = 22
fs_leg = 18

oe_folder = os.path.join(os.getcwd(), 'OE_Data/multiple_sources/multiple_sources_2018-01-12_16-48-23')
data_folder = os.path.join(os.getcwd(), 'Python_Data')


file = pyopenephys.File(oe_folder)
exp1 = file.experiments[0]
rec1 = exp1.recordings[0]

tracking = rec1.tracking
frequencies = np.linspace(30, 300, len(tracking))

nsamples = [len(tr.times) for tr in tracking]
tracking = list(np.array(tracking)[np.argsort(nsamples)])

data_py = np.load(os.path.join(data_folder, 'multiple_sources_data_array.npy'))
ts_py = np.load(os.path.join(data_folder, 'multiple_sources_timestamps.npy'))*pq.s
ts_oe = [tr.times for tr in tracking]

# ts_py = [t - t[0] for t in ts_py]
# ts_oe = [tr.times - tr.times[0] for tr in tracking]
#
# ts_errors = [(t - t_py).magnitude for (t, t_py) in zip(ts_oe, ts_py)]
# ts_errors_cl = [err[np.where(np.abs(err) < 0.005)] for err in ts_errors]

dt_py = [np.diff(t) for t in ts_py]
dt_oe = [np.diff(tr.times) for tr in tracking]

ts_diff_errors = [(d_oe - d_py).magnitude for (d_oe, d_py) in zip(dt_oe, dt_py)]
ts_diff_errors_cl = [err[np.where(np.abs(err) < 0.01)] for err in ts_diff_errors]

# ts_diff_errors = [(np.diff(t) - np.diff(t_py)).magnitude for (t, t_py) in zip(ts_oe, ts_py)]
# ts_diff_errors_cl = [err[np.where(np.abs(err) < 0.005)] for err in ts_errors]
#
# ts_diff_errors_cl = [err-np.mean(err) for err in ts_diff_errors_cl]

# [plt.hist(err*1000, bins=300, alpha=0.4, label=str(int(frequencies[i]))+' Hz')
#  for i, err in enumerate(ts_diff_errors_cl)]

fig = plt.figure(figsize=[15, 12])
ax = fig.add_subplot(111)
for i, err in enumerate(ts_diff_errors_cl):
    sns.distplot(err*1000, kde=False, label=str(int(frequencies[i]))+' Hz', ax=ax)

ax.set_xlim([-0.5, 0.5])
plt.xticks([-0.5, -0.25, 0, 0.25, 0.5], fontsize=fs_tick)
plt.yticks([5000, 10000, 15000], fontsize=fs_tick)
ax.set_xlabel('timestamp error (ms)', fontsize=fs)
ax.set_ylabel('density (#)', fontsize=fs)
ax.legend(fontsize=fs_leg)

simplify_axes(ax)

print('Number of samples: Python: ', sum([len(t) for t in ts_py]), 'OE: ', sum([len(t) for t in ts_oe]))
print('Average errors and std: ')
for err in ts_diff_errors:
    print(np.mean(err), ' pm ', np.std(err))

print('Percent above 10ms: ', sum([len(np.where(np.abs(err)>0.01)[0]) for err in ts_diff_errors]),
      'out of ', sum([len(err) for err in ts_diff_errors]),
      ' (', sum([len(np.where(np.abs(err)>0.01)[0]) for err in ts_diff_errors])
      /sum([len(err) for err in ts_diff_errors])*100, ' %)')

print('Percent above 5ms: ', sum([len(np.where(np.abs(err)>0.005)[0]) for err in ts_diff_errors]),
      'out of ',sum([len(err) for err in ts_diff_errors]),
      ' (', sum([len(np.where(np.abs(err)>0.005)[0]) for err in ts_diff_errors])
      /sum([len(err) for err in ts_diff_errors])*100, ' %)')

print('Percent above 1ms: ', sum([len(np.where(np.abs(err)>0.001)[0]) for err in ts_diff_errors]),
      'out of ', sum([len(err) for err in ts_diff_errors]),
      ' (', sum([len(np.where(np.abs(err)>0.001)[0]) for err in ts_diff_errors])
      /sum([len(err) for err in ts_diff_errors])*100, ' %)')

print('Percent above 500us: ', sum([len(np.where(np.abs(err)>0.0005)[0]) for err in ts_diff_errors]),
      'out of ',sum([len(err) for err in ts_diff_errors]),
      ' (', sum([len(np.where(np.abs(err)>0.0005)[0]) for err in ts_diff_errors])
      /sum([len(err) for err in ts_diff_errors])*100, ' %)')

plt.ion()
plt.show()