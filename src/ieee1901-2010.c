#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include "ieee1901-2010.h"

static int _parse_mm_type_cc_discover_list_confirm(struct ieee1901_2010_ctx *ctx, uint8_t *payload, size_t len);

static int _parse_mm_type_cm_nw_info_confirm(struct ieee1901_2010_ctx *ctx, uint8_t *payload, size_t len);

static int _parse_mm_type_cm_nw_stats_confirm(struct ieee1901_2010_ctx *ctx, uint8_t *payload, size_t len);

static int _parse_mm_type_cm_link_stats_confirm(struct ieee1901_2010_ctx *ctx, uint8_t *payload, size_t len);

int ieee1901_2010_init(struct ieee1901_2010_ctx *ctx) {
    if (ctx == NULL) {
        return EXIT_FAILURE;
    }

    memset(ctx, 0, sizeof(*ctx));

    return EXIT_SUCCESS;
}

int ieee1901_2010_handle_message(struct ieee1901_2010_ctx *ctx, uint8_t *payload, size_t len) {
    struct MM_APDU_Standard_Header *std_hdr;
    if (ctx == NULL || payload == NULL || len < sizeof(struct MM_APDU_Standard_Header)) {
        return EXIT_FAILURE;
    }

    std_hdr = (struct MM_APDU_Standard_Header *) payload;

    if (ntohs(std_hdr->l2.mtype) != IEEE1901_2010_ETHERTYPE) {
        return EXIT_FAILURE;
    }

    // no support for manufacturer or vendor specific messages
    if (ntohs(std_hdr->apdu_hdr.mmtype) > 0x7ffc) {
        return EXIT_FAILURE;
    }

    // no support for fragmented messages for now
    if (std_hdr->nf_mi != 0) {
        return EXIT_FAILURE;
    }

    switch ((std_hdr->apdu_hdr.mmtype)) {
        case MM_TYPE_CC_DISCOVER_LIST | MMOperationTypeConfirm:
            if (ctx->cc_discover_list_confirm_network_information_cb != NULL ||
                ctx->cc_discover_list_confirm_station_information_cb != NULL) {
                return _parse_mm_type_cc_discover_list_confirm(ctx, payload, len);
            }
            break;
        case MM_TYPE_CM_NW_INFO | MMOperationTypeConfirm:
            if (ctx->cm_nw_info_confirm_nwinfo_cb != NULL) {
                return _parse_mm_type_cm_nw_info_confirm(ctx, payload, len);
            }
            break;
        case MM_TYPE_CM_NW_STATS | MMOperationTypeConfirm:
            if (ctx->cm_nw_stats_confirm_stats_cb != NULL) {
                return _parse_mm_type_cm_nw_stats_confirm(ctx, payload, len);
            }
            break;
        case MM_TYPE_CM_LINK_STATS | MMOperationTypeConfirm:
            if (ctx->cm_link_stats_confirm_stats_cb != NULL) {
                return _parse_mm_type_cm_link_stats_confirm(ctx, payload, len);
            }
            break;
        default:
            return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

static int _parse_mm_type_cm_link_stats_confirm(struct ieee1901_2010_ctx *ctx, uint8_t *payload, size_t len) {

    struct MM_APDU_Standard_Header *std_hdr = (struct MM_APDU_Standard_Header *) payload;
    struct ieee1901_2010_cm_link_stats_confirm_head *lsch;
    int remaining_size = len - sizeof(*std_hdr);

    if (remaining_size < sizeof(*lsch)) {
        return EXIT_FAILURE;
    }

    payload = payload + sizeof(*std_hdr);
    lsch = (struct ieee1901_2010_cm_link_stats_confirm_head *) payload;
    remaining_size -= sizeof(*lsch);
    payload = payload + sizeof(*lsch);

    // this is the minimum size for an receive link stat field, transmit field needs even more...
    if (remaining_size < sizeof(struct ieee1901_2010_cm_link_stats_receive_confirm)) {
        return EXIT_FAILURE;
    }

    if (ctx->cm_link_stats_confirm_stats_cb != NULL) {
        ctx->cm_link_stats_confirm_stats_cb(std_hdr->l2.osa,
                                            lsch,
                                            payload,
                                            remaining_size,
                                            ctx->cm_link_stats_confirm_stats_cb_data);
    }

    return EXIT_SUCCESS;
}


static int _parse_mm_type_cc_discover_list_confirm(struct ieee1901_2010_ctx *ctx, uint8_t *payload, size_t len) {
    struct MM_APDU_Standard_Header *std_hdr = (struct MM_APDU_Standard_Header *) payload;
    uint8_t *asdu;
    int remaining_size = len - sizeof(*std_hdr);
    uint8_t station_count, network_count;
    size_t i;

    // one station and one network count field
    if (remaining_size < 2) {
        return EXIT_FAILURE;
    }
    payload = payload + sizeof(*std_hdr);

    station_count = *payload;
    remaining_size--;
    payload++;

    for (i = 0; i < station_count; i++) {
        if (remaining_size < sizeof(struct ieee1901_2010_station_information)) {
            return EXIT_FAILURE;
        }

        if (ctx->cc_discover_list_confirm_station_information_cb != NULL) {
            ctx->cc_discover_list_confirm_station_information_cb(std_hdr->l2.osa,
                                                                 (struct ieee1901_2010_station_information *) payload,
                                                                 ctx->cc_discover_list_confirm_station_information_cb_data);
        }

        remaining_size -= sizeof(struct ieee1901_2010_station_information);
        payload += sizeof(struct ieee1901_2010_station_information);
    }

    if (remaining_size < 1) {
        return EXIT_FAILURE;
    }

    network_count = *payload;
    remaining_size--;
    payload++;

    for (i = 0; i < network_count; i++) {
        if (remaining_size < sizeof(struct ieee1901_2010_network_information)) {
            return EXIT_FAILURE;
        }

        if (ctx->cc_discover_list_confirm_network_information_cb != NULL) {
            ctx->cc_discover_list_confirm_network_information_cb(std_hdr->l2.osa,
                                                                 (struct ieee1901_2010_network_information *) payload,
                                                                 ctx->cc_discover_list_confirm_network_information_cb_data);
        }

        remaining_size -= sizeof(struct ieee1901_2010_network_information);
        payload += sizeof(struct ieee1901_2010_network_information);
    }

    return EXIT_FAILURE;
}

static int _parse_mm_type_cm_nw_stats_confirm(struct ieee1901_2010_ctx *ctx, uint8_t *payload, size_t len) {

    struct MM_APDU_Standard_Header *std_hdr = (struct MM_APDU_Standard_Header *) payload;
    uint8_t *asdu;
    int remaining_size = len - sizeof(*std_hdr);
    uint8_t station_count;
    size_t i;

    // station count field
    if (remaining_size < 1) {
        return EXIT_FAILURE;
    }

    payload = payload + sizeof(*std_hdr);
    station_count = *payload;
    remaining_size--;
    payload++;

    for (i = 0; i < station_count; i++) {
        if (remaining_size < sizeof(struct ieee1901_2010_stats)) {
            return EXIT_FAILURE;
        }

        if (ctx->cm_nw_stats_confirm_stats_cb != NULL) {
            ctx->cm_nw_stats_confirm_stats_cb(std_hdr->l2.osa,
                                              (struct ieee1901_2010_stats *) payload,
                                              ctx->cm_nw_stats_confirm_stats_cb_data);
        }

        remaining_size -= sizeof(struct ieee1901_2010_stats);
        payload += sizeof(struct ieee1901_2010_stats);
    }

    return EXIT_SUCCESS;
}

static int _parse_mm_type_cm_nw_info_confirm(struct ieee1901_2010_ctx *ctx, uint8_t *payload, size_t len) {

    struct MM_APDU_Standard_Header *std_hdr = (struct MM_APDU_Standard_Header *) payload;
    int remaining_size = len - sizeof(*std_hdr);
    uint8_t network_count;
    size_t i;

    // nwinfo count field
    if (remaining_size < 1) {
        return EXIT_FAILURE;
    }

    payload = payload + sizeof(*std_hdr);
    network_count = *payload;
    remaining_size--;
    payload++;

    for (i = 0; i < network_count; i++) {
        if (remaining_size < sizeof(struct ieee1901_2010_nwinfo)) {
            return EXIT_FAILURE;
        }

        if (ctx->cm_nw_info_confirm_nwinfo_cb != NULL) {
            ctx->cm_nw_info_confirm_nwinfo_cb(std_hdr->l2.osa,
                                              (struct ieee1901_2010_nwinfo *) payload,
                                              ctx->cm_nw_info_confirm_nwinfo_cb_data);
        }

        remaining_size -= sizeof(struct ieee1901_2010_nwinfo);
        payload += sizeof(struct ieee1901_2010_nwinfo);
    }

    return EXIT_SUCCESS;
}

int ieee1901_2010_register_cc_discover_list_confirm_station_information_callback(struct ieee1901_2010_ctx *ctx,
                                                                                 void (*cb)(uint8_t osa[6],
                                                                                            struct ieee1901_2010_station_information *data,
                                                                                            void *usr),
                                                                                 void *usr_data) {
    if (ctx == NULL) {
        return EXIT_FAILURE;
    }

    ctx->cc_discover_list_confirm_station_information_cb = cb;
    ctx->cc_discover_list_confirm_station_information_cb_data = usr_data;

    return EXIT_SUCCESS;
}

int ieee1901_2010_register_cc_discover_list_confirm_network_information_callback(struct ieee1901_2010_ctx *ctx,
                                                                                 void (*cb)(uint8_t osa[6],
                                                                                            struct ieee1901_2010_network_information *data,
                                                                                            void *usr),
                                                                                 void *usr_data) {
    if (ctx == NULL) {
        return EXIT_FAILURE;
    }

    ctx->cc_discover_list_confirm_network_information_cb = cb;
    ctx->cc_discover_list_confirm_network_information_cb_data = usr_data;

    return EXIT_SUCCESS;
}

int ieee1901_2010_register_cm_nw_info_confirm_nwinfo_callback(struct ieee1901_2010_ctx *ctx, void (*cb)(uint8_t osa[6],
                                                                                                        struct ieee1901_2010_nwinfo *data,
                                                                                                        void *usr),
                                                              void *usr_data) {
    if (ctx == NULL) {
        return EXIT_FAILURE;
    }

    ctx->cm_nw_info_confirm_nwinfo_cb = cb;
    ctx->cm_nw_info_confirm_nwinfo_cb_data = usr_data;

    return EXIT_SUCCESS;
}

int ieee1901_2010_register_cm_nw_stats_confirm_stats_callback(struct ieee1901_2010_ctx *ctx, void (*cb)(uint8_t osa[6],
                                                                                                        struct ieee1901_2010_stats *data,
                                                                                                        void *usr),
                                                              void *usr_data) {
    if (ctx == NULL) {
        return EXIT_FAILURE;
    }

    ctx->cm_nw_stats_confirm_stats_cb = cb;
    ctx->cm_nw_stats_confirm_stats_cb_data = usr_data;

    return EXIT_SUCCESS;
}

int ieee1901_2010_register_cm_link_stats_confirm_callback(struct ieee1901_2010_ctx *ctx, void (*cb)(uint8_t osa[6],
                                                                                                    struct ieee1901_2010_cm_link_stats_confirm_head *data,
                                                                                                    uint8_t *payload,
                                                                                                    size_t len,
                                                                                                    void *usr),
                                                          void *usr_data) {
    if (ctx == NULL) {
        return EXIT_FAILURE;
    }

    ctx->cm_link_stats_confirm_stats_cb = cb;
    ctx->cm_link_stats_confirm_stats_cb_data = usr_data;

    return EXIT_SUCCESS;
}

int ieee1901_2010_message_send(int fd,
                               uint8_t oda[6], uint8_t osa[6],
                               uint8_t mmv,
                               uint16_t type, enum MMOperationTypes oper,
                               uint8_t *payload, size_t len) {

    struct MM_APDU_Standard_Header *hdr;
    uint8_t frame[IEEE1901_2010_MIN_MSG_SIZE];
    uint8_t pad[IEEE1901_2010_MIN_MSG_SIZE];
    size_t apdu_len;
    size_t iov_cnt;
    struct iovec iov[3];

    hdr = (struct MM_APDU_Standard_Header *) frame;

    memset(frame, 0, IEEE1901_2010_MIN_MSG_SIZE);
    memset(pad, 0, IEEE1901_2010_MIN_MSG_SIZE);

    memcpy(hdr->l2.oda, oda, sizeof(hdr->l2.oda));
    memcpy(hdr->l2.osa, osa, sizeof(hdr->l2.osa));
    hdr->l2.mtype = htons(IEEE1901_2010_ETHERTYPE);

    hdr->apdu_hdr.mmv = mmv;
    hdr->apdu_hdr.mmtype = type | oper;

    hdr->fmsn = 0;
    hdr->fn_mi = 0;
    hdr->nf_mi = 0;

    if (len == 0) {
        return write(fd, frame, IEEE1901_2010_MIN_MSG_SIZE);
    }

    apdu_len = sizeof(*hdr);

    if (payload == NULL) {
        return -EINVAL;
    }

    if (len > (1500 - sizeof(*hdr))) {
        return -EINVAL;
    }

    iov[0].iov_base = hdr;
    iov[0].iov_len = sizeof(*hdr);

    iov[1].iov_base = payload;
    iov[1].iov_len = len;

    apdu_len += len;
    iov_cnt = 2;

    if (apdu_len < IEEE1901_2010_MIN_MSG_SIZE) {
        iov[2].iov_len = IEEE1901_2010_MIN_MSG_SIZE - apdu_len;
        memset(pad, 0, iov[2].iov_len);
        iov[2].iov_base = pad;
        iov_cnt++;
    }

    return writev(fd, iov, iov_cnt);
}

int ieee1901_2010_vendor_message_send(int fd,
                                      uint8_t oda[6], uint8_t osa[6],
                                      uint8_t mmv,
                                      uint16_t type, enum MMOperationTypes oper,
                                      uint8_t oui[3],
                                      uint8_t *payload, size_t len) {
    /* TODO */
    return 0;
}