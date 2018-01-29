cd ../..
g++ -O3 -o simplifyDriver drivers/simplification.cc -I src build/src/libiegenlib.a -lisl -std=c++11
cd "data/3 Gauss-Seidel BCSR"/
time ../../simplifyDriver gs_bcsr.json > gs_bcsr.out 2> /dev/null
echo ""
echo "The output is written to gs_bcsr.out"
echo ""
