#define LGFX_USE_V1
#include <LovyanGFX.hpp>
//#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
//#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <driver/i2c.h>

class LGFX : public lgfx::LGFX_Device
{
   lgfx::Panel_ST7789     _panel_instance;
//    lgfx::Panel_ILI9488     _panel_instance;
    // lgfx::Panel_ILI9341     _panel_instance;
    lgfx::Bus_SPI       _bus_instance;   // SPIバスのインスタンス
    lgfx::Touch_FT5x06  _touch_instance;

  public:
    LGFX(void) {
      {
        auto cfg = _bus_instance.config();

        // SPIバスの設定
        cfg.spi_host = SPI2_HOST;  // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
        // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。 
        cfg.spi_mode = 0;                   // SPI通信モードを設定 (0 ~ 3)
        cfg.freq_write = 80000000;          // 传输时的SPI时钟（最高80MHz，四舍五入为80MHz除以整数得到的值）
        cfg.freq_read = 16000000;           // 接收时的SPI时钟
        cfg.spi_3wire = false;               // 受信をMOSIピンで行う場合はtrueを設定
        cfg.use_lock = true;                // 使用事务锁时设置为 true
        cfg.dma_channel = SPI_DMA_CH_AUTO;  // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
        // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
        cfg.pin_sclk = 42;  // SPIのSCLKピン番号を設定
        cfg.pin_mosi = 39;  // SPIのCLKピン番号を設定
        cfg.pin_miso = -1;  // SPIのMISOピン番号を設定 (-1 = disable)
        cfg.pin_dc = 41;     // SPIのD/Cピン番号を設定  (-1 = disable)

        _bus_instance.config(cfg);               // 設定値をバスに反映します。
        _panel_instance.setBus(&_bus_instance);  // バスをパネルにセットします。
      }

      { // 表示パネル制御の設定を行います。
        auto cfg = _panel_instance.config();  // 表示パネル設定用の構造体を取得します。

        cfg.pin_cs = 40;    // CSが接続されているピン番号   (-1 = disable)
        //        cfg.pin_rst = 9;   // RSTが接続されているピン番号  (-1 = disable)
        cfg.pin_rst = -1;   // RSTが接続されているピン番号  (-1 = disable)
        cfg.pin_busy = -1;  // BUSYが接続されているピン番号 (-1 = disable)

        // ※ 以下の設定値はパネル毎に一般的な初期値が設定さ BUSYが接続されているピン番号 (-1 = disable)れていますので、不明な項目はコメントアウトして試してみてください。

        cfg.memory_width = 240;    // ドライバICがサポートしている最大の幅
        cfg.memory_height = 320;   // ドライバICがサポートしている最大の高さ
        cfg.panel_width = 240;     // 実際に表示可能な幅
        cfg.panel_height = 320;    // 実際に表示可能な高さ
        cfg.offset_x = 0;          // パネルのX方向オフセット量
        cfg.offset_y = 0;         // パネルのY方向オフセット量
        cfg.offset_rotation = 3;   //值在旋转方向的偏移0~7（4~7是倒置的）
        cfg.dummy_read_pixel = 8;  // 在读取像素之前读取的虚拟位数
        cfg.dummy_read_bits = 1;   // 读取像素以外的数据之前的虚拟读取位数
        cfg.readable = false;      // 如果可以读取数据，则设置为 true
        cfg.invert = true;         // 如果面板的明暗反转，则设置为 true
        cfg.rgb_order = false;      // 如果面板的红色和蓝色被交换，则设置为 true
        cfg.dlen_16bit = false;    // 对于以 16 位单位发送数据长度的面板，设置为 true
        cfg.bus_shared = true;    // 如果总线与 SD 卡共享，则设置为 true（使用 drawJpgFile 等执行总线控制）

        _panel_instance.config(cfg);
      }

      { // タッチスクリーン制御の設定を行います。（必要なければ削除）
        auto cfg = _touch_instance.config();

        cfg.x_min = 0;            // タッチスクリーンから得られる最小のX値(生の値)
        cfg.x_max = 239;          // タッチスクリーンから得られる最大のX値(生の値)
        cfg.y_min = 0;            // タッチスクリーンから得られる最小のY値(生の値)
        cfg.y_max = 319;          // タッチスクリーンから得られる最大のY値(生の値)
        cfg.pin_int = 47;         // INTが接続されているピン番号
        cfg.bus_shared = false;   // 如果您使用与屏幕相同的总线，则设置为 true
        cfg.offset_rotation = 6;  // 显示和触摸方向不匹配时的调整 设置为 0 到 7 的值

        // I2C接続の場合
        cfg.i2c_port = 0;     // 使用するI2Cを選択 (0 or 1)
        cfg.i2c_addr = 0x38;  // I2Cデバイスアドレス番号
        cfg.pin_sda = 15;     // SDAが接続されているピン番号
        cfg.pin_scl = 16;     // SCLが接続されているピン番号
        cfg.freq = 400000;    // I2Cクロックを設定

        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
      }

      setPanel(&_panel_instance);
    }
};
