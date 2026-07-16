#!/bin/bash

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
    cd $dir	
fi

# Build everything with make
echo "Installing ${dir}"
make 

sudo cp bgit /usr/local/bin/bgit

echo "Install complete"
