#!/usr/bin/python
# -*- coding: utf-8 -*-

from optparse import OptionParser

import struct
import os
import sys
import math

#
# wavヘッダを書きだす
#
def wave_write_hdr ( of , freq , ch , bits , size ):
	of.seek( 0 )
	of.write( "RIFF" )

	bytes = ch * (bits/8) * size;

	filesize = bytes + 44

	# filesize
	of.write( struct.pack( "<L" , filesize - 8 ) )

	of.write( "WAVE" )
	of.write( "fmt " )

	# chunk size (d)
	of.write( struct.pack( "<L" , 16 ) )

	# format id (w)
	of.write( struct.pack( "<H" , 1  ) )

	# number of channels (w)
	of.write( struct.pack( "<H" , ch  ) )

	# sampling frequency (d)
	of.write( struct.pack( "<L" , freq ) )

	# data speed (d)
	of.write( struct.pack( "<L" , freq * ch * ( bits / 8 ) ) )

	# block size (w)
	of.write( struct.pack( "<H" , ch * ( bits / 8 ) ) )

	# bits per sample (w)
	of.write( struct.pack( "<H" , bits ) )

	of.write( "data" )

	# data size (d)
	of.write( struct.pack( "<L" , bytes ) )

#
# 符号付き16ビットデータをファイルに書きこむ
#
def wave_write_data16 ( of , data ):

	for value in data:
		of.write ( struct.pack ("<h" , value ) )

#
# 8ビットデータをファイルに書きこむ
#
def wave_write_data8 ( of , data ):

	for value in data:
		of.write ( struct.pack ("<B" , value ) )

#
# 正弦波の作成
#
def wave_make_tone16(sampling, sin_freq, size):
	out = []
	step = 2 * math.pi / (sampling / sin_freq)

	for i in range (size):
		out.append ( math.sin ( step * i ) * 30000 )

	return out

#
# テストデータの作成
#
def wave_make_test16(sampling, freq, size):
	out = []

	for i in range (size):
		out.append (i % 30000)

	return out

#
# pd4をデコードする(補間なし周波数変換付き)
#
def decode_pd4(data, pd4freq, outfreq):

	out = []

	step = float(pd4freq) / outfreq
	count = 0.0
	mul = 0xFFF

	for value in data:

		v = ord(value)

		s16 = ( ( (v >> 4) & 0xf ) - 0x08) * mul

		while count < 1:
			out.append ( s16 )
			count = count + step


		count = count - 1


		s16 = ( ( (v     ) & 0xf ) - 0x08) * mul

		while count < 1:
			out.append ( s16 )
			count = count + step

		count = count - 1

	return out

#
# pd5をデコードする(補間なし周波数変換付き)
#
def decode_pd5(data, pd4freq, outfreq):

	out = []

	step = float(pd4freq) / outfreq
	count = 0.0

	nextbuf = 0
	nextbuf_count = 0
	databit = 5
	half = 0x10
	mul = (0x800 - 1)
	pos = 0

	stopflag = False

	while not stopflag:
		buf = 0

		for idx in range(databit):
			# 次のバッファを取得
			if nextbuf_count == 0:
				# データ終端
				if pos >= len(data):
					return out

				#　バッファ取得
				nextbuf_count = 8
				nextbuf = ord(data[pos])
				pos += 1

			# バッファを進める
			buf <<= 1
			if nextbuf & 0x80:
				buf |= 0x01
			nextbuf <<= 1
			nextbuf_count -= 1

		# s16 = 1サンプル取得
		s16 = (buf - half) * mul

		# デバッグ表示
		if options.debug:
			print "%d %d" % (buf, s16)

		# 指定カウントまで埋める
		while count < 1:
			out.append(s16)
			count = count + step

		count = count - 1

	return out





#
# wavファイルテスト出力
#
def wave_output_testdata(outfile, outfreq):

	f = open(outfile, "wb")

	data = wave_make_tone16(outfreq, 440, outfreq)

	wave_write_hdr(f, outfreq, 1, 16, len(data))
	wave_write_data16(f, data)

	f.close()


#
# wavファイルへと書きだす
#
def wave_output_data (outfile, data, outfreq):
	f = open(outfile, "wb")

	wave_write_hdr(f, outfreq, 1, 16, len(data))
	wave_write_data16(f, data)

	f.close()

#
# pcmを読みだす
#
def load_pcm(infile):
	f = open( infile , "rb" )

	data = f.read()
	f.close()

	return data


#
# メイン
#

print "PD4WAV.py version 1.01"


usage = "Usage pd4wav [options...] <infile>"

parser = OptionParser(usage = usage)
parser.add_option("-o", "--output", dest="filename",
                  help="output PCM to FILE", metavar="FILE")

parser.add_option("-5", "--5bit", action="store_true", dest="use_5bit",
                  help="use 5bit mode")

parser.add_option("--debug", action="store_true", dest="debug",
                  help="use debug mode")


parser.add_option("--maketest", action="store_true", dest="maketest",
                  help="Make test wav file")

(options, args) = parser.parse_args()

if options.maketest:
	# テスト出力
	test_wav = "testtone.wav"
	print "output %s..." % test_wav
	wave_output_testdata(test_wav, 44100)


if len(args) < 1:
	parser.print_help()
	quit()

infile = args[0]

if not options.filename:
	outfile = infile
else:
	outfile = options.filename

#
count = 0
cntstr = ""
# 分離
dirpack = os.path.split(outfile)
outdir = dirpack[0]
outname = dirpack[1]

while True:

	pos = outname.find('.')

	if pos >= 0:
		outname = outname[0:pos] + cntstr + ".wav"
	else:
		outname = outname + cntstr + ".wav"

	# 合成
	outfile = os.path.join(outdir, outname)

	if not os.path.exists(outfile):
		break

	# カウント文字列作成

	cntstr = ".%03d" % count
	count += 1

	if count > 999:
		print "Error: no empty to output!"
		quit()

print "in  :" + infile
print "out :" + outfile

# pcm読み出し
data = load_pcm(infile)

# デコード
if options.use_5bit:
	pcm = decode_pd5(data, 6992, 44100)
else:
	pcm = decode_pd4(data, 6992, 44100)

# wav書き出し
wave_output_data(outfile, pcm, 44100)
