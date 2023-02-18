#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char* read_ip () {
    char* ip_address = (char *) malloc(16 * sizeof(char));

    printf("IP: ");

    if (fgets(ip_address, 16, stdin) == NULL) {
        fprintf(stderr, "Error reading input\n");
        return "";
    }

    int len = strlen(ip_address);

    if (len > 0 && ip_address[len - 1] == '\n') {
        ip_address[len - 1] = '\0';
    }

    return ip_address;
}


void handle_sqlite_error (sqlite3* db, char* message) {
    fprintf(stderr, "%s: %s\n", message, sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
}


void sqlite_connect (sqlite3** db, char* database_file) {
    int rc;

    rc = sqlite3_open(database_file, db);

    if (rc != SQLITE_OK) {
        handle_sqlite_error(*db, "Error opening database");
    }
}


int insert_ip (sqlite3* db, const char* ip_str) {
    int ip[4];

    ip_str_to_array(ip_str, ip);

    char* sql_query = "INSERT INTO ip_addresses (address1, address2, address3, address4) VALUES (?, ?, ?, ?);";

    sqlite3_stmt* statement = NULL;

    int rc = sqlite3_prepare_v2(db, sql_query, strlen(sql_query) + 1, &statement, NULL);

    if (rc != SQLITE_OK) {
        handle_sqlite_error(db, "Error preparing statement");
    }

    for (int i = 0; i < 4; i++) {
        sqlite3_bind_int(statement, i + 1, ip[i]);
    }

    rc = sqlite3_step(statement);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error executing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(statement);
        return 1;
    }

    sqlite3_finalize(statement);

    return 0;
}


void print_binary(int number) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (number >> i) & 1);
    }
}


void print_ip_binary (int ip[]) {
    for (int i = 0; i < 4; i++) {
        print_binary(ip[i]);
        printf(".");
    }

    printf("\n");
}

char* ip_to_string (int ip[]) {
    char* ip_str = (char *)malloc(16 * sizeof(char));

    if (ip_str == NULL) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }

    sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    return ip_str;
}


int ip_str_to_array(const char* ip_str, int* ip) {
    if (sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) != 4) {
        fprintf(stderr, "Invalid IP address format: %s\n", ip_str);
        return 0;
    }

    return 1;
}

int is_private_ip (const char* ip_str) {
    int ip[4];

    ip_str_to_array(ip_str, ip);
    
    if (ip[0] == 10 || // 10.0.0.0
        (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) || // 172.16.0.0 - 172.31.0.0
        (ip[0] == 192 && ip[1] == 168) // 192.168.0.0
    ) {
        return 1;
    }

    return 0;
}


int is_loopback_ip (const char* ip_str) {
    struct in_addr ip;

    if (inet_pton(AF_INET, ip_str, &ip) != 1) {
        return -1; // Invalid ip address format
    }

    return ip.s_addr == htonl(INADDR_LOOPBACK);
}


int is_link_local_ip(const char* ip_str) {
    int ip[4];

    ip_str_to_array(ip_str, ip);

    return ip[0] == 169 && ip[1] == 254;
}


int is_multicast_ip (const char* ip_str) {
    int ip[4];

    ip_str_to_array(ip_str, ip);

    return ip[0] >= 224 && ip[0] <= 239;
}


int is_broadcast_ip (const char* ip_str)  {
    int ip[4];

    ip_str_to_array(ip_str, ip);

    return (ip[0] == 255 && ip[1] == 255 && ip[2] == 255 && ip[3] == 255);
}


int is_unspecified_ip (const char* ip_str) {
    int ip[4];

    ip_str_to_array(ip_str, ip);

    return (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0);
}


void fancy_print_ip_address (int ip[]) {
    char* ip_str = ip_to_string(ip);

    if (
        is_multicast_ip(ip_str) || 
        is_link_local_ip(ip_str) || 
        is_loopback_ip(ip_str) ||
        is_broadcast_ip(ip_str) ||
        is_unspecified_ip(ip_str)
    ) {
        printf("The IP address is special\n\n");
    } else if (is_private_ip(ip_str)) {
        printf("The IP address is private\n\n");
    } else {
        printf("The IP address is public\n\n");
    }

    printf("Decimal: \t%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    printf("Hexadecimal: \t0x%x.0x%x.0x%x.0x%x\n", ip[0], ip[1], ip[2], ip[3]);
    printf("Binary: \t");
    print_ip_binary(ip);
    printf("\n----------------------------------------------------\n\n");
}


void display_ip_addresses (sqlite3* db, char* mask_str) {
    sqlite3_stmt* statement = NULL;
    const char* sql_query = "SELECT address1, address2, address3, address4 FROM ip_addresses;";
    int rc = sqlite3_prepare_v2(db, sql_query, strlen(sql_query) + 1, &statement, NULL);

    if (rc != SQLITE_OK) {
        handle_sqlite_error(db, "Error preparing statement");
    }

    int ip[4];

    while ((rc = sqlite3_step(statement)) == SQLITE_ROW) {
        for (int i = 0; i < 4; i++) {
            ip[i] = sqlite3_column_int(statement, i);
        }  

        fancy_print_ip_address(ip);
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error executing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(statement);
        exit(1);
    }

    sqlite3_finalize(statement);
}