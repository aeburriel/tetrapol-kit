#!/bin/env python3

import sys

class Framer:
    def __init__(self, infile):
        self.FRAME_LEN = 160
        self.SYNC = (1, 0, 1, 0, 0, 1, 1)
        self._inf = infile
        self._sync = False
        self._buf = b''

    def sync_errs(self, buf, polarity):
        serr = 0
        for i in range(len(self.SYNC)):
            if (self.SYNC[i] ^ polarity) != buf[i+1]:
                serr +=1
        return serr

    def find_sync(self):
        for i in range(self.FRAME_LEN):
            for polarity in (0, 1):
                serr = self.sync_errs(self._buf[i:i+8], polarity)
                if serr < 1:
                    serr += self.sync_errs(self._buf[self.FRAME_LEN+i:self.FRAME_LEN+i+8], polarity)
                    if serr < 2:
                        self._polarity = polarity
                        self._buf = self._buf[i:]
                        self._sync = True
                        return (8, serr)
        self._buf = self._buf[self.FRAME_LEN:]
        return (8, 8)

    def next(self):
        """Process one frame, returns tuple (bits, errors)"""
        if len(self._buf) < 2*self.FRAME_LEN + 8:
            self._buf += self._inf.read(2*self.FRAME_LEN + 8)
        if len(self._buf) < 2*self.FRAME_LEN + 8:
            return

        if not self._sync:
            return self.find_sync()

        serr1 = self.sync_errs(self._buf, self._polarity)
        serr2 = self.sync_errs(self._buf[self.FRAME_LEN:self.FRAME_LEN+8], self._polarity)
        if (serr1 + serr2) < 2:
            self._buf = self._buf[self.FRAME_LEN:]
            return (8, serr1 + serr2)

        for i in (-2, -1, 1, 2):
            serr2 = self.sync_errs(self._buf[self.FRAME_LEN+i:self.FRAME_LEN+8+i], self._polarity)
            if (serr1 + serr2) < 2:
                self._buf = self._buf[self.FRAME_LEN+i:]
                return (8, serr1 + serr2 + abs(i))

        self._buf = self._buf[self.FRAME_LEN-8:]
        self._sync = False
        return (8, 8)


if __name__ == '__main__':

    framer = Framer(open(sys.argv[1], 'rb'))
    ret = framer.next()
    bc = 0
    errc = 0
    n = 0
    while ret is not None:
        b, err = ret
        bc += b
        errc += err
        n += 1
        if n == 100:
            print("BER: %f%%" % (errc / bc * 100))
            n = 0
            bc = 0
            errc = 0
        ret = framer.next()

