#include "database_handler.h"

void main_menu (sqlite3* db, int*** ip_addresses, int*** masks, int rows);
void print_title(char *title);
int char_to_int(char buf);


int main (int argc, char* argv[]) {

    system("clear");

    sqlite3* db = NULL;

    sqlite_connect(&db, "ip.db");

    const int rows = number_of_ip(db);

    int** ip_addresses = allocate_2d_array_memory(rows, 4);

    int** masks = allocate_2d_array_memory(rows, 4);

    load_ip_addresses(db, &ip_addresses, &masks, rows);

    main_menu(db, &ip_addresses, &masks, rows);

    free_2d_array(ip_addresses, number_of_ip(db));

    free_2d_array(masks, number_of_ip(db));

    sqlite3_close(db);

    return 0;
}


void main_menu (sqlite3* db, int*** ip_addresses, int*** masks, int rows) {
    print_title("IP-Manager");

    int rows_updated = rows;
    int ip_added = 0;

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
        
        if (ip_added) {
            rows_updated = number_of_ip(db);
            load_ip_addresses(db, ip_addresses, masks, rows_updated);
            ip_added = 0;
        }

        switch (choice) {
            case 1:
                add_ip_to_database(db);
                ip_added = 1;
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

                display_ip_addresses(mask_filter, (*ip_addresses), (*masks), rows_updated);
                break;
            case 3:
                exit(0);
                break;
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

