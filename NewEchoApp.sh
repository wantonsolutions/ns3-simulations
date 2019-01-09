#!/bin/bash


function findReplace() {
	filename=$1
	transformfilename=$2
	while IFS='' read -r line || [[ -n "$line" ]]; do
	    echo " replacing $line"
	    sed -i "s/$line/g" $filename
	done < "$transformfilename"
}



echo Coping $1 into new set of files $2
modelPrefix=src/applications/model
helperPrefix=src/applications/helper
clientcc=$1-client.cc
clienth=$1-client.h
servercc=$1-server.cc
serverh=$1-server.h
helpercc=$1-helper.cc
helperh=$1-helper.h

nclientcc=$2-client.cc
nclienth=$2-client.h
nservercc=$2-server.cc
nserverh=$2-server.h
nhelpercc=$2-helper.cc
nhelperh=$2-helper.h



if [ ! -f $modelPrefix/$clientcc ]; then
	echo "client c++ file not found"
	exit
fi
if [ ! -f $modelPrefix/$clienth ]; then
	echo "client header file not found"
	exit
fi
if [ ! -f $modelPrefix/$servercc ]; then
	echo "$modelprefix/$servercc server c++ file not found"
	exit
fi
if [ ! -f $modelPrefix/$serverh ]; then
	echo "server header file not found"
	exit
fi
if [ ! -f $helperPrefix/$helpercc ]; then
	echo "helper c++ file not found"
	exit
fi
if [ ! -f $helperPrefix/$helperh ]; then
	echo "helper header file not found"
	exit
fi

#copy phase
cp $modelPrefix/$clientcc $modelPrefix/$nclientcc 
cp $modelPrefix/$clienth $modelPrefix/$nclienth
cp $modelPrefix/$servercc $modelPrefix/$nservercc 
cp $modelPrefix/$serverh $modelPrefix/$nserverh 
cp $helperPrefix/$helpercc $helperPrefix/$nhelpercc
cp $helperPrefix/$helperh $helperPrefix/$nhelperh 

#safty check
if [ ! -f $modelPrefix/$nclientcc ]; then
	echo "client c++ file not found"
	exit
fi
if [ ! -f $modelPrefix/$nclienth ]; then
	echo "client header file not found"
	exit
fi
if [ ! -f $modelPrefix/$nservercc ]; then
	echo "$modelprefix/$servercc server c++ file not found"
	exit
fi
if [ ! -f $modelPrefix/$nserverh ]; then
	echo "server header file not found"
	exit
fi
if [ ! -f $helperPrefix/$nhelpercc ]; then
	echo "helper c++ file not found"
	exit
fi
if [ ! -f $helperPrefix/$nhelperh ]; then
	echo "helper header file not found"
	exit
fi

if [ ! -f $3 ]; then
	echo "No transformation file provided, deleting copies to preserve the build"
	rm $modelPrefix/$nclientcc 
	rm $modelPrefix/$nclienth
	rm $modelPrefix/$nservercc 
	rm $modelPrefix/$nserverh 
	rm $helperPrefix/$nhelpercc
	rm $helperPrefix/$nhelperh 
fi


echo "Transformation File"
cat $3


findReplace $modelPrefix/$nclientcc $3
findReplace $modelPrefix/$nclienth $3
findReplace $modelPrefix/$nservercc $3
findReplace $modelPrefix/$nserverh $3
findReplace $helperPrefix/$nhelpercc $3
findReplace $helperPrefix/$nhelperh $3

echo "add the following to src/applications/wscript"
echo "model/$nclientcc,"
echo "model/$nservercc," 
echo "helper/$nhelpercc," 
echo "model/$nclienth," 
echo "model/$nserverh,"
echo "helper/$nhelperh," 


#copy phase
#rm $modelPrefix/$nclientcc 
#rm $modelPrefix/$nclienth
#rm $modelPrefix/$nservercc 
#rm $modelPrefix/$nserverh 
#rm $helperPrefix/$nhelpercc
#rm $helperPrefix/$nhelperh 
