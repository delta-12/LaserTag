/**
 * @file BlePeripheral.c
 *
 * @brief Simple BLE peripheral using Bluetooth controller and NimBLE stack.
 * Creates a GATT server, advertises, and waits to be connected to a GATT
 * client.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "BlePeripheral.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

/* Defines
 ******************************************************************************/

#define BLEPERIPHERAL_OK 0U
#define BLEPERIPHERAL_PRESENT 1U
#define BLEPERIPHERAL_COMPLETE 1U
#define BLEPERIPHERAL_TARGET_SERVICE 0x4C54U

/* Globals
 ******************************************************************************/

static const char *BlePeripheral_LogTag = "BlePeripheral";
static const char *BlePeripheral_GapDeviceName = "Laser Target";

static const ble_uuid128_t BlePeripheral_GattServerServiceUuid =
    BLE_UUID128_INIT(0x32U, 0x69U, 0x5AU, 0xC2U, 0x4DU, 0x4BU, 0x49U, 0xB5U,
                     0xAAU, 0xF4U, 0x49U, 0xE9U, 0x30U, 0xA8U, 0x45U, 0xDDU);

static uint16_t BlePeripheral_GattServerCharacteristicValHandle;
static const ble_uuid128_t BlePeripheral_GattServerCharacteristicUuid =
    BLE_UUID128_INIT(0x90U, 0x12U, 0xB4U, 0x5EU, 0x62U, 0x87U, 0x42U, 0x32U,
                     0x9BU, 0x36U, 0x99U, 0x85U, 0xCBU, 0xB8U, 0xA8U, 0x3A);

static char BlePeripheral_UuidStringBuffer[BLE_UUID_STR_LEN];

static bool BlePeripheral_Connected = false;

/* Function Prototypes
 ******************************************************************************/

/* TODO */
void ble_store_config_init(void);

static int BlePeripheral_GattServerInit(void);
static void BlePeripheral_GattServerRegisterCallback(struct ble_gatt_register_ctxt *ctxt, void *arg);
static int BlePeripheral_GattServerAccess(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static void BlePeripheral_HostTask(void *arg);
static void BlePeripheral_OnSync(void);
static void BlePeripheral_OnReset(const int reason);
static void BlePeripheral_Advertise(void);
static int BlePeripheral_HandleGapEvent(struct ble_gap_event *event, void *arg);
static int BlePeripheral_HandleConnect(const struct ble_gap_event *const event);
static int BlePeripheral_HandleDisconnect(const struct ble_gap_event *const event);
static int BlePeripheral_HandleTransmitPower(const struct ble_gap_event *const event);
static int BlePeripheral_HandlePathlossThreshold(const struct ble_gap_event *const event);

/* TODO move struct to globals */
static const struct ble_gatt_svc_def BlePeripheral_GattServerServices[] = {
    {
        /*** Service ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &BlePeripheral_GattServerServiceUuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = &BlePeripheral_GattServerCharacteristicUuid.u,
                .access_cb = BlePeripheral_GattServerAccess,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &BlePeripheral_GattServerCharacteristicValHandle,
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
void BlePeripheral_Init(void)
{
    ESP_ERROR_CHECK(nimble_port_init());

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = BlePeripheral_OnReset;
    ble_hs_cfg.sync_cb = BlePeripheral_OnSync;
    ble_hs_cfg.gatts_register_cb = BlePeripheral_GattServerRegisterCallback;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    assert(BlePeripheral_GattServerInit() == BLEPERIPHERAL_OK);

    /* Set the default device name. */
    assert(ble_svc_gap_device_name_set(BlePeripheral_GapDeviceName) == BLEPERIPHERAL_OK);

    /* TODO Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(BlePeripheral_HostTask);
}

bool BlePeripheral_IsConnected(void)
{
    return BlePeripheral_Connected;
}

/* TODO move into BlePeripheral_Init? */
static int BlePeripheral_GattServerInit(void)
{
    int returnCode = BLEPERIPHERAL_OK;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    returnCode = ble_gatts_count_cfg(BlePeripheral_GattServerServices);
    if (returnCode == BLEPERIPHERAL_OK)
    {
        returnCode = ble_gatts_add_svcs(BlePeripheral_GattServerServices);
    }

    return returnCode;
}

static void BlePeripheral_GattServerRegisterCallback(struct ble_gatt_register_ctxt *context, void *arg)
{
    switch (context->op)
    {
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(BlePeripheral_LogTag, "Registered service %s with handle %d",
                 ble_uuid_to_str(context->svc.svc_def->uuid, BlePeripheral_UuidStringBuffer),
                 context->svc.handle);
        break;
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(BlePeripheral_LogTag, "Registering characteristic %s with def_handle %d and val_handle %d",
                 ble_uuid_to_str(context->chr.chr_def->uuid, BlePeripheral_UuidStringBuffer),
                 context->chr.def_handle,
                 context->chr.val_handle);
        break;
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(BlePeripheral_LogTag, "Registering descriptor %s with handle %d",
                 ble_uuid_to_str(context->dsc.dsc_def->uuid, BlePeripheral_UuidStringBuffer),
                 context->dsc.handle);
        break;
    default:
        assert(0);
        break;
    }
}

static int BlePeripheral_GattServerAccess(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt *context, void *arg)
{
    /* TODO fix static buffers */
    static uint8_t data[10];
    static uint8_t len;
    static struct os_mbuf *om;
    int returnCode = BLE_ATT_ERR_UNLIKELY;

    switch (context->op)
    {
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        ESP_LOGI(BlePeripheral_LogTag, "Characteristic write. conn_handle: %d", connHandle);
        if (attrHandle == BlePeripheral_GattServerCharacteristicValHandle)
        {
            om = context->om;
            len = os_mbuf_len(om);
            if (len >= sizeof(data))
            {
                len = sizeof(data);
            }
            assert(os_mbuf_copydata(om, 0U, len, data) == BLEPERIPHERAL_OK);
            ESP_LOG_BUFFER_HEX(BlePeripheral_LogTag, data, len);
            returnCode = BLEPERIPHERAL_OK;
        }
        break;
    default:
        break;
    }

    return returnCode;
}

static void BlePeripheral_HostTask(void *arg)
{
    ESP_LOGI(BlePeripheral_LogTag, "BLE Host Task Started");

    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

static void BlePeripheral_OnSync(void)
{
    BlePeripheral_Advertise();
}

static void BlePeripheral_OnReset(const int reason)
{
    ESP_LOGE(BlePeripheral_LogTag, "Resetting state. Reason: %d", reason);
}

static void BlePeripheral_Advertise(void)
{
    if (!ble_gap_adv_active())
    {
        struct ble_gap_adv_params advParams;
        struct ble_hs_adv_fields fields;
        const char *gapDeviceName;
        ble_addr_t gapDeviceAddress;

        /* Generate static random address */
        assert(ble_hs_id_gen_rnd(0U, &gapDeviceAddress) == BLEPERIPHERAL_OK);

        /* Set generated address */
        assert(ble_hs_id_set_rnd(gapDeviceAddress.val) == BLEPERIPHERAL_OK);

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
        fields.tx_pwr_lvl_is_present = BLEPERIPHERAL_PRESENT;
        fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

        gapDeviceName = ble_svc_gap_device_name();
        fields.name = (uint8_t *)gapDeviceName;
        fields.name_len = strlen(gapDeviceName);
        fields.name_is_complete = BLEPERIPHERAL_COMPLETE;

        fields.uuids16 = (ble_uuid16_t[]){BLE_UUID16_INIT(BLEPERIPHERAL_TARGET_SERVICE)};
        fields.num_uuids16 = BLEPERIPHERAL_PRESENT;
        fields.uuids16_is_complete = BLEPERIPHERAL_COMPLETE;

        if (ble_gap_adv_set_fields(&fields) != BLEPERIPHERAL_OK)
        {
            ESP_LOGE(BlePeripheral_LogTag, "Failed to set advertisement data.");
        }
        else
        {
            /* Begin advertising. */
            memset(&advParams, 0U, sizeof(advParams));
            advParams.conn_mode = BLE_GAP_CONN_MODE_UND;
            advParams.disc_mode = BLE_GAP_DISC_MODE_GEN;
            if (ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, BLE_HS_FOREVER, &advParams, BlePeripheral_HandleGapEvent, NULL) == BLEPERIPHERAL_OK)
            {
                ESP_LOGI(BlePeripheral_LogTag, "Started advertising.");
            }
            else
            {
                ESP_LOGE(BlePeripheral_LogTag, "Failed to enable advertisement.\n");
            }
        }
    }
}

static int BlePeripheral_HandleGapEvent(struct ble_gap_event *event, void *arg)
{
    int returnCode = BLEPERIPHERAL_OK;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        returnCode = BlePeripheral_HandleConnect(event);
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        returnCode = BlePeripheral_HandleDisconnect(event);
        break;
    case BLE_GAP_EVENT_TRANSMIT_POWER:
        returnCode = BlePeripheral_HandleTransmitPower(event);
        break;
    case BLE_GAP_EVENT_PATHLOSS_THRESHOLD:
        returnCode = BlePeripheral_HandlePathlossThreshold(event);
        break;
    default:
        break;
    }

    return returnCode;
}

static int BlePeripheral_HandleConnect(const struct ble_gap_event *const event)
{
    if (event->connect.status == BLEPERIPHERAL_OK)
    {
        /* A new connection was established. */
        ESP_LOGI(BlePeripheral_LogTag, "Connection established. Handle: %d", event->connect.conn_handle);

        BlePeripheral_Connected = true;
    }
    else
    {
        /* Restart the advertising */
        BlePeripheral_Advertise();
    }

    return BLEPERIPHERAL_OK;
}

static int BlePeripheral_HandleDisconnect(const struct ble_gap_event *const event)
{
    ESP_LOGI(BlePeripheral_LogTag, "Disconnected. Handle: %d, Reason: %d", event->disconnect.conn.conn_handle, event->disconnect.reason);

    BlePeripheral_Connected = false;

    /* Connection terminated; resume advertising. */
    BlePeripheral_Advertise();

    return BLEPERIPHERAL_OK;
}

static int BlePeripheral_HandleTransmitPower(const struct ble_gap_event *const event)
{
#if MYNEWT_VAL(BLE_POWER_CONTROL)
    ESP_LOGD(BlePeripheral_LogTag, "Transmit power event. Status: %d, Conn_handle: %d, reason: %d, "
                                   "phy: %d, power_level: %x, power_level_flag: %d, delta: %d",
             event->transmit_power.status, event->transmit_power.conn_handle,
             event->transmit_power.reason, event->transmit_power.phy,
             event->transmit_power.transmit_power_level,
             event->transmit_power.transmit_power_level_flag, event->transmit_power.delta);
#endif

    return BLEPERIPHERAL_OK;
}

static int BlePeripheral_HandlePathlossThreshold(const struct ble_gap_event *const event)
{
#if MYNEWT_VAL(BLE_POWER_CONTROL)
    ESP_LOGD(BlePeripheral_LogTag, "Pathloss threshold event. Conn handle: %d, Current path loss: %d, Zone entered: %d",
             event->pathloss_threshold.conn_handle,
             event->pathloss_threshold.current_path_loss, event->pathloss_threshold.zone_entered);
#endif

    return BLEPERIPHERAL_OK;
}