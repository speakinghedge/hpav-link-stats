//
// Created by hecke on 3/20/16.
//

#ifndef HOMEPLUG_AV_STATS_HELPER_H
#define HOMEPLUG_AV_STATS_HELPER_H

#include <inttypes.h>

#define _PANIC(...) fprintf(stderr, __VA_ARGS__); exit(-1);

int hpav_parse_mac(uint8_t dst[6], const char* mac_str);
int hpav_parse_nid(uint8_t nid[7], const char* nid_str);

#endif //HOMEPLUG_AV_STATS_HELPER_H
