#include "st_spi_api.h"

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
// #define PIN_NUM_CS   

const char * TAG = "[SPI]";

typedef struct {
    spi_host_device_t host; ///< The SPI host used, set before calling `spi_eeprom_init()`
    gpio_num_t cs_io;       ///< CS gpio number, set before calling `spi_eeprom_init()`
    gpio_num_t miso_io;     ///< MISO gpio number, set before calling `spi_eeprom_init()`
    bool intr_used;         ///< Whether to use polling or interrupt when waiting for write to be done. Set before calling `spi_eeprom_init()`.
} eeprom_config_t;

struct eeprom_context_t{
    eeprom_config_t cfg;        ///< Configuration by the caller.
    spi_device_handle_t spi;    ///< SPI device handle
    SemaphoreHandle_t ready_sem; ///< Semaphore for ready signal
};

struct eeprom_context_t ctx = {
    .cfg = {
        .host = HSPI_HOST,
        .cs_io = -1,
        .miso_io = PIN_NUM_MISO
    }
};

static void nullcb(spi_transaction_t* t){
}

// static spi_device_handle_t st_spi_dev;

void st_spi_init(void){
    esp_err_t ret;
    
    spi_bus_config_t buscfg={
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    spi_device_interface_config_t devcfg={
        .command_bits = 10,
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,          //SPI mode 0
        /*
         * The timing requirements to read the busy signal from the EEPROM cannot be easily emulated
         * by SPI transactions. We need to control CS pin by SW to check the busy signal manually.
         */
        .spics_io_num = -1,
        .queue_size = 1,
        .flags =  SPI_DEVICE_3WIRE,
        .pre_cb = nullcb,
        .post_cb = nullcb,
        .input_delay_ns = 0,  //the EEPROM output the data half a SPI clock behind.
    };
    // gpio_config_t cs_cfg = {
    //     .pin_bit_mask = BIT64(ctx->cfg.cs_io),
    //     .mode = GPIO_MODE_OUTPUT,
    // };
    // gpio_config(&cs_cfg);
    //Initialize the SPI bus
    ESP_LOGI(TAG, "Initializing bus SPI%d...", HSPI_HOST);
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "Initializing bus SPI%d done", HSPI_HOST);
    spi_bus_add_device(HSPI_HOST, &devcfg, &(ctx.spi));
    ESP_LOGI(TAG, "Initializing SPI%d device", HSPI_HOST);
    ctx.ready_sem = xSemaphoreCreateBinary();
    if (ctx.ready_sem == NULL) {
        ESP_LOGE(TAG, "Sem not creat");
    }
}

void st_spi_trans(uint8_t data){
    esp_err_t err;
    spi_transaction_t t = {
        .cmd = 0x200,
        .flags = SPI_TRANS_USE_TXDATA,
        .tx_data = {data},
    };
    err = spi_device_acquire_bus((ctx.spi), portMAX_DELAY);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "acquire");
        return;
    }
    err = spi_device_polling_transmit((ctx.spi), &t);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "poll");
        return;
    }

    spi_device_release_bus((ctx.spi));
}