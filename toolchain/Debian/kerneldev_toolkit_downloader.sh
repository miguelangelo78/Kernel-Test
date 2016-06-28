#!/bin/bash
cd `dirname $0`
clear

pkg_count=0

function NEW_PKG {
	pkg_count=$((pkg_count+1))
	pkg_name[${pkg_count}]=$1
	pkg_setup[${pkg_count}]=$2
	pkg_exec[${pkg_count}]=$3
}

#!!!!!!!!!!!!!!!!!!!!!!!! CREATE PACKAGES HERE !!!!!!!!!!!!!!!!!!!!!!!!
NEW_PKG "Update System" update update_dummy_cmd
NEW_PKG "GCC" gcc gcc
NEW_PKG "G++" g++ g++
NEW_PKG "Python 2.7.11" python2.7 python
NEW_PKG Make make make
NEW_PKG GDB gdb gdb
NEW_PKG "Clang 3.8.0 (LLVM)" clang-3.8 clang-3.8
NEW_PKG Git git git
NEW_PKG QEMU qemu-system-x86 qemu-system-x86_64
#!!!!!!!!!!!!!!!!!!!!!!!! CREATE PACKAGES HERE !!!!!!!!!!!!!!!!!!!!!!!!

printf "********* Kernel Development Toolkit Downloader *********\n"
printf "* The script will now download and install the following packages:\n"

for i in `seq 1 ${pkg_count}`;
do
	echo $i- ${pkg_name[$i]}
done

while true; do
	read -p "* Do you accept these changes [Y]/n: " yn
	case $yn in
		[Yy]* ) 
			############### Add repositories: ###############
			# Core components:
			sudo apt-get install software-properties-common
			# Clang:
			printf "\n***** Step 1: Adding repositories *****\n"
			printf "    1- Adding Clang repositories\n"
			wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -
			sudo add-apt-repository "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.8 main"
			
			############### Download and install packages: ###############
			printf "\n***** Step 2: Downloading and installing Packages *****\n"
			for i in `seq 1 ${pkg_count}`;
			do
				printf "   $i- Installing '${pkg_name[$i]}'\n"
				if hash ${pkg_exec[$i]} 2>/dev/null; then
					printf "    ** The program '${pkg_name[$i]}' is already installed\n"
				else
					if [ "${pkg_setup[$i]}" = "update" ] 
					then
						sudo apt-get ${pkg_setup[$i]}
					else
						sudo apt-get install ${pkg_setup[$i]}
					fi
					printf "    ** Done\n"
				fi
			done
			
			############### Install Kernel Source Code: ###############
			printf "\n***** Step 3: Installing Kernel Source Code *****\n"
			git clone https://github.com/miguelangelo78/Kernel-Test.git
			cd Kernel-Test
			git pull
			find . -type f -name *.sh -o -name *.pyc -exec chmod +x {} \;
			cd ..
			
			############### FINISHED ###############
			printf "\n***** Step 4: Success *****\n"
			printf "\n** Download and Installation complete! **\n"
			read -p "Press Enter key to exit..."
			
			break;;
		[Nn]* ) exit;;
		* ) echo "Please answer yes or no.";;
	esac
done
