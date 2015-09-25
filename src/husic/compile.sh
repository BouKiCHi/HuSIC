
export TOP_DIR="../.."
export PCE_INCLUDE="$TOP_DIR/include/pce"

$TOP_DIR/bin/huc main.c

cp main.s $TOP_DIR/hescode/


if [ $# -gt 0 ];
then
	MODE=$1
fi

if [ "x$MODE" != "xDEBUG" ];
then
	rm main.s
fi
