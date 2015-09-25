#!/bin/sh


FBASE=`basename $1 .mml`
FDIR=`dirname $1`
FIN=$FDIR/$FBASE
FOUT=$FBASE

EXEDIR=`dirname $0`


export PCE_INCLUDE="$EXEDIR/../;$EXEDIR/../hescode;$EXEDIR/../include/pce"
$EXEDIR/../bin/hmckc -i $FIN.mml

if [ ! -s effect.h ];
then
	exit
fi

$EXEDIR/../bin/pceas -scd makepce.s

if [ ! -s makepce.iso ];
then
	exit
fi

mv makepce.iso husic.iso
cp $EXEDIR/iso.cue husic.cue 
cp $EXEDIR/iso_wav.cue husic_iw.cue

if [ $# -gt 1 ];
then
	MODE=$2
fi

if [ "x$MODE" != "xDEBUG" ];
then
	rm $FIN.h
	rm define.inc
	rm effect.h
	rm makepce.lst
	rm makepce.sym
fi
