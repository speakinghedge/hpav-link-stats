//
// Created by hecke on 3/21/16.
//
// send ieee 1901-2010 MAC Management message type 0x0014 - CC_DISCOVER_LIST.request to selected MAC
// (default: ff:ff:ff:ff:ff:ff) to discover HP AV compatible devices
//
// you may use the qualcom-atheros-local-management-address 00:b0:52:00:00:01 (flag -l/--localcast
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

enum ReqWhatTypes {
    REQ_ALL_STATS = 0,
    REQ_NETWORK_STATS = 1 << 0,
    REQ_LINK_STATS = 1 << 1,
};

enum ReqLinkTypes {
    REQ_RX_TX_LINK_STATS = 0,
    REQ_RX_LINK_STATS = 1 << 0,
    REQ_TX_LINK_STATS = 1 << 1,
};

enum ReqLIDRangeTypes {
    REQ_LID_TYPE_PRIORITY_LINK = 1 << 0, // 0x00..0x03
    REQ_LID_TYPE_LOCAL_LINK = 1 << 1, // 0x04..0x7f
    REQ_LID_TYPE_GLOBAL_LINK = 1 << 2, // 0x80..0xff
    REQ_LID_TYPE_ALL = REQ_LID_TYPE_GLOBAL_LINK | REQ_LID_TYPE_LOCAL_LINK | REQ_LID_TYPE_PRIORITY_LINK
};

struct cmd_cnf {
    char if_name[IF_NAMESIZE];
    uint8_t oda[6];

    uint8_t hpav_ver;

    enum ReqWhatTypes req_what;
    enum ReqLinkTypes req_link_dir;

    uint8_t nid[7];
    int lid;
    uint8_t da_sa_mac[6];

    enum ReqLIDRangeTypes lid_range_type;
    uint32_t message_gap_usec;
};


/**
 * beside the req_id there is no clean way to distinguish a link stats confirm-message
 * containing transmit statistics from one containing receive statistics
 * (at least my devolo 550+ responds always with a frame of the same size)
 */
enum LINK_STATS_REQ_TYPES {
    LINK_STATS_REQ_TYPE_TRANSMIT = 0,
    LINK_STATS_REQ_TYPE_RECEIVE
};

struct app_ctx {
    struct net_ctx *nctx;
    struct events_ctx *ectx;
    struct ieee1901_2010_ctx ieee1901_ctx;
    struct cmd_cnf conf;

    enum LINK_STATS_REQ_TYPES link_stats_req_type;
};

#define _PANIC(...) fprintf(stderr, __VA_ARGS__); exit(-1);

static error_t _parse_opt(int key, char *arg, struct argp_state *state);

static char _app_doc[] = "\nSend IEEE 1901-2010 MAC Management message type 0x0014 - CC_DISCOVER_LIST.request to selected MAC.";

static struct argp_option _cmd_options[] = {
        {"broadcast",         'b', NULL,          0, "set oda to ff:ff:ff:ff:ff:ff (default)"},
        {"localcast",         'l', NULL,          0, "set oda to Qualcomm Atheros LMA 00:b0:52:00:00:01"},
        {"oda",               'd', "DST-MAC",     0, "set oda to given mac address"},
        {"v1-0",              '0', NULL,          0, "use version homeplug av 1.0"},
        {"v1-1",              '1', NULL,          0, "use version homeplug av 1.1 (default)"},
        {"nw-stats",          'N', NULL,          0, "only request network statistics (default: req. nw and link stats)"},
        {"link-stats",        'L', NULL,          0, "only request link statistics (default: req. nw and link stats)"},
        {"link-stats-rx",     'r', NULL,          0, "only request link statistics for receive link (default: req. both)"},
        {"link-stats-tx",     't', NULL,          0, "only request link statistics for transmit link (default: req. both)"},
        {"nid",               'n', "NETZWORK-ID", 0, "network id to request the link-stats for (only required for link stats)"},
        {"lid",               'i', "LINK-ID",     0, "request link stats for given link id (default: query LIDs 0..3 (priority links))"},
        {"all-lids",          'a', NULL,          0, "query full lid range 0..255 (this may take some time)"},
        {"link-stats-da-sa",  'm', "DA-SA-MAC",   0, "destination/source MAC used for link stats query"},
        {"inter-message-gap", 'g', "USEC",        0, "GAP between hpav-messages in micro seconds (default: 200000)"},

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
    static bool nid_set = false;
    static bool da_sa_mac_set = false;
    char *end_ptr;

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
        case 'N':
            conf->req_what |= REQ_NETWORK_STATS;
            break;
        case 'L':
            conf->req_what |= REQ_LINK_STATS;
            break;
        case 'r':
            conf->req_link_dir |= REQ_RX_LINK_STATS;
            break;
        case 't':
            conf->req_link_dir |= REQ_TX_LINK_STATS;
            break;
        case 'n':
            if (hpav_parse_nid(conf->nid, arg) != EXIT_SUCCESS) {
                fprintf(stderr, "Given NID has invalid format. Expected: aa:bb:cc:dd:ee:ff:gg\n");
                argp_usage(state);
            }
            nid_set = true;
            break;
        case 'i':
            if (1 != sscanf(arg, "%d", &conf->lid)) {
                fprintf(stderr, "Given lid has invalid format. Expected a number 0..255.\n");
                argp_usage(state);
            }
            if (conf->lid < 0 || conf->lid > 255) {
                fprintf(stderr, "Given lid has invalid format. Expected a number 0..255.\n");
                argp_usage(state);
            }
            break;
        case 'm':
            if (hpav_parse_mac(conf->da_sa_mac, arg) != EXIT_SUCCESS) {
                fprintf(stderr, "Given DA/SA.MAC address has invalid format. Expected: aa:bb:cc:dd:ee:ff\n");
                argp_usage(state);
            }
            da_sa_mac_set = true;
            break;
        case 'a':
            conf->lid_range_type = REQ_LID_TYPE_ALL;
            break;
        case 'g':
            conf->message_gap_usec = strtoul(arg, &end_ptr, 10);
            if (errno == ERANGE || *end_ptr != '\0')
            {
                argp_error(state, "Failed to read parameter inter-message-gap - expected time in useconds");
            }
            break;
        case ARGP_KEY_ARG: // interface
            if (state->arg_num > 0) {
                argp_error(state, "Only one name for attribute interface allowed.\n");
            }
            strncpy(conf->if_name, arg, IF_NAMESIZE);
            break;
        case ARGP_KEY_END:
            if (state->arg_num != 1) {
                argp_error(state, "Missing attribute interface name.\n");
            }

            if (conf->req_what == REQ_ALL_STATS) {
                conf->req_what = REQ_LINK_STATS | REQ_NETWORK_STATS;
            }

            if (conf->req_link_dir == REQ_RX_TX_LINK_STATS) {
                conf->req_link_dir = REQ_RX_LINK_STATS | REQ_TX_LINK_STATS;
            }

            if ((conf->req_what & REQ_LINK_STATS) && nid_set == false) {
                argp_error(state, "Need parameter network id to run link stats query.\n");
            }

            if ((conf->req_what & REQ_LINK_STATS) && da_sa_mac_set == false) {
                argp_error(state, "Need parameter da/sa MAC address to run link stats query.\n");
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

static inline uint32_t _get_time()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (uint32_t)tv.tv_sec;
}

static void _nw_stats(uint8_t osa[6], struct ieee1901_2010_stats *stats, void *usr_data) {

    struct app_ctx *ctx = usr_data;
    static size_t i;

    printf("network_stats_%lu:\n", i++);
    printf("  ts: %u\n", _get_time());
    printf("  osa: %02x:%02x:%02x:%02x:%02x:%02x\n", osa[0], osa[1], osa[2], osa[3], osa[4], osa[5]);
    printf("  destination-address: %02x:%02x:%02x:%02x:%02x:%02x\n", stats->da[0], stats->da[1], stats->da[2],
           stats->da[3], stats->da[4], stats->da[5]);
    printf("  avg-tx-rate: %d\n", stats->avg_pgy_tx);
    printf("  avg-rx-rate: %d\n", stats->avg_pgy_rx);
}

static void _link_stats(uint8_t osa[6], struct ieee1901_2010_cm_link_stats_confirm_head *data, uint8_t *payload,
                        size_t len, void *usr) {

    struct app_ctx *ctx = usr;
    static size_t rx_ls, tx_ls;
    struct ieee1901_2010_cm_link_stats_transmit_confirm *ls_tx_confirm;
    struct ieee1901_2010_cm_link_stats_receive_confirm *ls_rx_confirm;

    if (data->rsp_type != 0x00) {
        return;
    }

    switch (ctx->link_stats_req_type) {
        case LINK_STATS_REQ_TYPE_TRANSMIT:
            if (len < sizeof(struct ieee1901_2010_cm_link_stats_transmit_confirm)) {
                return;
            }
            ls_tx_confirm = (struct ieee1901_2010_cm_link_stats_transmit_confirm *) payload;

            printf("transmit_link_stats_%lu:\n", tx_ls++);
            printf("  ts: %u\n", _get_time());
            printf("  osa: %02x:%02x:%02x:%02x:%02x:%02x\n", osa[0], osa[1], osa[2], osa[3], osa[4], osa[5]);
            printf("  req-id: 0x%02x\n", data->req_id);
            printf("  lid: 0x%02x\n", data->req_id);
            printf("  rsp-type: 0x%02x\n", data->rsp_type);

            printf("  beacon-period-cnt: %u\n", ls_tx_confirm->beacon_period_cnt);
            printf("  tx-num-msduds: %u\n", ls_tx_confirm->tx_num_msduds);
            printf("  tx-octets: %u\n", ls_tx_confirm->tx_octets);
            printf("  tx-num-segs: %u\n", ls_tx_confirm->tx_num_segs);
            printf("  tx-num-seg-suc: %u\n", ls_tx_confirm->tx_num_seg_suc);
            printf("  tx-num-seg-dropped: %u\n", ls_tx_confirm->tx_num_seg_dropped);
            printf("  tx-num-pbs: %u\n", ls_tx_confirm->tx_num_pbs);
            printf("  tx-num-mpdus: %u\n", ls_tx_confirm->tx_num_mpdus);
            printf("  tx-num-bursts: %u\n", ls_tx_confirm->tx_num_bursts);
            printf("  tx-num-sacks: %u\n", ls_tx_confirm->tx_num_sacks);
            printf("  num-lat-bins: %u\n", ls_tx_confirm->num_lat_bins);
            // TODO: evaluate latency bins...
            break;
        case LINK_STATS_REQ_TYPE_RECEIVE:
            if (len < sizeof(struct ieee1901_2010_cm_link_stats_receive_confirm)) {
                return;
            }
            ls_rx_confirm = (struct ieee1901_2010_cm_link_stats_receive_confirm *) payload;

            printf("receive_link_stats_%lu:\n", rx_ls++);
            printf("  ts: %u\n", _get_time());
            printf("  osa: %02x:%02x:%02x:%02x:%02x:%02x\n", osa[0], osa[1], osa[2], osa[3], osa[4], osa[5]);
            printf("  req-id: 0x%02x\n", data->req_id);
            printf("  lid: 0x%02x\n", data->req_id);
            printf("  rsp-type: 0x%02x\n", data->rsp_type);

            printf("  beacon-period-cnt: %u\n", ls_rx_confirm->beacon_period_cnt);
            printf("  rx-num-msduds: %u\n", ls_rx_confirm->rx_num_msduds);
            printf("  rx-octets: %u\n", ls_rx_confirm->rx_octets);
            printf("  rx-num-seg-suc: %u\n", ls_rx_confirm->rx_num_seg_suc);
            printf("  rx-num-seg-missed: %u\n", ls_rx_confirm->rx_num_seg_missed);
            printf("  rx-num-pbs: %u\n", ls_rx_confirm->rx_num_pbs);
            printf("  rx-num-bursts: %u\n", ls_rx_confirm->rx_num_bursts);
            printf("  rx-num-mpdus: %u\n", ls_rx_confirm->rx_num_mpdus);
            printf("  num-icv-fails: %u\n", ls_rx_confirm->num_icv_fails);
            break;
        default:
            return;
    }
}

static void _req_link_stats(struct app_ctx *ctx, struct ieee1901_2010_cm_link_stats_request *link_req_cfg) {
    ieee1901_2010_message_send(ctx->nctx->sock,
                               ctx->conf.oda,
                               ctx->nctx->if_cfg.sll_addr,
                               ctx->conf.hpav_ver,
                               MM_TYPE_CM_LINK_STATS,
                               MMOperationTypeRequest,
                               (uint8_t *) link_req_cfg, sizeof(*link_req_cfg));
}

int main(int argc, char **argv) {

    struct app_ctx ctx;
    struct ieee1901_2010_cm_link_stats_request cm_link_stats_request;
    uint16_t lid, lid_lim = 4;

    memset(&ctx.conf, 0, sizeof(ctx.conf));
    memcpy(ctx.conf.oda, ETHERNET_BROADCAST_ADDRESS, sizeof(ctx.conf.oda));
    ctx.conf.if_name[0] = '\0';
    ctx.conf.hpav_ver = MM_APDU_MMV_1_1;
    ctx.conf.req_what = REQ_ALL_STATS;
    ctx.conf.req_link_dir = REQ_RX_TX_LINK_STATS;
    ctx.conf.lid = -1;
    ctx.conf.lid_range_type = REQ_LID_TYPE_PRIORITY_LINK;
    ctx.conf.message_gap_usec = 200000; // worked for my devolo 550+, smaller value -> missing response (randomly)

    argp_parse(&_argp, argc, argv, 0, 0, &ctx.conf);

    if (ctx.conf.lid_range_type == REQ_LID_TYPE_ALL) {
        lid_lim = 255;
    }

    if ((ctx.ectx = events_setup()) == NULL) {
        _PANIC("Failed to create event handler.\n");
    }

    if ((ctx.nctx = net_create(ctx.conf.if_name, IEEE1901_2010_ETHERTYPE)) == NULL) {
        _PANIC("Failed to open interface %s.\n", ctx.conf.if_name);
    }

    ieee1901_2010_init(&ctx.ieee1901_ctx);

    ieee1901_2010_register_cm_nw_stats_confirm_stats_callback(&ctx.ieee1901_ctx, _nw_stats, &ctx);
    ieee1901_2010_register_cm_link_stats_confirm_callback(&ctx.ieee1901_ctx, _link_stats, &ctx);

    events_add_reader(ctx.ectx, ctx.nctx->sock, _handle_data, &ctx, NULL);

    if (ctx.conf.req_what & REQ_NETWORK_STATS) {
        // get the PHY RX/TX stats
        ieee1901_2010_message_send(ctx.nctx->sock,
                                   ctx.conf.oda,
                                   ctx.nctx->if_cfg.sll_addr,
                                   ctx.conf.hpav_ver,
                                   MM_TYPE_CM_NW_STATS,
                                   MMOperationTypeRequest,
                                   NULL, 0);
    }

    if (ctx.conf.req_what & REQ_LINK_STATS) {

        usleep(ctx.conf.message_gap_usec * 2);

        // get link stats (STA_A-MAC_A-PHY_A___LINK(s)___PHY_B-MAC_B-STA_B)

        // abuse the req-id to store the link id the stats are requested for
        // (link id is not part of the confirm message. using the req-id avoids
        // the need of creating a list containing req-id-2-link-id associations)

        cm_link_stats_request.req_type = CM_LINK_STATS_REQUEST_TYPE_STATISTIC_GET;
        memcpy(cm_link_stats_request.da_sa, ctx.conf.da_sa_mac, 6);
        memcpy(cm_link_stats_request.nid, ctx.conf.nid, 7);
        cm_link_stats_request.mgmt_flag = CM_LINK_STATS_MGMT_FLAG_NOT_MANAGEMENT_LINK;

        ctx.link_stats_req_type = LINK_STATS_REQ_TYPE_TRANSMIT;
        if (ctx.conf.req_link_dir & REQ_TX_LINK_STATS) {

            cm_link_stats_request.tl_flag = CM_LINK_STATS_TL_FLAG_TRANSMIT_LINK;

            if (ctx.conf.lid != -1) {
                cm_link_stats_request.lid = ctx.conf.lid & 0xff;
                cm_link_stats_request.req_id = cm_link_stats_request.lid;
                _req_link_stats(&ctx, &cm_link_stats_request);
            } else {
                for (lid = 0; lid <= lid_lim; lid++) {
                    cm_link_stats_request.lid = lid;
                    cm_link_stats_request.req_id = lid;
                    _req_link_stats(&ctx, &cm_link_stats_request);
                    usleep(ctx.conf.message_gap_usec);
                }
            }
        }

        if (ctx.conf.req_link_dir & REQ_RX_LINK_STATS) {

            // wait a little...
            usleep(ctx.conf.message_gap_usec * 2);

            ctx.link_stats_req_type = LINK_STATS_REQ_TYPE_RECEIVE;

            cm_link_stats_request.tl_flag = CM_LINK_STATS_TL_FLAG_RECEIVE_LINK;

            if (ctx.conf.lid != -1) {
                cm_link_stats_request.lid = ctx.conf.lid & 0xff;
                cm_link_stats_request.req_id = cm_link_stats_request.lid;
                _req_link_stats(&ctx, &cm_link_stats_request);
            } else {
                for (lid = 0; lid <= lid_lim; lid++) {
                    cm_link_stats_request.lid = lid;
                    cm_link_stats_request.req_id = lid;
                    _req_link_stats(&ctx, &cm_link_stats_request);
                    usleep(ctx.conf.message_gap_usec);
                }
            }
        }
    }

    usleep(ctx.conf.message_gap_usec * 5);

    events_teardown(ctx.ectx);
    net_destroy(ctx.nctx);

    return 0;
}
