
/* GPIO interrupt & soft interrupt */
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(motion_int, LOG_LEVEL_DBG);

static const struct gpio_dt_spec motion_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(my_pin), gpios);
static struct gpio_callback motion_cb;

// Forward declaration
void trigger_motion_isr_soft(void);

static void motion_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (pins & BIT(motion_pin.pin)) {
        LOG_INF("Motion detected! (source: %s)", 
                k_is_in_isr() ? "HARDWARE" : "SOFTWARE");
    }
}

/* soft interrupt trigger */
void trigger_motion_isr_soft(void) {
    const struct device *fake_dev = motion_pin.port;
    uint32_t fake_pins = BIT(motion_pin.pin);
    motion_isr(fake_dev, &motion_cb, fake_pins);
}

/* hard interrupt init */
static int init_motion_interrupt(const struct device *dev) {
    ARG_UNUSED(dev);
    
    if (!device_is_ready(motion_pin.port)) {
        LOG_ERR("GPIO device not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&motion_pin, GPIO_INPUT);
    if (ret != 0) return ret;

    ret = gpio_pin_interrupt_configure_dt(&motion_pin, GPIO_INT_EDGE_RISING);
    if (ret != 0) return ret;

    gpio_init_callback(&motion_cb, motion_isr, BIT(motion_pin.pin));
    gpio_add_callback(motion_pin.port, &motion_cb);
    
    LOG_INF("Motion interrupt initialized");
    return 0;
}

SYS_INIT(init_motion_interrupt, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

/* Shell Command Implementation that calls above defined function */
/* no dash _ in command name */
static int cmd_triggermotion(const struct shell *sh, size_t argc, char **argv) {
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    
    trigger_motion_isr_soft();
    shell_print(sh, "Software motion interrupt triggered");
    return 0;
}

/* Create shell command structure */
/* Creating root (level 0) command "trigger_motion" with handler function (fouth param is handler) */
SHELL_CMD_REGISTER(triggermotion, NULL, "Trigger motion ISR", cmd_triggermotion);

mumble jumble