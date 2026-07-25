#!/bin/bash

# Update system
sudo apt-get update

# Install build tools
sudo apt-get install -y build-essential

# Install ZeroMQ
sudo apt-get install -y libzmq3-dev

# Install libssl, libtool, m4, etc.
sudo apt-get install -y autogen automake ca-certificates cmake git libboost-dev libboost-thread-dev libsodium-dev libssl-dev libtool m4 texinfo yasm flex bison maven

PREFIX="/home/$USER/FROST"

if [ ! -d "$PREFIX" ]; then
  mkdir -p "$PREFIX"
fi

# Install GMP
export LDFLAGS="-L$PREFIX/lib/"
export CPPFLAGS="-I$PREFIX/include/"
wget https://ftp.gnu.org/gnu/gmp/gmp-6.3.0.tar.xz
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
cd ../..

# Set library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PREFIX/lib/
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$PREFIX/lib/" >> /home/$USER/.bashrc
source /home/$USER/.bashrc

# Install Python libraries
sudo apt-get install python3-pip
pip3 install numpy scipy matplotlib scikit-learn tqdm pytrends nltk

# Note that if the above setup process is stopped by the error: externally-managed-environment
# Then, do this (replace 3.12 with the Python version in the system): 
# sudo mv /usr/lib/python3.12/EXTERNALLY-MANAGED /usr/lib/python3.12/EXTERNALLY-MANAGED.old
# Followed by running again: 
# pip3 install numpy scipy matplotlib scikit-learn tqdm pytrends nltk

# Install libgsl for building graphm library
sudo apt-get install libgsl-dev

# Install Charm-Crypto for OSSE
wget https://crypto.stanford.edu/pbc/files/pbc-0.5.14.tar.gz
tar -xvf pbc-0.5.14.tar.gz
cd pbc-0.5.14/
./configure CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib" --prefix="$PREFIX"
make -j$(nproc)
make install 
cd ..
export CPPFLAGS="-I$PREFIX/include/" 
export LDFLAGS="-L$PREFIX/lib/"
pip3 install charm-crypto-framework

# Install Java for CLRZ 
wget https://download.java.net/java/GA/jdk25.0.1/2fbf10d8c78e40bd87641c434705079d/8/GPL/openjdk-25.0.1_linux-x64_bin.tar.gz
tar -xvf openjdk-25.0.1_linux-x64_bin.tar.gz
echo "export CLASSPATH=\$CLASSPATH:$PREFIX/others/Clusion/target:$PREFIX/others/Clusion/target/test-classes:$PREFIX/others/Clusion/target/classes:$PREFIX/others/Clusion/target/Clusion-1.0-SNAPSHOT-jar-with-dependencies.jar" >> /home/$USER/.bashrc
echo "export JAVA_HOME=$PREFIX/jdk-25.0.1" >> /home/$USER/.bashrc
echo "export PATH=/home/$USER/FROST/jdk-25.0.1/bin:$PATH" >> /home/$USER/.bashrc
source /home/$USER/.bashrc 
cd others/Clusion
mvn clean install
cd ../..
