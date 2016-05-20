#!/bin/bash

INPUTS=input
OUTPUTS=expected/output
ERRORS=expected/error
EXE=myparse

function test {
	for i in {0..4}; do
		for j in {0..5}; do
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
