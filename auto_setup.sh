#!/bin/bash

# Update system
sudo apt-get update

# Install build tools
sudo apt-get install -y build-essential

# Install ZeroMQ
sudo apt-get install -y libzmq3-dev

# Install libssl, libtool, m4, etc.
sudo apt-get install -y autogen automake ca-certificates cmake git libboost-dev libboost-thread-dev libsodium-dev libssl-dev libtool m4 texinfo yasm flex bison

PREFIX="/home/$USER/FROST"

if [ ! -d "$PREFIX" ]; then
  mkdir -p "$PREFIX"
fi

# Install GMP
export LDFLAGS="-L$PREFIX/lib/"
export CPPFLAGS="-I$PREFIX/include/"
wget https://gmplib.org/download/gmp/gmp-6.3.0.tar.xz
tar -xvf gmp-6.3.0.tar.xz
cd gmp-6.3.0/
./configure --prefix="$PREFIX"
make -j$(nproc)
make install
cd ..

# Install NTL
wget https://libntl.org/ntl-11.6.0.tar.gz
tar -xvf ntl-11.6.0.tar.gz
cd ntl-11.6.0/src/
./configure DEF_PREFIX=$PREFIX GMP_PREFIX=$PREFIX
make -j$(nproc)
make install
cd ..

# Set library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PREFIX/lib/
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$PREFIX/lib" >> /home/$USER/.bashrc

# Install Python libraries
sudo apt-get install python3-pip

# Note that if meet the error: externally-managed-environment
# Then, do this (replace 3.12 with the Python version in the system): 
# sudo mv /usr/lib/python3.12/EXTERNALLY-MANAGED /usr/lib/python3.12/EXTERNALLY-MANAGED.old
pip3 install numpy scipy matplotlib scikit-learn tqdm pytrends nltk

# Install libgsl for building graphm library
sudo apt-get install libgsl-dev
