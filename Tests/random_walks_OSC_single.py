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

        pos = np.zeros((self.nsamples, 2))
        vel = np.zeros((self.nsamples, 2))

        pos[0] = np.random.uniform(2)

        #while len(x) < self.nsamples:
        dt = 0.01
        for i in range(1, self.nsamples):
            dvel = 0.5 * np.random.randn(2)

            vel[i] = vel[i-1] + dvel
            vel[i] = vel[i] + 0.1 * vel[i] * (2 - np.linalg.norm(vel[i]))
            #vel[i] = vel[i] + (0.1 * - vel[i])
            pos[i] = pos[i-1] + vel[i-1] * dt

            if pos[i, 0] < 0:
                pos[i, 0] = -pos[i, 0]
                vel[i, 0] = 0
            if pos[i, 1] < 0:
                pos[i, 1] = -pos[i, 1]
                vel[i, 1] = 0
            if pos[i, 0] > width:
                pos[i, 0] = 2*width - pos[i, 0]
                vel[i, 0] = 0 
            if pos[i, 1] > height:
                pos[i, 1] = 2*height - pos[i, 1]
                vel[i, 1] = 0 

        self.x = pos[:, 0]
        self.y = pos[:, 1]
    
    def run(self):
        prev_t = time.time()
        count = 0
        t0 = time.time()
        try:
            while True:
                curr_t = time.time()

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

                time.sleep(self.period)


            t1 = time.time()
            print 'finished in ', t1 - t0

        except KeyboardInterrupt:
            pass
        

n_sources = 10
ports = range(27020, 27020 + n_sources)
addresses = ['/red', '/green', '/blue',  '/magenta', '/cyan', '/orange', '/pink', '/grey', '/violet', '/yellow']
frequencies = np.linspace(30, 300, n_sources)  # Hz
duration = 30  # sec
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

#for o in osc:
#    o.start()
osc[0].start()

ts = []
data = []

#for o in osc:
#    o.join()
#    print 'Finished: ', o.name

#    ts.append(np.array(o.timestamps))
#    data.append(np.array(o.sent_trajectory))

osc[0].join()

#print 'Saving'
#if not os.path.exists("data"):
#    os.makedirs("data")
#np.save('data/data_array', data)
#np.save('data/timestamps', ts)