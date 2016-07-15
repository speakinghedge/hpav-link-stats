#ifndef _IEEE1901_2010_H
#define _IEEE1901_2010_H

#include <inttypes.h>
#include <stdlib.h>

// ieee 1901-2010, tab. 6-58 (Interpretation of Two LSBs of MMTYPE)
enum MMOperationTypes {
	
	MMOperationTypeRequest 		= 0x0000,
	MMOperationTypeConfirm 		= 0x0001,
	MMOperationTypeIndication 	= 0x0002,
	MMOperationTypeResponse 	= 0x0003,

	MMOperationType_EOF
};

#define MMOperationTypeMask MMOperationTypeResponse

static const char *MMOperationTypeNames[MMOperationType_EOF] = {
	[MMOperationTypeRequest] 	= "request",
	[MMOperationTypeConfirm] 	= "confirm",
	[MMOperationTypeIndication] = "indication",
	[MMOperationTypeResponse] 	= "response"
};

static inline const char* hpav_mm_operation_type_name_get(uint16_t type)
{
	if ((type & MMOperationTypeResponse) >= MMOperationType_EOF) {
		return NULL;
	}

	return MMOperationTypeNames[type & MMOperationTypeResponse];
}

enum MMOperationTypeFlags {
	MMOperationTypeFlagRequest 		= 1 << MMOperationTypeRequest,
	MMOperationTypeFlagFlagConfirm 	= 1 << MMOperationTypeConfirm,
	MMOperationTypeFlagIndication 	= 1 << MMOperationTypeIndication,
	MMOperationTypeFlagResponse 	= 1 << MMOperationTypeResponse
};

// ieee 1901-2010, tab. 6-59 (Interpretation of Three MSBs of MMTYPE)
enum MMCategoryMasks {
	MMCategorySTA2BSSManager 		= 0x0000,
	MMCategoryProxy2BSSManager 		= 0x2000,
	MMCategoryBSSManager2BSSManager = 0x4000,
	MMCategorySTA2STA 				= 0x6000,
	MMCategoryManufactorSpecific 	= 0x8000,
	MMCategoryVendorSpecific 		= 0xa000,
	MMCategoryReserved 				= 0xc000,
};

#define MMCategoryMask 0xe000

static inline const char* hpav_mm_category_name_get(uint16_t type)
{
	uint16_t cat = type & MMCategoryMask;

	switch(cat)
	{
	case MMCategorySTA2BSSManager:
		return "STA2BSSManager";
	case MMCategoryProxy2BSSManager:
		return "Proxy2BSSManager";
	case MMCategoryBSSManager2BSSManager:
		return "BSSManager2BSSManager";
	case MMCategorySTA2STA:
		return "STA2STA";
	case MMCategoryManufactorSpecific:
		return "ManufactorSpecific";
	case MMCategoryVendorSpecific:
		return "VendorSpecific";
	default:
		return "reserved";
	}
}

// ieee1901-2010 mm types as defined in tab. 6-61
#define MM_TYPE_CC_BM_APPOINT 				0x0000
#define MM_TYPE_CC_BACKUP_APPOINT 			0x0004
#define MM_TYPE_CC_LINK_INFO 				0x0008
#define MM_TYPE_CC_HANDOVER 				0x000C
#define MM_TYPE_CC_HANDOVER_INFO 			0x0010
#define MM_TYPE_CC_DISCOVER_LIST 			0x0014
#define MM_TYPE_CC_LINK_NEW 				0x0018
#define MM_TYPE_CC_LINK_MOD 				0x001C
#define MM_TYPE_CC_LINK_SQZ 				0x0020
#define MM_TYPE_CC_LINK_REL 				0x0024
#define MM_TYPE_CC_DETECT_REPORT 			0x0028
#define MM_TYPE_CC_WHO_RU 					0x002C
#define MM_TYPE_CC_ASSOC 					0x0030
#define MM_TYPE_CC_LEAVE 					0x0034
#define MM_TYPE_CC_SET_TEI_MAP 				0x0038
#define MM_TYPE_CC_RELAY 					0x003C
#define MM_TYPE_CC_BEACON_RELIABILITY 		0x0040
#define MM_TYPE_CC_ALLOC_MOVE 				0x0044
#define MM_TYPE_CC_ACCESS_NEW 				0x0048
#define MM_TYPE_CC_ACCESS_REL 				0x004C
#define MM_TYPE_CC_DCPPC 					0x0050
#define MM_TYPE_CC_HP1_DET 					0x0054
#define MM_TYPE_CC_BLE_UPDATE 				0x0058
#define MM_TYPE_CC_BCAST_REPEAT 			0x005C
#define MM_TYPE_CC_MH_LINK_NEW 				0x0060
#define MM_TYPE_CC_ISP_DetectionReport 		0x0064
#define MM_TYPE_CC_ISP_StartReSync 			0x0068
#define MM_TYPE_CC_ISP_FinishReSync 		0x006C
#define MM_TYPE_CC_ISP_ReSyncDetected 		0x0070
#define MM_TYPE_CC_ISP_ReSyncTransmission 	0x0074
#define MM_TYPE_CP_PROXY_APPOINT 			0x2000
#define MM_TYPE_PH_PROXY_APPOINT 			0x2004
#define MM_TYPE_CP_PROXY_WAKE 				0x2008
#define MM_TYPE_NN_INL 						0x4000
#define MM_TYPE_NN_NEW_NET 					0x4004
#define MM_TYPE_NN_ADD_ALLOC 				0x4008
#define MM_TYPE_NN_REL_ALLOC 				0x400C
#define MM_TYPE_NN_REL_NET 					0x4010
#define MM_TYPE_CM_UNASSOCIATED_STA 		0x6000
#define MM_TYPE_CM_ENCRYPTED_PAYLOAD 		0x6004
#define MM_TYPE_CM_SET_KEY 					0x6008
#define MM_TYPE_CM_GET_KEY 					0x600C
#define MM_TYPE_CM_SC_JOIN 					0x6010
#define MM_TYPE_CM_CHAN_EST 				0x6014
#define MM_TYPE_CM_TM_UPDATE 				0x6018
#define MM_TYPE_CM_AMP_MAP 					0x601C
#define MM_TYPE_CM_BRG_INFO 				0x6020
#define MM_TYPE_CM_CONN_NEW 				0x6024
#define MM_TYPE_CM_CONN_REL 				0x6028
#define MM_TYPE_CM_CONN_MOD 				0x602C
#define MM_TYPE_CM_CONN_INFO 				0x6030
#define MM_TYPE_CM_STA_CAP 					0x6034
#define MM_TYPE_CM_NW_INFO 					0x6038
#define MM_TYPE_CM_GET_BEACON 				0x603C
#define MM_TYPE_CM_HFID 					0x6040
#define MM_TYPE_CM_MME_ERROR 				0x6044
#define MM_TYPE_CM_NW_STATS 				0x6048
#define MM_TYPE_CM_LINK_STATS 				0x604C
#define MM_TYPE_CM_ROUTE_INFO 				0x6050
#define MM_TYPE_CM_UNREACHABLE 				0x6054
#define MM_TYPE_CM_MH_CONN_NEW 				0x6058
#define MM_TYPE_CM_EXTENDEDTONEMASK 		0x605C

struct MMType {
	uint16_t type_base;	// !< base value of the management message type
	char name[32]; // !< message type name as given by the standard}

	enum MMOperationTypeFlags ops; // !< operations supported by the MMType
};

static struct MMType MMTypes[] = {
	/* station - bm */
	{MM_TYPE_CC_BM_APPOINT, "CC_BM_APPOINT", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_BACKUP_APPOINT, "CC_BACKUP_APPOINT", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_LINK_INFO, "CC_LINK_INFO", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm | MMOperationTypeFlagIndication | MMOperationTypeFlagResponse},
	{MM_TYPE_CC_HANDOVER, "CC_HANDOVER", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_HANDOVER_INFO, "CC_HANDOVER_INFO", MMOperationTypeFlagIndication | MMOperationTypeFlagResponse},
	{MM_TYPE_CC_DISCOVER_LIST, "CC_DISCOVER_LIST", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm | MMOperationTypeFlagIndication},
	{MM_TYPE_CC_LINK_NEW, "CC_LINK_NEW", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_LINK_MOD, "CC_LINK_MOD", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_LINK_SQZ, "CC_LINK_SQZ", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_LINK_REL, "CC_LINK_REL", MMOperationTypeFlagRequest | MMOperationTypeFlagIndication},
	{MM_TYPE_CC_DETECT_REPORT, "CC_DETECT_REPORT", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_WHO_RU, "CC_WHO_RU", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_ASSOC, "CC_ASSOC", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_LEAVE, "CC_LEAVE", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm | MMOperationTypeFlagIndication | MMOperationTypeFlagResponse},
	{MM_TYPE_CC_SET_TEI_MAP, "CC_SET_TEI_MAP", MMOperationTypeFlagRequest | MMOperationTypeFlagIndication},
	{MM_TYPE_CC_RELAY, "CC_RELAY", MMOperationTypeFlagRequest | MMOperationTypeFlagIndication},
	{MM_TYPE_CC_BEACON_RELIABILITY, "CC_BEACON_RELIABILITY", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_ALLOC_MOVE, "CC_ALLOC_MOVE", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_ACCESS_NEW, "CC_ACCESS_NEW", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm | MMOperationTypeFlagIndication | MMOperationTypeFlagResponse},
	{MM_TYPE_CC_ACCESS_REL, "CC_ACCESS_REL", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm | MMOperationTypeFlagIndication | MMOperationTypeFlagResponse},
	{MM_TYPE_CC_DCPPC, "CC_DCPPC", MMOperationTypeFlagIndication | MMOperationTypeFlagResponse},
	{MM_TYPE_CC_HP1_DET, "CC_HP1_DET", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_BLE_UPDATE, "CC_BLE_UPDATE", MMOperationTypeFlagIndication},
	{MM_TYPE_CC_BCAST_REPEAT, "CC_BCAST_REPEAT", MMOperationTypeFlagIndication | MMOperationTypeFlagResponse},
	{MM_TYPE_CC_MH_LINK_NEW, "CC_MH_LINK_NEW", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CC_ISP_DetectionReport, "CC_ISP_DetectionReport", MMOperationTypeFlagIndication},
	{MM_TYPE_CC_ISP_StartReSync, "CC_ISP_StartReSync", MMOperationTypeFlagRequest},
	{MM_TYPE_CC_ISP_FinishReSync, "CC_ISP_FinishReSync", MMOperationTypeFlagRequest},
	{MM_TYPE_CC_ISP_ReSyncDetected, "CC_ISP_ReSyncDetected", MMOperationTypeFlagIndication},
	{MM_TYPE_CC_ISP_ReSyncTransmission, "CC_ISP_ReSyncTransmission", MMOperationTypeFlagRequest},
	/* proxy - bss manager */
	{MM_TYPE_CP_PROXY_APPOINT, "CP_PROXY_APPOINT", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_PH_PROXY_APPOINT, "PH_PROXY_APPOINT", MMOperationTypeFlagIndication},
	{MM_TYPE_CP_PROXY_WAKE, "CP_PROXY_WAKE", MMOperationTypeFlagRequest},
	/* bm - bm */
	{MM_TYPE_NN_INL, "NN_INL", MMOperationTypeFlagRequest},
	{MM_TYPE_NN_NEW_NET, "NN_NEW_NET", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm | MMOperationTypeFlagIndication},
	{MM_TYPE_NN_ADD_ALLOC, "NN_ADD_ALLOC", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm | MMOperationTypeFlagIndication},
	{MM_TYPE_NN_REL_ALLOC, "NN_REL_ALLOC", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_NN_REL_NET, "NN_REL_NET", MMOperationTypeFlagIndication},
	/* station - station */
	{MM_TYPE_CM_UNASSOCIATED_STA, "CM_UNASSOCIATED_STA", MMOperationTypeFlagIndication},
	{MM_TYPE_CM_ENCRYPTED_PAYLOAD, "CM_ENCRYPTED_PAYLOAD", MMOperationTypeFlagIndication | MMOperationTypeFlagResponse},
	{MM_TYPE_CM_SET_KEY, "CM_SET_KEY", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_GET_KEY, "CM_GET_KEY", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_SC_JOIN, "CM_SC_JOIN", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_CHAN_EST, "CM_CHAN_EST", MMOperationTypeFlagIndication},
	{MM_TYPE_CM_TM_UPDATE, "CM_TM_UPDATE", MMOperationTypeFlagIndication},
	{MM_TYPE_CM_AMP_MAP, "CM_AMP_MAP", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_BRG_INFO, "CM_BRG_INFO", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_CONN_NEW, "CM_CONN_NEW", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_CONN_REL, "CM_CONN_REL", MMOperationTypeFlagIndication | MMOperationTypeFlagResponse},
	{MM_TYPE_CM_CONN_MOD, "CM_CONN_MOD", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_CONN_INFO, "CM_CONN_INFO", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_STA_CAP, "CM_STA_CAP", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_NW_INFO, "CM_NW_INFO", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_GET_BEACON, "CM_GET_BEACON", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_HFID, "CM_HFID", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_MME_ERROR, "CM_MME_ERROR", MMOperationTypeFlagIndication},
	{MM_TYPE_CM_NW_STATS, "CM_NW_STATS", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_LINK_STATS, "CM_LINK_STATS", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_ROUTE_INFO, "CM_ROUTE_INFO", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm | MMOperationTypeFlagIndication},
	{MM_TYPE_CM_UNREACHABLE, "CM_UNREACHABLE", MMOperationTypeFlagIndication},
	{MM_TYPE_CM_MH_CONN_NEW, "CM_MH_CONN_NEW", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm},
	{MM_TYPE_CM_EXTENDEDTONEMASK, "CM_EXTENDEDTONEMASK", MMOperationTypeFlagRequest | MMOperationTypeFlagFlagConfirm}
};

#define MM_TYPE_COUNT 	(sizeof(MMTypes) / sizeof(struct MMType))

#define IEEE1901_2010_MIN_MSG_SIZE	60
#define IEEE1901_2010_ETHERTYPE		0x88e1

struct MM_L2_Header
{
	uint8_t oda[6]; // !< destination MAC address (original destination address)
	uint8_t osa[6]; // !< source MAC address (original source address)

	uint16_t mtype; // !< ether type (0x88e1)
}__attribute((packed));

#if 0
static inline void ieee1901_2010_print_mac(uint8_t mac[6])
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
#endif

#define MM_APDU_MMV_1_0			0x00
#define MM_APDU_MMV_1_1			0x01	// !< ieee1901-2010:6.4.1.2.3.5 (p. 127): use 0x01 for in-home FTT MME
#define MM_APDU_MMV_ACCESS_MME 	0x80

struct MM_APDU_Header
{
	uint8_t mmv; // !< management message version

	uint16_t mmtype; // !< management message type
}__attribute((packed));

/* if the category of a mm falls in 
 * - STA2BSSManager,
 * - Proxy2BSSManager,
 * - BSSManager2BSSManager,
 * - STA2STA or
 * - ManufactorSpecific
 * the following header is used:
 */
struct MM_APDU_Standard_Header
{
	struct MM_L2_Header		l2;
	struct MM_APDU_Header 	apdu_hdr;

	uint8_t nf_mi:4; // !< number of fragments
	uint8_t fn_mi:4; // !< fragment number

	uint8_t fmsn; // !< fragmentation message sequence number

	// uint8_t payload[0];
}__attribute((packed));

/* this header is used for vendor specific mm */
struct MM_APDU_Vendor_Header
{

	struct MM_L2_Header		l2;
	struct MM_APDU_Header 	apdu_hdr;

	uint8_t oui[3];	// !< organizationally unique identifier

	// uint8_t payload[0];
}__attribute((packed));

int ieee1901_2010_message_send(int fd,
							   uint8_t oda[6], uint8_t osa[6], 
							   uint8_t mmv, 
							   uint16_t type, enum MMOperationTypes oper, 
							   uint8_t *payload, size_t len);

int ieee1901_2010_vendor_message_send(int fd,
									  uint8_t oda[6], uint8_t osa[6], 
									  uint8_t mmv, 
									  uint16_t type, enum MMOperationTypes oper, 
									  uint8_t oui[3],
									  uint8_t *payload, size_t len);

// ieee1901-2010 - tab. 6-173 - format of station info (used in CC_DISCOVER_LIST.confirm)
struct ieee1901_2010_station_information {

	uint8_t  mac_addr[6]; // !< MAC addr of the discovered STA
	uint8_t  tei;	// !< TEI of the discovered STA
	uint8_t  same_network; // !< 0x00 - the discovered STA is associated with a different network, 0x01 - the discovered STA is associated with same network, 0x02-0xff reserverd
	uint8_t snid:4; // !< short network id
	uint8_t access:4;  // !< 0x00 - in-home network, 0x01 - access network
	uint8_t reserved:1;
	uint8_t bm_capability:2; // !<
	uint8_t proxy_networking_capability:1; // !<
	uint8_t backup_bm_capability:1; // !<
	uint8_t bm_status:1; // !<
	uint8_t pbm_status:1; // !<
	uint8_t backup_bm_status:1; // !<
	uint8_t signal_level; // !<
	uint8_t average_ble; // !<

} __attribute__((packed));

// ieee1901-2010 - tab. 6-174 - format of network info (used in CC_DISCOVER_LIST.confirm)
struct ieee1901_2010_network_information {

	uint8_t  nid[7]; // !< network identifier
	uint8_t snid:4; // !< short network id
	uint8_t access:4;  // !< 0x00 - in-home network, 0x01 - access network
	uint8_t hm; // !< two LSB -> FFT hybrid mode of the BBS
	uint8_t num_slots; // !< number of beacon slots 0x08-0xff reserved
	uint8_t coordinating_status; // !< coordinating status of the BM
	uint16_t offset; // !< between beacon region of the discovered network and the beacon region of the STAs own network

} __attribute__((packed));

// ieee1901-2010 - tab. 6-290 - format of network info (used in CM_NW_INFO.confirm)
struct ieee1901_2010_nwinfo {

	uint8_t  nid[7]; // !< network identifier
	uint8_t snid:4; // !< short network id
	uint8_t tei; // !< terminal equipment ID
	uint8_t station_role; // !< 0x00 - STA, 0x01 - proxy BBS manager, 0x02 - BM, 0x03..0xff - reserved
	uint8_t bm_mac[6]; // !< MAC address of the BM of the network
	uint8_t access; // !< type of the network, 0x00 - in home, 0x01 - access network, 0x02..0xff - reserved
	uint8_t num_cord_nws; // !< number of coordinated networks, 0x00 - none, 0x01 - one, 0x02 - two, .. 0xff - 255

} __attribute__((packed));

// ieee1901-2010 - tab. 6-295 - format of CM_NW_STATS.confirm
struct ieee1901_2010_stats {
	uint8_t da[6]; // !< destination (MAC) address
	uint8_t avg_pgy_tx; // !< average PHY TX data rate in MBit/s, 0x00 - unknown
	uint8_t avg_pgy_rx; // !< average PHY RX data rate in MBit/s, 0x00 - unknown
} __attribute__((packed));

static inline const char * ieee1901_2010_same_network_string(uint8_t type)
{
	switch(type)
	{
		case 0x00:
			return "no";
		case 0x01:
			return "yes";
		default:
			return "reserved";
	}
}

static inline const char * ieee1901_2010_station_role_string(uint8_t type)
{
	switch(type)
	{
		case 0x00:
			return "station";
		case 0x01:
			return "proxy BBS manager";
		case 0x02:
			return "basic service set manager";
		default:
			return "reserved";
	}
}

static inline const char *ieee1901_2010_network_type_string(uint8_t type)
{
	switch(type)
	{
		case 0x00:
			return "in home";
		case 0x01:
			return "access network";
		default:
			return "reserved";
	}
}

// ieee1901-2010 - tab. 6-296 - format of CM_LINK_STATS.request
// in 10.6.1.1.4.1 - link identifiers
struct ieee1901_2010_cm_link_stats_request
{
	uint8_t req_type;	// !< request type, 0x00 - reset stats, 0x01 - get stats, 0x02 get and reset stats, 0x03..0xff - reserved
	uint8_t req_id;	// !< request ID - unique identifier used for request and response
	uint8_t nid[7]; // !< network identifier of the STA(s) the stats are requested for
	uint8_t lid;	// !< link identifier, only valid if mgmt_flag is set to 0x00 (not management link)
	uint8_t tl_flag;	// !< transmit link flag, 0x00 - transmit link, 0x01 - receive link, 0x02..0xff - reserved
	uint8_t mgmt_flag;	// !< management link, 0x00 - not management link, 0x01 - management link, 0x02..0xff - reserved
	uint8_t da_sa[6];	// !< if tl_flag = 0x00 (tx link) - destination MAC, tl_flag = 0x01 (rx link) - source MAC

} __attribute__((packed));

// ieee1901-2010 - tab. 6-297 - format of CM_LINK_STATS.confirm head (same for transmit and receive MFS)
struct ieee1901_2010_cm_link_stats_confirm_head
{
	uint8_t req_id;	// !< request ID - unique identifier used for request and response
	uint8_t rsp_type;	// !< reponse type, 0x00 - success, 0x01 - failure, 0x02..0xff - reserved

} __attribute__((packed));

// ieee1901-2010 - tab. 6-298 - link stats field format for transmit
struct ieee1901_2010_cm_link_stats_transmit_confirm
{
	uint16_t beacon_period_cnt; // !< number of beacon periods used to aggregate statistic data
	uint32_t tx_num_msduds; // !< number of MSDUs (MAC Service Data Units) received from HLE
	uint32_t tx_octets; // !< number of octets of MSDU payload received from HLE
	uint32_t tx_num_segs; // !< number of generated segments
	uint32_t tx_num_seg_suc; // !< number of segments successfully delivered
	uint32_t tx_num_seg_dropped; // !< number of segments dropped
	uint32_t tx_num_pbs; // !< number of PBs (PHY Blocks) handed over to the PHY for transmission
	uint32_t tx_num_mpdus; // !< number of MPDUs (MAC Protocol Data Unit) transmitted
	uint32_t tx_num_bursts; // !< number of transmitted bursts
	uint32_t tx_num_sacks; // !< number of MPDUs that were successfully acknowledged
	uint8_t num_lat_bins; // !< number of bins in which latency information is collected

} __attribute__((packed));

// if num_lat_bins > 0 -> this is the 'payload' of struct ieee1901_2010_cm_link_stats_transmit_confirm
struct ieee1901_2010_cm_link_stats_transmit_confirm_lat_bins
{
	uint8_t lat_bin_gran; // !< granularity of latency bin in beacon periods
	uint32_t lat_bin[255];
} __attribute__((packed));

// ieee1901-2010 - tab. 6-298 - link stats field format for receive
struct ieee1901_2010_cm_link_stats_receive_confirm
{
	uint16_t beacon_period_cnt; // !< number of beacon periods used to aggregate statistic data
	uint32_t rx_num_msduds; // !< number of MSDUs successfully received
	uint32_t rx_octets; // !< number of octets of MSDU payload received
	uint32_t rx_num_seg_suc; // !< number of segments successfully received
	uint32_t rx_num_seg_missed; // !< number of segments missed
	uint32_t rx_num_pbs; // !< number of BPs handed over from the PHY to the MAC
	uint32_t rx_num_bursts; // !< number of received bursts
	uint32_t rx_num_mpdus; // !< number of MPDUs received
	uint32_t num_icv_fails; // !< number of received MAC frames for which ICV failed
} __attribute__((packed));

enum cm_link_stats_request_types
{
	CM_LINK_STATS_REQUEST_TYPE_STATISTIC_RESET 			= 0x00,
	CM_LINK_STATS_REQUEST_TYPE_STATISTIC_GET 			= 0x01,
	CM_LINK_STATS_REQUEST_TYPE_STATISTIC_GET_AND_RESET 	= 0x02
};

enum cm_link_stats_tl_flags
{
	CM_LINK_STATS_TL_FLAG_TRANSMIT_LINK = 0x00,
	CM_LINK_STATS_TL_FLAG_RECEIVE_LINK = 0x01
};

enum cm_link_stats_mgmt_flags
{
	CM_LINK_STATS_MGMT_FLAG_NOT_MANAGEMENT_LINK = 0x00,
	CM_LINK_STATS_MGMT_FLAG_MANAGEMENT_LINK = 0x01
};

struct ieee1901_2010_ctx {

	void (*cc_discover_list_confirm_station_information_cb)(uint8_t osa[6], struct ieee1901_2010_station_information *data, void *usr_data);
	void *cc_discover_list_confirm_station_information_cb_data;
	void (*cc_discover_list_confirm_network_information_cb)(uint8_t osa[6], struct ieee1901_2010_network_information *data, void *usr_data);
	void *cc_discover_list_confirm_network_information_cb_data;
	void (*cm_nw_info_confirm_nwinfo_cb)(uint8_t osa[6], struct ieee1901_2010_nwinfo *data, void *usr_data);
	void *cm_nw_info_confirm_nwinfo_cb_data;

	void (*cm_nw_stats_confirm_stats_cb)(uint8_t osa[6], struct ieee1901_2010_stats *data, void *usr_data);
	void *cm_nw_stats_confirm_stats_cb_data;

	void (*cm_link_stats_confirm_stats_cb)(uint8_t osa[6], struct ieee1901_2010_cm_link_stats_confirm_head *data, uint8_t *payload, size_t len, void *usr_data);
	void *cm_link_stats_confirm_stats_cb_data;

};

int ieee1901_2010_init(struct ieee1901_2010_ctx *ctx);

int ieee1901_2010_handle_message(struct ieee1901_2010_ctx *ctx,
								 uint8_t *payload, size_t len);

int ieee1901_2010_register_cc_discover_list_confirm_station_information_callback(struct ieee1901_2010_ctx *ctx, void (*cb)(uint8_t osa[6], struct ieee1901_2010_station_information *data, void *usr), void *usr_data);
int ieee1901_2010_register_cc_discover_list_confirm_network_information_callback(struct ieee1901_2010_ctx *ctx, void (*cb)(uint8_t osa[6], struct ieee1901_2010_network_information *data, void *usr), void *usr_data);
int ieee1901_2010_register_cm_nw_info_confirm_nwinfo_callback(struct ieee1901_2010_ctx *ctx, void (*cb)(uint8_t osa[6], struct ieee1901_2010_nwinfo *data, void *usr), void *usr_data);
int ieee1901_2010_register_cm_nw_stats_confirm_stats_callback(struct ieee1901_2010_ctx *ctx, void (*cb)(uint8_t osa[6], struct ieee1901_2010_stats *data, void *use), void *usr_data);
int ieee1901_2010_register_cm_link_stats_confirm_callback(struct ieee1901_2010_ctx *ctx, void (*cb)(uint8_t osa[6], struct ieee1901_2010_cm_link_stats_confirm_head *data, uint8_t *payload, size_t len, void *usr), void *usr_data);



#endif // _IEEE1901_2010_H