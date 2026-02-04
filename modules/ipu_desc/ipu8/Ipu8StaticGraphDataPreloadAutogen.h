/*
* INTEL CONFIDENTIAL
* Copyright (c) 2026 Intel Corporation
* All Rights Reserved.
*
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation or its
* suppliers or licensors. Title to the Material remains with Intel
* Corporation or its suppliers and licensors. The Material may contain trade
* secrets and proprietary and confidential information of Intel Corporation
* and its suppliers and licensors, and is protected by worldwide copyright
* and trade secret laws and treaty provisions. No part of the Material may be
* used, copied, reproduced, modified, published, uploaded, posted,
* transmitted, distributed, or disclosed in any way without Intel's prior
* express written permission.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or
* delivery of the Materials, either expressly, by implication, inducement,
* estoppel or otherwise. Any license under such intellectual property rights
* must be express and approved by Intel in writing.
*
* Unless otherwise agreed by Intel in writing, you may not remove or alter
* this notice or any other notice embedded in Materials by Intel or Intels
* suppliers or licensors in any way.
*/

#ifndef DATA_RANGE_H_
#define DATA_RANGE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0

typedef struct
{
    uint32_t key;
    uint32_t value;
}GraphHashCode;

typedef struct
{
    uint32_t numOfGraphs;
    GraphHashCode* hashCodes;
}GraphHashCodesTable;

typedef struct {
    char* format;
    char* subFormat;
    unsigned long fourCC;
    int Bpp;
} FrameFormatDesc;

typedef enum
{
    enPreview,
    enVideo,
    enStills,
    enRaw,
    enIr,
    enNumOfOutPins

}DataRangePins;

typedef struct {

    uint32_t sapAttributes;
    uint32_t binaryCommonHashCode;
    uint32_t numberOfResolutions;
    uint32_t numberOfSensorModes;

}BinaryHeader;

typedef struct DriverDesc {

    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t fps;

}DriverDesc;

typedef struct DataRangeHeader {

    uint32_t NumberOfPinResolutions[enNumOfOutPins];

}DataRangeHeader;

typedef struct DataRange
{
    DataRangeHeader dataRangeHeader;
    DriverDesc** dataRangeMap;

}DataRange;

static int distinctGraphsCount = 142;

static GraphHashCode hashCodeLookup[] = {
       {0, 0x4229ABEE},
       {100000, 0x274DFCAB},
       {100001, 0x513C5C0E},
       {100002, 0xDFF5303},
       {100003, 0xF1CDFFC2},
       {100004, 0x70DEA50D},
       {100005, 0x113D8DE4},
       {100006, 0x49A083FD},
       {100007, 0x897C7BC0},
       {100008, 0x7D0963A7},
       {100009, 0xAD054036},
       {100010, 0x49AB695F},
       {100011, 0x52A76E5A},
       {100012, 0x7304B1F5},
       {100013, 0xC01767C2},
       {100014, 0x1B86E725},
       {100015, 0xD849E6E6},
       {100016, 0xC7DF838C},
       {100017, 0xC5D56877},
       {100018, 0x79E8D648},
       {100019, 0x1584533F},
       {100020, 0xE4755386},
       {100021, 0x6DD04F23},
       {100022, 0x6656A3CE},
       {100023, 0xDD5FE263},
       {100024, 0x72B86072},
       {100026, 0xCAE45B9C},
       {100027, 0x1CF9C737},
       {100028, 0x4D910412},
       {100029, 0x20B9B42F},
       {100030, 0x6B1BD1A6},
       {100031, 0xA3184D5F},
       {100032, 0xF5E37182},
       {100033, 0xE1EBD877},
       {100034, 0xD0B0EF16},
       {100035, 0xF195CFAA},
       {100036, 0xF195CFAA},
       {100037, 0x2185E008},
       {100038, 0x72282526},
       {100039, 0x72282526},
       {100040, 0xB4DF03EB},
       {100041, 0xBB37A983},
       {100042, 0x7C4EE476},
       {100045, 0xB2139259},
       {100058, 0x2185E008},
       {100059, 0x2185E008},
       {100066, 0xE2462673},
       {100067, 0x13B71B02},
       {100079, 0x1FBFBDB5},
       {100080, 0x8FE2FDB7},
       {100081, 0xEECE4A7},
       {100100, 0x3A619A16},
       {100101, 0x6E21DB67},
       {100102, 0x9237750},
       {100103, 0x9C19F4AD},
       {100104, 0x3F7B1354},
       {100105, 0xFC0DC1B1},
       {100106, 0x74EB66E2},
       {100107, 0xCBB2C27B},
       {100108, 0xAA8CF966},
       {100109, 0x8450111B},
       {100110, 0xA0A1E120},
       {100111, 0xF3242241},
       {100112, 0xC90E28E6},
       {100113, 0x2E3F7547},
       {100114, 0xB78BDD52},
       {100115, 0xF1F392CD},
       {100116, 0x647BC377},
       {100117, 0x96CFE776},
       {100118, 0xDF585AF5},
       {100119, 0x34B24F18},
       {100120, 0x997F9943},
       {100121, 0x467C7722},
       {100122, 0x7691A361},
       {100123, 0xB2A106F0},
       {100127, 0x27ADB174},
       {100128, 0xAA0D88A1},
       {100129, 0xFB2764DA},
       {100130, 0x33D88713},
       {100131, 0x9B9DA69A},
       {100132, 0xE8A1B49B},
       {100133, 0x576FD1E4},
       {100134, 0xEA7E87D1},
       {100135, 0xC8271562},
       {100136, 0x64F939BA},
       {100137, 0x4545E450},
       {100138, 0xC77E7A18},
       {100139, 0x7B895C1A},
       {100140, 0x9AE28560},
       {100141, 0x54222F24},
       {100142, 0x916CAAF2},
       {100143, 0x770C7E68},
       {100144, 0x11B9A10C},
       {100145, 0x8AFD0DF5},
       {100146, 0x782FB5A8},
       {100157, 0x49262A9A},
       {100162, 0x17DB7D62},
       {100166, 0xA455C810},
       {100169, 0x17DB7D62},
       {100200, 0x7DCF6206},
       {100201, 0x9FE3E955},
       {100202, 0xB8A7C9D7},
       {100203, 0xD7CA4E76},
       {100204, 0x62FBCD41},
       {100205, 0x70439A78},
       {100206, 0x1EE4C961},
       {100207, 0xBADC1484},
       {100208, 0x4DB85EF3},
       {100209, 0x847FF112},
       {100210, 0xB50F811B},
       {100211, 0x1F3767A6},
       {100212, 0xB3780C1},
       {100213, 0x1C303C4E},
       {100214, 0x25149B01},
       {100215, 0x23DE8802},
       {100216, 0xADDECD88},
       {100217, 0xD48F3923},
       {100218, 0x392A5914},
       {100219, 0x4336947B},
       {100220, 0x19466902},
       {100221, 0x1355D33F},
       {100222, 0xB092FAEA},
       {100223, 0x3B68B59F},
       {100224, 0x14982BD7},
       {100227, 0xC4364E73},
       {100228, 0x790831DE},
       {100229, 0xDA8BFDB},
       {100230, 0x4D1B5962},
       {100231, 0x10F443BB},
       {100232, 0x4A9C74EE},
       {100233, 0x6AA1DEC3},
       {100234, 0xB8014172},
       {100235, 0x93B9AB2F},
       {100236, 0x3EB27672},
       {100240, 0xE573AC46},
       {100241, 0xE935BC7A},
       {100242, 0x9EFC977F},
       {100245, 0x3FCD7DA5},
       {100266, 0x45A66067},
       {100267, 0x4C6A3D56},
       {100279, 0x92A0C559},
       {100280, 0x65BC4DEB},
       {100281, 0xAB02B9FB}
 };

static FrameFormatDesc formatsDB[] = {
      {"YUV", "NV12", 0x3231564E, 8},
      {"YUV", "P010", 0x30313050, 10},
      {"YUV", "P012", 0x32313050, 12},
      {"YUV", "P016", 0x36313050, 16},
      {"BGGR", "BGGR", 0x52474742, 8},
      {"BGGR", "BG10", 0x30314742, 10},
      {"BGGR", "BG12", 0x32314742, 12},
      {"BGGR", "BG16", 0x36314742, 16},
      {"BGGRP", "BG0P", 0x50304742, 8},
      {"BGGRP", "BG1P", 0x50314742, 10},
      {"BGGRP", "BG2P", 0x50324742, 12},
      {"BGGRP", "BG3P", 0x50334742, 16},
      {"BGGRD", "BG0D", 0x44304742, 8},
      {"BGGRD", "BG1D", 0x44314742, 10},
      {"BGGRD", "BG2D", 0x44324742, 12},
      {"BGGRD", "BG3D", 0x44334742, 16},
      {"GBRG", "GBRG", 0x47524247, 8},
      {"GBRG", "GB10", 0x30314247, 10},
      {"GBRG", "GB12", 0x32314247, 12},
      {"GBRG", "GB16", 0x36314247, 16},
      {"GBRGP", "GB0P", 0x50304247, 8},
      {"GBRGP", "GB1P", 0x50314247, 10},
      {"GBRGP", "GB2P", 0x50324247, 12},
      {"GBRGP", "GB3P", 0x50334247, 16},
      {"GBRGD", "GB0D", 0x44304247, 8},
      {"GBRGD", "GB1D", 0x44314247, 10},
      {"GBRGD", "GB2D", 0x44324247, 12},
      {"GBRGD", "GB3D", 0x44334247, 16},
      {"GRBG", "GRBG", 0x47425247, 8},
      {"GRBG", "GR10", 0x30315247, 10},
      {"GRBG", "GR12", 0x32315247, 12},
      {"GRBG", "GR16", 0x36315247, 16},
      {"GRBGP", "GR0P", 0x50305247, 8},
      {"GRBGP", "GR1P", 0x50315247, 10},
      {"GRBGP", "GR2P", 0x50325247, 12},
      {"GRBGP", "GR3P", 0x50335247, 16},
      {"GRBGD", "GR0D", 0x44305247, 8},
      {"GRBGD", "GR1D", 0x44315247, 10},
      {"GRBGD", "GR2D", 0x44325247, 12},
      {"GRBGD", "GR3D", 0x44335247, 16},
      {"RGGB", "RGGB", 0x42474752, 8},
      {"RGGB", "RG10", 0x30314752, 10},
      {"RGGB", "RG12", 0x32314752, 12},
      {"RGGB", "RG16", 0x36314752, 16},
      {"RGGBP", "RG0P", 0x50304752, 8},
      {"RGGBP", "RG1P", 0x50314752, 10},
      {"RGGBP", "RG2P", 0x50324752, 12},
      {"RGGBP", "RG3P", 0x50334752, 16},
      {"RGGBD", "RG0D", 0x44304752, 8},
      {"RGGBD", "RG1D", 0x44314752, 10},
      {"RGGBD", "RG2D", 0x44324752, 12},
      {"RGGBD", "RG3D", 0x44334752, 16},
      {"BGIR", "BGI0", 0x30494742, 8},
      {"BGIR", "BGI1", 0x31494742, 10},
      {"BGIR", "BGI2", 0x32494742, 12},
      {"BGIR", "BGI3", 0x33494742, 16},
      {"BGIRP", "BGP0", 0x30504742, 8},
      {"BGIRP", "BGP1", 0x31504742, 10},
      {"BGIRP", "BGP2", 0x32504742, 12},
      {"BGIRP", "BGP3", 0x33504742, 16},
      {"GRBI", "GRI0", 0x30495247, 8},
      {"GRBI", "GRI1", 0x31495247, 10},
      {"GRBI", "GRI2", 0x32495247, 12},
      {"GRBI", "GRI3", 0x33495247, 16},
      {"GRBIP", "GRP0", 0x30505247, 8},
      {"GRBIP", "GRP1", 0x31505247, 10},
      {"GRBIP", "GRP2", 0x32505247, 12},
      {"GRBIP", "GRP3", 0x33505247, 16},
      {"IRBG", "IRB0", 0x30425249, 8},
      {"IRBG", "IRB1", 0x31425249, 10},
      {"IRBG", "IRB2", 0x32425249, 12},
      {"IRBG", "IRB3", 0x33425249, 16},
      {"IRBGP", "IRP0", 0x30505249, 8},
      {"IRBGP", "IRP1", 0x31505249, 10},
      {"IRBGP", "IRP2", 0x32505249, 12},
      {"IRBGP", "IRP3", 0x33505249, 16},
      {"RGIB", "RGI0", 0x30494752, 8},
      {"RGIB", "RGI1", 0x31494752, 10},
      {"RGIB", "RGI2", 0x32494752, 12},
      {"RGIB", "RGI3", 0x33494752, 16},
      {"RGIBP", "RGP0", 0x30504752, 8},
      {"RGIBP", "RGP1", 0x31504752, 10},
      {"RGIBP", "RGP2", 0x32504752, 12},
      {"RGIBP", "RGP3", 0x33504752, 16},
      {"RIGB", "RIG0", 0x30474952, 8},
      {"RIGB", "RIG1", 0x31474952, 10},
      {"RIGB", "RIG2", 0x32474952, 12},
      {"RIGB", "RIG3", 0x33474952, 16},
      {"RIGBP", "RIP0", 0x30504952, 8},
      {"RIGBP", "RIP1", 0x31504952, 10},
      {"RIGBP", "RIP2", 0x32504952, 12},
      {"RIGBP", "RIP3", 0x33504952, 16},
      {"BIGR", "BIG0", 0x30474942, 8},
      {"BIGR", "BIG1", 0x31474942, 10},
      {"BIGR", "BIG2", 0x32474942, 12},
      {"BIGR", "BIG3", 0x33474942, 16},
      {"BIGRP", "BIP0", 0x30504942, 8},
      {"BIGRP", "BIP1", 0x31504942, 10},
      {"BIGRP", "BIP2", 0x32504942, 12},
      {"BIGRP", "BIP3", 0x33504942, 16},
      {"GBRI", "GBI0", 0x30494247, 8},
      {"GBRI", "GBI1", 0x31494247, 10},
      {"GBRI", "GBI2", 0x32494247, 12},
      {"GBRI", "GBI3", 0x33494247, 16},
      {"GBRIP", "GBP0", 0x30504247, 8},
      {"GBRIP", "GBP1", 0x31504247, 10},
      {"GBRIP", "GBP2", 0x32504247, 12},
      {"GBRIP", "GBP3", 0x33504247, 16},
      {"IBRG", "IBR0", 0x30524249, 8},
      {"IBRG", "IBR1", 0x31524249, 10},
      {"IBRG", "IBR2", 0x32524249, 12},
      {"IBRG", "IBR3", 0x33524249, 16},
      {"IBRGP", "IBP0", 0x30504249, 8},
      {"IBRGP", "IBP1", 0x31504249, 10},
      {"IBRGP", "IBP2", 0x32504249, 12},
      {"IBRGP", "IBP3", 0x33504249, 16},
      {"BGRG_GIGI_RGBG_GIGI", "BG0I", 0x49304742, 8},
      {"BGRG_GIGI_RGBG_GIGI", "BG1I", 0x49314742, 10},
      {"BGRG_GIGI_RGBG_GIGI", "BG2I", 0x49324742, 12},
      {"BGRG_GIGI_RGBG_GIGI", "BG3I", 0x49334742, 16},
      {"BGRG_GIGI_RGBG_GIGIP", "PG0I", 0x49304750, 8},
      {"BGRG_GIGI_RGBG_GIGIP", "PG1I", 0x49314750, 10},
      {"BGRG_GIGI_RGBG_GIGIP", "PG2I", 0x49324750, 12},
      {"BGRG_GIGI_RGBG_GIGIP", "PG3I", 0x49334750, 16},
      {"GRGB_IGIG_GBGR_IGIG", "GR0I", 0x49305247, 8},
      {"GRGB_IGIG_GBGR_IGIG", "GR1I", 0x49315247, 10},
      {"GRGB_IGIG_GBGR_IGIG", "GR2I", 0x49325247, 12},
      {"GRGB_IGIG_GBGR_IGIG", "GR3I", 0x49335247, 16},
      {"GRGB_IGIG_GBGR_IGIGP", "PR0I", 0x49305250, 8},
      {"GRGB_IGIG_GBGR_IGIGP", "PR1I", 0x49315250, 10},
      {"GRGB_IGIG_GBGR_IGIGP", "PR2I", 0x49325250, 12},
      {"GRGB_IGIG_GBGR_IGIGP", "PR3I", 0x49335250, 16},
      {"RGBG_GIGI_BGRG_GIGI", "RG0I", 0x49304752, 8},
      {"RGBG_GIGI_BGRG_GIGI", "RG1I", 0x49314752, 10},
      {"RGBG_GIGI_BGRG_GIGI", "RG2I", 0x49324752, 12},
      {"RGBG_GIGI_BGRG_GIGI", "RG3I", 0x49334752, 16},
      {"RGBG_GIGI_BGRG_GIGIP", "RP0I", 0x49305052, 8},
      {"RGBG_GIGI_BGRG_GIGIP", "RP1I", 0x49315052, 10},
      {"RGBG_GIGI_BGRG_GIGIP", "RP2I", 0x49325052, 12},
      {"RGBG_GIGI_BGRG_GIGIP", "RP3I", 0x49335052, 16},
      {"GBGR_IGIG_GRGB_IGIG", "GB0I", 0x49304247, 8},
      {"GBGR_IGIG_GRGB_IGIG", "GB1I", 0x49314247, 10},
      {"GBGR_IGIG_GRGB_IGIG", "GB2I", 0x49324247, 12},
      {"GBGR_IGIG_GRGB_IGIG", "GB3I", 0x49334247, 16},
      {"GBGR_IGIG_GRGB_IGIGP", "GP0I", 0x49305047, 8},
      {"GBGR_IGIG_GRGB_IGIGP", "GP1I", 0x49315047, 10},
      {"GBGR_IGIG_GRGB_IGIGP", "GP2I", 0x49325047, 12},
      {"GBGR_IGIG_GRGB_IGIGP", "GP3I", 0x49335047, 16},
      {"GIGI_RGBG_GIGI_BGRG", "GIR0", 0x30524947, 8},
      {"GIGI_RGBG_GIGI_BGRG", "GIR1", 0x31524947, 10},
      {"GIGI_RGBG_GIGI_BGRG", "GIR2", 0x32524947, 12},
      {"GIGI_RGBG_GIGI_BGRG", "GIR3", 0x33524947, 16},
      {"GIGI_RGBG_GIGI_BGRGP", "GPR0", 0x30525047, 8},
      {"GIGI_RGBG_GIGI_BGRGP", "GPR1", 0x31525047, 10},
      {"GIGI_RGBG_GIGI_BGRGP", "GPR2", 0x32525047, 12},
      {"GIGI_RGBG_GIGI_BGRGP", "GPR3", 0x33525047, 16},
      {"IGIG_GBGR_IGIG_GRGB", "IGG0", 0x30474749, 8},
      {"IGIG_GBGR_IGIG_GRGB", "IGG1", 0x31474749, 10},
      {"IGIG_GBGR_IGIG_GRGB", "IGG2", 0x32474749, 12},
      {"IGIG_GBGR_IGIG_GRGB", "IGG3", 0x33474749, 16},
      {"IGIG_GBGR_IGIG_GRGBP", "IPG0", 0x30475049, 8},
      {"IGIG_GBGR_IGIG_GRGBP", "IPG1", 0x31475049, 10},
      {"IGIG_GBGR_IGIG_GRGBP", "IPG2", 0x32475049, 12},
      {"IGIG_GBGR_IGIG_GRGBP", "IPG3", 0x33475049, 16},
      {"GIGI_BGRG_GIGI_RGBG", "GIB0", 0x30424947, 8},
      {"GIGI_BGRG_GIGI_RGBG", "GIB1", 0x31424947, 10},
      {"GIGI_BGRG_GIGI_RGBG", "GIB2", 0x32424947, 12},
      {"GIGI_BGRG_GIGI_RGBG", "GIB3", 0x33424947, 16},
      {"GIGI_BGRG_GIGI_RGBGP", "GIP0", 0x30504947, 8},
      {"GIGI_BGRG_GIGI_RGBGP", "GIP1", 0x31504947, 10},
      {"GIGI_BGRG_GIGI_RGBGP", "GIP2", 0x32504947, 12},
      {"GIGI_BGRG_GIGI_RGBGP", "GIP3", 0x33504947, 16},
      {"IGIG_GRGB_IGIG_GBGR", "IGR0", 0x30524749, 8},
      {"IGIG_GRGB_IGIG_GBGR", "IGR1", 0x31524749, 10},
      {"IGIG_GRGB_IGIG_GBGR", "IGR2", 0x32524749, 12},
      {"IGIG_GRGB_IGIG_GBGR", "IGR3", 0x33524749, 16},
      {"IGIG_GRGB_IGIG_GBGRP", "IGP0", 0x30504749, 8},
      {"IGIG_GRGB_IGIG_GBGRP", "IGP1", 0x31504749, 10},
      {"IGIG_GRGB_IGIG_GBGRP", "IGP2", 0x32504749, 12},
      {"IGIG_GRGB_IGIG_GBGRP", "IGP3", 0x33504749, 16},
      {"RGGBPD", "RG0B", 0x42304752, 8},
      {"RGGBPD", "RG1B", 0x42314752, 10},
      {"RGGBPD", "RG2B", 0x42324752, 12},
      {"RGGBPD", "RG3B", 0x42334752, 16},
      {"BGGRPD", "BG0B", 0x42304742, 8},
      {"BGGRPD", "BG1B", 0x42314742, 10},
      {"BGGRPD", "BG2B", 0x42324742, 12},
      {"BGGRPD", "BG3B", 0x42334742, 16},
      {"GBRGPD", "GB0B", 0x42304247, 8},
      {"GBRGPD", "GB1B", 0x42314247, 10},
      {"GBRGPD", "GB2B", 0x42324247, 12},
      {"GBRGPD", "GB3B", 0x42334247, 16},
      {"GRBGPD", "GR0B", 0x42305247, 8},
      {"GRBGPD", "GR1B", 0x42315247, 10},
      {"GRBGPD", "GR2B", 0x42325247, 12},
      {"GRBGPD", "GR3B", 0x42335247, 16}
};

#endif/*DATA_RANGE_H_*/
