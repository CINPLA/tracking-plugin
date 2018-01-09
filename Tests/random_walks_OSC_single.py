'''
Simulate and send spiral 2D trajectory via OSC message
Input to OpenEphys GUI
'''

import os
import OSC
import time
import numpy as np
import matplotlib.pylab as plt
import threading

from random_walks import *

duration = 600  # sec
nsamples = []
osc_ports = []
periods = []
counts = []
x_vec = []
y_vec = []
width = 1.
height = 1.

osc = []

thread = OSCthread(name = 'Tread', port=27021, addr='/red', freq=30, duration=300)
thread.setup(width, height)

thread.run()
