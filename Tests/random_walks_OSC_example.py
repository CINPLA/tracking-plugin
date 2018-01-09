'''
Simulate and send spiral 2D trajectory via OSC message
Input to OpenEphys GUI
'''


import OSC
import time
import os
import numpy as np
import matplotlib.pylab as plt
import threading

class OSCthread(threading.Thread):
    def __init__(self, name, addr, port, freq, duration):
        super(OSCthread, self).__init__(name=name)
        self.osc = []
        self.port = port
        self.addr = addr
        self.freq = freq
        self.duration = duration
        self.x = []
        self.y = []
        self.sent_trajectory = []
        self.timestamps = []

    
    def setup(self, width, height):
        self.osc = OSC.OSCClient()
        self.osc.connect(('127.0.0.1', p))
        self.nsamples = int(self.duration * freq)
        self.period = 1. / float(freq)
        self.width = width
        self.height = height

        print 'Created source: ', self.freq,  ' ', self.port, ' ', self.addr, ' ', self.period

        x = []
        y = []
        x.append(np.random.uniform(0, 1))
        y.append(np.random.uniform(0, 1))

        while len(x) < self.nsamples:
            dx, dy = 0.05 * np.random.randn(2)
            x_n, y_n = (x[-1] + dx) * width, (y[-1] + dy) * height

            while x_n < 0 or y_n < 0 or x_n > width or y_n > height:
                dx, dy = 0.05 * np.random.randn(2)
                x_n, y_n = (x[-1] + dx) * width, (y[-1] + dy) * height

            x.append(x_n)
            y.append(y_n)
            
        self.x = x
        self.y = y
    
    def run(self):
        prev_t = time.time()
        count = 0
        t0 = time.time()
        try:
            while True:
                curr_t = time.time()

                if curr_t - prev_t > self.period:
                    if count < self.nsamples:
                        m_x = self.x[count]
                        m_y = self.y[count]
                    else:
                        print Exception('Finished sending samples for ' +  self.name )
                        break

                    # rand = np.random.rand(1)
                    # if rand > 0.7:
                    #     m_x = np.nan
                    #     m_y = np.nan
                    #
                    # print m_x, m_y

                    count += 1

                    oscmsg = OSC.OSCMessage()
                    oscmsg.setAddress(self.addr)
                    oscmsg.append([m_x, m_y, width, height])
                    self.sent_trajectory.append([m_x, m_y, width, height])
                    self.timestamps.append(curr_t - t0)
                    self.osc.send(oscmsg)

                    prev_t = curr_t
                    # print 'sent port: ', self.port, 'address: ', self.addr, curr_t - t0
            t1 = time.time()
            print 'finished in ', t1 - t0

        except KeyboardInterrupt:
            pass
        

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

save_data = True

osc = []

for idx, (p, addr, freq) in enumerate(zip(ports, addresses, frequencies)):
    thread = OSCthread(name = 'Tread' + str(idx), port=p, addr=addr, freq=freq, duration=duration)
    thread.setup(width, height)
    osc.append(thread)

ts = []
data = []

for o in osc:
    o.start()

for o in osc:
    o.join()
    print 'Finished: ', o.name

    ts.append(np.array(o.timestamps))
    data.append(np.array(o.sent_trajectory))

if save_data:
    print 'Saving...'
    if not os.path.isdir(os.path.join(os.getcwd(), 'Python_Data')):
        os.mkdir(os.path.join(os.getcwd(), 'Python_Data'))
    np.save('Python_Data/data_array', data)
    np.save('Python_Data/timestamps', ts)
    print('Done saving!')