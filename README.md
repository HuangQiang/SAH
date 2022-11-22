# SAH: Shifting-aware Asymmetric Hashing for Reverse *k*-Maximum Inner Product Search

Welcome to the **SAH** GitHub!

## Introduction

**SAH** is a repository for the problem of **Reverse *k*-Maximum Inner Product Search (R*k*MIPS)**. Given a query (item) vector, a set of item vectors, and a set of user vectors, the problem of R*k*MIPS aims to find a set of user vectors whose inner products with the query vector are one of the *k* largest among the query and item vectors. This problem is essential for market analysis from a *reverse* perspective, i.e., the perspective of service providers instead of users. For example, when an e-commerce service promotes a discounted product or launches a new product, a vital issue for designing an effective advertising campaign is identifying the customers who may want to buy this product.

This repository provides the implementations and experiments of our work entitled [SAH: Shifting-aware Asymmetric Hashing for Reverse *k*-Maximum Inner Product Search](~) in [AAAI 2023](~). We implement **SAH** for solving R*k*MIPS in high-dimensional Euclidean spaces and include the only known R*k*MIPS method [**Simpfer**](https://github.com/amgt-d1/Simpfer) and a state-of-the-art *k*-Maximum Inner Product Search (*k*MIPS) method [**H2-ALSH**](https://github.com/HuangQiang/H2_ALSH) as baselines for evaluation. To make a comprehensive yet systematic comparison, we also incorporate the R*k*MIPS optimizations of Simpfer into H2-ALSH and SA-ALSH as two new baselines named **H2-Simpfer** and **SA-Simpfer**, respectively.

## Datasets

### Dataset Details

We use five real-world recommendation datasets in our experiments for perfroming R*k*MIPS, i.e., **Amazon-Auto**, **Amazon-CDs**, **MovieLens**, **Music100**, and **Netflix**. For the datasets Amazon-Auto, Amazon-CDs, MovieLens, and Netflix, we first retrieve their sparse user-item rating matrix $R$, where $R(i,j)$ is the rating of user $i$ for item $j$. Then, we set up the latent dimensionality $d=100$ and apply the [Non-Negative Matrix Factorization (NMF)](https://pytorch-nmf.readthedocs.io/en/stable/) on $R$ to obtain their latent user and item matrices. For the dataset [Music100](https://github.com/stanis-morozov/ip-nsw), as the authors only provided a single dense matrix (not rating matrix), we use it as both user and item matrices. Finally, for each dataset, we randomly select 100 item vectors from the item matrix and use them as the query set. The details of datasets are depicted in the following table.

| Datasets    | # Items     | # Users   | # Queries | # Dim | Itemset Size | Userset Size |
| ----------- | ----------- | --------- | --------- | ----- | ------------ | ------------ |
| Amazon-Auto | 925,387     | 3,873,247 | 100       | 100   | 370.1 MB     | 1.5 GB       |
| Amazon-CDs  | 64,443      | 75,258    | 100       | 100   | 25.7 MB      | 30.1 MB      |
| MovieLens   | 10,681      | 71,567    | 100       | 100   | 4.2 MB       | 27.9 MB      |
| Music100    | 1,000,000   | 1,000,000 | 100       | 100   | 400.0 MB     | 400.0 MB     |
| Netflix     | 17,770      | 480,189   | 100       | 100   | 7.1 MB       | 192.1 MB     |

Moreover, we conduct a comparison of **H2-ALSH** and **SA-ALSH** for *k*MIPS. In addition to the five recommendation datasets used for perfroming R*k*MIPS, we we also choose five commonly used datasets to benchmark *k*MIPS algorithms, namely **DEEP**, **Gist**, **GloVe**, **ImageNet**, and **Msong**. The number of their vectors and the dimensionality $(n, d)$ are $(1,000,000, 256)$, $(982,694, 960)$, $(1,183,514, 100)$, $(2,340,373, 150)$, and $(992,272, 420)$, respectively. For the five recommendation datasets used in previous experiments, we use item vectors as the dataset and choose 100 user vectors uniformly at random as queries; for the five new datasets, we randomly draw 100 data vectors and remove them from the dataset as queries. All the datasets and queries can be download [here](https://drive.google.com/drive/folders/16tlJl4IE0Tcd4Dz9PXkhLt14MFhG67Zx?usp=sharing).

### Data Format

To reduce the I/O cost, all the datasets and queries are stored in a binary format. For a dataset (or query set) with *n* vectors in *d* dimensions, the binary file comparises with a `float` array of `n*d` length. You should know the cardinality *n* and data dimension *d* in advance.

## Requirements

- Ubuntu 18.04 (or higher version)
- g++ 8.3.1 (or higher version) and OpenMP.
- Python 3.7 (or higher version)

## Compilation

The source codes are implemented by `C++`, which requires `g++-8` with `c++17` for compilation. Thus, please check whether the `g++-8` is installed. If not, we recommend a way to install `g++-8` in `Ubuntu 18.04` (or higher versions) as follows.

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install g++-8
sudo apt-get install gcc-8 (optional)
```

Once the `g++-8` has been installed, users can use the following command to compile the `C++` source codes in parallel:

```bash
cd methods/
make -j
```

## Experiments

We have provided bash scripts to run all experiments. Please ensure you have downloaded the datasets and completed the compilation.

### Expt-1: R*k*MIPS Experiments

To reproduce the R*k*MIPS experiments of **H2-ALSH**, **H2-Simpfer**, **H2-Cone**, **SA-Simpfer**, and **SAH**, please run the following commands:

```bash
cd methods/
bash run.sh
```

Moreover, one can run commands below to reproduce the R*k*MIPS experiments of **Simpfer**:

```bash
cd simpfer/
bash run.sh
```

### Expt-2: *k*MIPS Experiments

To reproduce the *k*MIPS experiments of **H2-ALSH** and **SA-ALSH**, please run the following commands:

```bash
cd kmips/
bash run.sh
```

### Expt-3: The Curve of Query Time vs. F1-Score

You can plot the curves of query time vs. f1-score for **H2-ALSH**, **H2-Simpfer**, and **SAH** by running the following commands:

```bash
cd curves/
bash run.sh
```

## Visualization

Finally, we have privoded `python` scripts for visualization. These scripts require `python 3.7` (or higher versions) with **numpy, scipy, and matplotlib**. If you do not have such an environment, you might need to use `anaconda` to create a new virtual one and apply `pip` to install those packages. After you have completed all experiments, you can plot all figures appeared in our AAAI 2023 paper and the supplementary material with the python scripts in `visualization/`.

## Reference

Thank you so much for being patient to read the user manual. We will appreciate using the following BibTeX to cite this work when you use SAH in your paper.

```tex
@inproceedings{huang2023a,
  title={SAH: Shifting-aware Asymmetric Hashing for Reverse k-Maximum Inner Product Search},
  author={Huang, Qiang and Wang, Yanhao and Tung, Anthony K. H.},
  booktitle={The Thirty-Seventh AAAI Conference on Artificial Intelligence (AAAI)},
  year={2023}
}
```

It is welcome to contact me (huangq@comp.nus.edu.sg) if you meet any issue. Thank you.
