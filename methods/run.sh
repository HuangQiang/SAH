#!/bin/bash
make clean
make -j

# ./run_optdigits.sh 
# ./run_movielens.sh 
# ./run_netflix.sh 
# ./run_music100.sh 

./run_cd_vinyl.sh 
./run_movies_tv.sh 
./run_books.sh 

./run_automotive.sh 
./run_tools.sh 
