#!/bin/sh
gcc -Wall -Wextra -pedantic main.c || {
    echo "❌ Compile error"
    exit 1
}

for infile in in/*_in.txt; do
    number=$(basename "$infile" | cut -d "_" -f 1)
    outfile="out/${number}_out.txt"

    echo "Running test $number..."

    ./a.out < "$infile" > tmp

    if diff -q tmp "$outfile" >/dev/null; then
        echo "  ✔ OK"
    else
        echo "  ❌ FAIL"
        diff tmp "$outfile"
        exit 1
    fi
done

rm -f tmp
rm -f a.out

exit 0