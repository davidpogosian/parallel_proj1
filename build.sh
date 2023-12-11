#!/bin/bash

mkdir -p obj

for file in *.c; do
    filename=$(basename $file .c)
    mpicc -c "$file" -o "obj/$filename.o"
done

mpicc -o main obj/*.o

mpirun -np 4 main