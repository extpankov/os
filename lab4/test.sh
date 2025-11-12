rm -f file.txt
touch file.txt

echo "a) ./mychmod +x file.txt"
./mychmod +x file.txt
ls -l file.txt | grep file.txt

echo "b) ./mychmod u-r file.txt"
./mychmod u-r file.txt
ls -l file.txt | grep file.txt

echo "c) ./mychmod g+rw file.txt"
./mychmod g+rw file.txt
ls -l file.txt | grep file.txt

echo "d) ./mychmod ug+rw file.txt"
./mychmod ug+rw file.txt
ls -l file.txt | grep file.txt

echo "e) ./mychmod uga+rwx file.txt"
./mychmod uga+rwx file.txt
ls -l file.txt | grep file.txt

echo "f) ./mychmod 766 file.txt"
./mychmod 766 file.txt
ls -l file.txt | grep file.txt
