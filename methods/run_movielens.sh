#!/bin/bash
make clean
make -j

# ------------------------------------------------------------------------------
#  MovieLens
# ------------------------------------------------------------------------------
name=MovieLens
n=10581
m=69874
qn=100
d=100
items=../data/${name}/${name}.items 
users=../data/${name}/${name}.users 
query=../data/${name}/${name}.query 
truth=../data/${name}/${name}.truth
folder=../results/${name}/

# ------------------------------------------------------------------------------
#  Ground Truth
# ------------------------------------------------------------------------------
./rmips -alg 0 -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items} -us ${users} \
  -qs ${query} -ts ${truth} -of ${folder}

# --------------------------------------------------------------------------------
#  H2_ALSH
# --------------------------------------------------------------------------------
for b in 0.1 0.3 0.5 0.7 0.9
do 
  ./rmips -alg 1 -n ${n} -m ${m} -qn ${qn} -d ${d} -b ${b} -is ${items} \
    -us ${users} -qs ${query} -ts ${truth} -of ${folder}
done

# --------------------------------------------------------------------------------
#  H2_Simpfer
# --------------------------------------------------------------------------------
for b in 0.1 0.3 0.5 0.7 0.9
do 
  ./rmips -alg 2 -n ${n} -m ${m} -qn ${qn} -d ${d} -b ${b} -is ${items} \
    -us ${users} -qs ${query} -ts ${truth} -of ${folder}
done

# --------------------------------------------------------------------------------
#  SA_Simpfer
# --------------------------------------------------------------------------------
for K in 128 # 64 128 192 256
do 
  for b in 0.1 0.3 0.5 0.7 0.9
  do 
    ./rmips -alg 3 -n ${n} -m ${m} -qn ${qn} -d ${d} -K ${K} -b ${b} \
      -is ${items} -us ${users} -qs ${query} -ts ${truth} -of ${folder}
  done
done

# --------------------------------------------------------------------------------
#  SAH
# --------------------------------------------------------------------------------
for K in 128 # 64 128 192 256
do 
  for b in 0.1 0.3 0.5 0.7 0.9
  do 
    for leaf in 20 50 100 200
    do 
      ./rmips -alg 4 -n ${n} -m ${m} -qn ${qn} -d ${d} -K ${K} -l ${leaf} -b ${b} \
        -is ${items} -us ${users} -qs ${query} -ts ${truth} -of ${folder}
    done
  done
done

