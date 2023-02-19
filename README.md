# IP-Manager
You will need a Debian with Aptitude to make this program work
## Install the fancy menu
```
sudo apt install figlet
```
## Install & prepare the SQLite3 database
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