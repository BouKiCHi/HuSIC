#!/bin/sh
#
# make_pce.sh
#
# Usage : make_pce.sh song.mml [DEBUG]
#

FBASE=$(basename $1)
FBASE=${FBASE%.*}

FDIR=$(dirname $1)

FIN="$FDIR/$FBASE"
FOUT=$FBASE

EXEDIR=$(dirname $0)
HUSIC_DIR="$EXEDIR/.."

ASMNAME=makepce
OUTEXT=.pce


export PCE_INCLUDE="$HUSIC_DIR;$HUSIC_DIR/hescode;$HUSIC_DIR/include/pce"

FMML="$FIN"

if [ ! -s $FMML ];
then
	FMML="$FIN.mml"
fi

$HUSIC_DIR/bin/hmckc -i $FMML

if [ ! -s effect.h ];
then
	exit
fi

$HUSIC_DIR/bin/pceas -raw ${ASMNAME}.s

if [ ! -s ${ASMNAME}.pce ];
then
	exit
fi

mv ${ASMNAME}.pce ${FOUT}${OUTEXT}

if [ $# -gt 1 ];
then
	MODE=$2
fi

if [ "x$MODE" != "xDEBUG" ];
then
	rm $FIN.h
	rm define.inc
	rm effect.h
	rm ${ASMNAME}.lst
	rm ${ASMNAME}.sym
fi

