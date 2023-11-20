# CompSys A4 solution README
This system is slightly more complicated to run than A3 as in theory every peer on the network will be exactly the same. In order to properly test your system you will need to start different peers in varying order, with multiple at the same time in some cases. To aide this two idnetical Python peers have been provided. 

## To run a Python peer from within either the *python/first_peer* directory or *python/second_peer* directory:
    python3 peer.py config.yaml

## To compile the C peer from within the *src* directory:
    make

## To run C peer from within the *src* directory:
    ./peer config.yaml

## To clean C peer from within the *src* directory:
    make clean

## To check correctness of results from within the *src* directory:
    diff tiny.txt ../python/first_peer/tiny.txt && diff hamlet.txt ../python/first_peer/hamlet.txt
The above command should produce no output if everything has worked correctly
