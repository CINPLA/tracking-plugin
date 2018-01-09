'''
Simulate and send 10 random walks wit different frequency via OSC message
Input to OpenEphys GUI
'''

import os
import OSC
import time
import numpy as np
import matplotlib.pylab as plt
import threading

from random_walks import *

n_sources = 10
ports = range(27020, 27020 + n_sources)
addresses = ['/red', '/green', '/blue',  '/magenta', '/cyan', '/orange', '/pink', '/grey', '/violet', '/yellow']
frequencies = np.linspace(30, 300, n_sources)  # Hz
duration = 60  # sec
nsamples = []
osc_ports = []
periods = []
counts = []
x_vec = []
y_vec = []
width = 1.
height = 1.

osc = []

for idx, (p, addr, freq) in enumerate(zip(ports, addresses, frequencies)):
    thread = OSCthread(name = 'Tread' + str(idx), port=p, addr=addr, freq=freq, duration=duration)
    thread.setup(width, height)
    osc.append(thread)

for o in osc:
    o.start()

ts = []
data = []

for o in osc:
    o.join()
    print 'Finished: ', o.name

    ts.append(np.array(o.timestamps))
    data.append(np.array(o.sent_trajectory))

print 'Saving'
if not os.path.exists("Python_Data"):
    os.makedirs("Python_Data")
np.save('Python_Data/multiple_sources_data_array', data)
np.save('Python_Data/multiple_sources_timestamps', ts)