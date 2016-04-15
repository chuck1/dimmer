#!/usr/bin/env python
import argparse
import smbus
import time
import struct

address = 0x08

codes = {
	"set": {
		"A": 0,
		"B": 1},
	"get": {
		"A": 2,
		"B": 3}}

def read():
	print bus.read_byte(address)
	print bus.read_byte(address)
	print bus.read_byte(address)
	print bus.read_byte(address)

def set(dimmer, value):
	#c = struct.pack("H", value)
	bus.write_word_data(address, codes["set"][dimmer], value)

def get(dimmer):
	#bus.write_byte(address, codes["get"][dimmer])

	res = bus.read_word_data(address, codes["get"][dimmer])

	print "res", repr(res)

	return res	

#####

parser = argparse.ArgumentParser()
parser.add_argument("code")
parser.add_argument("dimmer")
parser.add_argument("--value")
args = parser.parse_args()

bus = smbus.SMBus(1)

if args.code == "set":
	set(args.dimmer, int(args.value))
elif args.code == "get":
	v = get(args.dimmer)
	print v










