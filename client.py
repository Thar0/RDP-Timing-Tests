#!/usr/bin/env python3
#
#   Flashcart USB Client
#

import argparse, math, signal, struct, sys, time
import serial, serial.tools.list_ports

def pad_buffer(buf, boundary=512):
    if len(buf) % boundary != 0:
        diff = math.ceil(len(buf) / boundary) * boundary - len(buf)
        buf += b"\0" * diff
    return buf

class ExtDevice:
    """
    External device facilitating communication with the N64.

    Each device must provide an implementation of all of the following.
    """

    def __init__(self):
        raise NotImplementedError()

    def close(self):
        raise NotImplementedError()

    def reset(self):
        raise NotImplementedError()

    def wait(self):
        raise NotImplementedError()

    def read(self, n):
        raise NotImplementedError()

    def write(self, data):
        raise NotImplementedError()

    def boot_rom(self, filename):
        raise NotImplementedError()

    def whoami(self):
        raise NotImplementedError()

    @staticmethod
    def try_detect():
        dev = ED64Device.try_detect()
        if dev is not None:
            return dev
        # TODO support more flashcarts
        return None

ED64_CMD_TEST       = 't'
ED64_CMD_ROM_START  = 's'
ED64_CMD_ROM_WRITE  = 'W'
ED64_CMD_ROM_READ   = 'R'
ED64_CMD_ROM_FILL   = 'c'
ED64_CMD_RAM_READ   = 'r'
ED64_CMD_FPGA_WRITE = 'f'

def ed64_make_cmd(cmd, address=0, length=0, arg=0):
    buf = bytearray()
    buf.extend([ord('c'), ord('m'), ord('d'), ord(cmd)])
    buf.extend(struct.pack(">I", address))
    buf.extend(struct.pack(">I", length // 512))
    buf.extend(struct.pack(">I", arg))
    return buf

class ED64Device(ExtDevice):

    def __init__(self, ser : serial.Serial):
        self.ser = ser
        self.reset()

    def close(self):
        self.ser.close()

    def reset(self):
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

    def wait(self):
        data_len = self.ser.inWaiting()
        while not (data_len > 0):
            data_len = self.ser.inWaiting()
        return data_len

    def read(self, n):
        return self.ser.read(n)

    def write(self, data):
        self.ser.write(data)

    def close(self):
        self.ser.close()

    def boot_rom(self, filename):
        rom_data = None
        with open(filename, "rb") as infile:
            rom_data = bytes(infile.read())

        rom_data = pad_buffer(rom_data)
        size = len(rom_data)

        print(f"Uploading ROM.. (0x{size:X} bytes)")

        if size < 0x101000:
            self.write(ed64_make_cmd(ED64_CMD_ROM_FILL, 0x10000000, 0x101000))
        self.write(ed64_make_cmd(ED64_CMD_ROM_WRITE, 0x10000000, size))

        rom_mem = memoryview(rom_data)

        t = time.time()
        offset = 0
        rem = size
        while rem > 0:
            blksize = min(1024 * 1024, rem)
            self.write(rom_mem[offset:offset+blksize])
            offset += blksize
            rem -= blksize
            print(f"Uploaded {offset / size * 100:.2f}%")
        print(f"Upload took {time.time() - t} seconds")

        print("Booting ROM..")
        self.write(ed64_make_cmd(ED64_CMD_ROM_START, arg=1))

        # TODO add the filename in this packet
        self.write(bytearray(256))

    def whoami(self):
        return f"Everdrive 64 on {self.ser.port}"

    @staticmethod
    def try_detect():
        found = None

        for port in serial.tools.list_ports.comports():
            try:
                with serial.Serial(port.device, 9600, timeout=1, writeTimeout=1, rtscts=1) as ser:
                    ser.write(ed64_make_cmd(ED64_CMD_TEST))
                    dat = ser.read(512)
                    if dat[0:4].decode('ascii') == 'cmdr':
                        found = port
                        break
            except OSError as e:
                pass

        if found is None:
            return None

        return ED64Device(serial.Serial(found.device, 9600, timeout=5, writeTimeout=2, rtscts=True))

class PacketStream:

    def __init__(self, dev : ExtDevice):
        self.dev : ExtDevice = dev
        self.data = b''

    def more_data(self):
        # we clobber the existing data so there better be none of it
        assert len(self.data) == 0
        # wait for data, TODO we might be able to switch to just reading n and waiting in ser.read()
        data_len = self.dev.wait()
        # read all waiting data
        self.data = self.dev.read(data_len)

    def peekc(self):
        # read 1 byte and don't advance

        # get data if there is none
        if len(self.data) == 0:
            self.more_data()
        # return the first byte without advancing
        return self.data[0]

    def getc(self):
        # read 1 byte and advance
        u8 = self.peekc()
        self.data = self.data[1:]
        return u8

    def get_n(self, n):
        # read n bytes and advance

        # get data if there is none
        if len(self.data) == 0:
            self.more_data()

        # try to read n bytes of data and advance
        data_out = self.data[:n]
        self.data = self.data[n:]
        # check how much data we actually got
        got_len = len(data_out)
        if got_len != n:
            # didn't get enough, read remaining
            data_out += self.get_n(n - got_len)

        return data_out

    def get_pkt(self):
        # if we get '\0' it means console was powered off
        if self.peekc() == '\0':
            return None

        # check dma@
        dma = self.get_n(4)
        assert dma == b'DMA@' , f"Not DMA@? {dma + self.data}"

        # read packet header and body
        pkt_type = self.getc()
        pkt_length = (self.getc() << 16) | (self.getc() << 8) | self.getc()
        pkt_data = self.get_n(pkt_length)

        # check cmph
        cmph = self.get_n(4)
        assert cmph == b'CMPH' , f"Not CMPH? {cmph + self.data}"

        # discard up to 2-byte boundary (old protocol was 16-byte)
        pos = 4 + 1 + 3 + pkt_length + 4
        aligned = (pos + 1) & ~1
        _ = self.get_n(aligned - pos)

        # return type + data
        return pkt_type, pkt_data

def listen_spinloop(dev, tcp_port=411):
    def handle_sigint(signum, frame):
        print("exit")
        dev.close()
        sys.exit(0)

    signal.signal(signal.SIGINT, handle_sigint)

    # Attach packet stream
    stream = PacketStream(dev)

    while True:
        # Wait for a message
        recv = stream.get_pkt()
        if recv is None:
            # Assume powered off, exit
            break

        # Handle packet

        pkt_type, data = recv

        if pkt_type == 0x01: # TEXT
            txt = data.decode("ASCII")
            print(txt, end='')
            if "!!DONE!!" in txt:
                # Quit when done signal arrives
                return
        elif pkt_type == 0x05: # HEARTBEAT
            pass
        else:
            # TODO others
            print(f"\ngotpkt type={pkt_type} data=[{data}]")

def main(rom_path, keep_alive):
    # Find flashcart device
    dev = ExtDevice.try_detect()
    if dev is None:
        print("No Device Found")
        sys.exit(1)
    print(dev.whoami())

    # Boot ROM
    dev.boot_rom(rom_path)

    # Await messages if keep alive
    if keep_alive:
        listen_spinloop(dev)

    # Done
    dev.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Flashcart USB communication")
    parser.add_argument("rom", help=".z64 rom file to run")
    parser.add_argument("--keep-alive", help="keep communication open during runtime", action="store_true")
    args = parser.parse_args()
    main(args.rom, args.keep_alive)
