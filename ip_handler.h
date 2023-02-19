#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int mask_to_prefix(char *mask);
int ip_str_to_array(const char *ip_str, int *ip);
void print_binary(int number);
void print_ip_binary(int ip[]);
char *ip_to_string(int ip[]);
int is_private_ip(const char *ip_str);
int is_loopback_ip(const char *ip_str);
int is_link_local_ip(const char *ip_str);
int is_multicast_ip(const char *ip_str);
int is_broadcast_ip(const char *ip_str);
int is_unspecified_ip(const char *ip_str);


char* prefix_to_mask (int prefix_length) {
    if (prefix_length < 8 || prefix_length > 31) {
        fprintf(stderr, "Invalid prefix length: %d\n", prefix_length);
        return NULL;
    }

    unsigned int mask = 0xffffffff << (32 - prefix_length);

    int octet[4];

    for (int i = 0, offset = 24; i < 4; i++, offset -= 8) {
        octet[i] = (mask >> offset) & 0xff;
    }

    char* mask_str = (char *)malloc(16 * sizeof(char));

    sprintf(mask_str, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]); // FIXME - Use snprintf for a more secure input

    return mask_str;
}


int mask_to_prefix(char *mask) {
    unsigned int ip[4];
    sscanf(mask, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);

    int prefix = 0;
    for(int i=0; i<4; i++) {
        unsigned int m = 0x80;
        for(int j=0; j<8; j++) {
            if(ip[i] & m) {
                prefix++;
            }
            else {
                return prefix;
            }
            m >>= 1;
        }
    }
    return prefix;
}


int ip_str_to_array(const char* ip_str, int* ip) {
    if (sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) != 4) {
        fprintf(stderr, "Invalid IP address format: %s\n", ip_str);
        return 0;
    }

    return 1;
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