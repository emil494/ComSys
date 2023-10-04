#!/usr/bin/env bash

# Exit immediately if any command below fails.
set -e

make


echo "Generating a test_files directory.."
mkdir -p test_files
rm -f test_files/*


echo "Generating test files.."

# EMPTY TESTS
printf "" > test_files/empty.input

# ASCII TESTS
printf "Hello, World!\n" > test_files/ascii.input # With line terminator
printf "Hello, World!" > test_files/ascii2.input # Without line terminator
printf "\x07\x0D\x1B\x20\x7E" > test_files/ascii3.input # Edge cases
printf "\n\n\nHello, World!\n\n\n" > test_files/ascii4.input

# ISO TESTS
echo "Hello World!" | iconv -f UTF-8 -t ISO-8859-1 > test_files/iso.input # Using incov to create ISO file
printf "Hello,\xA0World!" > test_files/iso2.input # Edge case, first value
printf "Hello,\xFFWorld!" > test_files/iso3.input # Edge case, last value
printf "Hello,\xC8World!\n" > test_files/iso4.input # Line terminator

# UTF TESTS
printf "Hello,\xD094World" > test_files/utf.input # 2 byte character
printf "ほかいど変態それわガメ\n" > test_files/utf2.input # 3 byte characters
printf "Hello,\xF09F838FWorld" > test_files/utf3.input # 4 byte character
printf "Hello,\xD881World\n" > test_files/utf4.input # Line terminator


# DATA TESTS
printf "Hello,\x00World!\n" > test_files/data.input # 0 byte
printf "\x01\x02\x03\x04\x05" > test_files/data2.input # Outside of ASCII set
printf "Hello,\x0E\x1A\x1C\x1FWorld!" > test_files/data3.input # Outside of ASCII set
#printf "Hello,\x80\9FWorld!" > test_files/data4.input # Outside of ISO set
printf "Hello,\x3A7DB28F61World" > test_files/data5.input # 5 byte character
printf "Hello,\x3A7DB28F617AWorld" > test_files/data6.input # 6 byte character

# ERRORS
printf "hemmelighed" > test_files/hemmelig_fil.input
chmod -r  test_files/hemmelig_fil.input


echo "Running the tests.."
exitcode=0
for f in test_files/*.input
do
  echo ">>> Testing ${f}.."
  file    ${f} | sed -e 's/ASCII text.*/ASCII text/' \
                         -e 's/UTF-8 Unicode text.*/UTF-8 Unicode text/' \
                         -e 's/ISO-8859 text.*/ISO-8859 text/' \
                         -e 's/cannot open: Permission denied/cannot determine (Permission denied)/' \
                         > "${f}.expected"
  ./file  "${f}" > "${f}.actual"

  if ! diff -u "${f}.expected" "${f}.actual"
  then
    echo ">>> Failed :-("
    exitcode=1
  else
    echo ">>> Success :-)"
  fi
done
if [ "$exitcode" -eq 0 ];
then
  echo "All tests: Success :-)"
else
  echo "All tests: Failed :-("
fi
exit $exitcode
