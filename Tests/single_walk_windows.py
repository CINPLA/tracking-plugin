#!/usr/bin/env python

"""     simpleOSC 0.3.2
    ixi software - dec, 2012
    www.ixi-software.net

    very simple API for the pyOSC at https://trac.v2.nl/wiki/pyOSC

    The main aim of this implementation is to provide with a the simplest way to deal
    with that OSC implementation and try to make life easier to those who don't have
    understanding of sockets or programming. 

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
"""
import time
import os, sys
import numpy as np
import matplotlib.pylab as plt

from simpleOSC import initOSCClient, initOSCServer, setOSCHandler, sendOSCMsg, closeOSC, \
     createOSCBundle, sendOSCBundle, startOSCServer

class OSCwalk():
    def __init__(self, addr, port, freq, duration):
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
        initOSCClient('127.0.0.1', self.port)
        self.nsamples = int(self.duration * self.freq)
        self.period = 1. / float(self.freq)
        self.width = width
        self.height = height

        print('Created source: ', self.freq,  ' ', self.port, ' ', self.addr, ' ', self.period)

        pos = np.zeros((self.nsamples, 2))
        vel = np.zeros((self.nsamples, 2))

        pos[0] = np.random.uniform(0, 1, 2)

        #while len(x) < self.nsamples:
        dt = 0.01
        for i in range(1, self.nsamples):
            dvel = 0.5 * np.random.randn(2)

            vel[i] = vel[i-1] + dvel
            vel[i] = vel[i] + 0.1 * vel[i] * (2 - np.linalg.norm(vel[i]))
            #vel[i] = vel[i] + (0.1 * - vel[i])
            pos[i] = pos[i-1] + vel[i-1] * dt

            if pos[i, 0] < 0:
                pos[i, 0] = 0
                vel[i, 0] = 0
            if pos[i, 1] < 0:
                pos[i, 1] = 0
                vel[i, 1] = 0
            if pos[i, 0] > width:
                pos[i, 0] = width
                vel[i, 0] = 0 
            if pos[i, 1] > height:
                pos[i, 1] = height
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
                    print(Exception('Finished sending samples for ' +  self.name ))
                    break
                # rand = np.random.rand(1)
                # if rand > 0.7:
                #     m_x = np.nan
                #     m_y = np.nan
                #
                print m_x, m_y
                count += 1
                msg = [m_x, m_y, self.width, self.height]
                self.sent_trajectory.append([m_x, m_y, self.width, self.height])
                self.timestamps.append(curr_t - t0)
                sendOSCMsg(self.addr, msg)
                time.sleep(self.period)


            t1 = time.time()
            print('finished in ', t1 - t0)

        except KeyboardInterrupt:
            closeOSC() # finally close the connection before exiting or program
            pass
        

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print 'Provide port, address, freq, and duration'
    elif len(sys.argv) == 5:
        port = int(sys.argv[1])
        addr = str(sys.argv[2])
        freq = float(sys.argv[3])
        dur = float(sys.argv[4])

        osc = OSCwalk(addr, port, freq, dur)
        osc.setup(1.,1.)
        osc.run()
        
    else:
        raise Exception('Provide port, address, freq, and duration')














