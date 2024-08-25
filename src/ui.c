#include <stdarg.h>
#include <string.h>
<<<<<<< HEAD
#include "fonts.h"
#include "utf.h"
#include "ui.h"
#include "printf.h"
#include "input.h"
#include "display.h"
#include "pio/pinoutRp2350defcon.h"
#include "hardware/regs/sio.h"
#include "hardware/structs/sio.h"
#include "timebase.h"
=======
#include "gameboy/gbCartHeader.h"
#include "settings.h"
#include "memMap.h"
#include "util/printf.h"
#include "sleep.h"
#include "util/fonts.h"
#include "sd/fatfs.h"
#include "qspi.h"
#include "2350.h"
#include "gameboy/mbc.h"
#include "util/utf.h"
#include "ui.h"
#include "sd/sd.h"
#include "gameboy/gb.h"

//#include "usb.h"
//#include "tusb.h"
>>>>>>> master

#include "usb.h"

#define MENU_SELECTION_CHAR				0xBB /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */

#ifdef UI_ROTATED
	#undef UI_ROTATED
	#define UI_ROTATED 1
#else
	#define UI_ROTATED	0
#endif


#define CANVAS_INITIALIZER				{.framebuffer = dispGetFb(), .w = UI_ROTATED ? DISP_HEIGHT : DISP_WIDTH, .h = UI_ROTATED ? DISP_WIDTH : DISP_HEIGHT, .bpp = DISP_BPP, .indexedLe = DISP_INDEXED_LE, .rotated = UI_ROTATED, }



struct ScreenPrintfData {
	struct Canvas *cnv;
	bool needsPreSpace;
	int32_t r, c;
	struct Utf8state u8s;
};

enum CurOp {
	CurentlyIdle,
	CurrentlyReading,
	CurrentlyWriting,	
};

static enum CurOp mCurCardOp = CurentlyIdle;
static uint32_t mCurCardSec;


enum DialogType {
	DialogTypeOk,
	DialogTypeYesNo,
};


struct BitBufferR {
	const uint8_t *src;

	uint8_t bitBuf;
	uint8_t numBitsHere;
};

#if DISP_BPP > 8
	
	static uint_fast16_t uiPrvGreyToColor(uint_fast8_t grey)
	{
		uint_fast16_t ret = grey & 0x0f;
		uint_fast16_t ret5 = ret * 2 + ret / 8;
		uint_fast16_t ret6 = ret * 4 + ret / 4;

		
		return (ret5 << 11) + (ret6 << 5) + ret5;
	}

#endif


static uint_fast8_t uiPrvGlyphHeight(const struct Canvas *cnv)
{
	return fontGetHeight((enum Font)cnv->font);
}

static uint_fast8_t uiPrvDrawOneChar(struct Canvas *cnv, int32_t r, int32_t c, wchar_t chr)	//return char width
{
	uint_fast8_t glyphHeight = uiPrvGlyphHeight(cnv), dr, dc;
	uint16_t *fb16 = cnv->framebuffer;
	uint8_t *fb8 = cnv->framebuffer;
	struct FontGlyphInfo gi;


	if (r <= -(signed)glyphHeight || r >= (int32_t)cnv->h || c >= (int32_t)cnv->w)
		return 0;
	
	if (!fontGetGlyphInfo(&gi, (enum Font)cnv->font, chr))
		return 0;
	
	if (c + gi.width <= 0)
		return 0;

	for (dr = 0; dr < gi.height; dr++) {
		for (dc = 0; dc < gi.width; dc++) {
			
			int32_t er = r + (int_fast16_t)(uint_fast16_t)dr, ec = c + (int_fast16_t)(uint_fast16_t)dc;
			bool on = fontGetGlyphPixel(&gi, dr, dc);
			int8_t color = on ? cnv->foreColor : cnv->backColor; 
			uint32_t ep;

			if (cnv->flipped)
				ep = cnv->rotated ? (cnv->w - 1 - ec) * cnv->h + er : (cnv->h - 1 - er) * cnv->w + (cnv->w - 1 - ec);
			else
				ep = cnv->rotated ? ec * cnv->h + cnv->h - 1 - er : er * cnv->w + ec;
			
			if (er >= 0 && (uint32_t)er < cnv->h && ec >= 0 && (uint32_t)ec < cnv->w && color >= 0) {
				
				#if DISP_BPP <= 8
					uint32_t mask = (1 << cnv->bpp) - 1, shift = ep * cnv->bpp % 8;
					uint8_t *pixByte = &fb8[ep * cnv->bpp / 8];
					
					if (!cnv->indexedLe)
						shift = 8 - cnv->bpp - shift;
					
					*pixByte = ((*pixByte) &~ (mask << shift)) + (color << shift);
				#else
					
					fb16[ep] = uiPrvGreyToColor(color);
					
				#endif
			}
			
		}
	}
	
	return gi.width;
}

static uint32_t uiPrvCharsMeasure(struct Canvas *cnv, const char *chars, uint32_t len, uint32_t maxWidth, uint32_t *numCharsDrawnP)	//returns width used
{
	struct Utf8state u8s = UTF8_STATE_STATIC_INITIALIZER;
	bool needPreSpace = false;
	struct FontGlyphInfo gi;
	uint32_t i, w = 0;
	wchar_t ch;
	
	for (i = 0; i < len; i++) {
		
		uint32_t ret, prevI;
		uint32_t chrWidth, nowWidth = 0;
		
		if (needPreSpace) {		//inter-char spacing
			

			if (fontGetGlyphInfo(&gi, (enum Font)cnv->font, 0x2009))
				nowWidth += gi.width;
		}

		prevI = i;
		while ((ret = utf8inputByte(&u8s, (unsigned char)*chars++)) == UTF_NO_OUTPUT && i < len)
			i++;

		if (ret == UTF_NO_OUTPUT || ret == UTF_ERROR || ret == 0) {
			
			i = prevI;
			break;
		}
		
		ch = ret;
		
		needPreSpace = ch != ' ';
		chrWidth = fontGetGlyphInfo(&gi, (enum Font)cnv->font, ch) ? gi.width : 0;
		nowWidth += chrWidth;
		
		if (w + nowWidth > maxWidth)
			break;
		w += nowWidth;
	}
	
	if (numCharsDrawnP)
		*numCharsDrawnP = i;
	
	return w;
}

static uint32_t uiPrvCharsWidth(struct Canvas *cnv, const char *chars, uint32_t len)
{
	return uiPrvCharsMeasure(cnv, chars, len, 0xffffffff, NULL);
}

static uint32_t uiPrvCharsFit(struct Canvas *cnv, const char *chars, uint32_t len, uint32_t maxWidth)
{
	uint32_t numCharsDrawn;
	
	(void)uiPrvCharsMeasure(cnv, chars, len, maxWidth, &numCharsDrawn);
	
	return numCharsDrawn;
}

static void uiPrvPrintCbk(void *userData, char chr)
{
	struct ScreenPrintfData *spd = (struct ScreenPrintfData*)userData;
	uint32_t ret = utf8inputByte(&spd->u8s, chr);

	if (ret == UTF_ERROR || ret == UTF_NO_OUTPUT)
		return;
	
	if (spd->needsPreSpace)
		spd->c += uiPrvDrawOneChar(spd->cnv, spd->r, spd->c, 0x2009);	//thin space
	spd->needsPreSpace = ret != ' ';
	spd->c += uiPrvDrawOneChar(spd->cnv, spd->r, spd->c, ret);
}

static uint32_t uiPuts(struct Canvas *cnv, int32_t r, int32_t c, const char *str, uint32_t maxLen)		//returns num columns used
{
	struct ScreenPrintfData spd = {.cnv = cnv, .r = r, .c = c, .u8s = UTF8_STATE_STATIC_INITIALIZER};
	uint32_t nDrawn = 0;
	wchar_t ch;
	
	while (maxLen-- && (ch = (unsigned char)*str++) != 0) {
		
		nDrawn++;
		uiPrvPrintCbk(&spd, ch);
	}
	
	return spd.c - c;
}

static void uiVprintf(struct Canvas *cnv, int32_t r, int32_t c, const char *format, va_list vl)
{
	struct ScreenPrintfData spd = {.cnv = cnv, .r = r, .c = c, .u8s = UTF8_STATE_STATIC_INITIALIZER};

	(void)vxprintf(&spd, uiPrvPrintCbk, format, vl);
}

static void uiPrintf(struct Canvas *cnv, int32_t r, int32_t c, const char *format, ...)
{
	struct ScreenPrintfData spd = {.cnv = cnv, .r = r, .c = c, .u8s = UTF8_STATE_STATIC_INITIALIZER};
	va_list vl;
	
	va_start(vl, format);
	(void)vxprintf(&spd, uiPrvPrintCbk, format, vl);
	va_end(vl);
}

uint_fast8_t uiGetKeys(void)
{
	uint32_t val, count = 0, countUntil = 10000, ourKeysMask = (1 << PIN_BTN_U) | (1 << PIN_BTN_D) | (1 << PIN_BTN_L) | (1 << PIN_BTN_R) | (1 << PIN_BTN_START) | (1 << PIN_BTN_SEL) | (1 << PIN_BTN_A) | (1 << PIN_BTN_B) | (1 << PIN_BTN_CENTER);

	while(1) {
		val = sio_hw->gpio_in & ourKeysMask;
		for (count = 0; count < countUntil && val == (sio_hw->gpio_in & ourKeysMask); count++);
		if (count == countUntil)
			return prvKeysMap(val);
	}
}

	static void uiPrvFillRectEx(struct Canvas *cnv, int32_t left, int32_t top, int32_t right, int32_t bottom, uint32_t pxColor)
	{
		uint32_t w, h, r, c, rowItems = cnv->rotated ? cnv->h : cnv->w;
		uint16_t *fb, *fbRow = cnv->framebuffer;
		
		if (cnv->flipped) {
			int32_t t;

			t = left;
			left = cnv->w - 1 - right;
			right = cnv->w - 1 - t;

			t = top;
			top = cnv->h - 1 - bottom;
			bottom = cnv->h - 1 - t;
		}

		if (left < 0)
			left = 0;
		if (top < 0)
			top = 0;
		if (right >= (int32_t)cnv->w)
			right = cnv->w - 1;
		if (bottom >= (int32_t)cnv->h)
			bottom = cnv->h - 1;
		
		if (left > right || top > bottom)
			return;
		
		if (cnv->rotated) {
			int32_t t;

			t = left;
			left = cnv->h - 1 - bottom;
			bottom = right;
			right = cnv->h - 1 - top;
			top = t;
		}

		w = right - left + 1;
		h = bottom - top + 1;
			
		fbRow += rowItems * top + left;	//go to right word
		
		for (r = 0; r < h; r++) {
						
			//pointers
			fb = fbRow;
			fbRow += rowItems;
			
			//full words
			for(c = 0; c < w; c++)
				*fb++ = pxColor;
		}
	}

	static void uiPrvFillRect(struct Canvas *cnv, int32_t left, int32_t top, int32_t right, int32_t bottom)
	{
		uiPrvFillRectEx(cnv, left, top, right, bottom, uiPrvGreyToColor(cnv->foreColor));
	}

static uint_fast8_t uiPrvRecvKeypress(void)
{
	uint_fast8_t val, prevVal = 0;
	
	while (!uiGetKeys());
	while ((val = uiGetKeys()) != 0)
		prevVal = val;
	
	return prevVal;
}

static uint_fast8_t uiPrvMenu(struct Canvas *cnv, uint_fast8_t curChoice, uint_fast8_t numChoices, uint8_t *btnsMaskP /* if passed in, return val is the button that was pressed */)
{
	uint_fast8_t i, itemHeight = uiPrvGlyphHeight(cnv) + 1, row = cnv->h - numChoices * itemHeight, fore = cnv->foreColor, back = cnv->backColor, gotKey;
	uint8_t btnsMask = btnsMaskP ? *btnsMaskP : KEY_BIT_A;
	
	while (1) {
		
		for (i = 0; i < numChoices; i++) {
			
			cnv->foreColor = (i == curChoice) ? fore : back;
			uiPrvDrawOneChar(cnv, row + itemHeight * i, 1, MENU_SELECTION_CHAR);
		}
		
		switch (gotKey = uiPrvRecvKeypress()) {
			case KEY_BIT_DOWN:
				if (curChoice < numChoices - 1)
					curChoice++;
				break;
			
			case KEY_BIT_UP:
				if (curChoice)
					curChoice--;
				break;
			
			default:
				if (btnsMask & gotKey) {
					if (btnsMaskP)
						*btnsMaskP = gotKey;
					return curChoice;
				}
				break;
		}
	}
	cnv->foreColor = fore;
}

static void uiPrvReset(struct Canvas *cnv, bool invert)
{
	static const char windowTitle[] = "MorgenBadge v0.1";
	memset(cnv->framebuffer, invert ? 0xff : 0, cnv->w * cnv->h * DISP_BPP / 8);
	
	//draw title
	cnv->foreColor = invert ? 0 : 9;
	cnv->font = FontLarge;
	uiPrvFillRect(cnv, 0, 0, cnv->w - 1, uiPrvGlyphHeight(cnv));
	cnv->foreColor = invert ? 9 : 0;
	cnv->backColor = invert ? 0 : 9;
	uiPrintf(cnv, 0, (cnv->w - uiPrvCharsWidth(cnv, windowTitle, sizeof(windowTitle) - 1)) / 2, windowTitle);

	//set colors for ui
	cnv->font = FontMedium;
	cnv->foreColor = invert ? 0 : 15;
	cnv->backColor = invert ? 15 : 0;
}

static uint32_t uiPrvDrawWrappedString(struct Canvas *cnv, const char *str, uint32_t r, uint32_t c)	//return num rows printed
{
	uint32_t t, i, numRowsUsed = 1;
	
	while (*str) {
		
		uint32_t nCharsFit = 0;
		
		//handle newlines
		while (*str == '\n') {
			r += uiPrvGlyphHeight(cnv);
			c = 0;
			str++;
			numRowsUsed++;
		}
		
		//handle being at the end of the screen
		if (c >= cnv->w) {
			r += uiPrvGlyphHeight(cnv);
			c = 0;
			numRowsUsed++;
		}
		
		//find string or line end
		for (t = 0; str[t] && str[t] != '\n'; t++);

		//see how many chars fit
		nCharsFit = uiPrvCharsFit(cnv, str, t, cnv->w - c);

		//find word end before the fit limit
		if (nCharsFit < t) {
			t = nCharsFit;
			while (t && str[t] != ' ' && str[t] != '\t' && str[t] != '-')
				t--;
		}

		//if rewinding back to last break caused us to end up with nothing, take extra actions
		if (t == 0 && (nCharsFit < 3 || t <= uiPrvCharsFit(cnv, str, t, cnv->w))) {
			
			r += uiPrvGlyphHeight(cnv);
			c = 0;
			numRowsUsed++;
			continue;
		}
		nCharsFit = t;
		
		c += uiPuts(cnv, r, c, str, nCharsFit);
		str += nCharsFit;
		
		//draw spaces right where we are (including offscreen)
		while (*str == ' ') {
			
			if (c < cnv->w)
				c += uiPrvDrawOneChar(cnv, r, c, ' ');
			str++;
		}
	}
	
	return numRowsUsed;
}

static bool uiPrvGetSimpleAnswer(struct Canvas *cnv, enum DialogType dialogType)
{
	static const char *textTypes[] = {
		[DialogTypeOk] = " A = OK ",
		[DialogTypeYesNo] = " A = Yes    B = No ",
	};

	const char *opts;
	uint_fast8_t key;

	//get message
	if ((unsigned)dialogType >= sizeof(textTypes) / sizeof(*textTypes) || !textTypes[dialogType])
		opts = textTypes[DialogTypeOk];
	else
		opts = textTypes[dialogType];
	
	cnv->foreColor = 0;
	cnv->backColor = 9;
	cnv->font = FontMedium;
	uiPrintf(cnv, cnv->h - uiPrvGlyphHeight(cnv) - 1, (cnv->w - uiPrvCharsWidth(cnv, opts, strlen(opts))) / 2, opts);
	while (1) {
		key = uiPrvRecvKeypress();
		
		if (key == KEY_BIT_A)
			return true;
		
		if (key == KEY_BIT_B)
			return dialogType == DialogTypeOk;
	}
}

static bool uiAlert(struct Canvas *cnv, const char *msg, enum DialogType dialogType)
{
	const char *msgEnd = msg + strlen(msg);
	uint32_t y;

	pr("UI_ALERT(%s)\n", msg);

	uiPrvReset(cnv, false);
	cnv->font = FontLarge;
	y = uiPrvGlyphHeight(cnv) + 1;

	cnv->font = FontMedium;
	uiPrvDrawWrappedString(cnv, msg, y, 10);
	
	return uiPrvGetSimpleAnswer(cnv, dialogType);
}

static uint_fast8_t uiPrvDrawScrollbar(struct Canvas *cnv, uint_fast16_t scrollTop, uint32_t numItems, uint32_t topItem, uint32_t onscreenItems)	//return width used
{
	const uint_fast16_t scrollWidth = 5, scrollBottom = cnv->h - 1, scrollRight = cnv->w - 1, scrollLeft = scrollRight - scrollWidth + 1, arrowsHeight = (scrollWidth + 1) / 2, carWidth = 3;
	uint_fast16_t i, lineHeight, carHeight, remainLineHeight, carPos;
	
	//do not bother if not enough items to scroll
	if (onscreenItems >= numItems)
		return 0;
	
	//erase the area
	cnv->foreColor = 0;
	uiPrvFillRect(cnv, scrollLeft - 1, scrollTop, scrollRight, scrollBottom);
	
	//draw arrows
	cnv->foreColor = 13;
	for (i = 0; i < arrowsHeight; i++) {
		uiPrvFillRect(cnv, scrollLeft + (arrowsHeight - 1 - i), scrollTop + i, scrollRight - (arrowsHeight - 1 - i), scrollTop + i);
		uiPrvFillRect(cnv, scrollLeft + (arrowsHeight - 1 - i), scrollBottom - i, scrollRight - (arrowsHeight - 1 - i), scrollBottom - i);
	}
	
	//draw line
	cnv->foreColor = 8;
	uiPrvFillRect(cnv, scrollLeft + scrollWidth / 2, scrollTop + arrowsHeight, scrollLeft + scrollWidth / 2, scrollBottom - arrowsHeight);
	
	//calc car
	//car height should represent what percent of the view we see, but it also needs to be reasonble in size (not too small or big)
	//first calc a fair car
	lineHeight = (scrollBottom - scrollTop + 1) - 2 * arrowsHeight - 2; /* 2 to allow one pix space between car and arrows */
	carHeight = (onscreenItems * lineHeight + (numItems - onscreenItems) / 2) / (numItems - onscreenItems);
	//then adjust to min/max
	if (carHeight < 8)
		carHeight = 8;
	if (carHeight > lineHeight * 7 / 8)
		carHeight = lineHeight * 7 / 8;
	
	//calc position
	remainLineHeight = lineHeight - carHeight;
	carPos = (topItem * remainLineHeight + (numItems - onscreenItems) / 2) / (numItems - onscreenItems);
	
	//avoid the visual mistake of showing the car at the limit of travel if more items exist in the direction
	if (carPos == 0 && topItem != 0)
		carPos++;
	else if (carPos + carHeight == lineHeight && topItem != numItems - onscreenItems)
		carPos--;
	
	carPos++;	/* account for that space between car and arrows */

	//draw the car
	cnv->foreColor = 12;
	uiPrvFillRect(cnv, scrollLeft + (scrollWidth - carWidth) / 2, carPos + arrowsHeight + scrollTop, scrollRight - (scrollWidth - carWidth) / 2, carPos + arrowsHeight + scrollTop + carHeight - 1);
	
	return scrollWidth + 1;
}

static void uiPrvDrawTruncText(struct Canvas *cnv, int32_t r, int32_t c, uint32_t maxWidth, const char *str)
{
	uint32_t stringLen = strlen(str), numCharsFit, widthUsed = uiPrvCharsMeasure(cnv, str, stringLen, maxWidth, &numCharsFit);
	static const char truncInd[] = "...";

	if (numCharsFit == stringLen)
		uiPuts(cnv, r, c, str, stringLen);
	else {
		
		uint32_t truncIndWidth = uiPrvCharsWidth(cnv, truncInd, sizeof(truncInd) - 1);
		
		if (truncIndWidth >= maxWidth)		//do not bother with the impossible cases
			return;
		
		widthUsed = uiPrvCharsMeasure(cnv, str, stringLen, maxWidth - truncIndWidth, &numCharsFit);
		uiPuts(cnv, r, c, str, numCharsFit);
		uiPuts(cnv, r, c + widthUsed, truncInd, sizeof(truncInd) - 1);
	}
}

static void __attribute__((noinline)) uiPrvCheckUSBConn(struct Canvas *cnv) {
	uiPrvReset(cnv, false);

	if(!isUsbMounted()) {
		uiAlert(cnv, 
					"No USB Connection Found!\n"
				, DialogTypeOk);
		return;
	}
	else {
		uiAlert(cnv, 
					"USB Connected!\n"
				, DialogTypeOk);
		return;
	}
}

static void __attribute__((noinline)) uiPrvUSBDucky(struct Canvas *cnv) {
	uiPrvReset(cnv, false);

	if(!isUsbMounted()) {
		uiAlert(cnv, 
					"Error: Check USB Connection!\n"
				, DialogTypeOk);
			return;
	}

	while(1) {
		int_fast8_t duckyTest = -1, backOption, itemHeight, selOption, numOptions=0;

		uiPrvReset(cnv, false);
		itemHeight = uiPrvGlyphHeight(cnv) + 1;

		// Ducky Test
		duckyTest = numOptions++;
		uiPuts(cnv, cnv->h - 2 * itemHeight, 10, "Ducky Test", -1);

		//Back option
		backOption = numOptions++;
		uiPuts(cnv, cnv->h - 1 * itemHeight, 10, "Back", -1);

		// See what option was chosen
		selOption = uiPrvMenu(cnv, 0, numOptions, NULL);
		if(selOption == duckyTest) {
			
			if(!usbDuckyTest()) {
				uiAlert(cnv, 
					"Ducky Test Failed!\n"
				, DialogTypeOk);
			}
			else {
				uiAlert(cnv, 
					"Ducky Test Passed!\n"
				, DialogTypeOk);
			}

			return;
		}
		else if(selOption == backOption) {
			break;
		}
	}
}

static void __attribute__((noinline)) uiPrvMorgenHacks(struct Canvas *cnv) {
	uiPrvReset(cnv, false);

	while(1) {
		int_fast8_t usbEnumOption = -1, usbDuckyOption, backOption, itemHeight, selOption, numOptions=0;

		uiPrvReset(cnv, false);
		itemHeight = uiPrvGlyphHeight(cnv) + 1;

		usbDuckyOption = numOptions++;
		uiPuts(cnv, cnv->h - 3 * itemHeight, 10, "USB Ducky", -1);

		// USB Device Enumeration option
		usbEnumOption = numOptions++;
		uiPuts(cnv, cnv->h - 2 * itemHeight, 10, "USB Connection Status", -1);

		//Back option
		backOption = numOptions++;
		uiPuts(cnv, cnv->h - 1 * itemHeight, 10, "Back", -1);

		// See what option was chosen
		selOption = uiPrvMenu(cnv, 0, numOptions, NULL);
		if(selOption == usbEnumOption) {
			uiPrvCheckUSBConn(cnv);
		}
		else if(selOption == usbDuckyOption) {
			uiPrvUSBDucky(cnv);
		}
		else if(selOption == backOption) {
			break;
		}
	}
}

static bool __attribute__((noinline)) uiPrvCommon(void)		//return true if emulator needs to restart
{
	struct Canvas canvas = CANVAS_INITIALIZER, *cnv = &canvas;
	bool ret = false;
	

	uiPrvReset(cnv, false);
	//uiPrvFwUpdate(cnv, true);

	while (1) {
		
		int_fast8_t playOption = -1, selectGameOption = -1, aboutOption, settingsOption, fwUpdateOption, rollOption, powerOffOption, numOptions = 0, selOption, itemHeight, morgenHackOption;
	
		uiPrvReset(cnv, false);
		itemHeight = uiPrvGlyphHeight(cnv) + 1;
	
		

		// Morgen hack option
		morgenHackOption = numOptions++;
		uiPuts(cnv, cnv->h - 4 * itemHeight, 10, "Morgen's Hacks", -1);

		settingsOption = numOptions++;
		uiPuts(cnv, cnv->h - 3 * itemHeight, 10, "Settings", -1);
		aboutOption = numOptions++;
		uiPuts(cnv, cnv->h - 2 * itemHeight, 10, "About...", -1);
		powerOffOption = numOptions++;
		uiPuts(cnv, cnv->h - 1 * itemHeight, 10, "Power Off", -1);

		selOption = uiPrvMenu(cnv, 0, numOptions, NULL);
		if (selOption == aboutOption) {
			
			uiAlert(cnv, 
					"uGB/TWISTED NEMATIC WITH MONULAR BEEPS\n"
					"Now with Some Cool Hacking Tools!\n"
					"HW: Rachel, MP, George, Matt, Louis\n"
					"SW: dmitryGR\n"
					"Morgen Hacks: Me!\n"
					"\n"
					"\xC2\xA9 2024\0""2024"
				, DialogTypeOk);
		}
		else if (selOption == settingsOption) {
			
			//if (uiPrvSettings(cnv))
				//ret = true;
		}
		else if (selOption == powerOffOption) {
			
			//doSleep();
		}

		else if (selOption == morgenHackOption) {
			uiPrvMorgenHacks(cnv);
		}
	}
	
	return ret;
}

void uiPreGame(void)
{
	struct Canvas canvas = CANVAS_INITIALIZER, *cnv = &canvas;

	uiPrvReset(cnv, false);

	//uiPrvFwUpdate(cnv, true);


	(void)uiPrvCommon();
}
