# IP-Manager
You will need a Debian with Aptitude to make this program work
## Install the dependancies
```
sudo apt install figlet sqlite3 libsqlite3-dev
```
Then you'll need to prepare the database
```
touch ip.db
sqlite3 ip.db < create.sql
```
## Compile
```
gcc -o prog ip_manager.c -lsqlite3
```