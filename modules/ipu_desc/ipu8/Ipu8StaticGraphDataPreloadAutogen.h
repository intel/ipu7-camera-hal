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

typedef struct GSFTimeStamp {

    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    uint16_t minute;
    uint16_t second;
} GSFTimeStamp;

typedef struct SysToolVersion {

    uint16_t year;
    uint16_t workWeek;
    uint16_t day;
    uint16_t patchVersion;
} SysToolVersion;

typedef struct {

    uint32_t sapAttributes;
    uint32_t additionalFeaturesBit;
    uint32_t binaryCommonHashCode;
    uint32_t numberOfResolutions;
    uint32_t numberOfSensorModes;
    GSFTimeStamp gsfTimeStamp;
    SysToolVersion sysToolVersion;

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

static int distinctGraphsCount = 192;

static GraphHashCode hashCodeLookup[] = {
       {0, 0x375F6AED},
       {100000, 0x5ABEA9CA},
       {100001, 0x41E9D9A9},
       {100002, 0x76C788CE},
       {100003, 0x69586BE1},
       {100004, 0x413DFD00},
       {100005, 0x3D59D087},
       {100006, 0x17D33D9C},
       {100007, 0xE49283B7},
       {100008, 0xB7D143AA},
       {100009, 0xA39F35C5},
       {100010, 0x3C87877E},
       {100011, 0x7018B37D},
       {100012, 0xCDBE7F4},
       {100013, 0xE80E9761},
       {100014, 0xF1C01500},
       {100015, 0xC2FD5549},
       {100016, 0x2B5EECF},
       {100017, 0x38DC5786},
       {100018, 0xE07F2FAF},
       {100019, 0xB1E4609A},
       {100020, 0xFD0A1CF5},
       {100021, 0x574F5BC6},
       {100022, 0x459B7C65},
       {100023, 0x57E500EE},
       {100024, 0xE7658ED5},
       {100026, 0xD453DAA3},
       {100027, 0xAE85C25E},
       {100028, 0x77E1AC0D},
       {100029, 0x36FF25C2},
       {100030, 0xB643D505},
       {100031, 0xD6A0ACFE},
       {100032, 0x784CCA65},
       {100033, 0x4BE7BE62},
       {100034, 0x74F4AB5D},
       {100035, 0x73BDD26D},
       {100036, 0x73BDD26D},
       {100037, 0x815DF227},
       {100038, 0x54ACC81},
       {100039, 0x54ACC81},
       {100040, 0xC187E47A},
       {100041, 0xF348519E},
       {100042, 0x34E5251D},
       {100045, 0x8B78006C},
       {100058, 0x815DF227},
       {100059, 0x815DF227},
       {100066, 0x9B469CF6},
       {100067, 0x83C28AB1},
       {100079, 0x2E151074},
       {100080, 0xA44CD036},
       {100081, 0xB5DB7F2E},
       {100100, 0xD3F9CB81},
       {100101, 0xD4915CEA},
       {100102, 0xB2B2AB13},
       {100103, 0xF2E0660C},
       {100104, 0x6BB3610F},
       {100105, 0x5C781D90},
       {100106, 0x239FCB4D},
       {100107, 0x51764D96},
       {100108, 0x7F319BB5},
       {100109, 0xA1182612},
       {100110, 0x95B86447},
       {100111, 0x9B5A9384},
       {100112, 0x594426F9},
       {100113, 0x5C192DA2},
       {100114, 0x10605BC5},
       {100115, 0x4A9490F4},
       {100116, 0x46EDB1C6},
       {100117, 0x9397714D},
       {100118, 0x957E5370},
       {100119, 0x2347DE97},
       {100120, 0xC37D51A},
       {100121, 0x9F7515F9},
       {100122, 0xAFDD5110},
       {100123, 0x1F390F13},
       {100127, 0x37088A87},
       {100128, 0xC42F6CC8},
       {100129, 0x1A6D489D},
       {100130, 0xB978B0B6},
       {100131, 0xDF3F0B4D},
       {100132, 0xA5B9C9CE},
       {100133, 0x3AC78CFF},
       {100134, 0x41926130},
       {100135, 0xB48D582D},
       {100136, 0x31C0E819},
       {100137, 0x2B190CF},
       {100138, 0x19CE77FF},
       {100139, 0x756124E1},
       {100140, 0xA87BB113},
       {100141, 0x1B600BBB},
       {100142, 0x71A1B7E1},
       {100143, 0x3F21AE3},
       {100144, 0xB4084C8B},
       {100145, 0x14822D84},
       {100146, 0xA28EE307},
       {100157, 0x705DA91},
       {100162, 0x8386DA6D},
       {100166, 0x972B6C8B},
       {100169, 0x8386DA6D},
       {100200, 0xD86C9069},
       {100201, 0xB17BE104},
       {100202, 0x10150502},
       {100203, 0x9A25F73D},
       {100204, 0x3B7A01C4},
       {100205, 0xDFD4FAE3},
       {100206, 0xD397B130},
       {100207, 0x91655343},
       {100208, 0xD7F18796},
       {100209, 0x9FB35FF9},
       {100210, 0xA19849CA},
       {100211, 0x683C1651},
       {100212, 0xF6149210},
       {100213, 0xB48641B5},
       {100214, 0x40DB15AC},
       {100215, 0xC441DFED},
       {100216, 0x7A8477D3},
       {100217, 0xEF3405F2},
       {100218, 0xBF368183},
       {100219, 0x5FBCBE36},
       {100220, 0xC257B449},
       {100221, 0xACC945B2},
       {100222, 0x270686F9},
       {100223, 0xC87832CA},
       {100224, 0x6E9841AA},
       {100227, 0x513AC1EA},
       {100228, 0xDE401521},
       {100229, 0x1921622E},
       {100230, 0x9D4BDD59},
       {100231, 0xA94F6E1A},
       {100232, 0x2647E8A9},
       {100233, 0xD605D9FE},
       {100234, 0xB90FA781},
       {100235, 0x97040CFE},
       {100236, 0xDCE6DB25},
       {100240, 0xF8897205},
       {100241, 0xFEE9D8FD},
       {100242, 0xCBE6197E},
       {100245, 0x3DCD4388},
       {100266, 0x1FDFFEAA},
       {100267, 0x19C12EDD},
       {100279, 0x689E4798},
       {100280, 0x1BDCB87A},
       {100281, 0x6FD032},
       {101114, 0x65049DDF},
       {101138, 0x54ACC81},
       {200000, 0xF9EE5587},
       {200001, 0xC4030517},
       {200002, 0x29391027},
       {200003, 0xD77C3F77},
       {200004, 0xEDFB2088},
       {200005, 0xF0469B90},
       {200006, 0xF080775E},
       {200007, 0x8F5FA856},
       {200008, 0x87343DB4},
       {200009, 0xA2B6A13C},
       {200010, 0xAC7BE0D0},
       {200011, 0x1B2E91F8},
       {200012, 0x1031653B},
       {200013, 0x4F7BE9AB},
       {200014, 0x200F9FE9},
       {200015, 0xC8331769},
       {200016, 0x5CB1321E},
       {200017, 0xC22BFA16},
       {200018, 0x488561D2},
       {200019, 0xB5CB5C0A},
       {200020, 0xACEB6A71},
       {200021, 0xD959F8F1},
       {200022, 0xEB1EA803},
       {200023, 0xA0E81373},
       {200024, 0xE1809BA7},
       {200025, 0xEFF7AF7},
       {200026, 0xF0AC02D7},
       {200027, 0x8CA38067},
       {200028, 0x5936EF0},
       {200029, 0xD218BC58},
       {200030, 0xEC95E6F6},
       {200031, 0xEB38A80E},
       {200032, 0xE0F70AC2},
       {200033, 0x459427BA},
       {200034, 0x87DD81A6},
       {200035, 0x2CA5633E},
       {200036, 0xF6AA8209},
       {200037, 0x1D758209},
       {200038, 0x26CC848B},
       {200039, 0xF1B8457B},
       {200040, 0xD0820F5C},
       {200041, 0x25DE1C4},
       {200042, 0x6EC7FC18},
       {200043, 0x8D88DDA0},
       {200044, 0x6B40772B},
       {200045, 0xAA31925B},
       {200046, 0xACEA3DC1},
       {200047, 0x42E20001}
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
