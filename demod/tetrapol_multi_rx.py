#!/usr/bin/env python2.7
##################################################
# Gnuradio Python Flow Graph
# Title: TETRAPOL multichannel reciever
# Generated: Wed Nov  5 08:57:55 2014
##################################################

from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.filter import pfb
from grc_gnuradio import blks2 as grc_blks2
from optparse import OptionParser
import SimpleXMLRPCServer
import osmosdr
import threading

class tetrapol_multi_rx(gr.top_block):

    def __init__(self, freq=394e6, gain=0, sample_rate=2400000, args="",
            channel_bw=12500, listen_port=60100, ppm=0,
            output="channel%d.bits", auto_tune=-1):
        gr.top_block.__init__(self, "TETRAPOL multichannel reciever")

        ##################################################
        # Parameters
        ##################################################
        self.freq = freq
        self.gain = gain
        self.sample_rate = sample_rate
        self.args = args
        self.channel_bw = channel_bw
        self.listen_port = listen_port
        self.ppm = ppm
        self.output = output
        self.auto_tune = auto_tune

        ##################################################
        # Variables
        ##################################################
        self.channels = channels = int(sample_rate/channel_bw)
        self.channel_symb_rate = channel_symb_rate = 16000

        ##################################################
        # Blocks - server and reciever
        ##################################################
        self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(
                ("localhost", listen_port), allow_none=True)
        self.xmlrpc_server_0.register_instance(self)
        threading.Thread(target=self.xmlrpc_server_0.serve_forever).start()

        self.src = osmosdr.source( args="numchan=" + str(1) + " " + "" )
        self.src.set_sample_rate(sample_rate)
        self.src.set_center_freq(freq, 0)
        self.src.set_freq_corr(ppm, 0)
# TODO: automatic gain control
        self.src.set_gain_mode(False, 0)
        self.src.set_gain(gain, 0)

        self.channelizer = pfb.channelizer_ccf(
              channels,
              (firdes.root_raised_cosine(1, sample_rate, channel_symb_rate, 0.5, 1024)),
              float(channel_symb_rate)/(sample_rate/channels),
              100)

        self.connect((self.src, 0), (self.channelizer, 0))

        self.valves = []
        self.gmsk_demods = []
        self.file_sinks = []
        for ch in range(0, channels):
            valve = grc_blks2.valve(item_size=gr.sizeof_gr_complex*1, open=bool(0))
            gmsk_demod = digital.gmsk_demod(
                    samples_per_symbol=2,
                    gain_mu=0.175,
                    mu=0.5,
                    omega_relative_limit=0.005,
                    freq_error=0.0,
                    verbose=False,
                    log=False,
                    )
            file_sinks = blocks.file_sink(gr.sizeof_char, output % ch, False)
            file_sinks.set_unbuffered(True)

            self.connect(
                    (self.channelizer, ch),
                    (valve, 0),
                    (gmsk_demod, 0),
                    (file_sinks, 0))

        ##################################################
        # Blocks - automatic fine tune
        ##################################################
# TODO
        if auto_tune >= 0:
            self.afc_selector = grc_blks2.selector(
                item_size=gr.sizeof_gr_complex*1,
                num_inputs=channels,
                num_outputs=1,
                input_index=auto_tune,
                output_index=0,
            )
            for ch in channels:
                self.connect((self.channelizer, ch), (self.afc_selector))

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self.src.set_center_freq(self.freq, 0)

    def get_gain(self):
        return self.gain

    def set_gain(self, gain):
        self.gain = gain
        self.src.set_gain(self.gain, 0)

    def get_sample_rate(self):
        return self.sample_rate

    def set_sample_rate(self, sample_rate):
        self.sample_rate = sample_rate
        self.set_channels(self.sample_rate/self.channel_bw)
        self.src.set_sample_rate(self.sample_rate)

    def get_args(self):
        return self.args

    def set_args(self, args):
        self.args = args

    def get_channel_bw(self):
        return self.channel_bw

    def set_channel_bw(self, channel_bw):
        self.channel_bw = channel_bw
        self.set_channels(self.sample_rate/self.channel_bw)

    def get_listen_port(self):
        return self.listen_port

    def set_listen_port(self, listen_port):
        self.listen_port = listen_port

    def get_ppm(self):
        return self.ppm

    def set_ppm(self, ppm):
        self.ppm = ppm
        self.src.set_freq_corr(self.ppm, 0)

    def get_output(self):
        return self.output

    def set_output(self, output):
        self.output = output

    def get_auto_tune(self):
        return self.auto_tune

    def set_auto_tune(self, auto_tune):
        self.auto_tune = auto_tune

    def get_channels(self):
        return self.channels

    def set_channels(self, channels):
        self.channels = channels

    def get_channel_symb_rate(self):
        return self.channel_symb_rate

    def set_channel_symb_rate(self, channel_symb_rate):
        self.channel_symb_rate = channel_symb_rate

if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    parser.add_option("-f", "--freq", dest="freq", type="eng_float",
            default=eng_notation.num_to_str(394e6),
            help="Set Frequency [default=%default]")
    parser.add_option("-g", "--gain", dest="gain", type="eng_float",
            default=eng_notation.num_to_str(0),
            help="Set Gain [default=%default]")
    parser.add_option("-s", "--sample-rate", dest="sample_rate", type="intx",
            default=2400000, help="Set Sample rate [default=%default]")
    parser.add_option("-a", "--args", dest="args", type="string", default="",
            help="Set osmo-sdr arguments [default=%default]")
    parser.add_option("-b", "--channel-bw", dest="channel_bw", type="intx",
            default=12500, help="Set Channel band width [default=%default]")
    parser.add_option("-l", "--listen-port", dest="listen_port", type="intx",
            default=60100, help="Set Server port [default=%default]")
    parser.add_option("-p", "--ppm", dest="ppm", type="eng_float",
            default=eng_notation.num_to_str(0),
            help="Set Frequency correction [default=%default]")
    parser.add_option("-o", "--output", dest="output", type="string",
            default="channel%d.bits", help="Set Output [default=%default]")
    parser.add_option("-t", "--auto-tune", dest="auto_tune", type="intx",
            default=-1, help="Set Allow automatic fine tunning [default=%default]")
    (options, args) = parser.parse_args()
    tb = tetrapol_multi_rx(
        freq=options.freq,
        gain=options.gain,
        sample_rate=options.sample_rate,
        args=options.args,
        channel_bw=options.channel_bw,
        listen_port=options.listen_port,
        ppm=options.ppm,
        output=options.output,
        auto_tune=options.auto_tune)
    tb.start()
    tb.wait()
