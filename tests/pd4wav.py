#!/usr/bin/python
# -*- coding: utf-8 -*-


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
def wave_make_tone16 ( sampling , freq , size ):
	out = []
	step = 2 * math.pi / ( sampling / freq )
	
	for i in range ( size ):
		out.append ( math.sin ( step * i ) * 30000 )
		
	return out

#
# テストデータの作成
#
def wave_make_test16 ( sampling , freq , size ):
	out = []
	
	for i in range ( size ):
		out.append ( i % 30000 )
		
	return out

#
# pd4をデコードする(補間なし周波数変換付き)
#	
def decode_pd4 ( data , pd4freq , outfreq ):

	out = []
	
	step = float( pd4freq ) / outfreq
	count = 0.0
	
	for value in data:
	
		v = ord(value)

		s16 = ( ( (v >> 4) & 0xf ) - 0x08) * 0x1000
		
		while count < 1:
			out.append ( s16 )
			count = count + step
		
			
		count = count - 1

		
		s16 = ( ( (v     ) & 0xf ) - 0x08) * 0x1000
		
		while count < 1:
			out.append ( s16 )
			count = count + step
			
		count = count - 1

	return out


#
# wavファイルへと書きだす
#
def wave_output_data ( outfile , data , outfreq ):


	f = open ( outfile , "wb" )
	
	wave_write_hdr ( f , outfreq , 1 , 16 , len ( data ) )
	wave_write_data16 ( f , data )
	
	f.close()


#
# pd4をpcmへ変換する
#
def convert_pd4_to_pcm( infile , infreq , outfreq ):

	f = open( infile , "rb" )

	data = f.read()		
	f.close()
	
	return decode_pd4 ( data , infreq , outfreq )
	
	

#
# メイン
#

print "PD4WAV.py version 1.0"

if len(sys.argv) < 2:
	print "Usage pd4wav <infile> [outfile]"
	quit()
	
infile = sys.argv[1]

if len(sys.argv) < 3:
	outfile = infile
	pos = outfile.rfind('.')
	if pos >= 0:
		outfile = outfile[ 0 : pos ] + ".wav"
	else:
		outfile = outfile + ".wav"	
else:
	outfile = sys.argv[2]
	pos = outfile.rfind('.')
	if pos < 0:
		outfile = outfile + ".wav"	
	

print "in  :" + infile
print "out :" + outfile


pcm = convert_pd4_to_pcm ( infile , 6992 , 44100 )
wave_output_data ( outfile , pcm , 44100 )


