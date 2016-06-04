'''
    matrix
    ~~~~~~
'''

import datetime
import os, os.path
import random
import struct
import time


def read_channel (fp):
    ''' Read bytes from `fp` until a channel byte is read, and return
        the channel number.

    :param fp: An open filepointer.

    :returns: The channel number.
    '''
    mask = 0b10000000
    channel = None

    while channel is None:
        byte = struct.unpack('B', fp.read(1))[0]
        if (byte & mask) == mask:
            channel = (byte >> 1) & 0b00111111

    assert 0 <= channel <= 63, 'Channel out of bounds: {}'.format(channel)

    return channel


def read_frame (fp):
    ''' Read a single frame of sensor data from `fp`, non-channel
        bytes are consumed by :func:`read_channel` until a channel
        byte is read, at which point two additional data bytes are
        read.

    :param s: An open filepointer.

    :returns: A `(channel, val)` tuple is returned.
    '''
    channel = read_channel(fp)
    val = 0

    msb, lsb = struct.unpack('BB', fp.read(2))

    val += ((msb & 0b00011110) >> 1) << 6
    val += (lsb >> 1) & 0b00111111

    assert 0 <= val <= 1023, 'Value out of bounds: {}'.format(val)

    return (channel, val)


class LogWriter (object):

    def __init__ (self, basedir='./data', filename='dump_%Y-%m-%d_%H-%M-%S.csv'):
        self.path = os.path.join(basedir, time.strftime(filename))

        if not os.path.isdir(basedir):
            os.mkdir(basedir)

        self.fmt = '{}\n'.format(','.join(['{}'] * 65))
        self.fp = None

        print('Logging data to {}'.format(self.path))


    def write (self, data):
        assert len(data) == 64, '{} != 64'.format(len(data))

        if self.fp is None:
            self.fp = open(self.path, 'w')

        self.fp.write(self.fmt.format(datetime.datetime.now().isoformat(),
            *data))


    def close (self):
        if self.fp:
            self.fp.close()


    def __enter__ (self):
        return self

    def __exit__ (self, *args):
        self.close()


class FauxSensor (object):
    ''' Emulate a filepointer that supplies sensor data, 
        instances act like a filepointer. '''

    def __init__ (self):
        self.channel = 0
        self.buf = []


    def close (self):
        pass


    def next_channel (self):
        self.channel = self.channel + 1 if self.channel < 63 else 0
        return self.channel


    def read (self, num):
        while len(self.buf) < num:
            self.buf.extend(self.read_sensor_value())

        data = self.buf[:num]
        self.buf = self.buf[num:]

        time.sleep(0.0001)

        return ''.join(data)


    def read_sensor_value (self):
        val = random.randint(0, 1023)
        channel = self.next_channel()

        msb = val >> 6
        lsb = val & 0b00111111

        return struct.pack('BBB', (channel << 1) | 0b10000000,
            msb << 1, (lsb << 1) | 0b00000001)
