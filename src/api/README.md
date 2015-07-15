# Byng API

You will need libboost, libpq, libpqxx and cppnetlib installed in their dev version.

## Compilation

byng-api is managed by cmake, for standard linux makefile, do:

    cmake . -G "Unix Makefiles"
    make
    
byng-api will be created in `bin/`

## Usage

    DBNAME=byng DBUSER=byng DBPASSWORD=byng DBHOST=127.0.0.1 ./bin/byng-api
    
You can also modify the listening port with `LISTENING_PORT`
