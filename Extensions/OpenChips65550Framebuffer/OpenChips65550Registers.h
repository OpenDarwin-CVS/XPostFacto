/*

 Copyright (c) 2004
 Other World Computing
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer as
 the first lines of this file unmodified.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 This software is provided by Other World Computing ``as is'' and any express or
 implied warranties, including, but not limited to, the implied warranties
 of merchantability and fitness for a particular purpose are disclaimed.
 In no event shall Other World Computing or Ryan Rempel be liable for any direct, indirect,
 incidental, special, exemplary, or consequential damages (including, but
 not limited to, procurement of substitute goods or services; loss of use,
 data, or profits; or business interruption) however caused and on any
 theory of liability, whether in contract, strict liability, or tort
 (including negligence or otherwise) arising in any way out of the use of
 this software, even if advised of the possibility of such damage.
 
*/

#ifndef __OPENCHIPS65550REGISTERS_H__
#define __OPENCHIPS65550REGISTERS_H__

#define ATTRIBUTE_INDEX				0x3C0
	#define AR_MODE_CONTROL						0x10
	#define OVERSCAN_COLOR						0x11
	#define MEMORY_PLANE_ENABLE					0x12
	#define HORIZONTAL_PIXEL_PANNING			0x13
	#define COLOR_SELECT						0x14

#define SEQUENCER_INDEX				0x3C4
	#define RESET								0x00
	#define CLOCKING_MODE						0x01
	#define MAP_MASK							0x02
	#define MEMORY_MODE							0x04

#define MISCELLANEOUS_OUTPUT_WRITE				0x3C2   // write-only
#define INPUT_STATUS_REGISTER_0					0x3C2   // read-only

#define PIXEL_DATA_MASK_REGISTER				0x3C6
#define DAC_STATE_REGISTER						0x3C7   // read-only
#define PALETTE_READ_INDEX_REGISTER				0x3C7   // write-only
#define PALETTE_WRITE_INDEX_REGISTER			0x3C8
#define PALLETE_DATA_REGISTER					0x3C9

#define FEATURE_CONTROL_READ					0x3CA   // read-only
#define MISCELLANEOUS_OUTPUT_READ				0x3CC

#define INPUT_STATUS_REGISTER_1					0x3DA   // read-only
#define FEATURE_CONTROL_WRITE					0x3DA   // write-only

#define GRAPHICS_CONTROLLER_INDEX	0x3CE
	#define SET_RESET							0x00
	#define ENABLE_SET_RESET					0x01
	#define COLOR_COMPARE						0x02
	#define DATA_ROTATE							0x03
	#define READ_MAP_SELECT						0x04
	#define GRAPHICS_MODE						0x05
	#define MISCELLANEOUS						0x06
	#define COLOR_DONT_CARE						0x07
	#define BIT_MASK							0x08

#define FLAT_PANEL_INDEX			0x3D0
	#define FEATURE								0x00
	#define CRT_FP_CONTROL						0x01
	#define FP_MODE_CONTROL						0x02
	#define DOT_CLOCK_SOURCE					0x03
	#define PANEL_POWER_SEQUENCING_DELAY		0x04
	#define POWER_DOWN_CONTROL_1				0x05
	#define FP_POWER_DOWN_CONTROL				0x06
	#define PIN_POLARITY						0x08
	#define PROGRAMMABLE_OUTPUT_DRIVE			0x0A
	#define PIN_CONTROL_1						0x0B
	#define PIN_CONTROL_2						0x0C
	#define ACTIVITY_TIMER_CONTROL				0x0F
	#define PANEL_FORMAT_0						0x10
	#define PANEL_FORMAT_1						0x11
	#define PANEL_FORMAT_2						0x12
	#define PANEL_FORMAT_3						0x13
	#define FRC_OPTION_SELECT					0x16
	#define POLYNOMIAL_FRC_CONTROL				0x17
	#define TEXT_MODE_CONTROL					0x18
	#define BLINK_RATE_CONTROL					0x19
	#define FRAME_BUFFER_CONTROL				0x1A
	#define M_ACDCLCK_CONTROL					0x1E
	#define DIAGNOSTIC							0x1F
	#define HORIZONTAL_PANEL_SIZE_LSB			0x20
	#define HORIZONTAL_SYNC_START_LSB			0x21
	#define FP_HORIZONTAL_SYNC_END				0x22
	#define HORIZONTAL_TOTAL_LSB				0x23
	#define FP_HSYNC_LP_DELAY_LSB				0x24
	#define HORIZONTAL_OVERFLOW_1				0x25
	#define HORIZONTAL_OVERFLOW_2				0x26
	#define FP_HSYNC_LP_WIDTH_AND_DISABLE		0x27
	#define VERTICAL_PANEL_SIZE_LSB				0x30
	#define VERTICAL_SYNC_START_LSB				0x31
	#define FP_VERTICAL_SYNC_END				0x32
	#define FP_VERTICAL_TOTAL					0x33
	#define FP_VSYNC_FLM_DELAY_LSB				0x34
	#define VERTICAL_OVERFLOW_1					0x35
	#define VERTICAL_OVERFLOW_2					0x36
	#define FP_VSYNC_FLM_DISABLE				0x37
	#define HORIZONTAL_COMPENSATION				0x40
	#define VERTICAL_COMPENSATION_1				0x41
	#define VERTICAL_COMPENSATION_2				0x48
	#define VERTICAL_LINE_REPLICATION			0x4D
	#define SELECTIVE_VERTICAL_STRETCHING_DISABLE   0x4E

#define CRT_CONTROLLER_INDEX		0x3D4
	#define HORIZONTAL_TOTAL					0x00
	#define HORIZONTAL_DISPLAY_ENABLE_END		0x01
	#define HORIZONTAL_BLANKING_START			0x02
	#define HORIZONTAL_BLANKING_END				0x03
	#define HORIZONTAL_SYNC_START				0x04
	#define CRT_HORIZONTAL_SYNC_END				0x05
	#define CRT_VERTICAL_TOTAL					0x06
	#define OVERFLOW							0x07
	#define PRESET_ROW_SCAN						0x08
	#define MAXIMUM_SCAN_LINE					0x09
	#define START_ADDRESS_HIGH					0x0C
	#define START_ADDRESS_LOW					0x0D
	#define VERTICAL_SYNC_START					0x10
	#define CRT_VERTICAL_SYNC_END				0x11
	#define VERTICAL_DISPLAY_ENABLE_END			0x12
	#define OFFSET								0x13
	#define UNDERLINE_LOCATION					0x14
	#define VERTICAL_BLANKING_START				0x15
	#define VERTICAL_BLANKING_END				0x16
	#define CRT_MODE_CONTROL					0x17
	#define LINE_COMPARE						0x18
	#define EXTENDED_VERTICAL_TOTAL				0x30
	#define EXTENDED_VERTICAL_DISPLAY_END		0x31
	#define EXTENDED_VERTICAL_SYNC_START		0x32
	#define EXTENDED_VERTICAL_BLANKING_START	0x33
	#define EXTENDED_START_ADDRESS				0x40
	#define EXTENDED_OFFSET						0x41
	#define INTERLACE_CONTROL					0x70
	#define NTSC_PAL_VIDEO_OUTPUT_CONTROL		0x71
	#define HORIZONTAL_SERRATION_1_START		0x72
	#define HORIZONTAL_SERRATION_2_START		0x73
	#define NTSC_PAL_HORIZONTAL_PULSE_WIDTH		0x74

#define EXTENSION_INDEX					0x3D6
	#define LINEAR_BASE_ADDRESS_LOW				0x05
	#define LINEAR_BASE_ADDRESS_HIGH			0x06
	#define CONFIGURATION						0x08	// read-only
	#define IO_CONTROL							0x09
	#define ADDRESS_MAPPING						0x0A
	#define BURST_WRITE_MODE					0x0B
	#define PAGE_SELECT							0x0E
	#define BITBLT_ENGINE_CONFIGURATION			0x20
	#define DRAM_TYPE							0x40
	#define BITBLT_CONTROL						0x41
	#define DRAM_CONFIGURATION					0x42
	#define DRAM_INTERFACE						0x43
	#define VIDEO_PIN_CONTROL					0x60
	#define DDC_SYNC_SELECT						0x61
	#define GPIO_DATA_1							0x62
	#define GPIO_DATA_2							0x63
	#define PIN_TRI_STATE_CONTROL				0x67
	#define CONFIGURATION_PINS_0				0x70	// read-only
	#define CONFIGURATION_PINS_1				0x71	// read-only
	#define PIXEL_PIPELINE_CONFIGURATION_0		0x80
	#define PIXEL_PIPELINE_CONFIGURATION_1		0x81
	#define PIXEL_PIPELINE_CONFIGURATION_2		0x82
	#define CURSOR_1_CONTROL					0xA0
	#define CURSOR_1_VERTICAL_EXTENSION			0xA1
	#define CURSOR_1_BASE_ADDRESS_LOW			0xA2
	#define CURSOR_1_BASE_ADDRESS_HIGH			0xA3
	#define CURSOR_1_X_POSITION_LOW				0xA4
	#define CURSOR_1_X_POSITION_HIGH			0xA5
	#define CURSOR_1_Y_POSITION_LOW				0xA6
	#define CURSOR_1_Y_POSITION_HIGH			0xA7
	#define CURSOR_2_CONTROL					0xA8
	#define CURSOR_2_VERTICAL_EXTENSION			0xA9
	#define CURSOR_2_BASE_ADDRESS_LOW			0xAA
	#define CURSOR_2_BASE_ADDRESS_HIGH			0xAB
	#define CURSOR_2_X_POSITION_LOW				0xAC
	#define CURSOR_2_X_POSITION_HIGH			0xAD
	#define CURSOR_2_Y_POSITION_LOW				0xAE
	#define CURSOR_2_Y_POSITION_HIGH			0xAF
	#define VIDEO_CLOCK_0_VCO_M_DIVISOR			0xC0
	#define VIDEO_CLOCK_0_VCO_N_DIVISOR			0xC1
	#define VIDEO_CLOCK_0_VCO_MN_DIVISOR		0xC2
	#define VIDEO_CLOCK_0_DIVISOR_SELECT		0xC3
	#define VIDEO_CLOCK_1_VCO_M_DIVISOR			0xC4
	#define VIDEO_CLOCK_1_VCO_N_DIVISOR			0xC5
	#define VIDEO_CLOCK_1_VCO_MN_DIVISOR		0xC6
	#define VIDEO_CLOCK_1_DIVISOR_SELECT		0xC7
	#define VIDEO_CLOCK_2_VCO_M_DIVISOR			0xC8
	#define VIDEO_CLOCK_2_VCO_N_DIVISOR			0xC9
	#define VIDEO_CLOCK_2_VCO_MN_DIVISOR		0xCA
	#define VIDEO_CLOCK_2_DIVISOR_SELECT		0xCB
	#define MEMORY_CLOCK_VCO_M_DIVISOR			0xCC
	#define MEMORY_CLOCK_VCO_N_DIVISOR			0xCD
	#define MEMORY_CLOCK_DIVISOR_SELECT			0xCE
	#define CLOCK_CONFIGURATION					0xCF	// read-only
	#define MODULE_POWERDOWN_0					0xD0

#endif