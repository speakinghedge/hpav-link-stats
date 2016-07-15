//
// Created by hecke on 3/20/16.
//
#include <regex.h>

#include "helper.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int hpav_parse_mac(uint8_t dst[6], const char* mac_str)
{
    regex_t r;
    int in[6];

    if (mac_str == NULL)
    {
        return EXIT_FAILURE;
    }

    if(regcomp(&r, "^([0-9A-Fa-f]{2}[:]{1}){5}[0-9A-Fa-f]{2}$", REG_EXTENDED) != 0)
    {
        return -EINVAL;
    }

    if (regexec (&r, mac_str, 0, NULL, 0) != 0)
    {
        regfree(&r);
        return -EINVAL;
    }

    regfree(&r);

    if (6 != sscanf(mac_str, "%x:%x:%x:%x:%x:%x", &in[0], &in[1], &in[2], &in[3], &in[4], &in[5]))
    {
        return EXIT_FAILURE;
    }

    dst[0] = in[0];
    dst[1] = in[1];
    dst[2] = in[2];
    dst[3] = in[3];
    dst[4] = in[4];
    dst[5] = in[5];

    return EXIT_SUCCESS;
}

int hpav_parse_nid(uint8_t nid[7], const char* nid_str)
{
    regex_t r;
    int in[7];

    if (nid_str == NULL)
    {
        return EXIT_FAILURE;
    }

    if(regcomp(&r, "^([0-9A-Fa-f]{2}[:]{1}){6}[0-9A-Fa-f]{2}$", REG_EXTENDED) != 0)
    {
        return -EINVAL;
    }

    if (regexec (&r, nid_str, 0, NULL, 0) != 0)
    {
        regfree(&r);
        return -EINVAL;
    }

    regfree(&r);

    if (7 != sscanf(nid_str, "%x:%x:%x:%x:%x:%x:%x", &in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6]))
    {
        return EXIT_FAILURE;
    }

    nid[0] = in[0];
    nid[1] = in[1];
    nid[2] = in[2];
    nid[3] = in[3];
    nid[4] = in[4];
    nid[5] = in[5];
    nid[6] = in[6];

    return EXIT_SUCCESS;
}