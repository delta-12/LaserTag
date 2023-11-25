/**
 * @file BleCentral.c
 *
 * @brief Initiates communication with Laser Target by scanning for nearby
 * peripherals and establishing connections with them if they support the
 * correct service.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "BleCentral.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"

/* Can only be included after headers above */
#include "esp_central.h"

/* Defines
 ******************************************************************************/

#define BLECENTRAL_OK 0U
#define BLECENTRAL_PRESENT 1U
#define BLECENTRAL_COMPLETE 1U
#define BLECENTRAL_MAX_SERVICES 64U
#define BLECENTRAL_MAX_CHARACTERISTICS 64U
#define BLECENTRAL_MAX_DESCRIPTORS 64U
#define BLECENTRAL_ADVERTISER_CONNECT_TIMEOUT_MS 30000U
#define BLECENTRAL_TARGET_SERVICE 0x4C54U
#define BLECENTRAL_STRINGBUFFER_LENGTH 18U

/* Globals
 ******************************************************************************/

static const char *BleCentral_LogTag = "BleCentral";
static const char *BleCentral_GapDeviceName = "Laser Blaster";

static const ble_uuid_t *BleCentral_GattServerServiceUuid =
    BLE_UUID128_DECLARE(0xD1U, 0xAFU, 0x96U, 0x88U, 0x7AU, 0x1DU, 0x4BU, 0x26U,
                        0x80U, 0x56U, 0xEFU, 0xCBU, 0x37U, 0x2AU, 0x9FU, 0x0DU);

// static uint16_t BleCentral_GattServerCharacteristicValHandle;
static const ble_uuid_t *BleCentral_GattServerCharacteristicUuid =
    BLE_UUID128_DECLARE(0xBAU, 0xB5U, 0x70U, 0xCAU, 0x13U, 0x61U, 0x44U, 0xC4U,
                        0x98U, 0x01U, 0x5CU, 0xFCU, 0x42U, 0xBAU, 0xE9U, 0xFAU);

static char BleCentral_StringBuffer[BLECENTRAL_STRINGBUFFER_LENGTH];

/* Typedefs
 ******************************************************************************/

typedef uint16_t BleCentral_ConnHandle_t;

/* Function Prototypes
 ******************************************************************************/

/* TODO */
void ble_store_config_init(void);

static void BleCentral_OnReset(const int reason);
static void BleCentral_OnSync(void);
static void BleCentral_HostTask(void *arg);
static void BleCentral_Scan(void);
static bool BleCentral_ShouldConnect(const struct ble_gap_disc_desc *const disc);
static void BleCentral_Connect(const void *const disc);
static void BleCentral_OnDiscComplete(const struct peer *peer, int status, void *arg);
static int BleCentral_HandleGapEvent(struct ble_gap_event *event, void *arg);
static int BleCentral_HandleDisc(const struct ble_gap_event *const event);
static int BleCentral_HandleConnect(const struct ble_gap_event *const event);
static int BleCentral_HandleDisconnect(const struct ble_gap_event *const event);
static int BleCentral_HandleDiscComplete(const struct ble_gap_event *const event);
static int BleCentral_HandleEncChange(const struct ble_gap_event *const event);
static int BleCentral_HandleNotifyRx(const struct ble_gap_event *const event);
static int BleCentral_HandleMtu(const struct ble_gap_event *const event);
static int BleCentral_HandleRepeatPairing(const struct ble_gap_event *const event);
static int BleCentral_HandleExtDisc(const struct ble_gap_event *const event);
static int BleCentral_HandleTransmitPower(const struct ble_gap_event *const event);
static int BleCentral_HandlePathlossThreshold(const struct ble_gap_event *const event);
static inline void BleCentral_AddrStr(const void *const addr, char *const buffer);

/* Function Definitions
 ******************************************************************************/

/* Initialize NVS before calling */
void BleCentral_Init(void)
{
    ESP_ERROR_CHECK(nimble_port_init());

    /* Initialize the NimBLE host configuration */
    ble_hs_cfg.reset_cb = BleCentral_OnReset;
    ble_hs_cfg.sync_cb = BleCentral_OnSync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Initialize data structures to track connected peers */
    assert(peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), BLECENTRAL_MAX_SERVICES, BLECENTRAL_MAX_CHARACTERISTICS, BLECENTRAL_MAX_DESCRIPTORS) == BLECENTRAL_OK);

    /* Set the default device name */
    assert(ble_svc_gap_device_name_set(BleCentral_GapDeviceName) == BLECENTRAL_OK);

    /* TODO Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(BleCentral_HostTask);
}

void BleCentral_DeInit(void)
{
    if (nimble_port_stop() == BLECENTRAL_OK)
    {
        nimble_port_deinit();
    }
    else
    {
        ESP_LOGE(BleCentral_LogTag, "Nimble port stop failed");
    }
}

static void BleCentral_OnReset(const int reason)
{
    ESP_LOGE(BleCentral_LogTag, "Resetting state. Reason: %d", reason);
}

static void BleCentral_OnSync(void)
{
    /* Make sure we have proper identity address set (public preferred) */
    assert(ble_hs_util_ensure_addr(0U) == BLECENTRAL_OK);

    /* Begin scanning for a peripheral to connect to */
    BleCentral_Scan();
}

static void BleCentral_HostTask(void *arg)
{
    ESP_LOGI(BleCentral_LogTag, "BLE Host Task Started");

    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

static void BleCentral_Scan(void)
{
    uint8_t ownAddrType;
    struct ble_gap_disc_params discParams;

    /* Determine which address to use while advertising */
    if (ble_hs_id_infer_auto(0U, &ownAddrType) == BLECENTRAL_OK)
    {
        /* Tell the controller to filter duplicates to avoid processing repeated advertisements from the same device */
        discParams.filter_duplicates = BLECENTRAL_PRESENT;

        /* Perform a passive scan, i.e. don't send follow-up scan requests to each advertiser */
        discParams.passive = BLECENTRAL_PRESENT;

        /* Use defaults for remaining parameters */
        discParams.itvl = 0U;
        discParams.window = 0U;
        discParams.filter_policy = 0U;
        discParams.limited = 0U;

        if (ble_gap_disc(ownAddrType, BLE_HS_FOREVER, &discParams, BleCentral_HandleGapEvent, NULL) != BLECENTRAL_OK)
        {
            ESP_LOGE(BleCentral_LogTag, "Error initiating GAP discovery procedure");
        }
    }
    else
    {
        ESP_LOGE(BleCentral_LogTag, "Error determining address type");
    }
}

static bool BleCentral_ShouldConnect(const struct ble_gap_disc_desc *const disc)
{
    struct ble_hs_adv_fields fields;
    bool shouldConnect = false;

    /* The device has to be advertising connectability. */
    if (disc->event_type == BLE_HCI_ADV_RPT_EVTYPE_ADV_IND || disc->event_type == BLE_HCI_ADV_RPT_EVTYPE_DIR_IND)
    {
        if (ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data) == BLECENTRAL_OK)
        {
            /* The device has to advertise support for the custom service */
            for (uint32_t i = 0U; i < fields.num_uuids16; i++)
            {
                if (ble_uuid_u16(&fields.uuids16[i].u) == BLECENTRAL_TARGET_SERVICE)
                {
                    shouldConnect = true;
                }
            }
        }
    }

    return shouldConnect;
}

static void BleCentral_Connect(const void *const disc)
{
    uint8_t ownAddrType;
    ble_addr_t *addr;

    /* Device should advertise connectability and support for correct services */
    if (BleCentral_ShouldConnect((struct ble_gap_disc_desc *)disc))
    {
        /* Scanning must be stopped before a connection can be initiated */
        if (ble_gap_disc_cancel() == BLECENTRAL_OK)
        {
            /* Determine which address to use for connecting */
            if (ble_hs_id_infer_auto(0U, &ownAddrType) == BLECENTRAL_OK)
            {
                /* Try to connect the the advertiser */
                addr = &((struct ble_gap_disc_desc *)disc)->addr;

                int connectResult = ble_gap_connect(ownAddrType, addr, BLECENTRAL_ADVERTISER_CONNECT_TIMEOUT_MS, NULL, BleCentral_HandleGapEvent, NULL);
                BleCentral_AddrStr(addr->val, BleCentral_StringBuffer);

                if (connectResult == BLECENTRAL_OK)
                {
                    ESP_LOGI(BleCentral_LogTag, "Connected to device. Addr type: %d, addr: %s", addr->type, BleCentral_StringBuffer);
                }
                else
                {
                    ESP_LOGE(BleCentral_LogTag, "Failed to connect to device. Addr type: %d, addr: %s", addr->type, BleCentral_StringBuffer);
                }
            }
            else
            {
                ESP_LOGE(BleCentral_LogTag, "Error determining address type");
            }
        }
        else
        {
            ESP_LOGD(BleCentral_LogTag, "Failed to cancel scan");
        }
    }
}

static void BleCentral_OnDiscComplete(const struct peer *peer, int status, void *arg)
{
    if (status == BLECENTRAL_OK)
    {
        /* Service discovery has completed successfully, obtained complete list of services, characteristics, and descriptors that the peer supports. */
        ESP_LOGI(BleCentral_LogTag, "Service discovery complete. Status: %d, conn_handle: %d", status, peer->conn_handle);

        /* TODO change GATT procedures for target */
        /* Now perform three GATT procedures against the peer: read,
         * write, and subscribe to notifications for the ANS service.
         */
        // blecent_read_write_subscribe(peer);
    }
    else
    {
        /* Service discovery failed */
        ESP_LOGE(BleCentral_LogTag, "Service discovery failed. Status :%d, conn_handle: %d", status, peer->conn_handle);

        /* Terminate the connection */
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    }
}

static int BleCentral_HandleGapEvent(struct ble_gap_event *event, void *arg)
{
    int returnCode = BLECENTRAL_OK;

    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC:
        returnCode = BleCentral_HandleDisc(event);
        break;
    case BLE_GAP_EVENT_CONNECT:
        returnCode = BleCentral_HandleConnect(event);
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        returnCode = BleCentral_HandleDisconnect(event);
        break;
    case BLE_GAP_EVENT_DISC_COMPLETE:
        returnCode = BleCentral_HandleDiscComplete(event);
        break;
    case BLE_GAP_EVENT_ENC_CHANGE:
        returnCode = BleCentral_HandleEncChange(event);
        break;
    case BLE_GAP_EVENT_NOTIFY_RX:
        returnCode = BleCentral_HandleNotifyRx(event);
        break;
    case BLE_GAP_EVENT_MTU:
        returnCode = BleCentral_HandleMtu(event);
        break;
    case BLE_GAP_EVENT_REPEAT_PAIRING:
        returnCode = BleCentral_HandleRepeatPairing(event);
        break;
    case BLE_GAP_EVENT_EXT_DISC:
        returnCode = BleCentral_HandleExtDisc(event);
        break;
    case BLE_GAP_EVENT_TRANSMIT_POWER:
        returnCode = BleCentral_HandleTransmitPower(event);
        break;
    case BLE_GAP_EVENT_PATHLOSS_THRESHOLD:
        returnCode = BleCentral_HandlePathlossThreshold(event);
        break;
    default:
        break;
    }

    return returnCode;
}

static int BleCentral_HandleDisc(const struct ble_gap_event *const event)
{
    struct ble_hs_adv_fields fields;

    /* Attempt to connect to the advertiser */
    if (ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data) == BLECENTRAL_OK)
    {
        BleCentral_Connect(&event->disc);
    }

    return BLECENTRAL_OK;
}

static int BleCentral_HandleConnect(const struct ble_gap_event *const event)
{
    struct ble_gap_conn_desc desc;

    /* A new connection was established or a connection attempt failed */
    if (event->connect.status == BLECENTRAL_OK)
    {
        /* Connection successfully established */
        ESP_LOGI(BleCentral_LogTag, "Connection established");

        assert(ble_gap_conn_find(event->connect.conn_handle, &desc) == BLECENTRAL_OK);

        /* Remember peer */
        if (peer_add(event->connect.conn_handle) == BLECENTRAL_OK)
        {

#if MYNEWT_VAL(BLE_POWER_CONTROL)
            blecent_power_control(event->connect.conn_handle);
#endif

#if MYNEWT_VAL(BLE_HCI_VS)
#if MYNEWT_VAL(BLE_POWER_CONTROL)
            memset(&params, 0U, sizeof(struct ble_gap_set_auto_pcl_params));

            params.conn_handle = event->connect.conn_handle;

            if (ble_gap_set_auto_pcl_param(&params) != BLECENTRAL_OK)
            {
                ESP_LOGI(BleCentral_LogTag, "Failed to send VSC");
            }
            else
            {
                ESP_LOGI(BleCentral_LogTag, "Successfully issued VSC");
#endif
#endif

                /* Perform service discovery */
                if (peer_disc_all(event->connect.conn_handle, BleCentral_OnDiscComplete, NULL) != BLECENTRAL_OK)
                {
                    ESP_LOGE(BleCentral_LogTag, "Failed to discover services");
                }

#if MYNEWT_VAL(BLE_HCI_VS)
#if MYNEWT_VAL(BLE_POWER_CONTROL)
            }
#endif
#endif
        }
        else
        {
            ESP_LOGE(BleCentral_LogTag, "Failed to add peer");
        }
    }
    else
    {
        /* Connection attempt failed */
        ESP_LOGE(BleCentral_LogTag, "Connection failed. Status=%d", event->connect.status);

        /* Resume scanning */
        BleCentral_Scan();
    }

    return 0;
}

static int BleCentral_HandleDisconnect(const struct ble_gap_event *const event)
{
    /* Connection terminated */
    ESP_LOGI(BleCentral_LogTag, "Disconnect. Reason: %d", event->disconnect.reason);

    /* Forget about peer */
    peer_delete(event->disconnect.conn.conn_handle);

    /* Resume scanning */
    BleCentral_Scan();

    return BLECENTRAL_OK;
}

static int BleCentral_HandleDiscComplete(const struct ble_gap_event *const event)
{
    ESP_LOGI(BleCentral_LogTag, "Discovery complete. Reason: %d", event->disc_complete.reason);

    return BLECENTRAL_OK;
}

static int BleCentral_HandleEncChange(const struct ble_gap_event *const event)
{
    struct ble_gap_conn_desc desc;

    /* Encryption has been enabled or disabled for this connection */
    ESP_LOGI(BleCentral_LogTag, "Encryption change event. Status: %d", event->enc_change.status);

    assert(ble_gap_conn_find(event->enc_change.conn_handle, &desc) == BLECENTRAL_OK);

    /* Go for service discovery after encryption has been successfully enabled */
    if (peer_disc_all(event->connect.conn_handle, BleCentral_OnDiscComplete, NULL) != BLECENTRAL_OK)
    {
        ESP_LOGE(BleCentral_LogTag, "Failed to discover services.");
    }

    return BLECENTRAL_OK;
}

static int BleCentral_HandleNotifyRx(const struct ble_gap_event *const event)
{
    /* Peer sent a notification or indication */
    ESP_LOGI(BleCentral_LogTag, "Received: %s; conn_handle: %d, attr_handle: %d, attr_len=%d",
             event->notify_rx.indication ? "indication" : "notification",
             event->notify_rx.conn_handle,
             event->notify_rx.attr_handle,
             OS_MBUF_PKTLEN(event->notify_rx.om));

    return BLECENTRAL_OK;
}

static int BleCentral_HandleMtu(const struct ble_gap_event *const event)
{
    ESP_LOGI(BleCentral_LogTag, "MTU update event. conn_handle: %d, channel ID: %d, MTU: %d",
             event->mtu.conn_handle,
             event->mtu.channel_id,
             event->mtu.value);

    return BLECENTRAL_OK;
}

/* TODO update comment block */
/* We already have a bond with the peer, but it is attempting to
 * establish a new secure link.  This app sacrifices security for
 * convenience: just throw away the old bond and accept the new link.
 */
static int BleCentral_HandleRepeatPairing(const struct ble_gap_event *const event)
{
    struct ble_gap_conn_desc desc;

    /* Delete the old bond */
    assert(ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) == BLECENTRAL_OK);
    ble_store_util_delete_peer(&desc.peer_id_addr);

    /* Indicate that the host should continue with the pairing operation */
    return BLE_GAP_REPEAT_PAIRING_RETRY;
}

static int BleCentral_HandleExtDisc(const struct ble_gap_event *const event)
{
    /* An advertisment report was received during GAP discovery */
    BleCentral_Connect(&event->disc);

    return BLECENTRAL_OK;
}

static int BleCentral_HandleTransmitPower(const struct ble_gap_event *const event)
{
#if MYNEWT_VAL(BLE_POWER_CONTROL)
    ESP_LOGI(BleCentral_LogTag, "Transmit power event. Status: %d, conn_handle: %d, reason: %d, phy: %d, power_level: %d, power_level_flag: %d, delta: %d",
             event->transmit_power.status,
             event->transmit_power.conn_handle,
             event->transmit_power.reason,
             event->transmit_power.phy,
             event->transmit_power.transmit_power_level,
             event->transmit_power.transmit_power_level_flag,
             event->transmit_power.delta);
#endif

    return BLECENTRAL_OK;
}

static int BleCentral_HandlePathlossThreshold(const struct ble_gap_event *const event)
{
#if MYNEWT_VAL(BLE_POWER_CONTROL)
    ESP_LOGI(BleCentral_LogTag, "Pathloss threshold event. conn_handle: %d, current path loss: %d, zone entered: %d",
             event->pathloss_threshold.conn_handle,
             event->pathloss_threshold.current_path_loss,
             event->pathloss_threshold.zone_entered);
#endif

    return BLECENTRAL_OK;
}

static inline void BleCentral_AddrStr(const void *const addr, char *const buffer)
{
    const uint8_t *const addrBuffer = addr;

    /* TODO fix magic numbers */
    sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x", addrBuffer[5U], addrBuffer[4U], addrBuffer[3U], addrBuffer[2U], addrBuffer[1U], addrBuffer[0U]);
}
