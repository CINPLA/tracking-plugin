"""
Python library for reading OpenEphys files.
Depends on: sys
            os
            glob
            datetime
            numpy
            quantities
            xmljson

Authors: Alessio Buccino @CINPLA,
         Svenn-Arne Dragly @CINPLA,
         Milad H. Mobarhan @CINPLA,
         Mikkel E. Lepperod @CINPLA
"""

# TODO: add extensive function descrption and verbose option for prints

from __future__ import division
from __future__ import print_function
from __future__ import with_statement

import quantities as pq
import os
import os.path as op
import numpy as np
from datetime import datetime
import locale
import struct
import platform
import xml.etree.ElementTree as ET
from xmljson import yahoo as yh

# TODO related files
# TODO append .continuous files directly to file and memory map in the end
# TODO ChannelGroup class - needs probe file
# TODO Channel class
# TODO add SYNC and TRACKERSTIM metadata


class Channel:
    def __init__(self, index, name, gain, channel_id):
        self.index = index
        self.id = channel_id
        self.name = name
        self.gain = gain


class AnalogSignal:
    def __init__(self, channel_id, signal, times):
        self.signal = signal
        self.channel_id = channel_id
        self.times = times

    def __str__(self):
        return "<OpenEphys analog signal:shape: {}, sample_rate: {}>".format(
            self.signal.shape, self.sample_rate
        )


class DigitalSignal:
    def __init__(self, times, channel_id, sample_rate):
        self.times = times
        self.channel_id = channel_id
        self.sample_rate = sample_rate

    def __str__(self):
        return "<OpenEphys digital signal: nchannels: {}>".format(
            self.channel_id
        )


class Sync:
    def __init__(self, times, channel_id, sample_rate):
        self.times = times
        self.channel_id = channel_id
        self.sample_rate = sample_rate

    def __str__(self):
        return "<OpenEphys sync signal: nchannels: {}>".format(
            self.channel_id
        )


class TrackingData:
    def __init__(self, times, x, y, width, height, channels, metadata):
        self.times = times
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.channels = channels
        self.metadata = metadata
    def __str__(self):
        return "<OpenEphys tracking data: times shape: {}, positions shape: {}>".format(
            self.times.shape, self.x.shape
        )


class SpikeTrain:
    def __init__(self, times, waveforms,
                 spike_count, channel_count, samples_per_spike,
                 sample_rate, t_stop, **attrs):
        assert(waveforms.shape[0] == spike_count), waveforms.shape[0]
        assert(waveforms.shape[1] == channel_count), waveforms.shape[1]
        assert(waveforms.shape[2] == samples_per_spike), waveforms.shape[2]
        assert(len(times) == spike_count)
        assert times[-1] <= t_stop, ('Spike time {}'.format(times[-1]) +
                                     ' exceeds duration {}'.format(t_stop))
        self.times = times
        self.waveforms = waveforms
        self.attrs = attrs
        self.t_stop = t_stop

        self.spike_count = spike_count
        self.channel_count = channel_count
        self.samples_per_spike = samples_per_spike
        self.sample_rate = sample_rate

    @property
    def num_spikes(self):
        """
        Alias for spike_count.
        """
        return self.spike_count

    @property
    def num_chans(self):
        """
        Alias for channel_count.
        """
        return self.channel_count

#todo fix channels where they belong!
class ChannelGroup:
    def __init__(self, channel_group_id, filename, channels,
                 fileclass=None, **attrs):
        self.attrs = attrs
        self.filename = filename
        self.id = channel_group_id
        self.channels = channels
        self.fileclass = fileclass

    def __str__(self):
        return "<OpenEphys channel_group {}: channel_count: {}>".format(
            self.id, len(self.channels)
        )

    @property
    def analog_signals(self):
        ana = self.fileclass.analog_signals[0]
        analog_signals = []
        for channel in self.channels:
            analog_signals.append(AnalogSignal(signal=ana.signal[channel.id],
                                               channel_id=channel.id,
                                               sample_rate=ana.sample_rate))
        return analog_signals

    @property
    def spiketrains(self):
        return [sptr for sptr in self.fileclass.spiketrains
                if sptr.attrs['channel_group_id'] == self.id]


class File:
    """
    Class for reading experimental data from an OpenEphys dataset.
    """
    def __init__(self, foldername, probefile=None):
        # TODO assert probefile is a probefile
        # TODO add default prb map and allow to add it later
        self._absolute_foldername = foldername
        self._path, relative_foldername = os.path.split(foldername)
        self._experiments_names = [f for f in os.listdir(self._absolute_foldername)
                                   if os.path.isdir(op.join(self._absolute_foldername, f))
                                   and 'experiment' in f]
        self._exp_ids = [int(exp[-1]) for exp in self._experiments_names]
        self._experiments = []

        for (rel_path, id) in zip(self._experiments_names, self._exp_ids):
            self._experiments.append(Experiment(op.join(self._absolute_foldername, rel_path), id, probefile))

    @property
    def session(self):
        return os.path.split(self._absolute_foldername)[-1]

    @property
    def datetime(self):
        return self._start_datetime

    @property
    def experiments(self):
        return self._experiments


class Experiment:
    def __init__(self, path, id, probefile=None):
        self._recordings = []
        self._prb_file = probefile
        self._absolute_foldername = path
        self._absolute_parentfolder = op.dirname(path)
        self.id = id
        self.sig_chain = dict()

        self._read_settings(id)
        self._recording_names = [f for f in os.listdir(self._absolute_foldername)
                                   if os.path.isdir(op.join(self._absolute_foldername, f))
                                   and 'recording' in f]
        self._rec_ids = [int(rec[-1]) for rec in self._recording_names]
        for (rel_path, id) in zip(self._recording_names, self._rec_ids):
            self._recordings.append(Recording(op.join(self._absolute_foldername, rel_path), id,
                                              self.sig_chain, self._prb_file))

    def _read_settings(self, id):
        print('Loading Open-Ephys: reading settings.xml...')
        if id == 1:
            set_fname = [fname for fname in os.listdir(self._absolute_parentfolder)
                         if fname == 'settings.xml']
        else:
            set_fname = [fname for fname in os.listdir(self._absolute_parentfolder)
                         if fname.startswith('settings') and fname.endswith('.xml') and str(id) in fname]
        self.rhythm = False
        self.rhythmID = []
        # rhythmRates = np.array([1., 1.25, 1.5, 2, 2.5, 3, 3.33, 4., 5., 6.25,
        #                         8., 10., 12.5, 15., 20., 25., 30.])
        self.track_port = False
        self.track_portID = []
        self.track_portInfo = dict()

        self.track_stim = False
        self.track_stimInfo = []

        self.fileReader = False

        self.sync = False
        self.syncID = []

        if not len(set_fname) == 1:
            raise IOError('Unique settings file not found')

        self._set_fname = op.join(self._absolute_parentfolder, set_fname[0])
        with open(self._set_fname) as f:
            xmldata = f.read()
            self.settings = yh.data(ET.fromstring(xmldata))['SETTINGS']
        # read date in US format
        if platform.system() == 'Windows':
            locale.setlocale(locale.LC_ALL, 'english')
        elif platform.system() == 'Darwin':
            # bad hack...
            try:
                locale.setlocale(locale.LC_ALL, 'en_US.UTF8')
            except Exception:
                pass
        else:
            locale.setlocale(locale.LC_ALL, 'en_US.UTF8')
        self._start_datetime = datetime.strptime(self.settings['INFO']['DATE'], '%d %b %Y %H:%M:%S')
        self._channel_info = {}
        self.nchan = 0
        FPGA_count = 0
        if isinstance(self.settings['SIGNALCHAIN'], list):
            sigchain_iter = self.settings['SIGNALCHAIN']
        else:
            sigchain_iter = [self.settings['SIGNALCHAIN']]
        for sigchain in sigchain_iter:
            if isinstance(sigchain['PROCESSOR'], list):
                processor_iter = sigchain['PROCESSOR']
            else:
                processor_iter = [sigchain['PROCESSOR']]
            for processor in processor_iter:
                self.sig_chain.update({processor['name']: True})
                if processor['name'] == 'Sources/Rhythm FPGA':
                    if FPGA_count > 0:
                        raise NotImplementedError
                        # TODO can there be multiple FPGAs ?
                    FPGA_count += 1
                    self._channel_info['gain'] = {}
                    self.rhythm = True
                    self.rhythmID = processor['NodeId']
                    # gain for all channels
                    gain = {ch['number']: float(ch['gain']) * pq.uV  # TODO assert is uV
                            for chs in processor['CHANNEL_INFO'].values()
                            for ch in chs}
                    for chan in processor['CHANNEL']:
                        if chan['SELECTIONSTATE']['record'] == '1':
                            self.nchan += 1
                            chnum = chan['number']
                            self._channel_info['gain'][chnum] = gain[chnum]

                # debug
                if processor['name'] == 'Sources/File Reader':
                    if FPGA_count > 0:
                        raise NotImplementedError
                        # TODO can there be multiple FPGAs ?
                    FPGA_count += 1
                    self._channel_info['gain'] = {}
                    self.fileReader = True
                    self.fileReaderID = processor['NodeId']
                    for chan in processor['CHANNEL']:
                        if chan['SELECTIONSTATE']['record'] == '1':
                            self.nchan += 1
                if processor['name'] == 'Sources/Tracking Port':
                    self.track_port = True
                    self.track_portID = processor['NodeId']
                    self.track_portInfo.update(processor['TrackingNode'])
                if processor['name'] == 'Sources/Sync Port':
                    self.sync = True
                    self.syncID = processor['NodeId']
                if processor['name'] == 'Sinks/Tracker Stimulator':
                    self.track_stim = True
                    self.track_stimID = processor['NodeId']
                    # new version
                    self.track_stimInfo = dict()
                    self.track_stimInfo['circles'] = processor['CIRCLES']
                    self.track_stimInfo['channels'] = processor['CHANNELS']
                    self.track_stimInfo['output'] = processor['SELECTEDCHANNEL']
                    self.track_stimInfo['sync'] = {'ttl': processor['EDITOR']['TRACKERSTIMULATOR']['syncTTLchan'],
                                                   'output': processor['EDITOR']['TRACKERSTIMULATOR'][
                                                       'syncStimchan']}

        # Check openephys format
        if self.settings['CONTROLPANEL']['recordEngine'] == 'OPENEPHYS':
            self._format = 'openephys'
        elif self.settings['CONTROLPANEL']['recordEngine'] == 'RAWBINARY':
            self._format = 'binary'
        else:
            self._format = None
        print('Decoding data from ', self._format, ' format')

        if self.rhythm:
            print('RhythmFPGA with ', self.nchan, ' channels. NodeId: ', self.rhythmID)
        if self.track_port:
            print('Tracking Port. NodeId: ', self.track_portID)

        if self.rhythm or self.fileReader:
            recorded_channels = sorted([int(chan) for chan in
                                        self._channel_info['gain'].keys()])
            self._channel_info['channels'] = recorded_channels
            if self._prb_file is not None:
                self._keep_channels = []
                self._probefile_ch_mapping = _read_python(self._prb_file)['channel_groups']
                for group_idx, group in self._probefile_ch_mapping.items():
                    group['gain'] = []
                    # prb file channels are sequential, 'channels' are not as they depend on FPGA channel selection
                    # -> Collapse them into array
                    for chan, oe_chan in zip(group['channels'],
                                             group['oe_channels']):
                        if oe_chan not in recorded_channels:
                            raise ValueError('Channel "' + str(oe_chan) +
                                             '" in channel group "' +
                                             str(group_idx) + '" in probefile' +
                                             self._prb_file +
                                             ' is not marked as recorded ' +
                                             'in settings file' +
                                             self._set_fname)
                        group['gain'].append(
                            self._channel_info['gain'][str(oe_chan)]
                        )
                        self._keep_channels.append(recorded_channels.index(oe_chan))
                print('Number of selected channels: ', len(self._keep_channels))
            else:
                self._keep_channels = None  # HACK
                # TODO sequential channel mapping
                print('sequential channel mapping')
        else:
            self._keep_channels = None

        if self.fileReader:
            recorded_channels = sorted([int(chan) for chan in
                                        self._channel_info['gain'].keys()])
            self._channel_info['channels'] = recorded_channels
            if self._prb_file is not None:
                self._keep_channels = []
                self._probefile_ch_mapping = _read_python(self._prb_file)['channel_groups']
                for group_idx, group in self._probefile_ch_mapping.items():
                    group['gain'] = []
                    # prb file channels are sequential, 'channels' are not as they depend on FPGA channel selection -> Collapse them into array
                    for chan, oe_chan in zip(group['channels'],
                                             group['oe_channels']):
                        if oe_chan not in recorded_channels:
                            raise ValueError('Channel "' + str(oe_chan) +
                                             '" in channel group "' +
                                             str(group_idx) + '" in probefile' +
                                             self._prb_file +
                                             ' is not marked as recorded ' +
                                             'in settings file' +
                                             self._set_fname)
                        group['gain'].append(
                            self._channel_info['gain'][str(oe_chan)]
                        )
                        self._keep_channels.append(recorded_channels.index(oe_chan))
                print('Number of selected channels: ', len(self._keep_channels))
            else:
                self._keep_channels = None  # HACK
                # TODO sequential channel mapping
                print('sequential channel mapping')

        self.sig_chain.update({'format': self._format, 'nchan': self.nchan, 'keep_channels': self._keep_channels})

    @property
    def recordings(self):
        return self._recordings


class Recording:
    def __init__(self, path, id, sig_chain, probefile=None):
        self.absolute_foldername = path
        self.prb_file = probefile
        self.sig_chain = sig_chain
        self._format = sig_chain['format']
        self._keep_channels = sig_chain['keep_channels']
        self.nchan = sig_chain['nchan']
        self.id = id

        self._analog_signals_dirty = True
        self._digital_signals_dirty = True
        self._channel_groups_dirty = True
        self._spiketrains_dirty = True
        self._tracking_dirty = True
        self._events_dirty = True
        self._times = []
        self._duration = []

        self._analog_signals = []
        self._digital_signals = []
        self._tracking_signals = []
        self._messages_signals = []

        self.__dict__.update(self._read_sync_message())


    @property
    def duration(self):
        if self.rhythm:
            self._duration = (self.analog_signals[0].signal.shape[1] /
                              self.analog_signals[0].sample_rate)
        elif self.track_port:
            self._duration = self.tracking[0].times[0][-1] - self.tracking[0].times[0][0]
        else:
            self._duration = 0

        return self._duration

    @property
    def sample_rate(self):
        if self.processor:
            return self._processor_sample_rate
        else:
            return self._software_sample_rate

    def channel_group(self, channel_id):
        if self._channel_groups_dirty:
            self._read_channel_groups()
        return self._channel_id_to_channel_group[channel_id]

    @property
    def channel_groups(self):
        if self._channel_groups_dirty:
            self._read_channel_groups()

        return self._channel_groups

    @property
    def analog_signals(self):
        if self._analog_signals_dirty:
            self._read_analog_signals()

        return self._analog_signals

    @property
    def spiketrains(self):
        if self._spiketrains_dirty:
            self._read_spiketrains()

        return self._spiketrains

    @property
    def digital_in_signals(self):
        if self._digital_signals_dirty:
            self._read_digital_signals()

        return self._digital_signals

    @property
    def sync_signals(self):
        if self._digital_signals_dirty:
            self._read_digital_signals()

        return self._sync_signals

    @property
    def events(self):
        if self._events_dirty:
            self._read_digital_signals()

        return self._events

    @property
    def tracking(self):
        if self._tracking_dirty:
            self._read_tracking()

        return self._tracking_signals

    @property
    def times(self):
        if self.rhythmID:
            self._times = self.analog_signals[0].times
        elif self.track_port:
            self._times = self.tracking[0].times[0]
        else:
            self._times = []

        return self._times

    def _read_sync_message(self):
        info = dict()
        stimes = []
        sync_messagefile = [f for f in os.listdir(self.absolute_foldername) if 'sync_messages' in f][0]
        with open(op.join(self.absolute_foldername, sync_messagefile), "r") as fh:
            while True:
                spl = fh.readline().split()
                if not spl:
                    break
                if 'Software' in spl:
                    self.processor = False
                    stime = spl[-1].split('@')
                    hz_start = stime[-1].find('Hz')
                    sr = float(stime[-1][:hz_start]) * pq.Hz
                    info['_software_sample_rate'] = sr
                    info['_software_start_time'] = int(stime[0])
                elif 'Processor:' in spl:
                    self.processor = True
                    stime = spl[-1].split('@')
                    hz_start = stime[-1].find('Hz')
                    stimes.append(float(stime[-1][:hz_start]))
                    sr = float(stime[-1][:hz_start]) * pq.Hz
                    info['_processor_sample_rate'] = sr
                    info['_processor_start_time'] = int(stime[0])
                else:
                    message = {'time': int(spl[0]),
                               'message': ' '.join(spl[1:])}
                    info['messages'].append(message)
        if any(np.diff(np.array(stimes, dtype=int))):
            raise ValueError('Found different processor start times')
        return info

    def _read_messages(self):
        events_folder = [op.join(self.absolute_foldername, f)
                         for f in os.listdir(self.absolute_foldername) if 'events' in f][0]
        message_folder = [op.join(events_folder, f) for f in os.listdir(events_folder)
                           if 'Message_Center' in f][0]
        text_groups = [f for f in os.listdir(message_folder)]
        if self._format == 'binary':
            for tg in text_groups:
                text = np.load(op.join(message_folder, tg, 'text.npy'))
                ts = np.load(op.join(message_folder, tg, 'timestamps.npy'))
                channels = np.load(op.join(message_folder, tg, 'channels.npy'))


    def _read_channel_groups(self):
        self._channel_id_to_channel_group = {}
        self._channel_group_id_to_channel_group = {}
        self._channel_count = 0
        self._channel_groups = []
        for channel_group_id, channel_info in self._probefile_ch_mapping.items():
            num_chans = len(channel_info['channels'])
            self._channel_count += num_chans
            channels = []
            for idx, chan in enumerate(channel_info['channels']):
                channel = Channel(
                    index=idx,
                    channel_id=chan,
                    name="channel_{}_channel_group_{}".format(chan,
                                                              channel_group_id),
                    gain=channel_info['gain'][idx]
                )
                channels.append(channel)

            channel_group = ChannelGroup(
                channel_group_id=channel_group_id,
                filename=None,#TODO,
                channels=channels,
                fileclass=self,
                attrs=None #TODO
            )


            self._channel_groups.append(channel_group)
            self._channel_group_id_to_channel_group[channel_group_id] = channel_group

            for chan in channel_info['channels']:
                self._channel_id_to_channel_group[chan] = channel_group

        # TODO channel mapping to file
        self._channel_ids = np.arange(self._channel_count)
        self._channel_groups_dirty = False

    def _read_tracking(self):
        if 'Sources/Tracking Port' in self.sig_chain.keys():
            # Check and decode files
            events_folder = [op.join(self.absolute_foldername, f)
                                 for f in os.listdir(self.absolute_foldername) if 'events' in f][0]
            tracking_folder = [op.join(events_folder, f) for f in os.listdir(events_folder)
                                if 'Tracking_Port' in f][0]
            binary_groups = [f for f in os.listdir(tracking_folder)]
            if self._format == 'binary':
                import struct
                for bg in binary_groups:
                    data_array = np.load(op.join(tracking_folder, bg, 'data_array.npy'))
                    ts = np.load(op.join(tracking_folder, bg, 'timestamps.npy'))
                    channels = np.load(op.join(tracking_folder, bg, 'channels.npy'))
                    metadata = np.load(op.join(tracking_folder, bg, 'metadata.npy'))
                    data_array = np.array([struct.unpack('4f', d) for d in data_array])

                    ts = ts / float(self.sample_rate)
                    x, y, w, h = data_array[:, 0], data_array[:, 1], data_array[:, 2], data_array[:, 3]

                    tracking_data = TrackingData(
                            times=ts,
                            x=x,
                            y=y,
                            channels=channels,
                            metadata=metadata,
                            width=w,
                            height=h
                        )

                    self._tracking_signals.append(tracking_data)
                self._tracking_dirty = False

    def _read_analog_signals(self):
        if self.processor:
            # Check and decode files
            continuous_folder = [op.join(self.absolute_foldername, f)
                                 for f in os.listdir(self.absolute_foldername) if 'continuous' in f][0]
            processor_folder = [op.join(continuous_folder, f) for f in os.listdir(continuous_folder)
                                if 'File_Reader' in f or 'RhythmFPGA' in f][0]

            filenames = [f for f in os.listdir(processor_folder)]
            if self._format == 'binary':
                if any('.dat' in f for f in filenames):
                    datfile = [f for f in filenames if '.dat' in f and 'continuous' in f][0]
                    print('.dat: ', datfile)
                    with open(op.join(processor_folder, datfile), "rb") as fh:
                        anas, nsamples = read_analog_binary_signals(fh, self.nchan)
                    ts = np.load(op.join(processor_folder, 'timestamps.npy')) / self.sample_rate
                else:
                    raise ValueError("'continuous.dat' should be in the folder")
            elif self._format == 'openephys':
                # Find continuous CH data
                contFiles = [f for f in os.listdir(self._absolute_foldername) if 'continuous' in f and 'CH' in f]
                contFiles = sorted(contFiles)
                if len(contFiles) != 0:
                    print('Reading all channels')
                    anas = np.array([])
                    for f in contFiles:
                        fullpath = op.join(self._absolute_foldername, f)
                        sig = read_analog_continuous_signal(fullpath)
                        if anas.shape[0] < 1:
                            anas = sig['data'][None, :]
                        else:
                            if sig['data'].size == anas[-1].size:
                                anas = np.append(anas, sig['data'][None, :], axis=0)
                            else:
                                raise Exception('Channels must have the same number of samples')
                    assert anas.shape[0] == len(self._channel_info['channels'])
                    nsamples = anas.shape[1]
                    print('Done!')
            # Keep only selected channels
            if self._keep_channels is not None:
                assert anas.shape[1] == nsamples, 'Assumed wrong shape'
                anas_keep = anas[self._keep_channels, :]
            else:
                anas_keep = anas
            self._analog_signals = [AnalogSignal(
                channel_id=range(anas_keep.shape[0]),
                signal=anas_keep,
                times=ts
            )]
        else:
            self._analog_signals = [AnalogSignal(
                channel_id=np.array([]),
                signal=np.array([]),
                times=np.array([])
            )]

        self._analog_signals_dirty = False

    def _read_spiketrains(self):
        if self.rhythm:
            # TODO check if spiketains are recorded from setings
            filenames = [f for f in os.listdir(self._absolute_foldername)
                         if f.endswith('.spikes')]
            self._spiketrains = []
            if len(filenames) == 0:
                return
            for fname in filenames:
                print('Loading spikes from ', fname.split('.')[0])
                data = loadSpikes(op.join(self._absolute_foldername, fname))
                clusters = data['recordingNumber']
                group_id = int(np.unique(data['source']))
                assert 'TT{}'.format(group_id) in fname
                for cluster in np.unique(clusters):
                    wf = data['spikes'][clusters == cluster]
                    wf = wf.swapaxes(1, 2)
                    sample_rate = int(data['header']['sampleRate'])
                    times = data['timestamps'][clusters == cluster]
                    times = (times - self.start_timestamp) / sample_rate
                    t_stop = self.duration.rescale('s')
                    self._spiketrains.append(
                        SpikeTrain(
                            times=times * pq.s,
                            waveforms=wf * pq.uV,
                            spike_count=len(times),
                            channel_count=int(data['header']['num_channels']),
                            sample_rate=sample_rate * pq.Hz,
                            channel_group_id=group_id,
                            samples_per_spike=40,  # TODO read this from file
                            gain=data['gain'][clusters == cluster],
                            threshold=data['thresh'][clusters == cluster],
                            name='Unit #{}'.format(cluster),
                            cluster_id=int(cluster),
                            t_stop=t_stop
                        )
                    )

        self._spiketrains_dirty = False

    def _read_digital_signals(self):
        filenames = [f for f in os.listdir(self._absolute_foldername)]
        if any('.events' in f and 'all_channels' in f for f in filenames):
            eventsfile = [f for f in filenames if '.events' in f and 'all_channels' in f][0]
            print('.events ', eventsfile)
            with open(op.join(self._absolute_foldername, eventsfile), "rb") as fh: #, encoding='utf-8', errors='ignore') as fh:
                data = {}

                print('loading events...')
                header = readHeader(fh)

                if float(header['version']) < 0.4:
                    raise Exception('Loader is only compatible with .events files with version 0.4 or higher')

                data['header'] = header

                struct_fmt = '=qH4BH'  # int[5], float, byte[255]
                struct_len = struct.calcsize(struct_fmt)
                struct_unpack = struct.Struct(struct_fmt).unpack_from

                nsamples = (os.fstat(fh.fileno()).st_size - fh.tell()) // struct_len
                print('Estimated events samples: ', nsamples)
                nread = 0

                read_data = []
                while True:
                    byte = fh.read(struct_len)
                    if not byte:
                        break
                    s = struct_unpack(byte)
                    read_data.append(s)
                    nread += 1

                print('Read event samples: ', nread)

                timestamps, sampleNum, eventType, nodeId, eventId, channel, recordingNumber = zip(*read_data)

                data['channel'] = np.array(channel)
                data['timestamps'] = np.array(timestamps)
                data['eventType'] = np.array(eventType)
                data['nodeId'] = np.array(nodeId)
                data['eventId'] = np.array(eventId)
                data['recordingNumber'] = np.array(recordingNumber)
                data['sampleNum'] = np.array(sampleNum)

                # TODO: check if data is null (data['event...'] is null?
                # Consider only TTL from FPGA (for now)
                num_channels = 8
                self._digital_signals = None
                if self.rhythm:
                    if len(data['timestamps']) != 0:
                        idxttl_fpga = np.where((data['eventType'] == 3) &
                                               (data['nodeId'] == int(self.rhythmID)))
                        digs = [list() for i in range(num_channels)]
                        if len(idxttl_fpga[0]) != 0:
                            print('TTLevents: ', len(idxttl_fpga[0]))
                            digchan = np.unique(data['channel'][idxttl_fpga])
                            print('Used IO channels ', digchan)
                            for chan in digchan:
                                idx_chan = np.where(data['channel'] == chan)
                                dig = data['timestamps'][idx_chan]
                                # Consider rising edge only
                                dig = dig[::2]
                                dig = dig - self.start_timestamp
                                dig = dig.astype('float') / self.sample_rate
                                digs[chan] = dig.rescale('s')

                        self._digital_signals = [DigitalSignal(
                            channel_id=list(range(num_channels)),
                            times=digs,
                            sample_rate=self.sample_rate
                        )]
                if self._digital_signals is None:
                    self._digital_signals = [DigitalSignal(
                        channel_id=np.array([]),
                        times=np.array([]),
                        sample_rate=[]
                    )]

                if self.sync:
                    if len(data['timestamps']) != 0:
                        idxttl_sync = np.where((data['eventType'] == 3) & (data['nodeId'] == int(self.syncID)))
                        syncchan = []
                        syncs = []
                        if len(idxttl_sync[0]) != 0:
                            print('TTL Sync events: ', len(idxttl_sync[0]))
                            syncchan = np.unique(data['channel'][idxttl_sync])
                            # TODO this should be reduced to a single loop
                            if len(syncchan) == 1:
                                # Single digital input
                                syncs = data['timestamps'][idxttl_sync]
                                # remove start_time (offset) and transform in seconds
                                syncs = syncs - self.start_timestamp
                                syncs = syncs.astype(dtype='float') / self.sample_rate
                                syncs = np.array([syncs]) * pq.s
                            else:
                                for chan in syncchan:
                                    idx_chan = np.where(data['channel'] == chan)
                                    new_sync = data['timestamps'][idx_chan]

                                    new_sync = new_sync - self.start_timestamp
                                    new_sync = new_sync.astype(dtype='float') / self.sample_rate
                                    syncs.append(new_sync)
                                syncs = np.array(syncs) * pq.s

                        self._sync_signals = [Sync(
                            channel_id=syncchan,
                            times=syncs,
                            sample_rate=self.sample_rate
                        )]
                    else:
                        self._sync_signals = [DigitalSignal(
                            channel_id=np.array([]),
                            times=np.array([]),
                            sample_rate=[]
                        )]
                else:
                    self._sync_signals = [Sync(
                        channel_id=np.array([]),
                        times=np.array([]),
                        sample_rate=[]
                    )]

                self._digital_signals_dirty = False
                self._events_dirty = False
                # self._events = data


    def clip_recording(self, clipping_times, start_end='start'):

        if clipping_times is not None:
            if clipping_times is not list:
                if type(clipping_times[0]) is not pq.quantity.Quantity:
                    raise AttributeError('clipping_times must be a quantity list of length 1 or 2')

            clipping_times = [t.rescale(pq.s) for t in clipping_times]

            for anas in self.analog_signals:
                anas.signal = clip_anas(anas, self.times, clipping_times, start_end)
            for digs in self.digital_in_signals:
                digs.times = clip_digs(digs, clipping_times, start_end)
                digs.times = digs.times - clipping_times[0]
            for track in self.tracking:
                track.positions, track.times = clip_tracking(track, clipping_times,start_end)

            self._times = clip_times(self._times, clipping_times, start_end)
            self._times -= self._times[0]
            self._duration = self._times[-1] - self._times[0]
        else:
            print('Empty clipping times list.')

# TOOLS FUNCTIONS#
def _read_python(path):
    from six import exec_
    path = op.realpath(op.expanduser(path))
    assert op.exists(path)
    with open(path, 'r') as f:
        contents = f.read()
    metadata = {}
    exec_(contents, {}, metadata)
    metadata = {k.lower(): v for (k, v) in metadata.items()}
    return metadata


def _cut_to_same_len(*args):
    out = []
    lens = []
    for arg in args:
        lens.append(len(arg))
    minlen = min(lens)
    for arg in args:
        out.append(arg[:minlen])
    return tuple(out)


def _start_from_zero_time(time, *args):
    out = []
    print('Tracking starts at {}'.format(min(time)))
    if not min(time) < 1 * pq.s:
        raise ValueError('No start time less than 1 s recorded, ' +
                         'min time = {}'.format(min(time)))
    start, = np.where(time == min(time))
    if len(start) > 1:
        raise ValueError('Multiple starting times recorded')
    if any(np.diff([len(x) for x in args])) or len(time) != len(args[0]):
        raise ValueError('All arguments must be of equal length')
    for arg in args:
        out.append(arg[int(start):])
    return time[int(start):], out


def _zeros_to_nan(*args):
    for arg in args:
        arg[arg == 0.0] = np.nan


def readHeader(fh):
    """Read header information from the first 1024 bytes of an OpenEphys file.

    Args:
        f: An open file handle to an OpenEphys file

    Returns: dict with the following keys.
        - bitVolts : float, scaling factor, microvolts per bit
        - blockLength : int, e.g. 1024, length of each record (see
            loadContinuous)
        - bufferSize : int, e.g. 1024
        - channel : the channel, eg "'CH1'"
        - channelType : eg "'Continuous'"
        - date_created : eg "'15-Jun-2016 21212'" (What are these numbers?)
        - description : description of the file format
        - format : "'Open Ephys Data Format'"
        - header_bytes : int, e.g. 1024
        - sampleRate : float, e.g. 30000.
        - version: eg '0.4'
        Note that every value is a string, even numeric data like bitVolts.
        Some strings have extra, redundant single apostrophes.

        Taken from OpenEphys team
    """
    header = {}

    # Read the data as a string
    # Remove newlines and redundant "header." prefixes
    # The result should be a series of "key = value" strings, separated
    # by semicolons.
    header_string = fh.read(1024).decode('utf-8').replace('\n','').replace('header.','')

    # Parse each key = value string separately
    for pair in header_string.split(';'):
        if '=' in pair:
            # print pair
            key, value = pair.split(' = ')
            key = key.strip()
            value = value.strip()

            # Convert some values to numeric
            if key in ['bitVolts', 'sampleRate']:
                header[key] = float(value)
            elif key in ['blockLength', 'bufferSize', 'header_bytes']:
                header[key] = int(value)
            else:
                # Keep as string
                header[key] = value

    return header


def get_number_of_records(filepath):
    # Open the file
    with open(filepath, 'rb') as f:
        # Read header info
        header = readHeader(f)

        # Get file length
        fileLength = os.fstat(f.fileno()).st_size

        # Determine the number of records
        record_length_bytes = 2 * header['blockLength'] + 22
        n_records = int((fileLength - 1024) / record_length_bytes)
        # if (n_records * record_length_bytes + 1024) != fileLength:
        #     print("file does not divide evenly into full records")
        #     # raise IOError("file does not divide evenly into full records")

    return n_records

# TODO require quantities and deal with it
def clip_anas(analog_signals, times, clipping_times, start_end):
    '''

    :param analog_signals:
    :param times:
    :param clipping_times:
    :param start_end:
    :return:
    '''

    if len(analog_signals.signal) != 0:
        times.rescale(pq.s)
        if len(clipping_times) == 2:
            idx = np.where((times > clipping_times[0]) & (times < clipping_times[1]))
        elif len(clipping_times) ==  1:
            if start_end == 'start':
                idx = np.where(times > clipping_times[0])
            elif start_end == 'end':
                idx = np.where(times < clipping_times[0])
        else:
            raise AttributeError('clipping_times must be of length 1 or 2')

        if len(analog_signals.signal.shape) == 2:
            anas_clip = analog_signals.signal[:, idx[0]]
        else:
            anas_clip = analog_signals.signal[idx[0]]

        return anas_clip
    else:
        return []


def clip_digs(digital_signals, clipping_times, start_end):
    '''

    :param digital_signals:
    :param clipping_times:
    :param start_end:
    :return:
    '''

    digs_clip = []
    for i, dig in enumerate(digital_signals.times):
        dig.rescale(pq.s)
        if len(clipping_times) == 2:
            idx = np.where((dig > clipping_times[0]) & (dig < clipping_times[1]))
        elif len(clipping_times) == 1:
            if start_end == 'start':
                idx = np.where(dig > clipping_times[0])
            elif start_end == 'end':
                idx = np.where(dig < clipping_times[0])
        else:
            raise AttributeError('clipping_times must be of length 1 or 2')
        digs_clip.append(dig[idx])

    return np.array(digs_clip) * pq.s


def clip_tracking(tracking, clipping_times, start_end):
    '''

    :param tracking:
    :param clipping_times:
    :param start_end:
    :return:
    '''
    assert len(tracking.positions) == len(tracking.times)

    track_clip = []
    t_clip = []

    for i, tr in enumerate(tracking.positions):
        tracking.times[i].rescale(pq.s)
        if len(clipping_times) == 2:
            idx = np.where((tracking.times[i] > clipping_times[0]) & (tracking.times[i] < clipping_times[1]))
        elif len(clipping_times) ==  1:
            if start_end == 'start':
                idx = np.where(tracking.times[i] > clipping_times[0])
            elif start_end == 'end':
                idx = np.where(tracking.times[i] < clipping_times[0])
        else:
            raise AttributeError('clipping_times must be of length 1 or 2')

        track_clip.append(np.array([led[idx[0]] for led in tr]))
        if start_end != 'end':
            times = tracking.times[i][idx[0]] - clipping_times[0]
        else:
            times = tracking.times[i][idx[0]]
        t_clip.append(times)

    return track_clip, t_clip


def clip_times(times, clipping_times, start_end='start'):
    '''

    :param times:
    :param clipping_times:
    :param start_end:
    :return:
    '''
    times.rescale(pq.s)

    if len(clipping_times) == 2:
        idx = np.where((times > clipping_times[0]) & (times <= clipping_times[1]))
    elif len(clipping_times) ==  1:
        if start_end == 'start':
            idx = np.where(times >= clipping_times[0])
        elif start_end == 'end':
            idx = np.where(times <= clipping_times[0])
    else:
        raise AttributeError('clipping_times must be of length 1 or 2')
    times_clip = times[idx]

    return times_clip


def loadSpikes(filepath):

    # constants for pre-allocating matrices:
    MAX_NUMBER_OF_SPIKES = int(1e6)
    data = { }

    f = open(filepath,'rb')
    header = readHeader(f)

    if float(header['version']) < 0.4:
        raise Exception('Loader is only compatible with .spikes files with version 0.4 or higher')

    data['header'] = header
    numChannels = int(header['num_channels'])
    numSamples = 40 # **NOT CURRENTLY WRITTEN TO HEADER**

    spikes = np.zeros((MAX_NUMBER_OF_SPIKES, numSamples, numChannels))
    timestamps = np.zeros(MAX_NUMBER_OF_SPIKES)
    source = np.zeros(MAX_NUMBER_OF_SPIKES)
    gain = np.zeros((MAX_NUMBER_OF_SPIKES, numChannels))
    thresh = np.zeros((MAX_NUMBER_OF_SPIKES, numChannels))
    sortedId = np.zeros((MAX_NUMBER_OF_SPIKES, numChannels))
    recNum = np.zeros(MAX_NUMBER_OF_SPIKES)

    currentSpike = 0

    while f.tell() < os.fstat(f.fileno()).st_size:

        eventType = np.fromfile(f, np.dtype('<u1'), 1) #always equal to 4, discard
        timestamps[currentSpike] = np.fromfile(f, np.dtype('<i8'), 1)
        software_timestamp = np.fromfile(f, np.dtype('<i8'), 1)
        source[currentSpike] = np.fromfile(f, np.dtype('<u2'), 1)
        numChannels = int(np.fromfile(f, np.dtype('<u2'), 1))
        numSamples = int(np.fromfile(f, np.dtype('<u2'), 1))
        sortedId[currentSpike] = np.fromfile(f, np.dtype('<u2'), 1)
        electrodeId = np.fromfile(f, np.dtype('<u2'), 1)
        channel = np.fromfile(f, np.dtype('<u2'), 1)
        color = np.fromfile(f, np.dtype('<u1'), 3)
        pcProj = np.fromfile(f, np.float32, 2)
        sampleFreq = np.fromfile(f, np.dtype('<u2'), 1)

        waveforms = np.fromfile(f, np.dtype('<u2'), numChannels * numSamples)
        wv = np.reshape(waveforms, (numChannels, numSamples)).T

        gain[currentSpike, :] = np.fromfile(f, np.float32, numChannels)
        thresh[currentSpike, :] = np.fromfile(f, np.dtype('<u2'), numChannels)

        recNum[currentSpike] = np.fromfile(f, np.dtype('<u2'), 1)

        for ch in range(numChannels):
            spikes[currentSpike, :, ch] = ((np.float64(wv[:, ch]) - 32768) /
                                           (gain[currentSpike, ch] / 1000))

        currentSpike += 1

    data['spikes'] = spikes[:currentSpike, :, :]
    data['timestamps'] = timestamps[:currentSpike]
    data['source'] = source[:currentSpike]
    data['gain'] = gain[:currentSpike, :]
    data['thresh'] = thresh[:currentSpike, :]
    data['recordingNumber'] = recNum[:currentSpike]
    data['sortedId'] = sortedId[:currentSpike]

    return data


def read_analog_binary_signals(filehandle, numchan):

    numchan=int(numchan)

    nsamples = os.fstat(filehandle.fileno()).st_size // (numchan*2)
    print('Estimated samples: ', int(nsamples), ' Numchan: ', numchan)

    samples = np.memmap(filehandle, np.dtype('i2'), mode='r',
                        shape=(nsamples, numchan))
    samples = np.transpose(samples)

    return samples, nsamples


def read_analog_continuous_signal(filepath, dtype=float, verbose=False,
                                  start_record=None, stop_record=None,
                                  ignore_last_record=True):
    """Load continuous data from a single channel in the file `filepath`.

    This is intended to be mostly compatible with the previous version.
    The differences are:
    - Ability to specify start and stop records
    - Converts numeric data in the header from string to numeric data types
    - Does not rely on a predefined maximum data size
    - Does not necessarily drop the last record, which is usually incomplete
    - Uses the block length that is specified in the header, instead of
        hardcoding it.
    - Returns timestamps and recordNumbers as int instead of float
    - Tests the record metadata (N and record marker) for internal consistency

    The OpenEphys file format breaks the data stream into "records",
    typically of length 1024 samples. There is only one timestamp per record.

    Args:
        filepath : string, path to file to load
        dtype : float or np.int16
            If float, then the data will be multiplied by bitVolts to convert
            to microvolts. This increases the memory required by 4 times.
        verbose : whether to print debugging messages
        start_record, stop_record : indices that control how much data
            is read and returned. Pythonic indexing is used,
            so `stop_record` is not inclusive. If `start` is None, reading
            begins at the beginning; if `stop` is None, reading continues
            until the end.
        ignore_last_record : The last record in the file is almost always
            incomplete (padded with zeros). By default it is ignored, for
            compatibility with the old version of this function.

    Returns: dict, with following keys
        data : array of samples of data
        header : the header info, as returned by readHeader
        timestamps : the timestamps of each record of data that was read
        recordingNumber : the recording number of each record of data that
            was read. The length is the same as `timestamps`.
    """
    if dtype not in [float, np.int16]:
        raise ValueError("Invalid data type. Must be float or np.int16")

    if verbose:
        print("Loading continuous data from " + filepath)

    """Here is the OpenEphys file format:
    'each record contains one 64-bit timestamp, one 16-bit sample
    count (N), 1 uint16 recordingNumber, N 16-bit samples, and
    one 10-byte record marker (0 1 2 3 4 5 6 7 8 255)'
    Thus each record has size 2*N + 22 bytes.
    """
    # This is what the record marker should look like
    spec_record_marker = np.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 255])

    # Lists for data that's read
    timestamps = []
    recordingNumbers = []
    samples = []
    samples_read = 0
    records_read = 0

    # Open the file
    with open(filepath, 'rb') as f:
        # Read header info, file length, and number of records
        header = readHeader(f)
        record_length_bytes = 2 * header['blockLength'] + 22
        fileLength = os.fstat(f.fileno()).st_size
        n_records = get_number_of_records(filepath)

        # Use this to set start and stop records if not specified
        if start_record is None:
            start_record = 0
        if stop_record is None:
            stop_record = n_records

        # We'll stop reading after this many records are read
        n_records_to_read = stop_record - start_record

        # Seek to the start location, relative to the current position
        # right after the header.
        f.seek(record_length_bytes * start_record, 1)

        # Keep reading till the file is finished
        while f.tell() < fileLength and records_read < n_records_to_read:
            # Skip the last record if requested, which usually contains
            # incomplete data
            if ignore_last_record and f.tell() == (
                fileLength - record_length_bytes):
                break

            # Read the timestamp for this record
            # litte-endian 64-bit signed integer
            timestamps.append(np.fromfile(f, np.dtype('<i8'), 1))

            # Read the number of samples in this record
            # little-endian 16-bit unsigned integer
            N = np.fromfile(f, np.dtype('<u2'), 1).item()
            if N != header['blockLength']:
                raise IOError('Found corrupted record in block')

            # Read and store the recording numbers
            # big-endian 16-bit unsigned integer
            recordingNumbers.append(np.fromfile(f, np.dtype('>u2'), 1))

            # Read the data
            # big-endian 16-bit signed integer
            data = np.fromfile(f, np.dtype('>i2'), N)
            if len(data) != N:
                raise IOError("could not load the right number of samples")

            # Optionally convert dtype
            if dtype == float:
                data = data * header['bitVolts']

            # Store the data
            samples.append(data)

            # Extract and test the record marker
            record_marker = np.fromfile(f, np.dtype('<u1'), 10)
            if np.any(record_marker != spec_record_marker):
                raise IOError("corrupted record marker at record %d" %
                    records_read)

            # Update the count
            samples_read += len(samples)
            records_read += 1

    # Concatenate results, or empty arrays if no data read (which happens
    # if start_sample is after the end of the data stream)
    res = {'header': header}
    if samples_read > 0:
        res['timestamps'] = np.concatenate(timestamps)
        res['data'] = np.concatenate(samples)
        res['recordingNumber'] = np.concatenate(recordingNumbers)
    else:
        res['timestamps'] = np.array([], dtype=np.int)
        res['data'] = np.array([], dtype=dtype)
        res['recordingNumber'] = np.array([], dtype=np.int)

    return res

def find_nearest(array, value, n=1, greater_than=None, not_in_idx=None):

    if not_in_idx is None:
        if greater_than is None:
            if n==1:
                idx = (np.abs(array-value)).argmin()
            else:
                idx = (np.abs(array-value)).xargsort()[:n]
        else:
            if n==1:
                for idx in (np.abs(array-value)).argsort()[:5]:
                    if array[idx] > greater_than:
                        return array[idx], idx
                        break
            else:
                idx = (np.abs(array-value)).argsort()[:n]
        return array[idx], idx
    else:
        if len(array) != 0:
            left_idx = np.ones(len(array), dtype=bool)
            left_idx[not_in_idx] = False
            left_array=array[left_idx]
            if len(left_array) != 0:
                if n==1:
                    idx = (np.abs(left_array-value)).argmin()
                else:
                    idx = (np.abs(left_array-value)).argsort()[:n]
                val = left_array[idx]
                idx = np.where(array==val)
                return array[idx], idx
            else:
                return array, []
        else:
            print('Array length must be greater than 0')
            return None, []

def assign_ttl(soft_ts, ttl):
    ts = np.zeros(len(soft_ts))
    ttl_idx = -1 * np.ones(len(soft_ts), dtype='int64')

    for i, s_ts in enumerate(soft_ts):
        if i == 0:
            ts[i], ttl_idx[i] = ttl[0], 0 #find_nearest(ttl, s_ts)
        else:
            ts[i], ttl_idx[i] = find_nearest(ttl, s_ts, greater_than=ts[i-1])
    return ts, ttl_idx

#
# def sync_tracking_from_events(self, ttl_events, parallel=False, nprocesses=None):
#     """Synchronizes tracking timestamps with ttl signal provided.
#
#     Parameters
#     ----------
#     ttl_events : quantity np.array
#                  array with ttl timestamps
#     parallel: bool
#               parallel processing or not
#     nprocesses: int
#                 number of parallel processes
#
#     Returns
#     -------
#     """
#     positions = []
#     times = []
#     ttl_id = []
#
#     print('Syncing tracking timestamps')
#     if not parallel:
#         for t, (pos, software_ts) in enumerate(zip(self.tracking[0].positions, self.tracking[0].times)):
#             timestamps, ttl_idxs = assign_ttl(software_ts, ttl_events)
#
#             order = np.argsort(ttl_idxs)
#             wrong_ts_idx = np.where(np.diff(order) <= 0)[0]
#             print('Reassigning incorrect ',
#                             len(wrong_ts_idx), ' out of ', len(software_ts))
#             if len(wrong_ts_idx) != 0:
#                 print('Reassigning incorrect ',
#                       len(wrong_ts_idx), ' out of ', len(software_ts))
#                 for i, w_ts in enumerate(wrong_ts_idx):
#                     val, idx = find_nearest(ttl_events, software_ts[w_ts], not_in_idx=np.unique(ttl_idxs))
#                     timestamps[w_ts] = val[0]
#                     ttl_idxs[w_ts] = idx[0].astype(int)
#                 wrong_ts_idx = np.where(np.diff(timestamps) == 0)[0]
#                 print('After final correction: ',
#                       len(wrong_ts_idx), ' out of ', len(software_ts))
#
#             # substitute missing positions with nans
#             missed_ttl = np.ones(len(ttl_events), dtype=bool)
#             missed_ttl[ttl_idxs] = False
#             new_pos = np.zeros((pos.shape[0], len(ttl_events)))
#             new_pos[:, ttl_idxs]  = pos
#             new_pos[:, missed_ttl] = np.nan
#
#             positions.append(new_pos)
#             times.append(ttl_events)
#             ttl_id.append(ttl_idxs)
#     else:
#         from joblib import Parallel, delayed
#         import multiprocessing
#
#         if nprocesses is None:
#             num_cores = multiprocessing.cpu_count()
#         else:
#             num_cores = nprocesses
#         # divide duration in num_cores (+ 1s to be sure to include all samples)
#         chunk_times = np.linspace(0,(self.duration+1*pq.s).rescale(pq.s).magnitude, num_cores+1)
#         chunks  = []
#         for i, start in enumerate(chunk_times[:-1]):
#             chunks.append([start, chunk_times[i+1]])
#
#         for t, (pos, software_ts) in enumerate(zip(self.tracking[0].positions, self.tracking[0].times)):
#             chunks_ts, chunks_ttl= [], []
#             for i, start in enumerate(chunk_times[:-1]):
#                 clip_ts = clip_times(software_ts, [start, chunk_times[i + 1]])
#                 clip_ttl = clip_times(ttl_events, [start, chunk_times[i + 1]])
#                 # print('chunk: ', i, ' clip_ts: ', clip_ts[0], clip_ts[-1], ' clip_ttl: ', clip_ttl[0], clip_ttl[-1])
#                 chunks_ts.append(clip_ts)
#                 chunks_ttl.append(clip_ttl)
#
#             results = Parallel(n_jobs=num_cores)(delayed(assign_ttl)(ts, ttl) for (ts, ttl) in zip(chunks_ts,
#                                                                                                    chunks_ttl))
#
#             timestamps, ttl_idxs = [], []
#             for ch, tt in enumerate(results):
#                 timestamps = np.append(timestamps, tt[0])
#                 chunk_start = np.where(ttl_events>=tt[0][0])[0][0]
#                 ttl_idxs = np.append(ttl_idxs, tt[1]+chunk_start).astype(int)
#
#             ttl_id.append(ttl_idxs)
#
#             # Correct wrong samples that might arise at boundaries
#             wrong_ts_idx = np.where(np.diff(ttl_idxs) <= 0)[0]
#             wrong_ts_idx += 1
#             wrong_pos_idxs = []
#             for w_ts in wrong_ts_idx:
#                 w_, w_id = find_nearest(software_ts, ttl_events[w_ts])
#                 wrong_pos_idxs.append(w_id.astype(int))
#
#             # #TODO remove right indeces
#             if len(wrong_ts_idx) != 0:
#                 print('Removing incorrect ', len(wrong_ts_idx), ' out of ', len(software_ts))
#                 ttl_idxs = np.delete(ttl_idxs, wrong_ts_idx)
#                 wrong_ts_idx = np.where(np.diff(ttl_idxs) <= 0)[0]
#                 print('After final correction: ',
#                       len(wrong_ts_idx), ' out of ', len(software_ts))
#
#             ttl_id.append(ttl_idxs)
#
#             # substitute missing positions with nans
#             missed_ttl = np.ones(len(ttl_events), dtype=bool)
#             missed_ttl[ttl_idxs] = False
#             new_pos = np.zeros((pos.shape[0], len(ttl_events)))
#             correct_pos = np.ones(pos.shape[1], dtype=bool)
#             correct_pos[wrong_pos_idxs] = False
#             new_pos[:, ttl_idxs] = pos[:, correct_pos]
#             new_pos[:, missed_ttl] = np.nan
#
#             positions.append(new_pos)
#             times.append(ttl_events)
#
#         self.tracking[0].positions = positions
#         self.tracking[0].times = times
