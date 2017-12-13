## ps_bench

### Build
`mkdir build`  
`cd build`  
`cmake .. -DMARIADB=/path/mariadb_config`  

### Usage
ps_bench options  
-u username  
-p password  
-h hostname (default = localhost)  
-s socket  
-d database  
-i iterations  
-t use text protocol  
-P port (default 3306)  
-o prepare once, execute multiple  
-c create table and fill data  
