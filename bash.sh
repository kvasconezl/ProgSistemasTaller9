#!/bin/bash
show_usage(){
	echo "[USAGE] argumentos IP PUERTO"
	exit 1
}

#IP="$1"
#PUERTO="$2"
cont=0
if [ $# -ne 2 ]; then
	show_usage
fi

for i in {1..10000}; do
if [ $cont -eq 50 ]; then
	cont=0
fi
sleep 0.005
./cliente $1 $2 archivos_generados/archivo$cont archivo$cont
cont=$((cont+ 1))

done 
