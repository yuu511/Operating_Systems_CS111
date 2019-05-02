#!/bin/sh
mkdir vessel > out
make clean >> out
make  >> out
./mk_disk 128 0 >> out
./fuse_routines vessel >> out
echo ''
echo ''
echo "*********   BEGIN TEST ***************"
echo ''
echo "**********  cd to root....  **************"
echo ''
echo "**********  ls  ***************"
echo ''
cd vessel
ls 
echo ''
echo "**********  mkdir  ***************"
echo ''
mkdir test
echo ''
echo "**********  ls  ***************"
echo ''
ls
echo ''
echo "******   cat file hello: ********"
echo ''
cat hello
echo ''
echo ''
echo "****** append "aaaa" to file hello: ********"
echo ''
echo "aaaa" >> hello
cat hello
echo ''
echo "***** cd  into sub_dir  ****"
echo ''
cd sub_dir
echo ''
echo "*****  ls -a in sub_dir  ****"
echo ''
ls -a
echo ''
echo "***** cd  back into root  ****"
cd ..
echo ''
echo "*****  ls  in root   ****"
echo ''
ls
