#!/usr/bin/env python2.7
##################################################
# Gnuradio Python Flow Graph
# Title: TETRAPOL multichannel reciever
# Generated: Wed Nov  5 08:57:55 2014
##################################################

from gnuradio import analog
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
import math
import osmosdr
import threading
import time

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

# TODO: parametrize
        self.debug = True

        ##################################################
        # Variables
        ##################################################
        self.channels = channels = int(sample_rate/channel_bw)
        self.channel_samp_rate = channel_samp_rate = 16000
        afc_period = 6
        afc_gain = 1
        self.afc_ppm_step = freq / 1e6

        ##################################################
        # Blocks - server and reciever
        ##################################################
        self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(
                ("localhost", listen_port), allow_none=True)
        self.xmlrpc_server_0.register_instance(self)
        threading.Thread(target=self.xmlrpc_server_0.serve_forever).start()

        self.src = osmosdr.source( args="numchan=" + str(1) + " " + "" + self.args )
        self.src.set_sample_rate(sample_rate)
        self.src.set_center_freq(freq, 0)
        self.src.set_freq_corr(ppm, 0)
# TODO: manual gain control
        self.src.set_gain_mode(True, 0)
        #self.src.set_gain(gain, 0)

        bw = (8000 + self.afc_ppm_step)/2
        self.channelizer = pfb.channelizer_ccf(
              channels,
              firdes.low_pass(1, sample_rate, bw, bw*0.15, firdes.WIN_HANN),
              float(channel_samp_rate)/(sample_rate/channels),
              100)

        self.connect((self.src, 0), (self.channelizer, 0))

        self.valves = []
        self.gmsk_demods = []
        self.file_sinks = []
        for ch in range(0, channels):
            valve = grc_blks2.valve(item_size=gr.sizeof_gr_complex*1, open=True)
            gmsk_demod = digital.gmsk_demod(
                    samples_per_symbol=2,
                    gain_mu=0.175,
                    mu=0.5,
                    omega_relative_limit=0.005,
                    freq_error=0.0,
                    verbose=False,
                    log=False,
                    )
            file_sink = blocks.file_sink(gr.sizeof_char, output % ch, False)
            file_sink.set_unbuffered(True)

            self.connect(
                    (self.channelizer, ch),
                    (valve, 0),
                    (gmsk_demod, 0),
                    (file_sink, 0))

            self.valves.append(valve)
            self.gmsk_demods.append(gmsk_demod)
            self.file_sinks.append(file_sink)

        ##################################################
        # Blocks - automatic fine tune
        ##################################################
        self.afc_selector = grc_blks2.selector(
            item_size=gr.sizeof_gr_complex*1,
            num_inputs=channels,
            num_outputs=1,
            input_index=0,
            output_index=0,
        )
        if auto_tune != -1:
            self.afc_selector.set_input_index(auto_tune)

        self.afc_demod = analog.quadrature_demod_cf(channel_samp_rate/(2*math.pi))
        afc_samp = channel_samp_rate * afc_period / 2
        self.afc_avg = blocks.moving_average_ff(afc_samp, 1./afc_samp*afc_gain)
        self.afc_probe = blocks.probe_signal_f()
        def _afc_probe():
            while True:
                time.sleep(afc_period)
                if self.auto_tune == -1:
                    continue
                err = self.afc_probe.level()
                if err > self.afc_ppm_step:
                    d = -1
                elif err < -self.afc_ppm_step:
                    d = 1
                else:
                    continue
                ppm = self.src.get_freq_corr() + d
                if self.debug:
                    print "PPM: % 4d, err: %f" % (ppm, err, )
                self.src.set_freq_corr(ppm)

        self._afc_err_thread = threading.Thread(target=_afc_probe)
        self._afc_err_thread.daemon = True
        self._afc_err_thread.start()

        for ch in range(0, channels):
            self.connect((self.channelizer, ch), (self.afc_selector, ch))
        self.connect((self.afc_selector, 0),
                (self.afc_demod, 0),
                (self.afc_avg, 0),
                (self.afc_probe, 0))

        ##################################################
        # Blocks - for signal strenght identification
        ##################################################
        self.pwr_probes = []
        for ch in range(self.channels):
            pwr_probe = analog.probe_avg_mag_sqrd_c(0, 1./channel_samp_rate)
            self.connect((self.channelizer, ch), (pwr_probe, 0))
            self.pwr_probes.append(pwr_probe)

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

    def get_channels_pwr(self, channels=None):
        if channels is None:
            channels = range(self.channels)
        pwr = []
        for ch in channels:
            p = self.pwr_probes[ch].level()
            if p > 0.:
                p = 10 * math.log10(p)
            else:
                p = None
            pwr.append((p, ch, ))
        return pwr

    def set_output_state(self, channel, open):
        self.valves[channel].set_open(not open)

    def get_channel_bw(self):
        return self.channel_bw

    def get_listen_port(self):
        return self.listen_port

    def get_ppm(self):
        return self.ppm

    def set_ppm(self, ppm):
        self.ppm = ppm
        self.src.set_freq_corr(self.ppm, 0)

    def get_output(self):
        return self.output

    def get_auto_tune(self):
        return self.auto_tune

    def set_auto_tune(self, auto_tune):
        self.auto_tune = auto_tune
        if auto_tune != -1:
            self.afc_selector.set_input_index(auto_tune)

    def get_channels(self):
        return self.channels

    def get_channel_samp_rate(self):
        return self.channel_samp_rate

if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    parser.add_option("-f", "--freq", dest="freq", type="eng_float",
            default=eng_notation.num_to_str(392e6),
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
