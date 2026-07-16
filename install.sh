#!/bin/bash

set -e # Ends on errors
repo=https://github.com/zehnderdev/BombaGit

dir=BombaGit

# need Makefile(-f)  and src Folder(-d)to work 
# clone if not there

 
if [ -f Makefile ] && [ -d src ]
  then
    echo "Using existing Repo"
  else
    echo "Cloning repository"
    git clone $repo
    cd $dir || exit 1;	
fi

echo "Checking dependencies"
# Check libssl-dev package and gcc for safety
# command to check if user has distro
if command -v apt &> /dev/null  # ubuntu
  then
    sudo apt install -y gcc libssl-dev
  elif command -v dnf &> /dev/null  # Fedora / redhat
  then
    sudo dnf install -y gcc openssl-devel 
  elif command -v pacman &> /dev/null  # arch
  then  
    sudo pacman -S --needed --noconfirm gcc openssl
  else
    echo "Unsupported package manager"
    echo "Follow github page for instructions"
    exit 1
fi
# Build everything with make
echo "Installing ${dir}"
make 

sudo cp bgit /usr/local/bin/bgit

echo "Install complete"
