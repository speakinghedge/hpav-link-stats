//
// Created by hecke on 3/6/16.
//

#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>

#include "net.h"

static int _get_if_cfg(const char *if_name, struct sockaddr_ll *ll_cfg)
{
    struct ifreq rq_data;
    int rq_fd;

    if ((rq_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
    {
        return EXIT_FAILURE;
    }

    strncpy(rq_data.ifr_name, if_name, IFNAMSIZ);

    if (ioctl(rq_fd, SIOCGIFHWADDR, &rq_data) == 0) {

        memcpy(&ll_cfg->sll_addr, &rq_data.ifr_addr.sa_data, 6);
    } else {
        close(rq_fd);
        return EXIT_FAILURE;
    }

    if (ioctl(rq_fd, SIOCGIFINDEX, &rq_data) == 0) {

        ll_cfg->sll_ifindex = rq_data.ifr_ifindex;
    } else {
        close(rq_fd);
        return EXIT_FAILURE;
    }

    close(rq_fd);

    return EXIT_SUCCESS;
}

/**
 * @brief create a non-blocking AF_PACKET socket bound to given interface and protocol
 */
struct net_ctx* net_create(const char *if_name, uint16_t l3_proto)
{
    struct net_ctx *ctx;

    if (!(ctx = calloc(1, sizeof(*ctx))))
    {
        return NULL;
    }

    if (_get_if_cfg(if_name, &ctx->if_cfg) == EXIT_FAILURE)
    {
        goto err_has_ctx;
    }

#if 0
    printf("idx: %d %02x:%02x:%02x:%02x:%02x:%02x\n",
    ctx->if_cfg.sll_ifindex,
    ctx->if_cfg.sll_addr[0],
           ctx->if_cfg.sll_addr[1],
           ctx->if_cfg.sll_addr[2],
           ctx->if_cfg.sll_addr[3],
           ctx->if_cfg.sll_addr[4],
           ctx->if_cfg.sll_addr[5]);
#endif

    ctx->sock = socket(AF_PACKET, SOCK_RAW | SOCK_NONBLOCK, htons(l3_proto));
    if (ctx->sock < 0)
    {
        goto err_has_ctx;
    }

    ctx->if_cfg.sll_family = AF_PACKET;
    ctx->if_cfg.sll_protocol = htons(l3_proto);

    if((bind(ctx->sock, (struct sockaddr*) &ctx->if_cfg, sizeof(ctx->if_cfg))))
    {
        goto err_has_sock;
    }

    return ctx;

err_has_sock:
    close(ctx->sock);
err_has_ctx:
    free(ctx);

    return NULL;
}

void net_destroy(struct net_ctx *ctx)
{

    close(ctx->sock);

    free(ctx);
}
