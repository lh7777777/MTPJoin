# FlightJoin

We have implemented the code for FlightJoin, including `\src`, `\data` and `SBF`.
The source code of FlightJoin is in `\src`, including `\src\client` and `\src\SGX`.
The SBF source code used in FlightJoin is in `SBF`.
It can be integrated into MySQL using UDF, with a testing environment of Ubuntu 16.04 and Intel SGX SDK 2.6.

## Compilation

### client

    $ make clean && make
    $ ./db

Then follow the instructions to enter the range join, for example:

    1. load csv files:

        load 3 ../data/2021.csv ../data/2022.csv ../data/2023.csv
   
    2. buildindex:

        buildindex 0 file/table0_key.txt /var/lib/mysql/file/eneq0.txt on ORIGIN
        buildindex 1 file/table1_key.txt /var/lib/mysql/file/eneq1.txt on ORIGIN
        buildindex 2 file/table2_key.txt /var/lib/mysql/file/eneq2.txt on ORIGIN
   
    3. rangejoin:

        rangejoin 3 on 0 1 2
   
    4. exit:

        exit

### SGX

    $ make clean && make
    $ make install

This will generate the `app.so` file into the MySQL related directory, and then use `CREATE JUNCTION ...` in MySQL to create and use UDF functions.

