'''
Simulate and send 1 random walk via OSC message
Input to OpenEphys GUI
'''

from __future__ import print_function

import os
import time
import numpy as np
import matplotlib.pylab as plt
import threading


from random_walks_osc import *

thread = OSCthread(name = 'Thread', port=27020, addr='/red', freq=60, duration=300)
thread.setup(1., 1.)

thread.run()
