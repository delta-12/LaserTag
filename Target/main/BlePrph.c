/**
 * @file BlePrph.c
 *
 * @brief Simple BLE peripheral using Bluetooth controller and NimBLE stack.
 * Creates a GATT server, advertises, and waits to be connected to a GATT
 * client.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "BlePrph.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

/* Defines
 ******************************************************************************/

#define BLEPRPH_OK 0U
#define BLEPRPH_PRESENT 1U
#define BLEPRPH_COMPLETE 1U

/* Globals
 ******************************************************************************/

static const char *BlePrph_LogTag = "BlePrph";
static const char *BlePrph_GapDeviceName = "BLE Target";

static const ble_uuid128_t BlePrph_GattServerServiceUuid =
    BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                     0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

static uint16_t BlePrph_GattServerCharacteristicValHandle;
static const ble_uuid128_t BlePrph_GattServerCharacteristicUuid =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
                     0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33);

static char BlePrph_UuidStringBuffer[BLE_UUID_STR_LEN];

/* Function Prototypes
 ******************************************************************************/

/* TODO */
void ble_store_config_init(void);

static int BlePrph_GattServerInit(void);
static void BlePrph_GattServerRegisterCallback(struct ble_gatt_register_ctxt *ctxt, void *arg);
static int BlePrph_GattServerAccess(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static void BlePrph_HostTask(void *arg);
static void BlePrph_OnSync(void);
static void BlePrph_OnReset(const int reason);
static void BlePrph_Advertise(void);
static int BlePrph_HandleGapEvent(struct ble_gap_event *event, void *arg);
static int BlePrph_HandleConnect(const struct ble_gap_event *const event);
static int BlePrph_HandleDisconnect(const struct ble_gap_event *const event);
static int BlePrph_HandleTransmitPower(const struct ble_gap_event *const event);
static int BlePrph_HandlePathlossThreshold(const struct ble_gap_event *const event);

/* TODO move struct to globals */
static const struct ble_gatt_svc_def BlePrph_GattServerServices[] = {
    {
        /*** Service ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &BlePrph_GattServerServiceUuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = &BlePrph_GattServerCharacteristicUuid.u,
                .access_cb = BlePrph_GattServerAccess,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &BlePrph_GattServerCharacteristicValHandle,
            },
            {
                0U, /* No more characteristics in this service. */
            }},
    },
    {
        0U, /* No more services. */
    },
};

/* Function Definitions
 ******************************************************************************/

/* Initialize NVS before calling */
void BlePrph_Init(void)
{
    ESP_ERROR_CHECK(nimble_port_init());

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = BlePrph_OnReset;
    ble_hs_cfg.sync_cb = BlePrph_OnSync;
    ble_hs_cfg.gatts_register_cb = BlePrph_GattServerRegisterCallback;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    assert(BlePrph_GattServerInit() == BLEPRPH_OK);

    /* Set the default device name. */
    assert(ble_svc_gap_device_name_set(BlePrph_GapDeviceName) == BLEPRPH_OK);

    /* TODO Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(BlePrph_HostTask);
}

/* TODO move into BlePrph_Init? */
static int BlePrph_GattServerInit(void)
{
    int returnCode = BLEPRPH_OK;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    returnCode = ble_gatts_count_cfg(BlePrph_GattServerServices);
    if (returnCode == BLEPRPH_OK)
    {
        returnCode = ble_gatts_add_svcs(BlePrph_GattServerServices);
    }

    return returnCode;
}

static void BlePrph_GattServerRegisterCallback(struct ble_gatt_register_ctxt *context, void *arg)
{
    switch (context->op)
    {
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(BlePrph_LogTag, "Registered service %s with handle %d",
                 ble_uuid_to_str(context->svc.svc_def->uuid, BlePrph_UuidStringBuffer),
                 context->svc.handle);
        break;
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(BlePrph_LogTag, "registering characteristic %s with def_handle %d and val_handle %d",
                 ble_uuid_to_str(context->chr.chr_def->uuid, BlePrph_UuidStringBuffer),
                 context->chr.def_handle,
                 context->chr.val_handle);
        break;
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(BlePrph_LogTag, "registering descriptor %s with handle %d",
                 ble_uuid_to_str(context->dsc.dsc_def->uuid, BlePrph_UuidStringBuffer),
                 context->dsc.handle);
        break;
    default:
        assert(0);
        break;
    }
}

static int BlePrph_GattServerAccess(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt *context, void *arg)
{
    /* TODO fix static buffers */
    static uint8_t data[10];
    static uint8_t len;
    static struct os_mbuf *om;
    int returnCode = BLE_ATT_ERR_UNLIKELY;

    switch (context->op)
    {
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        ESP_LOGI(BlePrph_LogTag, "Characteristic write. conn_handle: %d", connHandle);
        if (attrHandle == BlePrph_GattServerCharacteristicValHandle)
        {
            om = context->om;
            len = os_mbuf_len(om);
            if (len >= sizeof(data))
            {
                len = sizeof(data);
            }
            assert(os_mbuf_copydata(om, 0, len, data) == BLEPRPH_OK);
            ESP_LOG_BUFFER_HEX(BlePrph_LogTag, data, len);
            returnCode = 0U;
        }
        break;
    default:
        break;
    }

    return returnCode;
}

static void BlePrph_HostTask(void *arg)
{
    ESP_LOGI(BlePrph_LogTag, "BLE Host Task Started");

    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

static void BlePrph_OnSync(void)
{
    BlePrph_Advertise();
}

static void BlePrph_OnReset(const int reason)
{
    ESP_LOGE(BlePrph_LogTag, "Resetting state; reason=%d\n", reason);
}

static void BlePrph_Advertise(void)
{
    if (!ble_gap_adv_active())
    {
        struct ble_gap_adv_params advParams;
        struct ble_hs_adv_fields fields;
        const char *gapDeviceName;
        ble_addr_t gapDeviceAddress;

        /* Generate static random address */
        assert(ble_hs_id_gen_rnd(0U, &gapDeviceAddress) == BLEPRPH_OK);

        /* Set generated address */
        assert(ble_hs_id_set_rnd(gapDeviceAddress.val) == BLEPRPH_OK);

        /**
         *  Set the advertisement data included in our advertisements:
         *     o Flags (indicates advertisement type and other general info).
         *     o Advertising tx power.
         *     o Device name.
         */
        memset(&fields, 0U, sizeof(fields));

        /* Advertise two flags:
         *     o Discoverability in forthcoming advertisement (general)
         *     o BLE-only (BR/EDR unsupported).
         */
        fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

        /* Indicate that the TX power level field should be included; have the
         * stack fill this value automatically.  This is done by assigning the
         * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
         */
        fields.tx_pwr_lvl_is_present = BLEPRPH_PRESENT;
        fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

        gapDeviceName = ble_svc_gap_device_name();
        fields.name = (uint8_t *)gapDeviceName;
        fields.name_len = strlen(gapDeviceName);
        fields.name_is_complete = BLEPRPH_COMPLETE;

        if (ble_gap_adv_set_fields(&fields) != BLEPRPH_OK)
        {
            ESP_LOGE(BlePrph_LogTag, "Failed to set advertisement data.");
        }
        else
        {
            /* Begin advertising. */
            memset(&advParams, 0U, sizeof(advParams));
            advParams.conn_mode = BLE_GAP_CONN_MODE_UND;
            advParams.disc_mode = BLE_GAP_DISC_MODE_GEN;
            if (ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, BLE_HS_FOREVER, &advParams, BlePrph_HandleGapEvent, NULL) == BLEPRPH_OK)
            {
                ESP_LOGI(BlePrph_LogTag, "Started advertising.");
            }
            else
            {
                ESP_LOGE(BlePrph_LogTag, "Failed to enable advertisement.\n");
            }
        }
    }
}

static int BlePrph_HandleGapEvent(struct ble_gap_event *event, void *arg)
{
    int returnCode = BLEPRPH_OK;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        returnCode = BlePrph_HandleConnect(event);
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        returnCode = BlePrph_HandleDisconnect(event);
        break;
    case BLE_GAP_EVENT_TRANSMIT_POWER:
        returnCode = BlePrph_HandleTransmitPower(event);
        break;
    case BLE_GAP_EVENT_PATHLOSS_THRESHOLD:
        returnCode = BlePrph_HandlePathlossThreshold(event);
        break;
    default:
        break;
    }

    return returnCode;
}

static int BlePrph_HandleConnect(const struct ble_gap_event *const event)
{
    if (event->connect.status == BLEPRPH_OK)
    {
        /* A new connection was established. */
        ESP_LOGI(BlePrph_LogTag, "Connection established. Handle: %d", event->connect.conn_handle);
    }
    else
    {
        /* Restart the advertising */
        BlePrph_Advertise();
    }

    return BLEPRPH_OK;
}

static int BlePrph_HandleDisconnect(const struct ble_gap_event *const event)
{
    ESP_LOGI(BlePrph_LogTag, "Disconnected. Handle: %d, Reason: %d", event->disconnect.conn.conn_handle, event->disconnect.reason);

    /* Connection terminated; resume advertising. */
    BlePrph_Advertise();

    return BLEPRPH_OK;
}

static int BlePrph_HandleTransmitPower(const struct ble_gap_event *const event)
{
#if MYNEWT_VAL(BLE_POWER_CONTROL)
    ESP_LOGD(BlePrph_LogTag, "Transmit power event. Status: %d, Conn_handle: %d, reason: %d, "
                             "phy: %d, power_level: %x, power_level_flag: %d, delta: %d",
             event->transmit_power.status, event->transmit_power.conn_handle,
             event->transmit_power.reason, event->transmit_power.phy,
             event->transmit_power.transmit_power_level,
             event->transmit_power.transmit_power_level_flag, event->transmit_power.delta);
#endif

    return BLEPRPH_OK;
}

static int BlePrph_HandlePathlossThreshold(const struct ble_gap_event *const event)
{
#if MYNEWT_VAL(BLE_POWER_CONTROL)
    ESP_LOGD(BlePrph_LogTag, "Pathloss threshold event. Conn handle: %d, Current path loss: %d, Zone entered: %d",
             event->pathloss_threshold.conn_handle,
             event->pathloss_threshold.current_path_loss, event->pathloss_threshold.zone_entered);
#endif

    return BLEPRPH_OK;
}