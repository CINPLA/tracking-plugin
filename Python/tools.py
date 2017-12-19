from __future__ import division
from __future__ import print_function
from __future__ import with_statement

import quantities as pq
import os
import os.path as op
import numpy as np
import locale
import struct
import platform


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
