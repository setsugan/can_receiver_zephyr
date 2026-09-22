#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

CAN_MSGQ_DEFINE(can_rx_msgq, 4);

int main(void)
{
	int ret;

	printk("deadbeef_can start\n");

	if (!device_is_ready(can_dev)) {
		printk("CAN device is not ready: %s\n", can_dev->name);
		return 0;
	}

	printk("CAN device: %s\n", can_dev->name);

	ret = can_set_mode(can_dev, CAN_MODE_NORMAL);

	if (ret != 0) {
		printk("can_set_mode failed: %d\n", ret);
		return 0;
	}

	struct can_filter filter = {
        .id = 0x123,
        .mask = CAN_STD_ID_MASK,
        .flags = 0,
    };

	ret = can_add_rx_filter_msgq(
        can_dev,
        &can_rx_msgq,
        &filter
    );

	if (ret < 0) {
        printk("RX filter failed: %d\n", ret);
        return 0;
    }

	printk("RX filter ID: %d\n", ret);

	ret = can_start(can_dev);

	if (ret != 0) {
		printk("can_start failed: %d\n", ret);
		return 0;
	}

	printk("CAN started\n");

	while (1) {
		struct can_frame rx_frame;

		ret = k_msgq_get(
            &can_rx_msgq,
            &rx_frame,
            K_MSEC(100)
        );

		if (ret == 0) {
			printk("RX: ID=0x%03x DATA=", rx_frame.id);

			for (size_t i = 0; i < can_dlc_to_bytes(rx_frame.dlc); i++) {
				printk("%02X", rx_frame.data[i]);
			}

			printk("\n");
        } else {
            printk("RX timeout\n");
        }

		k_sleep(K_SECONDS(1));
	}

	return 0;
}
