# litedb

A tiny database

## Getting Started

### Build
    
    git clone https://github.com/jinyyu/litedb.git
    mkdir -p litedb/build
    cd litedb/build
    cmake ..
    make -j8
    
### Initializes

    ./litedb -i -d /tmp/data_dir
    
### Run

Start server

    ./litedb -d /tmp/data_dir -p 5432  

Connect to server, using [psql](https://www.postgresql.org/)

    psql -p 5432 -h 127.0.0.1
    
running SQL

    psql=> create table test(c1 int unique, c2 text primary key, check(c1<0));
    
    psql=> select * from sys_class where id>0;
    
### Roadmap

|  feature        | status  |
|  -------        | ------  |
| libpq protocol  | ok |
| SQL parser      | ok |
| transform the parse tree into a query tree  | ok |
| system catalog  | ok |
| sequential scanning  | ok |
| index scanning  | ok |
| planner  | no |
| executor | no |
| mvcc     | no |
    
    
    
    
    
    
    
