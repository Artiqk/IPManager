# IP-Manager
## Install & prepare the SQLite3 database
(I'm currently using Debian with Aptitude as my package manager) <br>
Install the package to use sqlite3
```
sudo apt install sqlite3 libsqlite3-dev
```
Then you'll need to prepare the database
```
sqlite3 ip.db < create.sql
```
## Compile
```
gcc -o prog ip_manager.c -lsqlite3
```