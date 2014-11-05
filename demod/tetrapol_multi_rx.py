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
            channel_bw=12500, server_port=60100, ppm=0,
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
        self.server_port = server_port
        self.ppm = ppm
        self.output = output
        self.auto_tune = auto_tune

        ##################################################
        # Variables
        ##################################################
        self.channels = channels = sample_rate/channel_bw
        self.channel_symb_rate = channel_symb_rate = 16000

        ##################################################
        # Blocks
        ##################################################
        self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(
                ("localhost", server_port), allow_none=True)
        self.xmlrpc_server_0.register_instance(self)
        threading.Thread(target=self.xmlrpc_server_0.serve_forever).start()
        self.valve_ch_0 = grc_blks2.valve(item_size=gr.sizeof_gr_complex*1, open=bool(0))
        self.pfb_channelizer_ccf_0 = pfb.channelizer_ccf(
              1,
              (),
              float(channel_symb_rate)/(sample_rate/channels),
              100)
        self.pfb_channelizer_ccf_0.set_channel_map(([]))
        self.pfb_channelizer_ccf_0.declare_sample_delay(0)

        self.osmosdr_source_0 = osmosdr.source( args="numchan=" + str(1) + " " + "" )
        self.osmosdr_source_0.set_sample_rate(sample_rate)
        self.osmosdr_source_0.set_center_freq(freq, 0)
        self.osmosdr_source_0.set_freq_corr(ppm, 0)
        self.osmosdr_source_0.set_dc_offset_mode(0, 0)
        self.osmosdr_source_0.set_iq_balance_mode(0, 0)
        self.osmosdr_source_0.set_gain_mode(False, 0)
        self.osmosdr_source_0.set_gain(gain, 0)
        self.osmosdr_source_0.set_if_gain(20, 0)
        self.osmosdr_source_0.set_bb_gain(20, 0)
        self.osmosdr_source_0.set_antenna("", 0)
        self.osmosdr_source_0.set_bandwidth(0, 0)

        self.digital_gmsk_demod_0 = digital.gmsk_demod(
            samples_per_symbol=2,
            gain_mu=0.175,
            mu=0.5,
            omega_relative_limit=0.005,
            freq_error=0.0,
            verbose=False,
            log=False,
        )
        self.blocks_file_sink_0 = blocks.file_sink(gr.sizeof_char*1, "channel_0.bits", False)
        self.blocks_file_sink_0.set_unbuffered(True)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.osmosdr_source_0, 0), (self.pfb_channelizer_ccf_0, 0))
        self.connect((self.valve_ch_0, 0), (self.digital_gmsk_demod_0, 0))
        self.connect((self.digital_gmsk_demod_0, 0), (self.blocks_file_sink_0, 0))
        self.connect((self.pfb_channelizer_ccf_0, 0), (self.valve_ch_0, 0))



    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self.osmosdr_source_0.set_center_freq(self.freq, 0)

    def get_gain(self):
        return self.gain

    def set_gain(self, gain):
        self.gain = gain
        self.osmosdr_source_0.set_gain(self.gain, 0)

    def get_sample_rate(self):
        return self.sample_rate

    def set_sample_rate(self, sample_rate):
        self.sample_rate = sample_rate
        self.set_channels(self.sample_rate/self.channel_bw)
        self.osmosdr_source_0.set_sample_rate(self.sample_rate)

    def get_args(self):
        return self.args

    def set_args(self, args):
        self.args = args

    def get_channel_bw(self):
        return self.channel_bw

    def set_channel_bw(self, channel_bw):
        self.channel_bw = channel_bw
        self.set_channels(self.sample_rate/self.channel_bw)

    def get_server_port(self):
        return self.server_port

    def set_server_port(self, server_port):
        self.server_port = server_port

    def get_ppm(self):
        return self.ppm

    def set_ppm(self, ppm):
        self.ppm = ppm
        self.osmosdr_source_0.set_freq_corr(self.ppm, 0)

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
    parser.add_option("-s", "--server-port", dest="server_port", type="intx",
            default=60100, help="Set Server port [default=%default]")
    parser.add_option("-p", "--ppm", dest="ppm", type="eng_float",
            default=eng_notation.num_to_str(0),
            help="Set Frequency correction [default=%default]"
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
        server_port=options.server_port,
        ppm=options.ppm,
        output=options.output,
        auto_tune=options.auto_tune)
    tb.start()
    tb.wait()
