#!/bin/bash

function test {
	for i in {0..1}; do
		echo "* Teste da versao 1."$i
		../myparse < input/v1.${i} > output_v1.${i} 2> erro_v1.${i}
		diff -Z output_v1.${i} expected/output/v1.${i} > result_output_v1.${i}
		if [ -s result_output_v1.${i} ]; then
			echo "** Falha no teste de output da versao 1."$i
			cat result_output_v1.${i}
		else
			echo "** Sucesso no teste de output da versao 1."$i
		fi
		diff -Z erro_v1.${i} expected/erro/v1.${i} > result_erro_v1.${i}
		if [ -s result_erro_v1.${i} ]; then
			echo "** Falha no teste de erro da versao 1."$i
			cat result_erro_v1.${i}
		else
			echo "** Sucesso no teste de erro da versao 1."$i
		fi
		rm output_v1.${i} erro_v1.${i} result_erro_v1.${i} result_output_v1.${i}
	done
}

test > Resumo.txt
cat Resumo.txt
