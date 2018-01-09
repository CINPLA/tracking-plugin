import numpy as np
import pyopenephys
import matplotlib.pylab as plt
import os

oe_folder = '/home/alessiob/Documents/Codes/C++/plugins/tracking-plugins/Tests/Testing/test_sources_error_2017-12-21_10-45-13'
data_folder = '/home/alessiob/Documents/Codes/C++/plugins/tracking-plugins/Tests/Python_Data'

file = pyopenephys.File(oe_folder)
exp1 = file.experiments[0]
rec1 = exp1.recordings[0]

tracking = rec1.tracking

nsamples = [len(tr.times) for tr in tracking]
tracking = list(np.array(tracking)[np.argsort(nsamples)])

data_py = np.load(os.path.join(data_folder, 'data_array.npy'), encoding='bytes')
ts_py = np.load(os.path.join(data_folder, 'timestamps.npy'), encoding='bytes')

ts_py = [t - t[0] for t in ts_py]
ts = [tr.times - tr.times[0] for tr in tracking]

ts_errors = [t - t_py for (t, t_py) in zip(ts, ts_py)]
[plt.hist(err, bins=100, alpha=0.3) for err in ts_errors]