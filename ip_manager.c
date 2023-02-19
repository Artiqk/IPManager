#include "database_handler.h"

int char_to_int(char buf);
void print_title(char *title);
void display_ip_addresses(int mask_filter, int (*ip_addresses)[4], int (*masks)[4], int rows);
void add_ip_to_database(sqlite3 *db);
void main_menu(sqlite3 *db, int (*ip_addresses)[4], int (*masks)[4], int rows);


int main (int argc, char* argv[]) {

    sqlite3* db = NULL;

    sqlite_connect(&db, "ip.db");

    const int rows = number_of_ip(db);

    int ip_addresses[rows][4];

    int masks[rows][4];

    load_ip_addresses(db, ip_addresses, masks, rows);

    main_menu(db, ip_addresses, masks, rows);

    sqlite3_close(db);

    return 0;
}


void main_menu (sqlite3* db, int (*ip_addresses)[4], int (*masks)[4], int rows) {
    print_title("IP-Manager");

    while (1) {
        printf("1. Add an IP\n");
        printf("2. Display stored IPs\n");
        printf("3. Quit\n");
        printf("> ");

        char menu_choice[3] = {0};

        fgets(menu_choice, 3, stdin);

        int choice = char_to_int(menu_choice[0]);

        if (choice < 1 || choice > 3) {
            continue;
        }

        switch (choice) {
            case 1:
                add_ip_to_database(db);
                rows = number_of_ip(db);
                load_ip_addresses(db, ip_addresses, masks, rows);
                break;
            case 2:
                int mask_filter = 0;
                char filter_choice[3] = {0};

                while (!(filter_choice[0] == 'y' || filter_choice[0] == 'n')) {
                    printf("Do you want to add add a mask filter ? (y/n): ");
                    fgets(filter_choice, 3, stdin);
                }

                if (filter_choice[0] == 'y') {
                    mask_filter = read_mask();
                }

                display_ip_addresses(mask_filter, ip_addresses, masks, rows);
                break;
            case 3:
                exit(0);
                break;
        }
    }   
}


void add_ip_to_database (sqlite3* db) {
    char* ip_str = read_ip();

    int prefix_length = read_mask();

    insert_ip(db, ip_str, prefix_length);
}


void display_ip_addresses (int mask_filter, int (*ip_addresses)[4], int (*masks)[4], int rows) {
    for (int i = 0; i < rows; i++) {
        int mask_prefix = mask_to_prefix(ip_to_string(masks[i]));
        if (mask_filter == mask_prefix || mask_filter == 0) {
            print_ip_address(ip_addresses[i], masks[i]);
        }
    }
}


void print_title(char* title) {
    int len = strlen(title);
    int cmd_len = len + 8; // 8 is for "figlet" + space + \0
    char* cmd = (char *) malloc(cmd_len * sizeof(char));
    snprintf(cmd, cmd_len, "figlet %s", title);
    system(cmd);
}


int char_to_int(char buf) {
    if (buf >= 48 && buf <= 57) {
        return buf - '0';
    }

    return -1;
}
