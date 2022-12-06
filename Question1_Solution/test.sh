echo "Version 1 Compilation"
echo "Enter the name of the Output File for Version 1 of the Program : "
read fileName

gcc -pthread version1.c -o $fileName
echo "Enter the number of child processes to test : "
read noChildProcesses
echo "Testing  Program Version 1 with $noChildProcesses Child Processes"
echo 

./$fileName $noChildProcesses
echo 

echo "-----------------------------------------------------------------------------------------------------------------------"
echo
echo "Version 2 Compilation"
echo "Enter the name of the Output File for Version 2 of the Program. Don't use the same file name as Version1 output file : "
read fileName
gcc -pthread version2.c -o $fileName
echo "Enter the number of child processes to test : "
read noChildProcesses
echo "Testing Program Version 2 with $noChildProcesses Child Processes"
echo 

./$fileName $noChildProcesses
echo




