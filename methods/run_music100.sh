#!/bin/bash
make clean
make -j

# ------------------------------------------------------------------------------
#  Music100
# ------------------------------------------------------------------------------
name=Music100
n=999900
m=999900
qn=100
d=100
items=../data/bin/${name}/${name}.items 
users=../data/bin/${name}/${name}.users 
query=../data/bin/${name}/${name}.query 
truth=../data/bin/${name}/${name}.truth
folder=../results/${name}/

# # ------------------------------------------------------------------------------
# #  Ground Truth
# # ------------------------------------------------------------------------------
# ./rmips -alg 0 -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items} -us ${users} \
#   -qs ${query} -ts ${truth} -of ${folder}

# # ------------------------------------------------------------------------------
# #  Exhaustive Scan
# # ------------------------------------------------------------------------------
# ./rmips -alg 1 -n ${n} -m ${m} -qn ${qn} -d ${d} -is ${items} -us ${users} \
#   -qs ${query} -ts ${truth} -of ${folder}

# --------------------------------------------------------------------------------
#  SA_ALSH (with Simpfer)
# --------------------------------------------------------------------------------
for K in 128 # 64 192 256
do 
  for b in 0.1 0.3 0.5 0.7 0.9
  do 
    ./rmips -alg 2 -n ${n} -m ${m} -qn ${qn} -d ${d} -K ${K} -b ${b} \
      -is ${items} -us ${users} -qs ${query} -ts ${truth} -of ${folder}
  done
done

# # --------------------------------------------------------------------------------
# #  SA_ALSH (with Simpfer and Cone_Tree)
# # --------------------------------------------------------------------------------
# for K in 64 192 256 # 128 
# do 
#   for b in 0.1 0.3 0.5 0.7 0.9
#   do 
#     for leaf in 20 50 100 200
#     do 
#       ./rmips -alg 3 -n ${n} -m ${m} -qn ${qn} -d ${d} -K ${K} -l ${leaf} -b ${b} \
#         -is ${items} -us ${users} -qs ${query} -ts ${truth} -of ${folder}
#     done
#   done
# done

# # --------------------------------------------------------------------------------
# #  SA_ALSH (with Simpfer and Ball_Tree)
# # --------------------------------------------------------------------------------
# for K in 128 # 64 128 192 256
# do 
#   for b in 0.1 0.3 0.5 0.7 0.9
#   do 
#     for leaf in 20 50 100 200
#     do 
#       ./rmips -alg 4 -n ${n} -m ${m} -qn ${qn} -d ${d} -K ${K} -l ${leaf} -b ${b} \
#         -is ${items} -us ${users} -qs ${query} -ts ${truth} -of ${folder}
#     done
#   done
# done

# # --------------------------------------------------------------------------------
# #  H2_ALSH with Simpfer
# # --------------------------------------------------------------------------------
# for b in 0.1 0.3 0.5 0.7 0.9
# do 
#   ./rmips -alg 7 -n ${n} -m ${m} -qn ${qn} -d ${d} -b ${b} -is ${items} \
#     -us ${users} -qs ${query} -ts ${truth} -of ${folder}
# done

# # --------------------------------------------------------------------------------
# #  H2_ALSH with Simpfer Lower Bounds
# # --------------------------------------------------------------------------------
# for b in 0.1 0.3 0.5 0.7 0.9
# do 
#   ./rmips -alg 6 -n ${n} -m ${m} -qn ${qn} -d ${d} -b ${b} -is ${items} \
#     -us ${users} -qs ${query} -ts ${truth} -of ${folder}
# done

# # --------------------------------------------------------------------------------
# #  H2_ALSH
# # --------------------------------------------------------------------------------
# for b in 0.5 # 0.1 0.3 0.5 0.7 0.9
# do 
#   ./rmips -alg 5 -n ${n} -m ${m} -qn ${qn} -d ${d} -b ${b} -is ${items} \
#     -us ${users} -qs ${query} -ts ${truth} -of ${folder}
# done

