#!/bin/bash

INPUTS=input
OUTPUTS=expected/output
ERRORS=expected/error
EXE=myparse

# use: sh tests.sh u v x y
# para controlar os indices dos FORs

minV=0 #min version [0-5]
if [ ! -z $1 ]; then
	minV=$1
fi
maxV=5 #max version [0-5]
if [ ! -z $2 ]; then
	maxV=$2
fi
minI=1 #min input [1-x]
if [ ! -z $3 ]; then
	minI=$3
fi
maxI=5 #max input [1-y]
if [ ! -z $4 ]; then
	maxI=$4
fi

function test {
	for (( i=minV; i <= maxV; i++ )) do
		for (( j=minI; j <= maxI; j++ )) do
			input="input${j}_v1.${i}"
			if [ -s $INPUTS/$input ]; then
				output="output${j}_v1.${i}"
				error="errors${j}_v1.${i}"
				result_output="result_output${j}_v1.${i}"
				result_error="result_error${j}_v1.${i}"

				echo "* Teste do $input"
				../$EXE < $INPUTS/$input > $output 2> $error
				diff -Z $OUTPUTS/$output $output > $result_output
				if [ -s $result_output ]; then
					echo "** Falha no teste de output do $input"
					cat $result_output
				else
					echo "** Sucesso no teste de output do $input"
				fi
				if [ ! -s $ERRORS/$error ]; then
					touch $ERRORS/$error
				fi
				diff -Z $ERRORS/$error $error > $result_error
				if [ -s $result_error ]; then
					echo "** Falha no teste de erro do $input"
					cat $result_error
				else
					echo "** Sucesso no teste de erro do $input"
				fi
				rm $output $error $result_output $result_error
			fi
		done
	done
}

test > Resumo.txt
cat Resumo.txt
