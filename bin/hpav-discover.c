//
// Created by hecke on 3/20/16.
//
// send ieee 1901-2010 MAC Management message type 0x0014 - CC_DISCOVER_LIST.request to selected MAC
// (default: ff:ff:ff:ff:ff:ff) to discover HP AV compatible devices
//
// you may use the qualcom-atheros-local-management-address 00:b0:52:00:00:01 (flag -l/--localcast)
// to discover local devices only
//

#include <stdio.h>
#include <argp.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "ieee1901-2010.h"

#include "homeplug-av-ext.h"
#include "events.h"
#include "net.h"
#include "helper.h"

struct cmd_cnf {
    char if_name[IF_NAMESIZE];
    uint8_t oda[6];

    uint8_t hpav_ver;

    bool discover_networks;
    bool numeric_output;
};

struct app_ctx {
    struct net_ctx *nctx;
    struct events_ctx *ectx;
    struct ieee1901_2010_ctx ieee1901_ctx;
    struct cmd_cnf conf;
};

#define _PANIC(...) fprintf(stderr, __VA_ARGS__); exit(-1);

static error_t _parse_opt(int key, char *arg, struct argp_state *state);

static char _app_doc[] = "\nSend IEEE 1901-2010 MAC Management message type 0x0014 - CC_DISCOVER_LIST.request to selected MAC.";

static struct argp_option _cmd_options[] = {
        {"broadcast",      'b', NULL,      0, "set oda to ff:ff:ff:ff:ff:ff (default)"},
        {"localcast",      'l', NULL,      0, "set oda to Qualcomm Atheros LMA 00:b0:52:00:00:01"},
        {"oda",            'd', "DST-MAC", 0, "set oda to given mac address"},
        {"v1-0",           '0', NULL,      0, "use version homeplug av 1.0"},
        {"v1-1",           '1', NULL,      0, "use version homeplug av 1.1 (default)"},
        {"no-nwinfo",      'n', NULL,      0, "don't request nwinfo for each discovered station"},
        {"numeric-output", 'N', NULL,      0, "show numeric values instead of translated strings"},
        {0}
};

static char _args_doc[] = "interface";

static struct argp _argp = {
        _cmd_options,
        _parse_opt,
        _args_doc,
        _app_doc
};

static error_t _parse_opt(int key, char *arg, struct argp_state *state) {
    struct cmd_cnf *conf = state->input;

    switch (key) {
        case 'b':
            memcpy(conf->oda, ETHERNET_BROADCAST_ADDRESS, sizeof(conf->oda));
            break;
        case 'l':
            memcpy(conf->oda, QUALCOMM_ATHEROS_LOCAL_MANAGEMENT_ADDRESS, sizeof(conf->oda));
            break;
        case '0':
            conf->hpav_ver = MM_APDU_MMV_1_0;
            break;
        case '1':
            conf->hpav_ver = MM_APDU_MMV_1_1;
            break;
        case 'd':
            if (hpav_parse_mac(conf->oda, arg) != EXIT_SUCCESS) {
                fprintf(stderr, "Given MAC address has invalid format. Expected: aa:bb:cc:dd:ee:ff\n");
                argp_usage(state);
            }
            break;
        case 'n':
            conf->discover_networks = false;
            break;
        case 'N':
            conf->numeric_output = true;
            break;
        case ARGP_KEY_ARG: // interface
            if (state->arg_num > 0) {
                fprintf(stderr, "Only one name for attribute interface allowed.\n");
                argp_usage(state);
            }
            strncpy(conf->if_name, arg, IF_NAMESIZE);
            break;
        case ARGP_KEY_END:
            if (state->arg_num != 1) {
                fprintf(stderr, "Missing attribute interface name.\n");
                argp_usage(state);
            }
            break;
    }

    return 0;
}

static void _handle_data(int fd, void *usr) {
    uint8_t buf[1500];
    ssize_t inb;
    struct app_ctx *ctx = (struct app_ctx *) usr;

    if (usr == NULL) {
        return;
    }

    inb = read(fd, buf, 1500);
    if (inb < IEEE1901_2010_MIN_MSG_SIZE) {
        return;
    }

    ieee1901_2010_handle_message(&ctx->ieee1901_ctx, buf, inb);
}

static void _station_info(uint8_t osa[6], struct ieee1901_2010_station_information *sta_info, void *usr_data) {
    struct app_ctx *ctx = usr_data;
    static size_t i;

    if (ctx == NULL) {
        return;
    }

    printf("discovered-station_%lu:\n", i++);
    printf("  osa: %02x:%02x:%02x:%02x:%02x:%02x\n", osa[0], osa[1], osa[2], osa[3], osa[4], osa[5]);
    printf("  mac: %02x:%02x:%02x:%02x:%02x:%02x\n", sta_info->mac_addr[0], sta_info->mac_addr[1],
           sta_info->mac_addr[2], sta_info->mac_addr[3], sta_info->mac_addr[4], sta_info->mac_addr[5]);
    printf("  tei: 0x%02x\n", sta_info->tei);
    printf("  snid: 0x%02x\n", sta_info->snid);


    if (ctx->conf.numeric_output == true) {
        printf("  same-network: 0x%02x\n", sta_info->same_network);
    } else {
        printf("  same-network: %s\n", ieee1901_2010_same_network_string(sta_info->same_network));
    }

    if (ctx->conf.discover_networks == true) {
        // request nwinfo for responding station
        ieee1901_2010_message_send(ctx->nctx->sock,
                                   osa,
                                   ctx->nctx->if_cfg.sll_addr,
                                   ctx->conf.hpav_ver,
                                   MM_TYPE_CM_NW_INFO,
                                   MMOperationTypeRequest,
                                   NULL, 0);
    }
}

#if 0
static void _network_info(uint8_t osa[6], struct ieee1901_2010_network_information *net_info, void *usr_data)
{
    ieee1901_2010_print_mac(osa);
    printf(" -> net info\n");
}
#endif

static void _nwinfo(uint8_t osa[6], struct ieee1901_2010_nwinfo *nwinfo, void *usr_data) {

    struct app_ctx *ctx = usr_data;
    static size_t i;

    printf("nwinfo_%lu:\n", i++);
    printf("  osa: %02x:%02x:%02x:%02x:%02x:%02x\n", osa[0], osa[1], osa[2], osa[3], osa[4], osa[5]);
    printf("  bm-mac: %02x:%02x:%02x:%02x:%02x:%02x\n", nwinfo->bm_mac[0], nwinfo->bm_mac[1], nwinfo->bm_mac[2],
           nwinfo->bm_mac[3], nwinfo->bm_mac[4], nwinfo->bm_mac[5]);
    printf("  tei: 0x%02x\n", nwinfo->tei);
    printf("  nid: %02x:%02x:%02x:%02x:%02x:%02x:%02x\n", nwinfo->nid[0], nwinfo->nid[1], nwinfo->nid[2],
           nwinfo->nid[3], nwinfo->nid[4], nwinfo->nid[5], nwinfo->nid[6]);
    printf("  snid: 0x%02x\n", nwinfo->snid);
    if (ctx->conf.numeric_output == true) {
        printf("  access: 0x%02x\n", nwinfo->access);
        printf("  role: 0x%02x\n", nwinfo->station_role);
    } else {
        printf("  access: %s\n", ieee1901_2010_network_type_string(nwinfo->access));
        printf("  role: %s\n", ieee1901_2010_station_role_string(nwinfo->station_role));
    }
}

int main(int argc, char **argv) {

    struct app_ctx ctx;

    memset(&ctx.conf, 0, sizeof(ctx.conf));
    memcpy(ctx.conf.oda, ETHERNET_BROADCAST_ADDRESS, sizeof(ctx.conf.oda));
    ctx.conf.if_name[0] = '\0';
    ctx.conf.hpav_ver = MM_APDU_MMV_1_1;
    ctx.conf.discover_networks = true;
    ctx.conf.numeric_output = false;

    argp_parse(&_argp, argc, argv, 0, 0, &ctx.conf);

    if ((ctx.ectx = events_setup()) == NULL) {
        _PANIC("Failed to create event handler.\n");
    }

    if ((ctx.nctx = net_create(ctx.conf.if_name, IEEE1901_2010_ETHERTYPE)) == NULL) {
        _PANIC("Failed to open interface %s.\n", ctx.conf.if_name);
    }

    ieee1901_2010_init(&ctx.ieee1901_ctx);

    ieee1901_2010_register_cc_discover_list_confirm_station_information_callback(&ctx.ieee1901_ctx, _station_info,
                                                                                 &ctx);
    //ieee1901_2010_register_cc_discover_list_confirm_network_information_callback(&ctx.ieee1901_ctx, _network_info, &ctx);
    ieee1901_2010_register_cm_nw_info_confirm_nwinfo_callback(&ctx.ieee1901_ctx, _nwinfo, &ctx);

    events_add_reader(ctx.ectx, ctx.nctx->sock, _handle_data, &ctx, NULL);

    ieee1901_2010_message_send(ctx.nctx->sock,
                               ctx.conf.oda,
                               ctx.nctx->if_cfg.sll_addr,
                               ctx.conf.hpav_ver,
                               MM_TYPE_CC_DISCOVER_LIST,
                               MMOperationTypeRequest,
                               NULL, 0);

    usleep(250000);

    events_teardown(ctx.ectx);
    net_destroy(ctx.nctx);

    return 0;
}