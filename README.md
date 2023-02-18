# IP-Manager
## Install & prepare the SQLite3 database
(I'm currently using Debian with Aptitude as my package manager)
```
sudo apt install sqlite3
```
Then you'll need to prepare the database
```
sqlite3 ip.db < create.sql
```