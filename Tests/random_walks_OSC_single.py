'''
Simulate and send 1 random walk via OSC message
Input to OpenEphys GUI
'''

import os
import OSC
import time
import numpy as np
import matplotlib.pylab as plt
import threading

from random_walks import *

thread = OSCthread(name = 'Tread', port=27020, addr='/red', freq=30, duration=600)
thread.setup(1., 1.)

thread.run()
