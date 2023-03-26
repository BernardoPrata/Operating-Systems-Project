(naming: Author-problem-testNumber)

## Problem 1

D-1-1: Writing exactly the maximum size of file  
D-1-2: Writing more than maximum of file size  
D-1-3: Writing the maximum. Open again, start reading and then write the maximum  
D-1-4: Writing partially, trying to read more than possible  
D-1-5: Writing the maximum and truncating a lot of times (memory should not grow).  
D-1-6: Reading without writing  
D-1-7: Appending only writing  
D-1-8: Write to the edge of direct blocks. Start writing from there up to the end.  
D-1-9: Copy arrays of struct, int and floats  
D-1-10: Try to open more files than allowed

## Problem 2

D-2-1: caso simples de criacao e para um ficheiro (vazio)  
D-2-2: caso em que o path de destino existe mas tem conteudo  
D-2-3: caso em que o path de origem nao tem conteudo  
D-2-4: caso em que o path de origem nao existe

## Problem 3

D-3-1: 3 threads que criam ficheiros diferentes
D-3-2: 3 threads a tentar ler um ficheiro criado na main
D-3-3: 3 threads que criam ficheiros com o mesmo nome (sem truncar) e escrevem o mesmo nele  
D-3-5: 3 threads que abrem um ficheiro (a truncar) e escrevem coisas diferentes  
D-3-6: 4000 threads todas a tentar criar um ficheiro e escrever nele  
D-3-7: ficheiro criado no main e usado em modo append por 3 threads  
D-3-9: 200 threads a tentar ler um ficheiro e altera-lo numa posicao  
D-3-10: 10 threads a tentar fazer coisas miscelaneas com varios ficheiros (not Working)
D-3-11: 3 threads a criar varios ficheiros consecutivamente  
D-3-12: 3 threads a criar ficheiros e preenche-los, enchendo memoria de dados  
D-3-14: Stress test de writes simultaneos (tentar abcedario em imensos ficheiros)
D-3-15: Tentar abrir o número máximo de ficheiros em duas threads
D-3-16: Forcar varios erros em contexto de threads paralelas
B-3-1: threads que escrevem conteudos diferentes nos mesmos blocos mas em ficheiros abertos diferentes
B-3-2: threads que leem conteudo em ficheiros abertos diferentes
B-3-3: threads que leem e escrevem em concurrencia nos mesmos blocos, mas em ficheiros abertos diferentes
B-3-4: threads que leem e escrevem em concurrencia nos mesmos blocos no mesmo ficheiro aberto
B-3-5: threads que escrevem e fecham um ficheiro aberto