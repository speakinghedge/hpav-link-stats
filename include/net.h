//
// Created by hecke on 3/6/16.
//

#ifndef HPAVMOND_NET_C_H
#define HPAVMOND_NET_C_H

#include <inttypes.h>
#include <linux/if_packet.h>

struct net_ctx{
    int sock;

    struct sockaddr_ll if_cfg;
};

struct net_ctx* net_create(const char *if_name, uint16_t l3_proto);
void net_destroy(struct net_ctx *ctx);

#endif //HPAVMOND_NET_C_H
