#include "ip_handler.h"

void handle_sqlite_error (sqlite3* db, char* message);
void sqlite_connect (sqlite3** db, char* database_file);
int number_of_ip (sqlite3* db);
int insert_ip (sqlite3* db, const char* ip_str, int prefix_length);
void print_ip_address (int ip[], int mask[]);
void load_ip_addresses (sqlite3* db, int (*ip_addresses)[4], int (*masks)[4], const int rows);
char* read_ip ();
int read_mask();

void flush_buffer () {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

char* read_ip () {
    char* ip_address = (char *) malloc(16 * sizeof(char));

    printf("IP: ");

    fgets(ip_address, 16, stdin);

    int len = strlen(ip_address);

    if (len > 0 && ip_address[len - 1] == '\n') {
        ip_address[len - 1] = '\0';
    }

    return ip_address;
}


int read_mask () {
    char prefix_length_str[4];
    int prefix_length = 0;

    while (prefix_length < 8 || prefix_length > 31) {
        printf("Subnet mask (8-31): ");

        fgets(prefix_length_str, 4, stdin);

        prefix_length = atoi(prefix_length_str);
    }

    return prefix_length;
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


int number_of_ip (sqlite3* db) {
    const char* sql_query = "SELECT COUNT(*) FROM ip_addresses;";

    sqlite3_stmt* statement = NULL;

    int result = -1;

    int rc = sqlite3_prepare_v2(db, sql_query, strlen(sql_query) + 1, &statement, NULL);

    if (rc == SQLITE_OK) {
        rc = sqlite3_step(statement);
        if (rc == SQLITE_ROW) {
            result = sqlite3_column_int(statement, 0);
        }
    }

    sqlite3_finalize(statement);
    
    return result;
}


int insert_ip (sqlite3* db, const char* ip_str, int prefix_length) {
    int ip[4];
    int mask[4];

    char* mask_str = prefix_to_mask(prefix_length);

    ip_str_to_array(ip_str, ip);
    ip_str_to_array(mask_str, mask);

    char* sql_query = "INSERT INTO ip_addresses (ip1, ip2, ip3, ip4, mask1, mask2, mask3, mask4) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* statement = NULL;

    int rc = sqlite3_prepare_v2(db, sql_query, strlen(sql_query) + 1, &statement, NULL);

    if (rc != SQLITE_OK) {
        handle_sqlite_error(db, "Error preparing statement");
    }

    for (int i = 0; i < 4; i++) {
        sqlite3_bind_int(statement, i + 1, ip[i]);
        sqlite3_bind_int(statement, i + 5, mask[i]);
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


void print_ip_address (int ip[], int mask[]) {
    char* ip_str = ip_to_string(ip);

    printf("-------------------------------------------------------------\n\n");

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

    printf("\tDecimal: \t%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    printf("\tBinary: \t");
    print_ip_binary(ip);
    printf("\tHexadecimal: \t0x%x.0x%x.0x%x.0x%x\n", ip[0], ip[1], ip[2], ip[3]);

    printf("\nMask:\n\n");

    printf("\tDecimal: \t%d.%d.%d.%d\n", mask[0], mask[1], mask[2], mask[3]);
    printf("\tBinary: \t");
    print_ip_binary(mask);
    printf("\tHexadecimal: \t0x%x.0x%x.0x%x.0x%x\n", mask[0], mask[1], mask[2], mask[3]);
    printf("\n-------------------------------------------------------------\n");
}


void load_ip_addresses (sqlite3* db, int (*ip_addresses)[4], int (*masks)[4], const int rows) {
    sqlite3_stmt* statement = NULL;
    const char* sql_query = "SELECT ip1, ip2, ip3, ip4, mask1, mask2, mask3, mask4 FROM ip_addresses;";
    int rc = sqlite3_prepare_v2(db, sql_query, strlen(sql_query) + 1, &statement, NULL);

    if (rc != SQLITE_OK) {
        handle_sqlite_error(db, "Error preparing statement");
    }

    int row_index = 0;

    while ((rc = sqlite3_step(statement)) == SQLITE_ROW) {
        for (int i = 0; i < 4; i++) {
            ip_addresses[row_index][i] = sqlite3_column_int(statement, i);
            masks[row_index][i] = sqlite3_column_int(statement, i + 4);
        }
        row_index++;
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error executing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(statement);
        exit(1);
    }

    sqlite3_finalize(statement);
}