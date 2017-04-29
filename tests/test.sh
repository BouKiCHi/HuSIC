#!/bin/sh

# test script
EXEDIR=`dirname $0`

$EXEDIR/../bin/xpcm tone_orig.wav -o tone.pd4

$EXEDIR/../songs/make_hes.sh test01.mml
$EXEDIR/../songs/make_hes.sh test02.mml
$EXEDIR/../songs/make_hes.sh test03.mml
$EXEDIR/../songs/make_hes.sh test04.mml
$EXEDIR/../songs/make_hes.sh test05.mml
