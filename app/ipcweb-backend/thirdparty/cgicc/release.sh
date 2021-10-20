echo DONT FORGET TO CHANGE VERSION increment on Makefile.Am
echo "libcgicc_la_LDFLAGS = -lstdc++ -version-info 5:2:0 "
echo The version info format is CURRENT:REVISION:AGE. 
echo "- If you have not changed the interface (bug fixes) bump the version to CURRENT : REVISION+1 : AGE "
echo "- If you have augmented the interface (new functions)bump the version to CURRENT+1 : 0 : AGE+1 "
echo "- If you have broken old interface (e.g. removed functions) bump the version to CURRENT+1 : 0 : 0" 
rm -rf cgicc-$1/*
rm -rf cgicc-$1
cd /tmp
rm -rf cgicc-$1/*
rm -rf cgicc-$1
cd -
mkdir /tmp/cgicc-$1
./autogen
cp -Rf * /tmp/cgicc-$1  
mv /tmp/cgicc-$1 .
cd cgicc-$1
echo "s/NOTVERSIONNEDPACKAGE/"$1"/" >tempGen.sed
echo "Writing"
sed -f tempGen.sed configure.ac>temp2
rm configure.ac
sed -f tempGen.sed configure>temp21
rm configure
mv temp21 configure
chmod +x configure
mv temp2 configure.ac
sed -f tempGen.sed Makefile.in>temp22
rm temp2 temp21 temp22
sed -f tempGen.sed doxygen.conf>temp3
rm doxygen.conf
mv temp3 doxygen.conf
rm tempGen.se

rm example/depcomp
rm example/INSTALL
rm example/config.guess
rm example/missing
rm example/ltmain.sh
rm example/install-sh
rm example/COPYING
rm example/config.sub
rm support/depcomp
rm support/config.guess
rm support/missing
rm support/ltmain.sh
rm support/install-sh
rm support/config.sub
rm INSTALL
rm cgicc/.libs/libcgicc.la
rm m4/ltsugar.m4
rm m4/lt~obsolete.m4
rm m4/ltoptions.m4
rm m4/ltversion.m4
rm m4/libtool.m4
rm example/ltmain.sh 
rm example/config.sub example/config.guess 

./autogen

rm -rf CVS
rm -rf cgicc/CVS
rm -rf contrib/CVS
rm -rf demo/CVS
rm -rf demo/images/CVS
rm -rf doc/CVS
rm -rf doc/images/CVS
rm -rf example/CVS
rm -rf info/CVS
rm -rf src/CVS
rm -rf support/CVS
rm -rf test/CVS
rm -rf win/CVS

rm -rf generatedDoc
./configure
make distclean

cd ..


tar -czf cgicc-$1.tar.gz cgicc-$1

