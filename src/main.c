#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/resets.h"
#include "hardware/pll.h"
#include "hardware/xosc.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "pio/pioWS2812.h"
#include "pio/pinoutRp2350defcon.h"
#include "timebase.h"
#include "display.h"
#include "ui.h"

void ws2812SetAllBlue(void) {
    for (uint32_t i = 0; i < NUM_WS2812s; i++) {
        ws2812Set(i, 0, 255);   // Set Blue component to 0
        ws2812Set(i, 1, 0);   // Set Green component to 0
        ws2812Set(i, 2, 0); // Set Red component to 255
    }
    ws2812refresh(); // Refresh the LEDs to apply the changes
}

void configure_clocks_and_resets(void) {
    // Tell refclock to use ROSC
    clocks_hw->clk[clk_ref].ctrl = (clocks_hw->clk[clk_ref].ctrl & ~CLOCKS_CLK_REF_CTRL_SRC_BITS) | (CLOCKS_CLK_REF_CTRL_SRC_VALUE_ROSC_CLKSRC_PH << CLOCKS_CLK_REF_CTRL_SRC_LSB);

    // Use ref clock for CPU, use sys clock for peripherals
    clocks_hw->clk[clk_peri].ctrl &= ~CLOCKS_CLK_PERI_CTRL_ENABLE_BITS;
    clocks_hw->clk[clk_sys].ctrl = (clocks_hw->clk[clk_sys].ctrl & ~CLOCKS_CLK_SYS_CTRL_SRC_BITS) | (CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF << CLOCKS_CLK_SYS_CTRL_SRC_LSB);
    clocks_hw->clk[clk_peri].ctrl = (clocks_hw->clk[clk_peri].ctrl & ~(CLOCKS_CLK_PERI_CTRL_KILL_BITS | CLOCKS_CLK_PERI_CTRL_AUXSRC_BITS)) | CLOCKS_CLK_PERI_CTRL_ENABLE_BITS | (CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS << CLOCKS_CLK_PERI_CTRL_AUXSRC_LSB);

    // Release some peripherals from reset
    uint32_t unitsToReset = RESETS_RESET_PWM_BITS | RESETS_RESET_UART1_BITS | RESETS_RESET_PADS_BANK0_BITS | RESETS_RESET_IO_BANK0_BITS | RESETS_RESET_PIO0_BITS | RESETS_RESET_PIO1_BITS | RESETS_RESET_DMA_BITS | RESETS_RESET_SPI1_BITS | RESETS_RESET_PLL_SYS_BITS;
    resets_hw->reset |= unitsToReset;
    resets_hw->reset &= ~unitsToReset;
    while ((resets_hw->reset_done & unitsToReset) != unitsToReset);

    // Start XOSC (by stopping it first...)
    xosc_hw->ctrl = (xosc_hw->ctrl & ~XOSC_CTRL_ENABLE_BITS) | (XOSC_CTRL_ENABLE_VALUE_DISABLE << XOSC_CTRL_ENABLE_LSB);
    while (xosc_hw->status & XOSC_STATUS_ENABLED_BITS);
    xosc_hw->startup = (xosc_hw->startup & ~XOSC_STARTUP_DELAY_BITS) | (8191 << XOSC_STARTUP_DELAY_LSB);
    xosc_hw->ctrl = (xosc_hw->ctrl & ~(XOSC_CTRL_FREQ_RANGE_BITS | XOSC_CTRL_ENABLE_BITS)) | (XOSC_CTRL_ENABLE_VALUE_ENABLE << XOSC_CTRL_ENABLE_LSB) | (XOSC_CTRL_FREQ_RANGE_VALUE_1_15MHZ << XOSC_CTRL_FREQ_RANGE_LSB);
    while ((xosc_hw->status & (XOSC_STATUS_STABLE_BITS | XOSC_STATUS_ENABLED_BITS)) != (XOSC_STATUS_STABLE_BITS | XOSC_STATUS_ENABLED_BITS));

    // Tell refclock to use XOSC
    clocks_hw->clk[clk_ref].ctrl = (clocks_hw->clk[clk_ref].ctrl & ~CLOCKS_CLK_REF_CTRL_SRC_BITS) | (CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC << CLOCKS_CLK_REF_CTRL_SRC_LSB);

    // Configure PLL for higher frequency
    pll_sys_hw->pwr |= (PLL_PWR_VCOPD_BITS | PLL_PWR_POSTDIVPD_BITS | PLL_PWR_PD_BITS); // Dividers on
    pll_sys_hw->fbdiv_int = (pll_sys_hw->fbdiv_int & ~PLL_FBDIV_INT_BITS) | ((TICKS_PER_SECOND / 1000000 / 2) << PLL_FBDIV_INT_LSB); // x100 = 1200MHz
    pll_sys_hw->prim = (pll_sys_hw->prim & ~(PLL_PRIM_POSTDIV1_BITS | PLL_PRIM_POSTDIV2_BITS)) | (6 << PLL_PRIM_POSTDIV1_LSB) | (1 << PLL_PRIM_POSTDIV2_LSB);
    pll_sys_hw->pwr &= ~(PLL_PWR_VCOPD_BITS | PLL_PWR_POSTDIVPD_BITS | PLL_PWR_PD_BITS); // Dividers on
    while (!(pll_sys_hw->cs & PLL_CS_LOCK_BITS));
    pll_sys_hw->cs &= ~PLL_CS_BYPASS_BITS;

    // Switch sys.AUX to PLL
    clocks_hw->clk[clk_sys].ctrl = (clocks_hw->clk[clk_sys].ctrl & ~CLOCKS_CLK_SYS_CTRL_AUXSRC_BITS) | (CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS << CLOCKS_CLK_SYS_CTRL_AUXSRC_LSB);

    // Switch sys to AUX and wait for it
    clocks_hw->clk[clk_sys].ctrl = (clocks_hw->clk[clk_sys].ctrl & ~CLOCKS_CLK_SYS_CTRL_SRC_BITS) | (CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX << CLOCKS_CLK_SYS_CTRL_SRC_LSB);
    while (((clocks_hw->clk[clk_sys].selected & CLOCKS_CLK_REF_SELECTED_BITS) >> CLOCKS_CLK_REF_SELECTED_LSB) != (1 << CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX));
}


static void gpiosConfig(bool firstTime)
{
	static struct {
		uint8_t pin;
		uint8_t padCfg;
		uint8_t funcSel			: 5;
		uint8_t oe				: 1;
		uint8_t val				: 1;
		uint8_t skipReconfig	: 1;	//do not reconfigure if not first call to gpiosConfig()
	} const cfgs[] = {
		{PIN_RAM_NCS, PADS_BANK0_GPIO0_SLEWFAST_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_XIP_SS_N_1, 1, 1},

		{PIN_LCD_DnC, 0, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 1, 0, 1},
		{PIN_LCD_SCK, PADS_BANK0_GPIO0_SLEWFAST_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_PIO0_0, 1, 0, 1},
		{PIN_LCD_DO, PADS_BANK0_GPIO0_SLEWFAST_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_PIO0_0, 1, 0, 1},
		{PIN_LCD_BL, 0, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_PWM_A_0, 1, 0, 1},
		{PIN_LCD_CS, 0, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 1, 1, 1},

		{PIN_TOUCHINT, PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_IE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},

		{PIN_WS2812, 0, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_PIO1_0, 1, 0},

		{PIN_SD_MISO, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SPI0_RX, 0, 0},
		{PIN_SD_NCS, PADS_BANK0_GPIO0_SLEWFAST_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 1, 1},
		{PIN_SD_SCK, PADS_BANK0_GPIO0_SLEWFAST_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SPI0_RX, 1, 0},
		{PIN_SD_MOSI, PADS_BANK0_GPIO0_SLEWFAST_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SPI0_RX, 1, 0},

		{PIN_BTN_L, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_PUE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},
		{PIN_BTN_R, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_PUE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},
		{PIN_BTN_U, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_PUE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},
		{PIN_BTN_D, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_PUE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},
		{PIN_BTN_A, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_PUE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},
		{PIN_BTN_B, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_PUE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},
		{PIN_BTN_START, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_PUE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},
		{PIN_BTN_SEL, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_PUE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},
		{PIN_BTN_CENTER, PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_PUE_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 0, 0},

		{PIN_IRDA_IN, PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO1_SCHMITT_BITS, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_PIO1_0, 0, 0},
		{PIN_IRDA_OUT, 0, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_PIO1_0, 1, 0},
		{PIN_IRDA_SD, 0, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 1, 1},

		{PIN_SELF_PWR, 0, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0, 1, 1},

		{PIN_SPQR, 0, IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_PWM_A_0, 1, 0},
	};
	
	uint32_t gpiosConfigged = 0;
	uint_fast8_t i;
	
	for (i = 0; i < sizeof(cfgs) / sizeof(*cfgs); i++) {
		
		uint_fast8_t pinNo = cfgs[i].pin;
		
		gpiosConfigged |= 1 << pinNo;

		if (firstTime || !cfgs[i].skipReconfig) {

			pads_bank0_hw->io[pinNo] = (pads_bank0_hw->io[pinNo] &~ (PADS_BANK0_GPIO0_ISO_BITS | PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_DRIVE_BITS | PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_PDE_BITS | PADS_BANK0_GPIO0_SCHMITT_BITS | PADS_BANK0_GPIO0_SLEWFAST_BITS)) | cfgs[i].padCfg;
			io_bank0_hw->io[pinNo].ctrl = (io_bank0_hw->io[pinNo].ctrl &~ IO_BANK0_GPIO0_CTRL_FUNCSEL_BITS) | (((uint32_t)cfgs[i].funcSel) << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);

			if (cfgs[i].oe) {

				if (cfgs[i].val)
					sio_hw->gpio_set = 1 << pinNo;
				else
					sio_hw->gpio_clr = 1 << pinNo;

				sio_hw->gpio_oe_set = 1 << pinNo;
			}
			else
				sio_hw->gpio_oe_clr = 1 << pinNo;
		}
	}

	//now the leftovers
	if (firstTime) {
		for (i = 0; i < 30; i++) {
			
			if (!(gpiosConfigged & (1 << i))) {
			
				pads_bank0_hw->io[i] |= PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_ISO_BITS;
				io_bank0_hw->io[i].ctrl |= IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_NULL;
			}
		}
	}
}

int main() {
    asm volatile("cpsie i");
	timebaseInit();

    configure_clocks_and_resets();

    gpiosConfig(true);

    ws2812init();

    ws2812SetAllBlue();

    // Setup display
    dispInit(30);
    dispSetContrast(5);
    dispSetBrightness(10);

    uiPreGame();

    while(1) {
        tight_loop_contents();
    }
}