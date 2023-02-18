#include "database_handler.h"


int main (int argc, char* argv[]) {

    sqlite3* db = NULL;

    sqlite_connect(&db, "ip.db");

    // char* ip = read_ip();

    // insert_ip(db, ip);

    display_ip_addresses(db, "255.255.0.0");

    sqlite3_close(db);

    return 0;
}