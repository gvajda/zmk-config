/*
 * Layer status GATT service.
 *
 * Exposes which keymap layers are active so a host app (ZMK Battery Bar) can
 * show a layer indicator. Two characteristics:
 *
 *   state  READ | NOTIFY  uint32 little-endian; bit i = layer at index i (in
 *                         priority order, as Studio shows it) is active.
 *                         Notified on every layer change.
 *   names  READ           layer names in index order, '\n'-separated (UTF-8,
 *                         no terminator). Empty entry = layer has no name.
 *
 * Reads and the CCC write require an encrypted link, so only a bonded host
 * can subscribe. Notifications go to the value attribute, not the
 * characteristic declaration (CVE-2026-2411 skips permission checks for the
 * latter).
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define LAYER_SVC_UUID                                                                             \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xF8066FA3, 0x1254, 0x4931, 0xA2CA, 0xF24F572961B6))
#define LAYER_STATE_UUID                                                                           \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x36FC306E, 0x8D90, 0x414C, 0x81EE, 0x052A5F4BF37E))
#define LAYER_NAMES_UUID                                                                           \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x5D03CE99, 0x6B0D, 0x4883, 0xBAC7, 0x3B5708296AB9))

static uint32_t active_layers_by_index(void) {
    uint32_t mask = 0;
    for (zmk_keymap_layer_index_t i = 0; i < ZMK_KEYMAP_LAYERS_LEN && i < 32; i++) {
        zmk_keymap_layer_id_t id = zmk_keymap_layer_index_to_id(i);
        if (id != ZMK_KEYMAP_LAYER_ID_INVAL && zmk_keymap_layer_active(id)) {
            mask |= BIT(i);
        }
    }
    return mask;
}

static ssize_t read_state(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                          uint16_t len, uint16_t offset) {
    uint32_t value = sys_cpu_to_le32(active_layers_by_index());
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t read_names(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                          uint16_t len, uint16_t offset) {
    // ponytail: static, not stack — GATT reads run one at a time on the BT RX
    // thread, whose stack is small. 512 = max ATT attribute length; names past
    // that are cut off.
    static char names[512];
    size_t n = 0;

    for (zmk_keymap_layer_index_t i = 0; i < ZMK_KEYMAP_LAYERS_LEN && i < 32; i++) {
        zmk_keymap_layer_id_t id = zmk_keymap_layer_index_to_id(i);
        const char *name = id == ZMK_KEYMAP_LAYER_ID_INVAL ? NULL : zmk_keymap_layer_name(id);
        int w = snprintk(names + n, sizeof(names) - n, "%s%s", i > 0 ? "\n" : "",
                         name ? name : "");
        if (w < 0 || (size_t)w >= sizeof(names) - n) {
            n = sizeof(names) - 1;
            break;
        }
        n += w;
    }

    return bt_gatt_attr_read(conn, attr, buf, len, offset, names, n);
}

static void state_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value) {
    LOG_INF("Layer status notifications %s", value == BT_GATT_CCC_NOTIFY ? "on" : "off");
}

BT_GATT_SERVICE_DEFINE(layer_svc, BT_GATT_PRIMARY_SERVICE(LAYER_SVC_UUID),
                       BT_GATT_CHARACTERISTIC(LAYER_STATE_UUID,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                                              BT_GATT_PERM_READ_ENCRYPT, read_state, NULL, NULL),
                       BT_GATT_CCC(state_ccc_changed,
                                   BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT),
                       BT_GATT_CHARACTERISTIC(LAYER_NAMES_UUID, BT_GATT_CHRC_READ,
                                              BT_GATT_PERM_READ_ENCRYPT, read_names, NULL, NULL));

// attrs: [0] service, [1] state declaration, [2] state value, [3] CCC, ...
#define LAYER_STATE_VALUE_ATTR (&layer_svc.attrs[2])

static int layer_status_listener(const zmk_event_t *eh) {
    uint32_t value = sys_cpu_to_le32(active_layers_by_index());
    int rc = bt_gatt_notify(NULL, LAYER_STATE_VALUE_ATTR, &value, sizeof(value));
    if (rc < 0 && rc != -ENOTCONN) {
        LOG_WRN("Failed to notify layer status: %d", rc);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_status_gatt, layer_status_listener);
ZMK_SUBSCRIPTION(layer_status_gatt, zmk_layer_state_changed);
