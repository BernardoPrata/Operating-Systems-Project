#! /bin/bash

cd tecnicofs
make -W=operations.c all 2> ../makeInfo > ../makeInfo
if [ $? -ne "0" ]; then
	echo "Error making all"
fi

cd ..

echo ""
echo "Official tests"
echo ""

for file in `ls ./tecnicofs/tests/official | grep .c$`; do

	temp=(${file//./ })
	rootName=${temp[0]}
	echo -n "$rootName - "
	./tecnicofs/tests/official/$rootName
done

echo ""
echo ""
echo "Custom tests"
echo ""


cd tecnicofs;

echo "" > ../makeInfo

for file in `ls ./tests/custom | grep .c$`; do
	temp=(${file//./ })
	rootName=${temp[0]}
	make tests/custom/$rootName 2>> ../makeInfo >> ../makeInfo
	if [ $? -ne "0" ]; then
		echo "Error making $rootName"
		exit 1;
	fi

	echo -n "$rootName - "
	./tests/custom/$rootName
done
