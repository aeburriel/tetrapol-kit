#!/usr/bin/env python

# Copyright 2012 Dimitri Stolnikov <horiz0n@gmx.net>

# Usage:
# src$ ./demod/python/osmosdr-tetra_demod_fft.py -o /dev/stdout | ./float_to_bits /dev/stdin /dev/stdout | ./tetra-rx /dev/stdin
#
# Adjust the center frequency (-f) and gain (-g) according to your needs.
# Use left click in Wideband Spectrum window to roughly select a TETRA carrier.
# In Wideband Spectrum you can also tune by 1/4 of the bandwidth by clicking on the rightmost/leftmost spectrum side.
# Use left click in Channel Spectrum windows to fine tune the carrier by clicking on the left or right side of the spectrum.


import sys
import math
from gnuradio import gr, gru, eng_notation, blocks, filter, digital
from gnuradio.eng_option import eng_option
from optparse import OptionParser
import osmosdr

# applies frequency translation, resampling and demodulation

class top_block(gr.top_block):
  def __init__(self):
    gr.top_block.__init__(self)

    options = get_options()

    self.ifreq = options.frequency
    self.rfgain = options.gain

    self.src = osmosdr.source(options.args)
    self.src.set_center_freq(self.ifreq)
    self.src.set_sample_rate(int(options.sample_rate))

    if self.rfgain is None:
        self.src.set_gain_mode(1)
        self.iagc = 1
        self.rfgain = 0
    else:
        self.iagc = 0
        self.src.set_gain_mode(0)
        self.src.set_gain(self.rfgain)

    # may differ from the requested rate
    sample_rate = self.src.get_sample_rate()
    sys.stderr.write("sample rate: %d\n" % (sample_rate))

    bitrate = 8000

    first_decim=125

    out_sample_rate=sample_rate/first_decim
    sys.stderr.write("output sample rate: %d\n" % (out_sample_rate))
   
    sps=out_sample_rate/bitrate
    sys.stderr.write("samples per symbol: %d\n" % (sps))

    self.offset = options.offset
    sys.stderr.write("offset is: %dHz\n" % self.offset)

    taps = filter.firdes.low_pass(1.0, sample_rate, options.low_pass, options.low_pass * 0.2, filter.firdes.WIN_HANN)
    self.tuner = filter.freq_xlating_fir_filter_ccf(first_decim, taps, self.offset, sample_rate)

    #self.demod = digital.gmsk_demod(samples_per_symbol=sps)
    self.demod = digital.gmsk_demod(samples_per_symbol=sps)

    self.output = blocks.file_sink(gr.sizeof_char, options.output_file)

    self.connect((self.src, 0), (self.tuner, 0))
    self.connect((self.tuner, 0), (self.demod, 0))
    self.connect((self.demod, 0), (self.output, 0))



def get_options():
    parser = OptionParser(option_class=eng_option)

    parser.add_option("-a", "--args", type="string", default="",
        help="gr-osmosdr device arguments")
    parser.add_option("-s", "--sample-rate", type="eng_float", default=2000000,
        help="set receiver sample rate (default 2000000)")
    parser.add_option("-f", "--frequency", type="eng_float", default=394e6,
        help="set receiver center frequency")
    parser.add_option("-g", "--gain", type="eng_float", default=None,
        help="set receiver gain")
    
    parser.add_option("-t", "--offset", type="eng_float", default=-267e3,
        help="set offset (default -267000)")

    # demodulator related settings
    parser.add_option("-l", "--log", action="store_true", default=False, help="dump debug .dat files")
    parser.add_option("-L", "--low-pass", type="eng_float", default=12.5e3, help="low pass cut-off", metavar="Hz")
    parser.add_option("-o", "--output-file", type="string", default="out.float", help="specify the bit output file")
    parser.add_option("-v", "--verbose", action="store_true", default=False, help="dump demodulation data")
    (options, args) = parser.parse_args()
    if len(args) != 0:
        parser.print_help()
        raise SystemExit, 1

    return (options)

if __name__ == '__main__':
        tb = top_block()
        tb.run(True)
