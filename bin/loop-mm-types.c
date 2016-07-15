//
// Created by hecke on 3/13/16.
//
// purpose: find standard messages (defined in ieee 1901-2010) my devolo 550+ responds to
// (avoid the use of Atheros specific extensions >= 0xa000)
//
// Send MAC Management message types defined in IEEE 1901-2010 via given interface.
// (payload set to NULL)
//
// WARNING: this is just for testing and may confuse the powerline adapters.
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

static error_t _parse_opt (int key, char *arg, struct argp_state *state);

static char _app_doc[] = "\nSend MAC Management message types defined in IEEE 1901-2010 via given interface.";

static struct argp_option _cmd_options[] = {
        {"broadcast", 'b', NULL, 0, "set oda to ff:ff:ff:ff:ff:ff (default)"},
        {"localcast", 'l', NULL, 0, "set oda to Qualcomm Atheros LMA 00:b0:52:00:00:01"},
        {"oda", 'd', "DST-MAC", 0, "set oda to given mac address"},
        {"v1-0", '0', NULL, 0, "use version homeplug av 1.0"},
        {"v1-1", '1', NULL, 0, "use version homeplug av 1.1 (default)"},
        {"request", 'r', NULL, 0, "send request messages (default)"},
        {"response", 'R', NULL, 0, "send response messages"},
        {"indication", 'i', NULL, 0, "send indication messages"},
        {"confirm", 'c', NULL, 0, "send confirm messages"},
        {"verbose-send", 'v', NULL, 0, "show send messages types and operations"},
        { 0 }
};

static char _args_doc[] = "interface";

static struct argp _argp = {
        _cmd_options,
        _parse_opt,
        _args_doc,
        _app_doc
};

struct cmd_cnf {
    char if_name[IF_NAMESIZE];
    uint8_t oda[6];

    uint8_t hpav_ver;

    u_int32_t msg_type_cnt[MMOperationType_EOF];

    bool show_tx_info;
};

static error_t _parse_opt (int key, char *arg, struct argp_state *state)
{
    struct cmd_cnf *conf= state->input;

    switch(key)
    {
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
            if (hpav_parse_mac(conf->oda, arg) != EXIT_SUCCESS)
            {
                fprintf(stderr, "Given MAC address has invalid format. Expected: aa:bb:cc:dd:ee:ff\n");
                argp_usage (state);
            }
            break;
        case 'r':
            conf->msg_type_cnt[MMOperationTypeRequest] += 2;
            break;
        case 'R':
            if (conf->msg_type_cnt[MMOperationTypeRequest] == 1)
            {
                conf->msg_type_cnt[MMOperationTypeRequest] = 0;
            }
            conf->msg_type_cnt[MMOperationTypeResponse]++;
            break;
        case 'i':
            if (conf->msg_type_cnt[MMOperationTypeRequest] == 1)
            {
                conf->msg_type_cnt[MMOperationTypeRequest] = 0;
            }
            conf->msg_type_cnt[MMOperationTypeIndication]++;
            break;
        case 'c':
            if (conf->msg_type_cnt[MMOperationTypeRequest] == 1)
            {
                conf->msg_type_cnt[MMOperationTypeRequest] = 0;
            }
            conf->msg_type_cnt[MMOperationTypeConfirm]++;
            break;
        case 'v':
            conf->show_tx_info = true;
            break;
        case ARGP_KEY_ARG: // interface
            if (state->arg_num > 0) {
                fprintf(stderr, "Only one name for attribute interface allowed.\n");
                argp_usage (state);
            }
            strncpy(conf->if_name, arg, IF_NAMESIZE);
            break;
        case ARGP_KEY_END:
            if(state->arg_num != 1)
            {
                fprintf(stderr, "Missing attribute interface name.\n");
                argp_usage (state);
            }
            break;
    }

    return 0;
}

int main(int argc, char **argv)
{
    struct net_ctx *nctx;
    struct events_ctx *ectx;

    size_t i;
    size_t oper;

    struct cmd_cnf conf;

    memset(&conf, 0, sizeof(conf));
    memcpy(conf.oda, ETHERNET_BROADCAST_ADDRESS, sizeof(conf.oda));
    conf.if_name[0] = '\0';
    conf.hpav_ver = MM_APDU_MMV_1_1;
    conf.msg_type_cnt[MMOperationTypeRequest] = 1;
    conf.show_tx_info = false;

    argp_parse (&_argp, argc, argv, 0, 0, &conf);

    if ((ectx = events_setup()) == NULL)
    {
        _PANIC("Failed to create event handler.\n");
    }

    if ((nctx = net_create(conf.if_name, IEEE1901_2010_ETHERTYPE)) == NULL)
    {
        _PANIC("Failed to open interface %s.\n", conf.if_name);
    }

    for(i = 0; i < MM_TYPE_COUNT; i++)
    {
        if (conf.show_tx_info == true) {
            printf("%s\n", MMTypes[i].name);
        }

        for (oper = 0; oper < MMOperationType_EOF; oper++)
        {
            if (MMTypes[i].ops & (1 << oper) && conf.msg_type_cnt[oper] > 0)
            {
                if (conf.show_tx_info == true) {
                    printf("\t%s\n", hpav_mm_operation_type_name_get(oper));
                }

                ieee1901_2010_message_send(nctx->sock,
                                           conf.oda,
                                           nctx->if_cfg.sll_addr,
                                           conf.hpav_ver,
                                           MMTypes[i].type_base,
                                           oper,
                                           NULL,0);
                usleep(100000);
            }
        }
    }

    events_teardown(ectx);
    net_destroy(nctx);

    return 0;
}