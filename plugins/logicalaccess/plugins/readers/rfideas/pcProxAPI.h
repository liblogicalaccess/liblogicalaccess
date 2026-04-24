// Copyright © 2025 rf IDEAS, Inc. as an unpublished work.
// All Rights Reserved


// To Enable logging in SDK in windows user can set environment variable PCPROXAPI_LOGGING as true.
// Example -  SET PCPROXAPI_LOGGING = true

#ifndef PCPROXAPI_H
#define PCPROXAPI_H

#if defined (__linux__) || defined(__APPLE__)
#include <unistd.h>
typedef     long                LONG;
typedef     unsigned char       BYTE;
typedef     int                 BOOL;
typedef     short               BSHRT;
typedef     unsigned short      USHORT;
typedef     unsigned short      WORD; //ITT: Change for Medtronic navigation escalation- service call 104411
typedef     short               SHORT;
typedef     unsigned long       DWORD;
typedef     unsigned int        UINT;
typedef     void *              GUID;
#else
#include <windows.h>
#endif

#ifdef __linux__

typedef     unsigned long       ULONG;
#define APIENTRY
/*Linking the definition of memcpy to the respective
**GLIBC mentioned below
*/
#ifdef __x86_64__
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");	//Linux 64
#elif __i386__
__asm__(".symver memcpy,memcpy@GLIBC_2.0");		//Linux 32
#elif __aarch64__
__asm__(".symver memcpy,memcpy@GLIBC_2.17");    //arm 64
#else
__asm__(".symver memcpy,memcpy@GLIBC_2.4");		//arm
#endif

#elif __APPLE__
#include <IOKit/hid/IOHIDManager.h>

typedef IOHIDDeviceRef  HANDLE;

#define APIENTRY

#endif


//-----------------------------------------------------------------------------
//
// Structure and function prototypes for use with the pcProxUSB API
// This library supports pcProx, pcSwipe, and pcProx Sonar product on
// both USB and Serial (RS-232 and Virtual COM ports) for Windows
// and Linux.
//
//=============================================================================
//============!!! Important Notes to Users of this Library !!!=================
//=============================================================================
//--- Visual C++ Structure Alignment: set to 8 BYTES in the build of this library
// VC++ /Zp8 g++
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Inherent Data Types:
// WINDEF.H has these defined and the Library is built as such
// typedef unsigned long       DWORD;
// typedef int                 BOOL;
// typedef unsigned char       BYTE;
// typedef unsigned short      WORD;
//
// A 'short' is guaranteed to be 16 bits (2 bytes) Nothing else is guaranteed across
// compilers A 'char' can be 1 or 2 bytes - we're using 1 byte chars The sizeof()
// operator tells the whole story For this API, here's what the compiler reports
// as sizeof():
// char, BYTE: 1 byte
// short, WORD, BSHRT: 2 bytes
// int, long, DWORD: 4 bytes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// The following ifdef block is the standard way of creating macros which make
// exporting from a library simpler. All files within this library are compiled
// with the USBWEJAPI_EXPORTS symbol defined on the command line. this symbol
// should not be defined on any project that uses this library. This way any other
// project whose source files include this file see USBWEJAPI_API functions as
// being imported from a DLL, whereas these symbols are defined with this macro
// as being exported.
//-----------------------------------------------------------------------------
#ifdef USBWEJAPI_EXPORTS
#define USBWEJAPI_API __declspec(dllexport)
#ifdef __GNUC__
#undef  USBWEJAPI_API
#define USBWEJAPI_API extern "C" __declspec(dllexport)
#endif
#else
#ifdef STATIC_API
#define USBWEJAPI_API  //nothing
#define USBWEJAPI_EXPORTS
#else
#if defined (__linux__) || defined(__APPLE__)
#define USBWEJAPI_API
#else
#define USBWEJAPI_API __declspec(dllimport)
#endif
#endif
#endif


// Max devices supported by Library
#define MAXDEVSOPEN     127


//-----------------------------------------------------------------------------
// Handling both USB Serial, devices...
// Return values from GetDevType().
#define PRXDEVTYP_ALL -1    // All types of interfaces USB, Serial, etc...
#define PRXDEVTYP_USB 0     // USB only
#define PRXDEVTYP_SER 1     // Serial (Virtual COM & RS-232) Only
#define PRXDEVTYP_TCP 2     // TCP/IP

//-----------------------------------------------------------------------------
// For linuxSetComDev
#define MAXLINUXDEVPATH 128     // Holds /dev/ttyNNNN stirng for mapping COMn to device
#define MINLINUXCOMPORT 1       // COM1..16 maps to /dev/Name above
#define MAXLINUXCOMPORT 16      //

//-----------------------------------------------------------------------------
// For Get/SetConnectProduct()
// Allows user to filter USBConnect() and also returns each connected device type
#define PRODUCT_ALL         0x0FFFFFFFF
#define PRODUCT_PCPROX      0x000000001
#define PRODUCT_PCSWIPE     0x000000002
#define PRODUCT_PCSONAR     0x000000004

//-----------------------------------------------------------------------------
// The maximum string length of the Device 'Friendly' name...
// Maximum size is increased to 511 bytes as on MAC Device name comes greater than 128
#define MAXDEVNAMESZ 511

//-----------------------------------------------------------------------------
// The maximum string length of the Device product name "RDR-6801AKU" null filled
#define MAXPRODUCTNAMESZ    24

//-----------------------------------------------------------------------------
// For BTLE STATE
#define BTLEOFF_RADIOOFF    	0
#define BTLEOFF_RADIOON    		1
#define BTLEON_RADIOOFF    		2
#define BTLEON_RADIOON    		3
#define BTLE_RADIOTOGGLE    	4
//----------------------------------------------------------------------------
// The maximum string length of the Firmware Filename
#define MAXFWFILENAME  64
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// The maximum string length of the Version number
#define MAXFWFULLNAME  64
//-----------------------------------------------------------------------------
//-----PRODUCT: pcProx------
//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sCfgFlags //++ Used for pcProx
{
    short bFixLenDsp;     // Send as fixed length with leading zeros as needed
    short bFrcBitCntEx;   // Force Rx'd bit count to be exact to be valid
    short bStripFac;      // Strip the FAC from the ID (not discarded)
    short bSndFac;        // Send the FAC (if stripped from data)
    short bUseDelFac2Id;  // Put a delimiter between FAC and ID on send
    short bNoUseELChar;   // Don't use a EndLine char on send (default to ENTER)
    short bSndOnRx;       // Send valid ID as soon as it is received
                          // (iIDLockOutTm timer not used)
    short bHaltKBSnd;     // Don't Send keys to USB (Get ID mechanism)
} tsCfgFlags;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sIDBitCnts //++ Used for pcProx
{
    short iLeadParityBitCnt;  // Wiegand Leading Parity bit count to be stripped
    short iTrailParityBitCnt; // Wiegand Trailing Parity bit count to be stripped
    short iIDBitCnt;          // If bStripFac, determines bit count of ID and FAC
    short iTotalBitCnt;       // If bFrcBitCntEx, card read (including parity)
                              // must match this
    short iPad4;
    short iPad5;
    short iPad6;
    short iPad7;
} tsIDBitCnts;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sIDDispParms //++ Used for pcProx
{
    short iFACIDDelim; // If bStripFac & bSndFac & bUseDelFac2Id, this char
                       // sent between FAC & ID
    short iELDelim;    // If NOT bNoUseELChar, this char sent at end of ID
    short iIDDispLen;  // If bFixLenDsp, ID padded with zeros to this length
    short iFACDispLen; // If bFixLenDsp, FAC padded with zeros to this length
    short iExOutputFormat ;  //If iExOutputFormat, Reader will output in Extended
                            //mode for Plus Extended readers
    short iPad5;
    short iPad6;
    short iPad7;
} tsIDDispParms;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sTimeParms //++ Used for pcProx
{
    short iBitStrmTO;   // Wiegand read T/O after this msec time (4ms)
    short iIDHoldTO;    // Card ID valid for this msec time (48ms)
    short iIDLockOutTm; // Squelch repetitive reader reports (usually > 1000)
                        // in msec (48msec gran.)
    short iUSBKeyPrsTm; // Sets USB inter-key 'Press' time in 4ms units
    short iUSBKeyRlsTm; // Sets USB inter-key 'Release' time in 4ms units
    short ExFeatures01; // Extended Features (big parity Azery ext precision)
    short iPad6;        // Spare Unused
    short iTPCfgFlg3;   // Bit mapped tp Flags3 -- Use Flags3 structure instead
} tsTimeParms;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sCfgFlags2 //++ Used for pcProx
{
    short bUseLeadChrs;  // Use leading chars in ID KB send
    short bDspHex;       // Display ID as ASCII Hex [not ASCII decimal]
    short bWiegInvData;  // Wiegand data on pins is inverted
    short bUseInvDataF;  // Use the bWiegInvData flag over hardware setting
    short bRevWiegBits;  // Reverse the Wiegand Rx bits
    short bBeepID;       // Beep when ID received
    short bRevBytes;     // Reverse byte order (CSN reader)
    short isASCIIpresent;//check if ASCII conversion type is present 
	                     //within the reader
    short isReverseBytesSupported; //Check if reverse-all-bytes is supported 
                                   //by reader in extended mode
} tsCfgFlags2;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sCfgFlags3 //++ Used for pcProx
{
    short bUseNumKP;     // Euro KB flag
    short bSndSFON;      // Split format ON = 1, old combined scheme = 0
    short bSndSFFC;      // 0 = FAC Decimal, 1 = FAC Hex
    short bSndSFID;      // 0 = ID Decimal, 1 = ID Hex
    short bPrxProEm;     // Use ProxPro emulation
    short bUse64Bit;     // 0 = 32-bit, 1 = 64-bit Display Math
    short bNotBootDev;   // USB Enum: 0=BootDevice, 1=NOTBootDevice
    short bLowerCaseHex; // Alpha hex output displayed as lower case
} tsCfgFlags3;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sIDDispParms2 //++ Used for pcProx
{
    short iLeadChrCnt; // Tf bUseLeadChrs, contains the lead char count (<=3)
    short iLeadChr0;   // These lead characters are filled in (up to 3)
    short iLeadChr1;   // Leading character
    short iLeadChr2;   // Leading character
    short iCrdGnChr0;  // If non-zero, sent when ID goes Invalid
    short iCrdGnChr1;  // If this and Chr0 non-zero, sent when ID goes Invalid
    short iPad6;
    short iPad7;
} tsIDDispParms2;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sIDDispParms3 //++ Used for pcProx
{
    short iTrailChrCnt; // This contains the trail char count (<=3)
    short iTrailChr0;   // These trail characters are filled in (up to 3)
    short iTrailChr1;   // NOTE: LeadChrCnt + TrailCheCnt <= 3
    short iTrailChr2;   //       LeadChrs have priority
    short iPad4;
    short iPad5;
    short iPad6;
    short iPad7;
} tsIDDispParms3;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sLEDCtrl //++ Used for pcProx
{
    // Set bAppCtrlsLED to 1 - The user of the SDK can control the LED according to
    //  the other settings in the sLEDCtrl struct.
    // Set bAppCtrlsLED to 0 - The reader will control the LED (ignoring the other 
    //  settings in the sLEDCtrl struct).
    short bAppCtrlsLED;
    // Note:- For AMBER both iRedLEDState and iGrnLEDState will be 1.
    short iRedLEDState; // 0 == Off, 1 == On
    short iGrnLEDState; // 0 == Off, 1 == On
    short iPad3;
    short iPad4;
    short iPad5;
    short iPad6;
    // Set bVolatile to 1 - this will write configuration to RAM and the LED color  
    //  will change. Setting will not be persistent (it will be volatile). Use this  
    //  mode if you do not want the LED color to persist. This mode also prevents 
    //  excessive writes to the reader's flash memory.
    // Set bVolatile to 0 - in order to change the LED color, you must follow this 
    //  command with a WriteCfg(), which will change the LED's color and also 
    //  persist the change to flash. Use this mode if you want to keep the setting 
    //  saved in the reader's flash memory, and avoid calling it frequently since 
    //  it can cause flash memory wear.
    short bVolatile;
    
} tsLEDCtrl;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sBprRlyCtrl //++ Used for pcProx and OEM W2-USB
{
    short iPad0;
    short iBeeperState; // 0 == Off, 1 == On
    short iRelayState;  // 0 == Off, 1 == On
    short iPad3;
    short iPad4;
    short iPad5;
    short iPad6;
    short bVolatile; // 0 == commit to EE, 1 == Don't store to EE
} tsBprRlyCtrl;

//-----------------------------------------------------------------------------
// CHUID FIPS201 Reader Support - Bit Fields & Separators Definitions
//-----------------------------------------------------------------------------
#define _CHUIDCAPBIT 6   //ITT Added : Get 6th bit from TimeParms_ExFeatures01 to detect presence of CHUID PLUS
// temporary hard-coded MAX structure size - now dynamic and found in device header
#define _TOTALFLDSEPBYTES 128 // ITT Modified in CHUID Phase 1: really read from header of glob (4th byte)

//ITT : Changes made for Hashing Key Conversion : Added By Ritesh
#define MAXHASHKEYSIZE 16 //  Extend 2: Hashing Key Data Size
//ITT : Changes made for Hashing Key Conversion : Added By Ritesh

//----- sizeof(things) Definition -----

typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sFSSz
{
    BYTE ucFSStrVer; // Fld/Sep structure Version High nibble, this Header size Low nibble
    BYTE ucFldEntryCnt; // upper 3 bits are ((Field Entry Size)/2), upper 5 bits are Field Entry Count
    BYTE ucSepEntryCnt; // upper 3 bits are ((Sep Entry Size)/2), upper 5 bits are Sep Entry Count
    BYTE ucFS8BCnt; // storage size in 8-byte units
} tsFSSz;

//----- Bit Field Definition -----
//;-----
//;   Each Data Field: [currently claim 4 bytes]
//;     - Usage Flags: 1 byte
//;       - 1 bit: used / valid, 0 is "not used"
//;       - 1 bit: conversion type (dec(0) / hex(1))
//;       - 1 bit: [undefined]
//;       - 5 bits: Fixed field length, if non-zero, it's fixed length
//;     - Extra Flags: 1 byte [undefined as yet]
//;       - 1 bit: bReverse bits
//;       - 1 bit: bReverse bytes
//;       - 1 bit: [undefined]
//;       - 1 bit: [undefined]
//;       - 4 bits: [undefined]
//;     - Field Definition: 2 bytes
//;       1st byte is Starting Bit position (at LSBit due to byte buffer order)
//;       2nd byte is the bit count
//;-----
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sFldDef
{
    BYTE ucUsageByte0;
    BYTE ucUsageByte1;
    BYTE ucStartBitPos;
    BYTE ucBitLength;
} tsFldDef;

//----- Field Separator Definition -----
//;-----
//;   Each Sep Field: [currently claim 2 bytes]
//;     - Usage Flags: 1 byte
//;       - 1 bit: used / valid, 0 is "not sent"
//;       - 1 bit: character size, 1 byte (==0) or 2 byte (==1)
//;       - 1 bit: [undefined]
//;       - 1 bit: [undefined]
//;       - 4 bits: count of virtual chars stored in general "pool" area
//;                 (thus limited to 15 virtual chars = 30 bytes / Sep if VCcnt bit is set)
//;     - Sep Offset: 1 byte (0-based offset from start of pool area)
//;-----
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sSepDef
{
    BYTE ucUsageByte0;
    BYTE ucDataOfst;
} tsSepDef;

// ITT: Extended Phase 2
// Only Plus Extended Readers have the space in the memory to store extra two bytes. Legacy CHUID readers will not use these two bytes.
// 1st Byte will store the state of Enhance Security Flag
// 2nd Byte will store the last configured profile written by the user.
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sAppData
{
BYTE ucEnhanceSecurityByte;    // To Store Enhance Security Flag status
BYTE ucFipsBitCount;  // To Store Last Configured Profile or Last Card read
}tsAppData;
// ITT

//----- Combined Field & Field Separator Definition & Field Separator Data -----
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sFldSepData
{
    sFSSz FldSepSizes;
	BYTE ucPad[_TOTALFLDSEPBYTES - sizeof(tsFSSz)- sizeof(tsAppData)]; // ITT Modified for Extended Phase 2: To add support two extra Bytes
	sAppData AppDataStruct;
} tsFldSepData; // 96  bytes for Legacy readers and 128 bytes for Plus Extended reader

//----- Different ways to look at the whole Glob -----
typedef union
#ifdef __GNUC__
__attribute__((packed))
#endif
uSFD
{
    BYTE BB[sizeof(tsFldSepData)];
    sFldSepData SFD;
} tuSFD;

//ITT : Changes made for Hashing Key Conversion : Added By Ritesh
// Structure to hold the Hashing Key Data
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sHashKeyData
{

   char HashKeyAData[MAXHASHKEYSIZE];
   char HashKeyBData[MAXHASHKEYSIZE];

} tsHashKeyData; // 32  bytes total

//ITT : Changes made for Hashing Key Conversion : Added By Ritesh
//-----------------------------------------------------------------------------
// -END- CHUID/FIPS201 Reader Support - Bit Fields & Separators Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// User's callback function to receive Debug Log Info
// The 1st parameter will be a pointer to a NULL terminated string describing the event.
// The 2nd parameter is just the length of the string (strlen(buffer))
typedef void (*RFPRXLOG)( char*, int ); // *** of type __cdecl, NOT pascal ***

//-----------------------------------------------------------------------------
// PASCAL type Calling Convention for no-fuss VB use.
#define USBWEJAPICC APIENTRY

//-----------------------------------------------------------------------------
//-----PRODUCT: pcProx SONAR------
//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sSonarParms //++ Used for pcProx Sonar
{
    short LEDFlags;  // LED usage flags  [0]
    short PingRate;  // Ping rate (msec) [332] (200 .. 1020)
    short Reserved1; // 40KHz cycle cnt (READ ONLY)
    short MinDist;   // Minimum Distance acceptable (inches) [14] (14 .. 59)
    short MaxDist;   // Maximum Distance acceptable (inches) [36] (15 .. 60)
    short ORDBTm;    // Debounce out of range time (seconds) [0]
    short IRDBTm;    // Debounce in range time (seconds)  [0]
    short StartDly;  // Cold start delay in seconds before joining USB  [0]
} tsSonarParms;

// LEDFlags Bit defines
#define bNoShowIRLED 0
#define bNoShowORLED 1
#define bNoShowXtLED 2

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sWalkAwayParms //++ Used for pcProx Sonar
{
    short KeyCount;      // Defined keys to follow (max 6 modifiers/keys) [0]
    short InterKeyDelay; // Time (ms) between keys [512] (range 64..16320)
    short Key1Mods;      // Key Modifiers...[0]
    short Key2Mods;
    short Key3Mods;
    short Key4Mods;
    short Key5Mods;
    short Key6Mods;
    short Flags0;
    short MinFltRepRate; // Near fault send repeat rate in seconds
    short Key1Code;      // Key Codes...[0]
    short Key2Code;
    short Key3Code;
    short Key4Code;
    short Key5Code;
    short Key6Code;
} tsWalkAwayParms;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sWalkUpParms //++ Used for pcProx Sonar
{
    short KeyCount;      // Defined keys to follow (max 6 modifiers/keys) [0]
    short InterKeyDelay; // Time (ms) between keys [512] (range 64..16320)
    short Key1Mods;      // Key Modifiers...[0]
    short Key2Mods;
    short Key3Mods;
    short Key4Mods;
    short Key5Mods;
    short Key6Mods;
    short Flags0;
    short Reserved2;
    short Key1Code;      // Key Codes...[0]
    short Key2Code;
    short Key3Code;
    short Key4Code;
    short Key5Code;
    short Key6Code;
} tsWalkUpParms;

// Flags0 Bit defines - both WalkAway and WalkUp
#define DBTmU 0 // Debounce time Units, 0=seconds, 1=minutes: ORDBTm and IRDBTm in SonarParms

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sIdleParms //++ Used for pcProx Sonar
{
    short Reserved1; //
    short PressRate; // Periodic Press Rate (1 Sec units) - Default Zero Off
    short Key1Mods;  // Key 1 modifier bit flags
    short Key1Code;  // Key 1 code
// If bit bDetDeadMan of Flags is SET, DMTODelta determines
// the spatial window within which the target must remain
// for DMTOTm seconds before being declared "dead".
// If DMTODelta is set to 1, the window is 1/3.472 ~= 0.3 inch.
// If DMTODelta is set to 3, the window is 3/3.472 ~= 0.9 inch.
//
// sIdleParms.Flags bit defines...
#define bDetDeadMan 0 // DeadMan T/O if set, use DMTODelta & DMTOTm above
#define bDetMinOOR  1 // Target too close to sensor (< MinDist out of range)
//
// sIdleParms.Flags.bDetMinOOR can be used in conjunction with
// sWalkAwayParms.MinFltRepRate to repeatedly send the sWalkAwayParms keys when
// the target is too close as specified by sSonarParms.MinDist for the time
// specified by sSonarParms.ORDBTm.
// This can be used for very close range detection of tape or "dixie cups" over
// the sensor or other objects placed in front of the sensor in an attempt to
// disable the send of the WalkAway key set. The repetitive send feature will
// disallow a manual logon after initial send of the WalkAway keys which results
// in an unprotected computer.
//
    short Flags;     // see above...
    short DMTODelta; // DeadMan T0Delta, 3.472 * Distance (in) = DMTODelta
    short DMTOTm;    // DeadMan Time Sec. untill Declared Dead while Deltas
                     // never exceeded
    short SNBlankTm; // Sonar drive blanking time (43us units)
} tsIdleParms;



//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sRangeInfo //++ Used for pcProx Sonar
{
  short wCurrRange;     // Current Object Range in Inches, Zero for > max or < min
  short bInRange;       // Object In-Range is non-zero
  short bInRangePend;   // Object In-Range Pending (still out-of-range)
  short bOutRangePend;  // Object Out-of-Range Pending (still in-range)
  short pad4;
  short pad5;
  short pad6;
  short pad7;
} tsRangeInfo;

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
HardwareInformation //++ Used for Installed Hardware Information
{
    bool SamSe;
    bool RfAms;
    bool Rf125;
    bool RfLegic;
    bool RfBle;
    bool Nxp;
    bool RfHidBle;
    bool Felica;
} tHardwareInformation;

//-----------------------------------------------------------------------------
enum class ReaderModuleId {
  UNDEFINED = 0,
  LOW_FREQUENCY_RADIO = 1,
  HIGH_FREQUENCY_RADIO = 2,
  BLE_RADIO = 3
};

//-----------------------------------------------------------------------------
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
sCardAnalyer //++ Used for Card Analyzer
{
    long CardType;
    int CardBits;
    char CardData[32];
}tsCardAnalyzer;

//-----------------------------------------------------------------------------
#if defined (__linux__) || defined(__APPLE__)
#define __event 
#else
#endif

typedef short BSHRT;  // a 16-bit Boolean

#if defined (__linux__) || defined(__APPLE__)
 #define USBWEJAPI_EXPORTS
#endif

#ifdef USBWEJAPI_EXPORTS
#ifdef __cplusplus
extern "C" {
#endif
USBWEJAPI_API void  USBWEJAPICC SetMarkerFunction(RFPRXLOG pfLog);

// [API_BEGIN_TAG]
USBWEJAPI_API BSHRT USBWEJAPICC DumpRawFeatureReports(char *fname);
USBWEJAPI_API short USBWEJAPICC SetRawPayload_index(short index, short c);
USBWEJAPI_API BYTE USBWEJAPICC GetRawPayload_index(short index);
USBWEJAPI_API unsigned long USBWEJAPICC GetVidPidFilterMask(int n);
USBWEJAPI_API unsigned long USBWEJAPICC GetVidPidFilterCompare(int n);
USBWEJAPI_API BSHRT  USBWEJAPICC BeepNow(BYTE count, BSHRT longBeep);
USBWEJAPI_API short  USBWEJAPICC ChkAddArrival(char *pBuf);
USBWEJAPI_API BSHRT  USBWEJAPICC ChkDelRemoval(char *szName);
USBWEJAPI_API BSHRT  USBWEJAPICC ComConnect(long* plDID);
USBWEJAPI_API BSHRT  USBWEJAPICC ComConnectPort(WORD iPort, long* plDID);
USBWEJAPI_API BSHRT  USBWEJAPICC ComDisconnect(void);
USBWEJAPI_API short  USBWEJAPICC FindXport(short ip0, short ip1, short ip2, short ip3begin, short ip3end);
USBWEJAPI_API BOOL	 USBWEJAPICC Get64BytesInPackets();
USBWEJAPI_API BSHRT  USBWEJAPICC GetAZERTYShiftLock(void);
USBWEJAPI_API short  USBWEJAPICC GetActConfig(void);
USBWEJAPI_API short  USBWEJAPICC GetActDev(void);
USBWEJAPI_API short  USBWEJAPICC GetActiveID(BYTE *pBuf, short wBufMaxSz);
USBWEJAPI_API short  USBWEJAPICC GetActiveID32(BYTE *pBuf, short wBufMaxSz);
USBWEJAPI_API BSHRT  USBWEJAPICC GetBprRlyCtrl(tsBprRlyCtrl *psBRCtrl);
USBWEJAPI_API const char* USBWEJAPICC GetBuildDate(void);
USBWEJAPI_API long   USBWEJAPICC GetCardPriority(void);
USBWEJAPI_API long   USBWEJAPICC GetCardType(void);
USBWEJAPI_API const char* USBWEJAPICC GetCopyrightInfo(void);
USBWEJAPI_API const char* USBWEJAPICC GetCPUInfo(void);
USBWEJAPI_API short  USBWEJAPICC GetDID(void);
USBWEJAPI_API short  USBWEJAPICC GetDevByLUID(short LUID, short index);
USBWEJAPI_API short  USBWEJAPICC GetDevCnt(void);
USBWEJAPI_API BSHRT  USBWEJAPICC GetDevName(char *szName);
USBWEJAPI_API short  USBWEJAPICC GetDevType(void);
USBWEJAPI_API BOOL	 USBWEJAPICC GetEasyReadMode();
USBWEJAPI_API BSHRT  USBWEJAPICC GetExtendedPrecisionMath(void);
USBWEJAPI_API DWORD  USBWEJAPICC GetFirmwareVersion(short hardware, short module);
USBWEJAPI_API BSHRT  USBWEJAPICC GetFlags(tsCfgFlags *psCfgFlgs);
USBWEJAPI_API BSHRT  USBWEJAPICC GetFlags2(tsCfgFlags2 *psCfgFlgs);
USBWEJAPI_API BSHRT  USBWEJAPICC GetFlags3(tsCfgFlags3 *psCfgFlgs);
USBWEJAPI_API BSHRT  USBWEJAPICC GetHIDGuid(GUID *pGuid);
USBWEJAPI_API BSHRT  USBWEJAPICC GetIDBitCnts(tsIDBitCnts *psIDBitCnts);
USBWEJAPI_API BSHRT  USBWEJAPICC GetIDDispParms(tsIDDispParms *psIDDispParms);
USBWEJAPI_API BSHRT  USBWEJAPICC GetIDDispParms2(tsIDDispParms2 *psIDDispParms);
USBWEJAPI_API BSHRT  USBWEJAPICC GetIDDispParms3(tsIDDispParms3 *psIDDispParms);
USBWEJAPI_API BOOL   USBWEJAPICC GetIdleParms(tsIdleParms* psIdleParms);
USBWEJAPI_API BSHRT  USBWEJAPICC GetLEDCtrl(tsLEDCtrl *psLEDCtrl);
USBWEJAPI_API short  USBWEJAPICC GetLUID(void);
USBWEJAPI_API long   USBWEJAPICC GetLastLibErr(void);
USBWEJAPI_API BSHRT  USBWEJAPICC GetLibVersion(short* piVerMaj, short* piVerMin, short* piVerDev);
USBWEJAPI_API WORD   USBWEJAPICC GetMaxConfig(void);
USBWEJAPI_API bool   USBWEJAPICC GetModuleState(short moduleID, short *moduleState);
USBWEJAPI_API ULONG  USBWEJAPICC GetMyIpAddress(void);
USBWEJAPI_API BOOL   USBWEJAPICC GetObjRangeInfo(tsRangeInfo* psRangeInfo);
USBWEJAPI_API BOOL	 USBWEJAPICC GetPrependCSN();
USBWEJAPI_API ULONG  USBWEJAPICC GetProduct(void);
USBWEJAPI_API const char* USBWEJAPICC GetPlatformType(void);
USBWEJAPI_API short  USBWEJAPICC GetQueuedID(short clearUID, short clearHold);
USBWEJAPI_API long   USBWEJAPICC GetQueuedID_index(short index);
USBWEJAPI_API DWORD  USBWEJAPICC GetSN(void);
USBWEJAPI_API BSHRT  USBWEJAPICC GetSepFldData(BYTE *pBuf, short wBufMaxSz);
USBWEJAPI_API const char* USBWEJAPICC GetSerialNumber(void);
USBWEJAPI_API BOOL   USBWEJAPICC GetSonarParms(tsSonarParms* psSonarParms);
USBWEJAPI_API BSHRT  USBWEJAPICC GetTimeParms(tsTimeParms *psTimeParms);
USBWEJAPI_API BSHRT  USBWEJAPICC GetUnsupportedProductErrorCode(void);
USBWEJAPI_API char * USBWEJAPICC GetVidPidVendorName(void);
USBWEJAPI_API BOOL   USBWEJAPICC GetWalkAwayParms(tsWalkAwayParms* psWalkAwayParms);
USBWEJAPI_API BOOL   USBWEJAPICC GetWalkUpParms(tsWalkUpParms* psWalkUpParms);
USBWEJAPI_API BOOL   USBWEJAPICC HaltKBSends(BOOL bHalt);
USBWEJAPI_API BSHRT  USBWEJAPICC IsCardTypeInList(WORD findCT);
USBWEJAPI_API long   USBWEJAPICC Ping(void);
USBWEJAPI_API DWORD  USBWEJAPICC QuickReadSerialPort(char *buf, DWORD count);
USBWEJAPI_API BSHRT  USBWEJAPICC ReadCfg(void);
USBWEJAPI_API BSHRT  USBWEJAPICC ReadDevCfgFmFile(char *szFileName);
USBWEJAPI_API BSHRT  USBWEJAPICC ReadDevCfgFromSecureFile(char *szFileName);
USBWEJAPI_API WORD   USBWEJAPICC ReadDevTypeFromFile(char *szFileName);
USBWEJAPI_API DWORD  USBWEJAPICC ReadSerialPort(char *buf, DWORD count);
USBWEJAPI_API BSHRT  USBWEJAPICC ResetFactoryDflts(void);
USBWEJAPI_API BOOL   USBWEJAPICC ResetUserDflts(void);
USBWEJAPI_API BOOL   USBWEJAPICC SaveUserDflts(void);
USBWEJAPI_API void   USBWEJAPICC SendGetFWVersion(void);
USBWEJAPI_API BOOL   USBWEJAPICC Set64BytesInPackets(BOOL on);
USBWEJAPI_API BSHRT  USBWEJAPICC SetAZERTYShiftLock(short on);
USBWEJAPI_API BSHRT  USBWEJAPICC SetActConfig(BYTE n);
USBWEJAPI_API BSHRT  USBWEJAPICC SetActDev(short iNdx);
USBWEJAPI_API BSHRT  USBWEJAPICC SetBprRlyCtrl(tsBprRlyCtrl *psBRCtrl);
USBWEJAPI_API BSHRT  USBWEJAPICC SetCardTypePriority(WORD cardType, BSHRT priority);
USBWEJAPI_API BSHRT  USBWEJAPICC SetComLinux(WORD index, const char *devName);
USBWEJAPI_API BSHRT  USBWEJAPICC SetComSrchRange(WORD iMin, WORD iMax);
USBWEJAPI_API BSHRT  USBWEJAPICC SetConnectProduct(ULONG bits);
USBWEJAPI_API BSHRT  USBWEJAPICC SetDevTypeSrch(short iSrchType);
USBWEJAPI_API BOOL	 USBWEJAPICC SetEasyReadMode(BOOL on);
USBWEJAPI_API BSHRT  USBWEJAPICC SetExtendedPrecisionMath(short on);
USBWEJAPI_API BSHRT  USBWEJAPICC SetFlags(tsCfgFlags *psCfgFlgs);
USBWEJAPI_API BSHRT  USBWEJAPICC SetFlags2(tsCfgFlags2 *psCfgFlgs);
USBWEJAPI_API BSHRT  USBWEJAPICC SetFlags3(tsCfgFlags3 *psCfgFlgs);
USBWEJAPI_API BSHRT  USBWEJAPICC SetIDBitCnts(tsIDBitCnts *psIDBitCnts);
USBWEJAPI_API BSHRT  USBWEJAPICC SetIDDispParms(tsIDDispParms *psIDDispParms);
USBWEJAPI_API BSHRT  USBWEJAPICC SetIDDispParms2(tsIDDispParms2 *psIDDispParms);
USBWEJAPI_API BSHRT  USBWEJAPICC SetIDDispParms3(tsIDDispParms3 *psIDDispParms);
USBWEJAPI_API BOOL   USBWEJAPICC SetIdleParms(tsIdleParms* psIdleParms);
USBWEJAPI_API BSHRT  USBWEJAPICC SetIpPort(BYTE i0, BYTE i1, BYTE i2, BYTE i3, unsigned short port);
USBWEJAPI_API BSHRT  USBWEJAPICC SetLEDCtrl(tsLEDCtrl *psLEDCtrl);
USBWEJAPI_API BSHRT  USBWEJAPICC SetLUID(short LUID);
USBWEJAPI_API bool   USBWEJAPICC SetModuleState(short moduleID, short desiredState);
USBWEJAPI_API BOOL	 USBWEJAPICC SetPrependCSN(BOOL on);
USBWEJAPI_API BSHRT  USBWEJAPICC SetSepFldData(BYTE *pBuf, short wBufMaxSz);
USBWEJAPI_API BOOL   USBWEJAPICC SetSonarParms(tsSonarParms* psSonarParms);
USBWEJAPI_API BSHRT  USBWEJAPICC SetTimeParms(tsTimeParms *psTimeParms);
USBWEJAPI_API void   USBWEJAPICC SetUnsupportedProductErrorCode(BSHRT v);
USBWEJAPI_API BOOL   USBWEJAPICC SetWalkAwayParms(tsWalkAwayParms* psWalkAwayParms);
USBWEJAPI_API BOOL   USBWEJAPICC SetWalkUpParms(tsWalkUpParms* psWalkUpParms);
USBWEJAPI_API BSHRT  USBWEJAPICC USBConnect(long* plDID);
USBWEJAPI_API BSHRT  USBWEJAPICC USBDisconnect(void);
USBWEJAPI_API BSHRT  USBWEJAPICC VirtualComSearchRange(WORD iMin, WORD iMax);
USBWEJAPI_API BSHRT  USBWEJAPICC WriteCfg(void);
USBWEJAPI_API BSHRT  USBWEJAPICC WriteDevCfgToFile(char *szFileName);
USBWEJAPI_API BSHRT  USBWEJAPICC WriteBLEDataToFile(char *szFileName);
USBWEJAPI_API BSHRT  USBWEJAPICC WriteBLEKeyToFile(char *szFileName);
USBWEJAPI_API BSHRT  USBWEJAPICC WriteBLEDataToReader(char* szFileName);
USBWEJAPI_API BSHRT  USBWEJAPICC WriteBLEKeyToReader(char* szFileName);
USBWEJAPI_API BSHRT  USBWEJAPICC WriteDevCfgToSecureFile(char *szFileName);
USBWEJAPI_API DWORD  USBWEJAPICC WriteSerialPort(char *buf, DWORD count);
USBWEJAPI_API short  USBWEJAPICC chkAddArrival_char(short index, char c);
USBWEJAPI_API short  USBWEJAPICC chkDelRemoval_char(short index, char c);
USBWEJAPI_API BSHRT  USBWEJAPICC comConnect(void);
USBWEJAPI_API BSHRT  USBWEJAPICC comConnectPort(WORD iPort);
USBWEJAPI_API BSHRT  USBWEJAPICC getActiveCardData(void);
USBWEJAPI_API BSHRT  USBWEJAPICC getActiveCardData_byte(short index);
USBWEJAPI_API short  USBWEJAPICC getActiveID(short wBufMaxSz);
USBWEJAPI_API short  USBWEJAPICC getActiveID32(short wBufMaxSz);
USBWEJAPI_API BYTE   USBWEJAPICC getActiveID_byte(short index);
USBWEJAPI_API short  USBWEJAPICC getBprRlyCtrl_bVolatile();
USBWEJAPI_API short  USBWEJAPICC getBprRlyCtrl_iBeeperState();
USBWEJAPI_API short  USBWEJAPICC getBprRlyCtrl_iPad0();
USBWEJAPI_API short  USBWEJAPICC getBprRlyCtrl_iPad3();
USBWEJAPI_API short  USBWEJAPICC getBprRlyCtrl_iPad4();
USBWEJAPI_API short  USBWEJAPICC getBprRlyCtrl_iPad5();
USBWEJAPI_API short  USBWEJAPICC getBprRlyCtrl_iPad6();
USBWEJAPI_API short  USBWEJAPICC getBprRlyCtrl_iRelayState();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags2_bBeepID();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags2_bDspHex();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags2_bRevBytes();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags2_bRevWiegBits();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags2_bUseInvDataF();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags2_bUseLeadChrs();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags2_bWiegInvData();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags2_isASCIIpresent();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags2_isReverseBytesSupported();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags3_bLowerCaseHex();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags3_bNotBootDev();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags3_bPrxProEm();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags3_bSndSFFC();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags3_bSndSFID();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags3_bSndSFON();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags3_bUse64Bit();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags3_bUseNumKP();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags_bFixLenDsp();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags_bFrcBitCntEx();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags_bHaltKBSnd();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags_bNoUseELChar();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags_bSndFac();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags_bSndOnRx();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags_bStripFac();
USBWEJAPI_API short  USBWEJAPICC getCfgFlags_bUseDelFac2Id();
USBWEJAPI_API char   USBWEJAPICC getDevName_char(short index);
USBWEJAPI_API const char* USBWEJAPICC getESN();
USBWEJAPI_API short  USBWEJAPICC getIDBitCnts_iIDBitCnt();
USBWEJAPI_API short  USBWEJAPICC getIDBitCnts_iLeadParityBitCnt();
USBWEJAPI_API short  USBWEJAPICC getIDBitCnts_iPad4();
USBWEJAPI_API short  USBWEJAPICC getIDBitCnts_iPad5();
USBWEJAPI_API short  USBWEJAPICC getIDBitCnts_iPad6();
USBWEJAPI_API short  USBWEJAPICC getIDBitCnts_iPad7();
USBWEJAPI_API short  USBWEJAPICC getIDBitCnts_iTotalBitCnt();
USBWEJAPI_API short  USBWEJAPICC getIDBitCnts_iTrailParityBitCnt();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms2_iCrdGnChr0();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms2_iCrdGnChr1();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms2_iLeadChr0();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms2_iLeadChr1();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms2_iLeadChr2();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms2_iLeadChrCnt();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms2_iPad6();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms2_iPad7();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms3_iPad4();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms3_iPad5();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms3_iPad6();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms3_iPad7();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms3_iTrailChr0();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms3_iTrailChr1();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms3_iTrailChr2();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms3_iTrailChrCnt();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms_iELDelim();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms_iFACDispLen();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms_iFACIDDelim();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms_iIDDispLen();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms_iExOutputFormat();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms_iPad5();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms_iPad6();
USBWEJAPI_API short  USBWEJAPICC getIDDispParms_iPad7();
USBWEJAPI_API short  USBWEJAPICC getLEDCtrl_bAppCtrlsLED();
USBWEJAPI_API short  USBWEJAPICC getLEDCtrl_bVolatile();
USBWEJAPI_API short  USBWEJAPICC getLEDCtrl_iGrnLEDState();
USBWEJAPI_API short  USBWEJAPICC getLEDCtrl_iPad3();
USBWEJAPI_API short  USBWEJAPICC getLEDCtrl_iPad4();
USBWEJAPI_API short  USBWEJAPICC getLEDCtrl_iPad5();
USBWEJAPI_API short  USBWEJAPICC getLEDCtrl_iPad6();
USBWEJAPI_API short  USBWEJAPICC getLEDCtrl_iRedLEDState();
USBWEJAPI_API short  USBWEJAPICC getLibraryVersion_Build(void);
USBWEJAPI_API short  USBWEJAPICC getLibraryVersion_Major(void);
USBWEJAPI_API short  USBWEJAPICC getLibraryVersion_Minor(void);
USBWEJAPI_API const char * USBWEJAPICC getLibraryVersion_Beta(void);
USBWEJAPI_API const char * USBWEJAPICC getPartNumberString(void);
USBWEJAPI_API char   USBWEJAPICC getPartNumberString_char(short index);
USBWEJAPI_API short  USBWEJAPICC getTimeParms_ExFeatures01(void);
USBWEJAPI_API short  USBWEJAPICC getTimeParms_iBitStrmTO();
USBWEJAPI_API short  USBWEJAPICC getTimeParms_iIDHoldTO();
USBWEJAPI_API short  USBWEJAPICC getTimeParms_iIDLockOutTm();
USBWEJAPI_API short  USBWEJAPICC getTimeParms_iPad6();
USBWEJAPI_API short  USBWEJAPICC getTimeParms_iTPCfgFlg3();
USBWEJAPI_API short  USBWEJAPICC getTimeParms_iUSBKeyPrsTm();
USBWEJAPI_API short  USBWEJAPICC getTimeParms_iUSBKeyRlsTm();
USBWEJAPI_API char   USBWEJAPICC getVidPidVendorName_char(short index);
USBWEJAPI_API BOOL   USBWEJAPICC pcProxPlusDefaults(void);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeClearDataAvailable(void);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeClearOverrun(void);
USBWEJAPI_API WORD   USBWEJAPICC pcSwipeGetBeeper(WORD i);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeGetFieldEnable(WORD field);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeGetFieldIgnoreLRC(WORD field);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeGetFieldKeyCount(WORD field);
USBWEJAPI_API WORD   USBWEJAPICC pcSwipeGetFieldKeydata(WORD field, BSHRT kindex);
USBWEJAPI_API WORD   USBWEJAPICC pcSwipeGetFieldLength(WORD field);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeGetFieldMagField(WORD field);
USBWEJAPI_API WORD   USBWEJAPICC pcSwipeGetFieldOffset(WORD field);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeGetFieldShowLRC(WORD field);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeGetFieldShowSepSen(WORD field);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeGetFieldSkip(WORD field);
USBWEJAPI_API WORD   USBWEJAPICC pcSwipeGetFieldTerm(WORD field);
USBWEJAPI_API WORD   USBWEJAPICC pcSwipeGetFieldTrack(WORD field);
USBWEJAPI_API WORD   USBWEJAPICC pcSwipeGetLED(WORD i);
USBWEJAPI_API DWORD  USBWEJAPICC pcSwipeGetSystemCardsRead(void);
USBWEJAPI_API long   USBWEJAPICC pcSwipeGetSystemInternalCount(int index);
USBWEJAPI_API DWORD  USBWEJAPICC pcSwipeGetSystemUptime(void);
USBWEJAPI_API const char * USBWEJAPICC pcSwipeGetTrackData(WORD track, BSHRT toAscii);
USBWEJAPI_API BYTE   USBWEJAPICC pcSwipeGetTrackData_BYTE(WORD track, BSHRT toAscii, WORD index);
USBWEJAPI_API WORD   USBWEJAPICC pcSwipeGetTrackEnables(void);
USBWEJAPI_API const char * USBWEJAPICC pcSwipeGetTrackFieldOffset(WORD track, WORD magField, WORD offset, WORD len, WORD term, BSHRT toAscii);
USBWEJAPI_API WORD   USBWEJAPICC pcSwipeIsDataAvailable(void);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetActiveID(WORD track, WORD magField, WORD offset, WORD term);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetBeeper(WORD i, WORD count, BSHRT longBeep);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldEnable(WORD field, WORD enable);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldIgnoreLRC(WORD field, BSHRT enable);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldKeyCount(WORD field, BYTE nKeys);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldKeydata(WORD field, BSHRT kindex, BSHRT value);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldLength(WORD field, WORD length);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldMagField(WORD field, BSHRT fieldNo);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldOffset(WORD field, WORD byteOfs);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldShowLRC(WORD field, BSHRT enable);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldShowSepSen(WORD field, BSHRT enable);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldSkip(WORD field, BSHRT enable);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldTerm(WORD field, WORD term);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetFieldTrack(WORD field, BSHRT trackNo);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetLED(WORD i, BSHRT v);
USBWEJAPI_API BSHRT  USBWEJAPICC pcSwipeSetTrackEnables(WORD trackEnables);
USBWEJAPI_API DWORD  USBWEJAPICC quickReadSerialPort_char(short index);
USBWEJAPI_API BSHRT  USBWEJAPICC readDevCfgFmFile_char(short index, char c);
USBWEJAPI_API DWORD  USBWEJAPICC readSerialPort_char(short index);
USBWEJAPI_API BSHRT  USBWEJAPICC rf_GetDevName(char *szName);
USBWEJAPI_API short  USBWEJAPICC rf_GetDevType(void);
USBWEJAPI_API BSHRT  USBWEJAPICC setBprRlyCtrl_bVolatile(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setBprRlyCtrl_iBeeperState(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setBprRlyCtrl_iPad0(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setBprRlyCtrl_iPad3(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setBprRlyCtrl_iPad4(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setBprRlyCtrl_iPad5(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setBprRlyCtrl_iPad6(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setBprRlyCtrl_iRelayState(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags2_bBeepID(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags2_bDspHex(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags2_bRevBytes(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags2_bRevWiegBits(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags2_bUseInvDataF(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags2_bUseLeadChrs(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags2_bWiegInvData(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags2_isASCIIpresent(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags3_bLowerCaseHex(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags3_bNotBootDev(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags3_bPrxProEm(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags3_bSndSFFC(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags3_bSndSFID(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags3_bSndSFON(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags3_bUse64Bit(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags3_bUseNumKP(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags_bFixLenDsp(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags_bFrcBitCntEx(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags_bHaltKBSnd(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags_bNoUseELChar(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags_bSndFac(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags_bSndOnRx(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags_bStripFac(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setCfgFlags_bUseDelFac2Id(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDBitCnts_iIDBitCnt(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDBitCnts_iLeadParityBitCnt(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDBitCnts_iPad4(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDBitCnts_iPad5(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDBitCnts_iPad6(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDBitCnts_iPad7(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDBitCnts_iTotalBitCnt(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDBitCnts_iTrailParityBitCnt(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms2_iCrdGnChr0(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms2_iCrdGnChr1(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms2_iLeadChr0(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms2_iLeadChr1(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms2_iLeadChr2(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms2_iLeadChrCnt(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms2_iPad6(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms2_iPad7(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms3_iPad4(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms3_iPad5(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms3_iPad6(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms3_iPad7(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms3_iTrailChr0(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms3_iTrailChr1(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms3_iTrailChr2(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms3_iTrailChrCnt(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms_iELDelim(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms_iFACDispLen(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms_iFACIDDelim(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms_iIDDispLen(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms_iExOutputFormat(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms_iPad5(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms_iPad6(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setIDDispParms_iPad7(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setLEDCtrl_bAppCtrlsLED(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setLEDCtrl_bVolatile(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setLEDCtrl_iGrnLEDState(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setLEDCtrl_iPad3(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setLEDCtrl_iPad4(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setLEDCtrl_iPad5(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setLEDCtrl_iPad6(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setLEDCtrl_iRedLEDState(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setTimeParms_ExFeatures01(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setTimeParms_iBitStrmTO(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setTimeParms_iIDHoldTO(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setTimeParms_iIDLockOutTm(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setTimeParms_iPad6(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setTimeParms_iTPCfgFlg3(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setTimeParms_iUSBKeyPrsTm(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC setTimeParms_iUSBKeyRlsTm(short value);
USBWEJAPI_API BSHRT  USBWEJAPICC usbConnect(void);
USBWEJAPI_API BSHRT  USBWEJAPICC writeDevCfgToFile_char(short index, char c);
//ITT : Extend 2
USBWEJAPI_API BSHRT USBWEJAPICC SetHashKeyData(tsHashKeyData *HashKeyData);
USBWEJAPI_API BOOL  USBWEJAPICC GetEnhanceSecurityFlag();
USBWEJAPI_API void  USBWEJAPICC SetEnhanceSecurityFlag(BOOL enhanceSecurityFlag);
//ITT: Loon Beeper Control
USBWEJAPI_API BSHRT USBWEJAPICC SetBeeperVolume(BSHRT volumeLevel);
USBWEJAPI_API BSHRT USBWEJAPICC GetBeeperVolume();
//ITT: EV1 Keys
USBWEJAPI_API bool USBWEJAPICC WriteSEKeyRefrence(BYTE *keyValue,int keyLength,int keyID = 0x01);
USBWEJAPI_API bool USBWEJAPICC WriteEV1FileRefrence(BYTE *keyValue, int keyLength);
//ITT: Secure API
USBWEJAPI_API bool USBWEJAPICC WriteSecureData(BYTE *blobBuffer, int blobLen);
USBWEJAPI_API bool USBWEJAPICC ReadSecureData(BYTE *headerBuffer, BYTE *dataBuffer);
USBWEJAPI_API short USBWEJAPICC LoadIniFile(char* fileName);
//ITT: TRANSPARENTMODE
USBWEJAPI_API BSHRT USBWEJAPICC CheckTransparentMode();
USBWEJAPI_API BSHRT USBWEJAPICC EnableTransparentMode(int writeToFlash);
USBWEJAPI_API BSHRT USBWEJAPICC DisableTransparentMode(int writeToFlash);
//ITT: BTLE
USBWEJAPI_API BSHRT USBWEJAPICC SetBTLEConfiguration(BSHRT level);
USBWEJAPI_API short USBWEJAPICC GetBTLEConfiguration();
USBWEJAPI_API short USBWEJAPICC IsBTLEPresent();
USBWEJAPI_API char * USBWEJAPICC GetFWFilename();
//Loon reader for version starting from 16.7
USBWEJAPI_API char * USBWEJAPICC GetFullFWVersion();
USBWEJAPI_API void  USBWEJAPICC SetBaudRate(int baudrate);
USBWEJAPI_API BSHRT USBWEJAPICC ReadBaudRate();
USBWEJAPI_API BSHRT USBWEJAPICC WriteBaudRate(BSHRT baudrateCode);
USBWEJAPI_API bool USBWEJAPICC CheckTamperedFile(char* szFileName, int moduleType);
USBWEJAPI_API tHardwareInformation* USBWEJAPICC InstalledHardware();
USBWEJAPI_API tHardwareInformation* USBWEJAPICC SupportedHardware();
//ITT: Card Analuyzer 
USBWEJAPI_API bool USBWEJAPICC SetCardAnalyzerControlState(int state);
USBWEJAPI_API int USBWEJAPICC GetCardAnalyzerStatus();
USBWEJAPI_API int USBWEJAPICC GetCardCount();
USBWEJAPI_API tsCardAnalyzer* USBWEJAPICC GetCardAnalyzerData();
#ifdef __cplusplus
}
#endif
#else
typedef BSHRT  (USBWEJAPICC *BeepNow)(BYTE count, BSHRT longBeep);
typedef bool (USBWEJAPICC *CheckTamperedFile)(char* szFileName, int moduleType);
typedef short  (USBWEJAPICC *ChkAddArrival)(char *pBuf);
typedef BSHRT  (USBWEJAPICC *ChkDelRemoval)(char *szName);
typedef BSHRT  (USBWEJAPICC *ComConnect)(long* plDID);
typedef BSHRT  (USBWEJAPICC *ComConnectPort)(WORD iPort, long* plDID);
typedef BSHRT  (USBWEJAPICC *ComDisconnect)(void);
typedef BSHRT  (USBWEJAPICC *DumpRawFeatureReports)(char *fname); //ITT : 08.08.2014.SP : Added for the Raw feature report request from NTWARE
typedef short  (USBWEJAPICC *FindXport)(short ip0, short ip1, short ip2, short ip3begin, short ip3end);
typedef BOOL   (USBWEJAPICC *Get64BytesInPackets)();
typedef BSHRT  (USBWEJAPICC *GetAZERTYShiftLock)(void);
typedef short  (USBWEJAPICC *GetActConfig)(void);
typedef short  (USBWEJAPICC *GetActDev)(void);
typedef short  (USBWEJAPICC *GetActiveID)(BYTE *pBuf, short wBufMaxSz);
typedef short  (USBWEJAPICC *GetActiveID32)(BYTE *pBuf, short wBufMaxSz);
typedef BSHRT  (USBWEJAPICC *GetBprRlyCtrl)(tsBprRlyCtrl *psBRCtrl);
typedef const char*  (USBWEJAPICC *GetBuildDate)(void);
typedef long   (USBWEJAPICC *GetCardPriority)(void);
typedef long   (USBWEJAPICC *GetCardType)(void);
typedef const char*  (USBWEJAPICC *GetCopyrightInfo)(void);
typedef const char*  (USBWEJAPICC *GetCPUInfo)(void);
typedef short  (USBWEJAPICC *GetDID)(void);
typedef short  (USBWEJAPICC *GetDevByLUID)(short LUID, short index);
typedef short  (USBWEJAPICC *GetDevCnt)(void);
typedef BSHRT  (USBWEJAPICC *GetDevName)(char *szName);
typedef short  (USBWEJAPICC *GetDevType)(void);
typedef BOOL   (USBWEJAPICC *GetEasyReadMode)();
typedef BSHRT  (USBWEJAPICC *GetExtendedPrecisionMath)(void);
typedef DWORD  (USBWEJAPICC *GetFirmwareVersion)(short hardware, short module);
typedef BSHRT  (USBWEJAPICC *GetFlags)(tsCfgFlags *psCfgFlgs);
typedef BSHRT  (USBWEJAPICC *GetFlags2)(tsCfgFlags2 *psCfgFlgs);
typedef BSHRT  (USBWEJAPICC *GetFlags3)(tsCfgFlags3 *psCfgFlgs);
typedef BSHRT  (USBWEJAPICC *GetHIDGuid)(GUID *pGuid);
typedef BSHRT  (USBWEJAPICC *GetIDBitCnts)(tsIDBitCnts *psIDBitCnts);
typedef BSHRT  (USBWEJAPICC *GetIDDispParms)(tsIDDispParms *psIDDispParms);
typedef BSHRT  (USBWEJAPICC *GetIDDispParms2)(tsIDDispParms2 *psIDDispParms);
typedef BSHRT  (USBWEJAPICC *GetIDDispParms3)(tsIDDispParms3 *psIDDispParms);
typedef BOOL   (USBWEJAPICC *GetIdleParms)(tsIdleParms* psIdleParms);
typedef BSHRT  (USBWEJAPICC *GetLEDCtrl)(tsLEDCtrl *psLEDCtrl);
typedef short  (USBWEJAPICC *GetLUID)(void);
typedef long   (USBWEJAPICC *GetLastLibErr)(void);
typedef BSHRT  (USBWEJAPICC *GetLibVersion)(short* piVerMaj, short* piVerMin, short* piVerDev);
typedef WORD   (USBWEJAPICC *GetMaxConfig)(void);
typedef bool   (USBWEJAPICC *GetModuleState)(short moduleID, short *moduleState);
typedef ULONG  (USBWEJAPICC *GetMyIpAddress)(void);
typedef BOOL   (USBWEJAPICC *GetObjRangeInfo)(tsRangeInfo* psRangeInfo);
typedef BOOL   (USBWEJAPICC *GetPrependCSN)();
typedef ULONG  (USBWEJAPICC *GetProduct)(void);
typedef const char*  (USBWEJAPICC *GetPlatformType)(void);
typedef short  (USBWEJAPICC *GetQueuedID)(short clearUID, short clearHold);
typedef long   (USBWEJAPICC *GetQueuedID_index)(short index);
typedef DWORD  (USBWEJAPICC *GetSN)(void);
typedef BSHRT  (USBWEJAPICC *GetSepFldData)(BYTE *pBuf, short wBufMaxSz);
typedef const char*  (USBWEJAPICC *GetSerialNumber)(void);
typedef BOOL   (USBWEJAPICC *GetSonarParms)(tsSonarParms* psSonarParms);
typedef BSHRT  (USBWEJAPICC *GetTimeParms)(tsTimeParms *psTimeParms);
typedef BSHRT  (USBWEJAPICC *GetUnsupportedProductErrorCode)(void);
typedef char * (USBWEJAPICC *GetVidPidVendorName)(void);
typedef BOOL   (USBWEJAPICC *GetWalkAwayParms)(tsWalkAwayParms* psWalkAwayParms);
typedef BOOL   (USBWEJAPICC *GetWalkUpParms)(tsWalkUpParms* psWalkUpParms);
typedef BOOL   (USBWEJAPICC *HaltKBSends)(BOOL bHalt);
typedef BSHRT  (USBWEJAPICC *IsCardTypeInList)(WORD findCT);
typedef long   (USBWEJAPICC *Ping)(void);
typedef DWORD  (USBWEJAPICC *QuickReadSerialPort)(char *buf, DWORD count);
typedef BSHRT  (USBWEJAPICC *ReadCfg)(void);
typedef BSHRT  (USBWEJAPICC *ReadDevCfgFmFile)(char *szFileName);
typedef BSHRT  (USBWEJAPICC *ReadDevCfgFromSecureFile)(char* szFileName);
typedef WORD   (USBWEJAPICC *ReadDevTypeFromFile)(char *szFileName);
typedef DWORD  (USBWEJAPICC *ReadSerialPort)(char *buf, DWORD count);
typedef BSHRT  (USBWEJAPICC *ResetFactoryDflts)(void);
typedef BOOL   (USBWEJAPICC *ResetUserDflts)(void);
typedef BOOL   (USBWEJAPICC *SaveUserDflts)(void);
typedef void   (USBWEJAPICC* SendGetFWVersion)(void);
typedef BOOL   (USBWEJAPICC *Set64BytesInPackets)(BOOL on);
typedef BSHRT  (USBWEJAPICC *SetAZERTYShiftLock)(short on);
typedef BSHRT  (USBWEJAPICC *SetActConfig)(BYTE n);
typedef BSHRT  (USBWEJAPICC *SetActDev)(short iNdx);
typedef BSHRT  (USBWEJAPICC *SetBprRlyCtrl)(tsBprRlyCtrl *psBRCtrl);
typedef BSHRT  (USBWEJAPICC *SetCardTypePriority)(WORD cardType, BSHRT priority);
typedef BSHRT  (USBWEJAPICC *SetComLinux)(WORD index, const char *devName);
typedef BSHRT  (USBWEJAPICC *SetComSrchRange)(WORD iMin, WORD iMax);
typedef BSHRT  (USBWEJAPICC *SetConnectProduct)(ULONG bits);
typedef BSHRT  (USBWEJAPICC *SetDevTypeSrch)(short iSrchType);
typedef BOOL   (USBWEJAPICC *SetEasyReadMode)(BOOL on);
typedef BSHRT  (USBWEJAPICC *SetExtendedPrecisionMath)(short on);
typedef BSHRT  (USBWEJAPICC *SetFlags)(tsCfgFlags *psCfgFlgs);
typedef BSHRT  (USBWEJAPICC *SetFlags2)(tsCfgFlags2 *psCfgFlgs);
typedef BSHRT  (USBWEJAPICC *SetFlags3)(tsCfgFlags3 *psCfgFlgs);
typedef BSHRT  (USBWEJAPICC *SetIDBitCnts)(tsIDBitCnts *psIDBitCnts);
typedef BSHRT  (USBWEJAPICC *SetIDDispParms)(tsIDDispParms *psIDDispParms);
typedef BSHRT  (USBWEJAPICC *SetIDDispParms2)(tsIDDispParms2 *psIDDispParms);
typedef BSHRT  (USBWEJAPICC *SetIDDispParms3)(tsIDDispParms3 *psIDDispParms);
typedef BOOL   (USBWEJAPICC *SetIdleParms)(tsIdleParms* psIdleParms);
typedef BSHRT  (USBWEJAPICC *SetIpPort)(BYTE i0, BYTE i1, BYTE i2, BYTE i3, unsigned short port);
typedef BSHRT  (USBWEJAPICC *SetLEDCtrl)(tsLEDCtrl *psLEDCtrl);
typedef BSHRT  (USBWEJAPICC *SetLUID)(short LUID);
typedef bool   (USBWEJAPICC *SetModuleState)(short moduleID, short desiredState);
typedef BOOL   (USBWEJAPICC *SetPrependCSN)(BOOL on);
typedef BSHRT  (USBWEJAPICC *SetSepFldData)(BYTE *pBuf, short wBufMaxSz);
typedef BOOL   (USBWEJAPICC *SetSonarParms)(tsSonarParms* psSonarParms);
typedef BSHRT  (USBWEJAPICC *SetTimeParms)(tsTimeParms *psTimeParms);
typedef void   (USBWEJAPICC *SetUnsupportedProductErrorCode)(BSHRT v);
typedef BOOL   (USBWEJAPICC *SetWalkAwayParms)(tsWalkAwayParms* psWalkAwayParms);
typedef BOOL   (USBWEJAPICC *SetWalkUpParms)(tsWalkUpParms* psWalkUpParms);
typedef BSHRT  (USBWEJAPICC *USBConnect)(long* plDID);
typedef BSHRT  (USBWEJAPICC *USBDisconnect)(void);
typedef BSHRT  (USBWEJAPICC *VirtualComSearchRange)(WORD iMin, WORD iMax);
typedef BSHRT  (USBWEJAPICC *WriteCfg)(void);
typedef BSHRT  (USBWEJAPICC *WriteDevCfgToFile)(char *szFileName);
typedef BSHRT  (USBWEJAPICC *WriteBLEDataToFile)(char *szFileName);
typedef BSHRT  (USBWEJAPICC *WriteBLEKeyToFile)(char *szFileName);
typedef BSHRT  (USBWEJAPICC *WriteBLEDataToReader)(char* szFileName);
typedef BSHRT  (USBWEJAPICC *WriteBLEKeyToReader)(char* szFileName);
typedef BSHRT  (USBWEJAPICC *WriteDevCfgToSecureFile)(char* szFileName);
typedef DWORD  (USBWEJAPICC *WriteSerialPort)(char *buf, DWORD count);
typedef short  (USBWEJAPICC *chkAddArrival_char)(short index, char c);
typedef short  (USBWEJAPICC *chkDelRemoval_char)(short index, char c);
typedef BSHRT  (USBWEJAPICC *comConnect)(void);
typedef BSHRT  (USBWEJAPICC *comConnectPort)(WORD iPort);
typedef BSHRT  (USBWEJAPICC *getActiveCardData)(void);
typedef BSHRT  (USBWEJAPICC *getActiveCardData_byte)(short index);
typedef short  (USBWEJAPICC *getActiveID)(short wBufMaxSz);
typedef short  (USBWEJAPICC *getActiveID32)(short wBufMaxSz);
typedef BYTE   (USBWEJAPICC *getActiveID_byte)(short index);
typedef short  (USBWEJAPICC *getBprRlyCtrl_bVolatile)();
typedef short  (USBWEJAPICC *getBprRlyCtrl_iBeeperState)();
typedef short  (USBWEJAPICC *getBprRlyCtrl_iPad0)();
typedef short  (USBWEJAPICC *getBprRlyCtrl_iPad3)();
typedef short  (USBWEJAPICC *getBprRlyCtrl_iPad4)();
typedef short  (USBWEJAPICC *getBprRlyCtrl_iPad5)();
typedef short  (USBWEJAPICC *getBprRlyCtrl_iPad6)();
typedef short  (USBWEJAPICC *getBprRlyCtrl_iRelayState)();
typedef short  (USBWEJAPICC *getCfgFlags2_bBeepID)();
typedef short  (USBWEJAPICC *getCfgFlags2_bDspHex)();
typedef short  (USBWEJAPICC *getCfgFlags2_bRevBytes)();
typedef short  (USBWEJAPICC *getCfgFlags2_bRevWiegBits)();
typedef short  (USBWEJAPICC *getCfgFlags2_bUseInvDataF)();
typedef short  (USBWEJAPICC *getCfgFlags2_bUseLeadChrs)();
typedef short  (USBWEJAPICC *getCfgFlags2_bWiegInvData)();
typedef short  (USBWEJAPICC *getCfgFlags2_isASCIIpresent)();
typedef short  (USBWEJAPICC *getCfgFlags2_isReverseBytesSupported)();
typedef short  (USBWEJAPICC *getCfgFlags3_bLowerCaseHex)();
typedef short  (USBWEJAPICC *getCfgFlags3_bNotBootDev)();
typedef short  (USBWEJAPICC *getCfgFlags3_bPrxProEm)();
typedef short  (USBWEJAPICC *getCfgFlags3_bSndSFFC)();
typedef short  (USBWEJAPICC *getCfgFlags3_bSndSFID)();
typedef short  (USBWEJAPICC *getCfgFlags3_bSndSFON)();
typedef short  (USBWEJAPICC *getCfgFlags3_bUse64Bit)();
typedef short  (USBWEJAPICC *getCfgFlags3_bUseNumKP)();
typedef short  (USBWEJAPICC *getCfgFlags_bFixLenDsp)();
typedef short  (USBWEJAPICC *getCfgFlags_bFrcBitCntEx)();
typedef short  (USBWEJAPICC *getCfgFlags_bHaltKBSnd)();
typedef short  (USBWEJAPICC *getCfgFlags_bNoUseELChar)();
typedef short  (USBWEJAPICC *getCfgFlags_bSndFac)();
typedef short  (USBWEJAPICC *getCfgFlags_bSndOnRx)();
typedef short  (USBWEJAPICC *getCfgFlags_bStripFac)();
typedef short  (USBWEJAPICC *getCfgFlags_bUseDelFac2Id)();
typedef char   (USBWEJAPICC *getDevName_char)(short index);
typedef const char*  (USBWEJAPICC *getESN)();
typedef short  (USBWEJAPICC *getIDBitCnts_iIDBitCnt)();
typedef short  (USBWEJAPICC *getIDBitCnts_iLeadParityBitCnt)();
typedef short  (USBWEJAPICC *getIDBitCnts_iPad4)();
typedef short  (USBWEJAPICC *getIDBitCnts_iPad5)();
typedef short  (USBWEJAPICC *getIDBitCnts_iPad6)();
typedef short  (USBWEJAPICC *getIDBitCnts_iPad7)();
typedef short  (USBWEJAPICC *getIDBitCnts_iTotalBitCnt)();
typedef short  (USBWEJAPICC *getIDBitCnts_iTrailParityBitCnt)();
typedef short  (USBWEJAPICC *getIDDispParms2_iCrdGnChr0)();
typedef short  (USBWEJAPICC *getIDDispParms2_iCrdGnChr1)();
typedef short  (USBWEJAPICC *getIDDispParms2_iLeadChr0)();
typedef short  (USBWEJAPICC *getIDDispParms2_iLeadChr1)();
typedef short  (USBWEJAPICC *getIDDispParms2_iLeadChr2)();
typedef short  (USBWEJAPICC *getIDDispParms2_iLeadChrCnt)();
typedef short  (USBWEJAPICC *getIDDispParms2_iPad6)();
typedef short  (USBWEJAPICC *getIDDispParms2_iPad7)();
typedef short  (USBWEJAPICC *getIDDispParms3_iPad4)();
typedef short  (USBWEJAPICC *getIDDispParms3_iPad5)();
typedef short  (USBWEJAPICC *getIDDispParms3_iPad6)();
typedef short  (USBWEJAPICC *getIDDispParms3_iPad7)();
typedef short  (USBWEJAPICC *getIDDispParms3_iTrailChr0)();
typedef short  (USBWEJAPICC *getIDDispParms3_iTrailChr1)();
typedef short  (USBWEJAPICC *getIDDispParms3_iTrailChr2)();
typedef short  (USBWEJAPICC *getIDDispParms3_iTrailChrCnt)();
typedef short  (USBWEJAPICC *getIDDispParms_iELDelim)();
typedef short  (USBWEJAPICC *getIDDispParms_iFACDispLen)();
typedef short  (USBWEJAPICC *getIDDispParms_iFACIDDelim)();
typedef short  (USBWEJAPICC *getIDDispParms_iIDDispLen)();
typedef short  (USBWEJAPICC *getIDDispParms_iExOutputFormat)();
typedef short  (USBWEJAPICC *getIDDispParms_iPad5)();
typedef short  (USBWEJAPICC *getIDDispParms_iPad6)();
typedef short  (USBWEJAPICC *getIDDispParms_iPad7)();
typedef short  (USBWEJAPICC *getLEDCtrl_bAppCtrlsLED)();
typedef short  (USBWEJAPICC *getLEDCtrl_bVolatile)();
typedef short  (USBWEJAPICC *getLEDCtrl_iGrnLEDState)();
typedef short  (USBWEJAPICC *getLEDCtrl_iPad3)();
typedef short  (USBWEJAPICC *getLEDCtrl_iPad4)();
typedef short  (USBWEJAPICC *getLEDCtrl_iPad5)();
typedef short  (USBWEJAPICC *getLEDCtrl_iPad6)();
typedef short  (USBWEJAPICC *getLEDCtrl_iRedLEDState)();
typedef short  (USBWEJAPICC *getLibraryVersion_Build)(void);
typedef short  (USBWEJAPICC *getLibraryVersion_Major)(void);
typedef short  (USBWEJAPICC *getLibraryVersion_Minor)(void);
typedef const char * (USBWEJAPICC *getLibraryVersion_Beta)(void);
typedef const char * (USBWEJAPICC *getPartNumberString)(void);
typedef char   (USBWEJAPICC *getPartNumberString_char)(short index);
typedef short  (USBWEJAPICC *getTimeParms_ExFeatures01)(void);
typedef short  (USBWEJAPICC *getTimeParms_iBitStrmTO)();
typedef short  (USBWEJAPICC *getTimeParms_iIDHoldTO)();
typedef short  (USBWEJAPICC *getTimeParms_iIDLockOutTm)();
typedef short  (USBWEJAPICC *getTimeParms_iPad6)();
typedef short  (USBWEJAPICC *getTimeParms_iTPCfgFlg3)();
typedef short  (USBWEJAPICC *getTimeParms_iUSBKeyPrsTm)();
typedef short  (USBWEJAPICC *getTimeParms_iUSBKeyRlsTm)();
typedef char   (USBWEJAPICC *getVidPidVendorName_char)(short index);
typedef BOOL   (USBWEJAPICC *pcProxPlusDefaults)(void);
typedef BSHRT  (USBWEJAPICC *pcSwipeClearDataAvailable)(void);
typedef BSHRT  (USBWEJAPICC *pcSwipeClearOverrun)(void);
typedef WORD   (USBWEJAPICC *pcSwipeGetBeeper)(WORD i);
typedef BSHRT  (USBWEJAPICC *pcSwipeGetFieldEnable)(WORD field);
typedef BSHRT  (USBWEJAPICC *pcSwipeGetFieldIgnoreLRC)(WORD field);
typedef BSHRT  (USBWEJAPICC *pcSwipeGetFieldKeyCount)(WORD field);
typedef WORD   (USBWEJAPICC *pcSwipeGetFieldKeydata)(WORD field, BSHRT kindex);
typedef WORD   (USBWEJAPICC *pcSwipeGetFieldLength)(WORD field);
typedef BSHRT  (USBWEJAPICC *pcSwipeGetFieldMagField)(WORD field);
typedef WORD   (USBWEJAPICC *pcSwipeGetFieldOffset)(WORD field);
typedef BSHRT  (USBWEJAPICC *pcSwipeGetFieldShowLRC)(WORD field);
typedef BSHRT  (USBWEJAPICC *pcSwipeGetFieldShowSepSen)(WORD field);
typedef BSHRT  (USBWEJAPICC *pcSwipeGetFieldSkip)(WORD field);
typedef WORD   (USBWEJAPICC *pcSwipeGetFieldTerm)(WORD field);
typedef WORD   (USBWEJAPICC *pcSwipeGetFieldTrack)(WORD field);
typedef WORD   (USBWEJAPICC *pcSwipeGetLED)(WORD i);
typedef DWORD  (USBWEJAPICC *pcSwipeGetSystemCardsRead)(void);
typedef long   (USBWEJAPICC *pcSwipeGetSystemInternalCount)(int index);
typedef DWORD  (USBWEJAPICC *pcSwipeGetSystemUptime)(void);
typedef const char * (USBWEJAPICC *pcSwipeGetTrackData)(WORD track, BSHRT toAscii);
typedef BYTE   (USBWEJAPICC *pcSwipeGetTrackData_BYTE)(WORD track, BSHRT toAscii, WORD index);
typedef WORD   (USBWEJAPICC *pcSwipeGetTrackEnables)(void);
typedef const char * (USBWEJAPICC *pcSwipeGetTrackFieldOffset)(WORD track, WORD magField, WORD offset, WORD len, WORD term, BSHRT toAscii);
typedef WORD   (USBWEJAPICC *pcSwipeIsDataAvailable)(void);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetActiveID)(WORD track, WORD magField, WORD offset, WORD term);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetBeeper)(WORD i, WORD count, BSHRT longBeep);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldEnable)(WORD field, WORD enable);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldIgnoreLRC)(WORD field, BSHRT enable);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldKeyCount)(WORD field, BYTE nKeys);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldKeydata)(WORD field, BSHRT kindex, BSHRT value);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldLength)(WORD field, WORD length);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldMagField)(WORD field, BSHRT fieldNo);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldOffset)(WORD field, WORD byteOfs);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldShowLRC)(WORD field, BSHRT enable);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldShowSepSen)(WORD field, BSHRT enable);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldSkip)(WORD field, BSHRT enable);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldTerm)(WORD field, WORD term);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetFieldTrack)(WORD field, BSHRT trackNo);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetLED)(WORD i, BSHRT v);
typedef BSHRT  (USBWEJAPICC *pcSwipeSetTrackEnables)(WORD trackEnables);
typedef DWORD  (USBWEJAPICC *quickReadSerialPort_char)(short index);
typedef BSHRT  (USBWEJAPICC *readDevCfgFmFile_char)(short index, char c);
typedef DWORD  (USBWEJAPICC *readSerialPort_char)(short index);
typedef BSHRT  (USBWEJAPICC *rf_GetDevName)(char *szName);
typedef short  (USBWEJAPICC *rf_GetDevType)(void);
typedef BSHRT  (USBWEJAPICC *setBprRlyCtrl_bVolatile)(short value);
typedef BSHRT  (USBWEJAPICC *setBprRlyCtrl_iBeeperState)(short value);
typedef BSHRT  (USBWEJAPICC *setBprRlyCtrl_iPad0)(short value);
typedef BSHRT  (USBWEJAPICC *setBprRlyCtrl_iPad3)(short value);
typedef BSHRT  (USBWEJAPICC *setBprRlyCtrl_iPad4)(short value);
typedef BSHRT  (USBWEJAPICC *setBprRlyCtrl_iPad5)(short value);
typedef BSHRT  (USBWEJAPICC *setBprRlyCtrl_iPad6)(short value);
typedef BSHRT  (USBWEJAPICC *setBprRlyCtrl_iRelayState)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags2_bBeepID)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags2_bDspHex)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags2_bRevBytes)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags2_bRevWiegBits)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags2_bUseInvDataF)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags2_bUseLeadChrs)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags2_bWiegInvData)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags2_isASCIIpresent)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags3_bLowerCaseHex)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags3_bNotBootDev)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags3_bPrxProEm)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags3_bSndSFFC)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags3_bSndSFID)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags3_bSndSFON)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags3_bUse64Bit)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags3_bUseNumKP)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags_bFixLenDsp)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags_bFrcBitCntEx)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags_bHaltKBSnd)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags_bNoUseELChar)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags_bSndFac)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags_bSndOnRx)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags_bStripFac)(short value);
typedef BSHRT  (USBWEJAPICC *setCfgFlags_bUseDelFac2Id)(short value);
typedef BSHRT  (USBWEJAPICC *setIDBitCnts_iIDBitCnt)(short value);
typedef BSHRT  (USBWEJAPICC *setIDBitCnts_iLeadParityBitCnt)(short value);
typedef BSHRT  (USBWEJAPICC *setIDBitCnts_iPad4)(short value);
typedef BSHRT  (USBWEJAPICC *setIDBitCnts_iPad5)(short value);
typedef BSHRT  (USBWEJAPICC *setIDBitCnts_iPad6)(short value);
typedef BSHRT  (USBWEJAPICC *setIDBitCnts_iPad7)(short value);
typedef BSHRT  (USBWEJAPICC *setIDBitCnts_iTotalBitCnt)(short value);
typedef BSHRT  (USBWEJAPICC *setIDBitCnts_iTrailParityBitCnt)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms2_iCrdGnChr0)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms2_iCrdGnChr1)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms2_iLeadChr0)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms2_iLeadChr1)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms2_iLeadChr2)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms2_iLeadChrCnt)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms2_iPad6)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms2_iPad7)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms3_iPad4)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms3_iPad5)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms3_iPad6)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms3_iPad7)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms3_iTrailChr0)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms3_iTrailChr1)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms3_iTrailChr2)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms3_iTrailChrCnt)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms_iELDelim)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms_iFACDispLen)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms_iFACIDDelim)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms_iIDDispLen)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms_iExOutputFormat)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms_iPad5)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms_iPad6)(short value);
typedef BSHRT  (USBWEJAPICC *setIDDispParms_iPad7)(short value);
typedef BSHRT  (USBWEJAPICC *setLEDCtrl_bAppCtrlsLED)(short value);
typedef BSHRT  (USBWEJAPICC *setLEDCtrl_bVolatile)(short value);
typedef BSHRT  (USBWEJAPICC *setLEDCtrl_iGrnLEDState)(short value);
typedef BSHRT  (USBWEJAPICC *setLEDCtrl_iPad3)(short value);
typedef BSHRT  (USBWEJAPICC *setLEDCtrl_iPad4)(short value);
typedef BSHRT  (USBWEJAPICC *setLEDCtrl_iPad5)(short value);
typedef BSHRT  (USBWEJAPICC *setLEDCtrl_iPad6)(short value);
typedef BSHRT  (USBWEJAPICC *setLEDCtrl_iRedLEDState)(short value);
typedef BSHRT  (USBWEJAPICC *setTimeParms_ExFeatures01)(short value);
typedef BSHRT  (USBWEJAPICC *setTimeParms_iBitStrmTO)(short value);
typedef BSHRT  (USBWEJAPICC *setTimeParms_iIDHoldTO)(short value);
typedef BSHRT  (USBWEJAPICC *setTimeParms_iIDLockOutTm)(short value);
typedef BSHRT  (USBWEJAPICC *setTimeParms_iPad6)(short value);
typedef BSHRT  (USBWEJAPICC *setTimeParms_iTPCfgFlg3)(short value);
typedef BSHRT  (USBWEJAPICC *setTimeParms_iUSBKeyPrsTm)(short value);
typedef BSHRT  (USBWEJAPICC *setTimeParms_iUSBKeyRlsTm)(short value);
typedef BSHRT  (USBWEJAPICC *usbConnect)(void);
typedef BSHRT  (USBWEJAPICC *writeDevCfgToFile_char)(short index, char c);
// ITT: Extend 2
typedef BSHRT  (USBWEJAPICC *SetHashKeyData)(tsHashKeyData *HashKeyData);
typedef BOOL  (USBWEJAPICC *GetEnhanceSecurityFlag)();
typedef void  (USBWEJAPICC *SetEnhanceSecurityFlag)(BOOL enhanceSecurityFlag);
// ITT: Loon Beeper Control
typedef BSHRT  (USBWEJAPICC *SetBeeperVolume)(BSHRT volumeLevel);
typedef BSHRT  (USBWEJAPICC *GetBeeperVolume)();
typedef bool (USBWEJAPICC *WriteSEKeyRefrence)(BYTE *keyValue,int keyLength, int keyID);
typedef bool (USBWEJAPICC *WriteEV1FileRefrence)(BYTE *keyValue, int keyLength);
typedef BSHRT (USBWEJAPICC *SetBTLEConfiguration)(BSHRT level);
typedef BSHRT (USBWEJAPICC *GetBTLEConfiguration)();
typedef BSHRT (USBWEJAPICC *IsBTLEPresent)();
//ITT: Secure API
typedef bool (USBWEJAPICC *WriteSecureData)(BYTE *blobBuffer, int blobLen);
typedef bool (USBWEJAPICC *ReadSecureData)(BYTE *headerBuffer, BYTE *dataBuffer);
typedef short (USBWEJAPICC *LoadIniFile)(char* fileName);
//ITT: Transparent Mode
typedef short (USBWEJAPICC* CheckTransparentMode)();
typedef short (USBWEJAPICC* EnableTransparentMode)(int writeToFlash);
typedef short (USBWEJAPICC* DisableTransparentMode)(int writeToFlash);
// [API_END_TAG]
typedef void  (USBWEJAPICC *SetMarkerFunction)(RFPRXLOG pfLog);
//
typedef char * (USBWEJAPICC *GetFWFilename)();
//
typedef char * (USBWEJAPICC *GetFullFWVersion)();
typedef void (USBWEJAPICC *SetBaudRate)(int baudrate);
typedef BSHRT(USBWEJAPICC *ReadBaudRate)();
typedef BSHRT(USBWEJAPICC *WriteBaudRate)(BSHRT baudrateCode);
typedef tHardwareInformation* (USBWEJAPICC InstalledHardware)();
typedef tHardwareInformation* (USBWEJAPICC SupportedHardware)();
//ITT:  Card Analyzer
typedef bool (USBWEJAPICC* SetCardAnalyzerControlState)(int state);
typedef int (USBWEJAPICC* GetCardAnalyzerStatus)();
typedef int (USBWEJAPICC* GetCardCount)();
typedef int (USBWEJAPICC* GetCardAnalyzerData)();
#endif

//----------------------------------------
// Error Bits: as return by GetLastLibErr()
//----------------------------------------
// USBConnect											0x00010000
//   Failed ReadCfg Loop								0x00011000
// ReadCfg												0x00020000
// WriteCfg												0x00040000
// ResetFactoryDflts									0x00080000
// Get(Structure)										0x00100000
//   NULL Pointer										0x00100001
//   ReadCfg not called									0x00100002
// Set(Structure)										0x00200000
//   NULL Pointer										0x00200001
//														0x00400000
//														0x00800000
// GetActiveID											0x01000000
//														0x02000000
//														0x04000000
//														0x08000000
//														0x10000000
//	SetBTLE												0x20000000
//	GetBTLE & IsBTLEPresent								0x40000000
// WriteSEKeyRefrence & WriteEV1FileRefrence			0x80000000
// GetFWFilename                                        0x50000
// ReadFile error when No Device Connected              0x60000000
// GetFullFWVersion                                     0x70000000
// Secure Configuration                                 0x81000000
// Secrure Token fails                                  0x91000000
// Unable to get BLE data                               0x30000000
// Unable to set BLE data                               0x30000001
// GetModuleState API failed                            0x40000001
// SetModuleState API failed                            0x20000001
// SetCardAnalyzerControlState failed                   0x05000001
// GetCardAnalyzerStatus failed                         0x05000002
// GetCardAnalyzerData failed                           0x05000003
//
//-------------------------------------
// Private Interface Error bits (OR'd w/public bits):
//-------------------------------------
//   USBDeviceConnect    0x000000xx, xx=
//        1:Couldn't open SETUPAPI.DLL
//        2:Unresolved SETUPAPI.DLL entry point
//        3:Couldn't open HID.DLL
//        4:Unresolved HID.DLL entry point
//        5:Unresolved DLL entry point
//        6:SetupDiGetClassDevs returned INVALID_HANDLE_VALUE
//        7:SetupDiEnumDeviceInterfaces failed or ran out of devices
//        8:SetupDiGetDeviceInterfaceDetail: ERROR_INSUFFICIENT_BUFFER != GetLastError()
//        9:Failed pDevIFDetail LocalAlloc
//       10:VendorID and/or ProductID not found
//       11:CreateFile failed
//       12:SetupDiGetDeviceInterfaceDetail returned 0
//   GetUSBDevFeatureRep , xx=
//        0:device not open0x000001xx
//        1:NULL module call reference
//        2:module call returned FALSE
//   SetUSBDevFeatureRep 0x000002xx, xx=
//        0:device not open
//        1:NULL module call reference
//        2:module call returned FALSE
//   CheckUserFlags      0x0000100x, x= [will never fail]
//   CheckUserBitCnts    0x0000101x, x=
//        0:iLeadParityBitCnt > 15
//        1:iTrailParityBitCnt > 15
//        2:(iIDBitCnt < 1) OR (iIDBitCnt > 64)
//        3:(iTotalBitCnt < 26) OR (iTotalBitCnt > 64)
//   CheckUserDispParms  0x0000102x, x=
//        0:iFACIDDelim > 255
//        1:iELDelim > 255
//        2:iIDDispLen > 10
//        3:iFACDispLen > 10
//   CheckUserTimeParms  0x0000103x, x=
//        0:iBitStrmTO > 1020
//        1:iIDHoldTO > 12750
//        2:iIDLockOutTm > 12750
//        3:iUSBKeyPrsTm > 1020
//        4:iUSBKeyRlsTm > 1020
//   CheckUserFlags2     0x0000104x, x= [will never fail]
//   CheckUserDispParms2 0x0000105x, x=
//        0:iLeadChrCnt > 3

//-----------------------------------------------------------------------------
//-----PRODUCT: pcSwipe------
//-----------------------------------------------------------------------------
// The maximum string length of the Device product name "MS3-00M1AKU" null filled
#define MAXSWIPEPRODUCTNAMESZ    20
//
// Track Sentinels
//
#define BYTE_STARTSEN1      0x15    // Track #1 start sentinel
#define BYTE_FIELDSEP1      0x3E
#define BYTE_ENDSEN1        0x5F
//
#define BYTE_STARTSEN23     0x0B
#define BYTE_FIELDSEP23     0x0D
#define BYTE_ENDSEN23       0x0F
// Track ascii sentinels
#define ASCII_STARTSEN1     '%'     // Track #1 start sentinel when in ASCII
#define ASCII_FIELDSEP1     '^'     // Track #1 field seperator when in ASCII
#define ASCII_ENDSEN1       '?'     // Track #1 stop sentinel when in ASCII
//
#define ASCII_STARTSEN23    ';'
#define ASCII_FIELDSEP23    '='
#define ASCII_ENDSEN23      '?'

/////////////////////////////
#define PCSWIPE_STATE_READY     0   // Can Set led color
#define PCSWIPE_STATE_READING   1   // Can Set led color
#define PCSWIPE_STATE_GOOD      2   // Can Set led color and beeper count
#define PCSWIPE_STATE_BAD       3   // Can Set led color and beeper count
#define PCSWIPE_STATE_SENDING   4   // Can Set led color
//
// pcSwipe Led Colors
//
#define LED_COLOR_OFF           0
#define LED_COLOR_RED           1
#define LED_COLOR_GREEN         2
#define LED_COLOR_AMBER         3
//
/////////////////////////////
#define    SWIPELIBERROR_CONNECT        0x00010000
#define    SWIPELIBERROR_READCFG        0x00020000
#define    SWIPELIBERROR_READCFG2       0x00020001
#define    SWIPELIBERROR_WRITECFG       0x00040000
#define    SWIPELIBERROR_WRITECFG2      0x00040001
#define    SWIPELIBERROR_FACTORY        0x00080000
#define    SWIPELIBERROR_GETID          0x01000000

// Key Modifiers byte pcProx,pcSwipe,pcProx Sonar
#define L_CTRL              0x01
#define L_SHIFT             0x02
#define L_ALT               0x04
#define L_GUI               0x08
#define R_CTRL              0x10
#define R_SHIFT             0x20
#define R_ALT               0x40
#define R_GUI               0x80

// pcProxPlus Card Types see function SetCardTypePriority();
//-----------------------------------------------------------------------------
// Low Freq 125Khz Card List Card Types Are 8000 thru FFFF Hex
// High Freq 13.56Mhz Card List Card Types Are 0000 thru 7FFF Hex
// Third nybble is for display only, so ABCD, AB1D and AB0D are equal.
// Last timestamp from cardtypes file: 211001
//-----------------------------------------------------------------------------
#define CARDTYPE_AWID_RDR698X_COMPATIBLE                   0xFA02  //; 125 KHZ FSK
#define CARDTYPE_ALLEGION_HCE                              0x0206  //; 13.56 MHZ
#define CARDTYPE_BLE_BEACON                                0x8201  //; BLUETOOTH
#define CARDTYPE_CARDAX_UID_RDR6C8X_COMPATIBLE             0xF602  //; 125 KHZ ASK
#define CARDTYPE_CASIRUSCO_GE_SECURITY_UTC_RDR628X_COMPATIBLE 0xFC02  //; 125 KHZ ASK
#define CARDTYPE_CDVI                                      0xE912  //; 125 KHZ
#define CARDTYPE_CEPAS									   0x7A01  //; 13.56 MHZ
#define CARDTYPE_COTAG									   0xDF01  //; 133 KHZ
#define CARDTYPE_DEISTER_UID                               0xF401  //; 125 KHZ ASK
#define CARDTYPE_DESFIRE_CSN_OYSTER_NFC4                   0x7F01  //; 13.56 MHZ
#define CARDTYPE_DIGITAG                                   0xFB11  //; 125 KHZ ASK
#define CARDTYPE_DIMPNA_UID                                0xF101  //; 125 KHZ ASK
#define CARDTYPE_ECP_2                                     0x030D  //; 13.56 MHz
#define CARDTYPE_EM_410X_RDR6E8X_COMPATIBLE                0xFB01  //; 125 KHZ ASK
#define CARDTYPE_EM_410X_ALTERNATE                         0xFB02  //; 125 KHZ ASK
#define CARDTYPE_EM_410X_WITHCHECKSUM                      0xFB03  //; 125 KHZ FSK
#define CARDTYPE_ETAG_CSN_SECURA_KEY                       0x7E31  //; 13.56 MHZ
#define CARDTYPE_FARPOINTE_DATA_PYRAMID_PSC1_26_BIT_RDR647X_COMPATIBLE 0xEA22  //; 125 KHZ FSL
#define CARDTYPE_FARPOINTE_DATA_PYRAMID_UID                0xEA21  //; 125 KHZ FSL
#define CARDTYPE_FELICA_NFC3							   0x7C02  //; 13.56 MHZ
#define CARDTYPE_FELICA_DATA_STANDARD					   0x0701  //; 13.56 MHZ
#define CARDTYPE_FELICA_DATA_Lite						   0x0702  //; 13.56 MHZ
#define CARDTYPE_FIDO2_U2F								   0x6F00  //; 13.56 MHz
#define CARDTYPE_gematik_ICCSN							   0x0401  //; 13.56 MHz 
#define CARDTYPE_GPROXII_UID_RDR6G8X_COMPATIBLE            0xF502  //; 125 KHZ ASK
#define CARDTYPE_GPROX_II_ID 							   0xF504  //; 125 kHz ASK
#define CARDTYPE_HID_ICLASS_CSN                            0x7D01  //; 13.56 MHZ
#define CARDTYPE_HID_ICLASS_ID_ICLASS_SE				   0x0100 //; 13.56 MHZ
#define CARDTYPE_HID_MOBILE_ACCESS						   0x8501 //; BLUETOOTH
#define CARDTYPE_HID_SEOS								   0x0101 //; 13.56 MHZ
#define CARDTYPE_HID_APPLE_WALLET                          0x0102  //; 13.56 MHZ
#define CARDTYPE_HID_PROX_RDR608X_COMPATIBLE               0xEF04  //; 125 KHZ FSK H10301
#define CARDTYPE_HID_PROX_UID							   0xEF01 //; 125 kHz FSK
#define CARDTYPE_HITAG_1_AND_S_RDR6H8X_COMPATIBLE          0xF302  //; 125 KHZ ASK
#define CARDTYPE_HITAG_1_AND_S_ALTERNATE                   0xF304  //; 125 KHZ ASK
#define CARDTYPE_HITAG_2_RDR6H8X_COMPATIBLE                0xF201  //; 125 KHZ ASK
#define CARDTYPE_HITAG_2_ALTERNATE                         0xF204  //; 125 KHZ ASK
#define CARDTYPE_HITAG_2_ATS8600                           0xF205  //; 125 KHZ ASK
#define CARDTYPE_HITAG_2_DOWNLOADER                        0xF206  //; 125 KHZ ASK
#define CARDTYPE_ICODE_CSN_PHILIPS_NXP                     0x7E01  //; 13.56 MHZ
#define CARDTYPE_ITAG_CSN_IBM                              0x7F11  //; 13.56 MHZ
#define CARDTYPE_ID_TECK_RDR_6A8x_COMPATIBLE	 		   0xE401 //; 125 KHZ
#define CARDTYPE_ID_TECK_ALTERNATE	 					   0xE402 //; 125 KHZ
#define CARDTYPE_ID_TECK_LEGACY_PLUS					   0xE403 //; 125 kHz
#define CARDTYPE_INDALA_ASP_26_BIT_MOTOROLA_RDR638X_COMPATIBLE 0xFD02  //; 125 KHZ PSK
#define CARDTYPE_INDALA_ASP_UID_MOTOROLA                   0xFD01  //; 125 KHZ PSK
#define CARDTYPE_INDALA_ASP_CUSTOM                         0xFD04  //; 125 KHZ PSK _PLEASE_CONTACT_RF_IDEAS_SALES
#define CARDTYPE_INDALA_ASPPLUS_UID_MOTOROLA               0xED02  //; 125 KHZ PSK
#define CARDTYPE_INDALA_ASPPLUS_CUSTOM                     0xED05  //; 125 KHZ PSK _PLEASE_CONTACT_RF_IDEAS_SALES
#define CARDTYPE_INDALA_ECR_CUSTOM                         0xFD05  //; 125 KHZ PSK _PLEASE_CONTACT_RF_IDEAS_SALES
#define CARDTYPE_IOPROX_KANTECH_RDR678X_COMPATIBLE         0xF902  //; 125 KHZ FSK
#define CARDTYPE_ISO_14443A_CSN                            0x7F21  //; 13.56 MHZ
#define CARDTYPE_ISO_14443B_CSN 	 					   0x7B01 //; 13.56 MHZ
#define CARDTYPE_ISO_14443B_SRX_CSN                        0x7B02 //; 13.56 MHz
#define CARDTYPE_ISO_15693_CSN                             0x7E21 //; 13.56 MHZ
#define CARDTYPE_Isonas 	 							   0xFA03 //; 125 kHz FSK
#define CARDTYPE_FICAM                                     0x7701 //; 13.56 MHz
#define CARDTYPE_KERI_NXT_UID                              0xEA01  //; 125 KHZ FSL
#define CARDTYPE_KERI_PSC1_26_BIT                          0xEA02  //; 125 KHZ FSL
#define CARDTYPE_KERI_UID_RDR6K8X_COMPATIBLE               0xF802  //; 125 KHZ FSL
#define CARDTYPE_LEGIC_ADVANT_14443A_CSN                   0x7F31  //; 13.56 MHZ
#define CARDTYPE_LEGIC_ADVANT_15693_CSN                    0x7E51  //; 13.56 MHZ
#define CARDTYPE_LEGIC_STANDARD_PRIME_CSN                  0x7901  //; 13.56 MHZ
#define CARDTYPE_LEGIC_STAMP                               0x7803  //; 13.56 MHZ
#define CARDTYPE_LEGIC_NEON                                0x0801  //; 13.56 MHZ
#define CARDTYPE_LEGIC_NEON_BLE                            0x0802  //; Bluetooth
#define CARDTYPE_LF_COMPOSITE_AWID_CASI_RUSCO_EM410x_INDALA_ASP     0xDE01 //; 125 kHz FSK 
#define CARDTYPE_MIFARE_2GO                                0x030E  //; 13.56 MHz
#define CARDTYPE_MIFARE_CSN_PHILIPS_NXP                    0x7F41  //; 13.56 MHZ
#define CARDTYPE_MiFARE_CLASSIC_SECURE_FILE_DATA           0x0301  //; 13.56 MHZ
#define CARDTYPE_MIFARE_DESFIRE_SECURE_FILE_DATA 	 	   0x0201  //; 13.56 MHZ
#define CARDTYPE_MIFARE_DESFIRE_CSN_SECURE_FILE   	 	   0x0202  //; 13.56 MHZ
#define CARDTYPE_MIFARE_PLUS_SECURE_FILE_DATA			   0x0601  //; 13.56 MHZ
#define CARDTYPE_MIFARE_ULTRALIGHT_SECURE_FILE_DATA        0x0501  //; 13.56 MHZ
#define CARDTYPE_MIFARE_ULTRALIGHT_CSN_PHILIPS_NXP         0x7F51  //; 13.56 MHZ
#define CARDTYPE_MIFARE_32_DOWNLOADER                      0x7F06  //; 13.56 MHz
#define CARDTYPE_MIFARE_48_DOWNLOADER                      0x7F07  //; 13.56 MHz
#define CARDTYPE_MIFARE_56_DOWNLOADER                      0x7F08  //; 13.56 MHz
#define CARDTYPE_MIFARE_32_ATS8600                         0x7F09  //; 13.56 MHz
#define CARDTYPE_MIFARE_48_ATS8600						   0x7F0A  //; 13.56 MHz
#define CARDTYPE_MIFARE_56_ATS8600						   0x7F0B  //; 13.56 MHz
#define CARDTYPE_MOBILE_CREDENTIAL                         0x8101  //; BLUETOOTH
#define CARDTYPE_MYD_CSN_INFINEON                          0x7E11  //; 13.56 MHZ
#define CARDTYPE_NEDAP   	 							   0xE301 //; 125 kHz FSK
#define CARDTYPE_NEXKEY_QUADRAKEY_KEYMATE_2SMART_KEY_HONEYWELL 0xF722  //; 125 KHZ PSK
#define CARDTYPE_NEXWATCH_HONEYWELL_RDR_6N8X_COMPATIBLE    0xF712  //; 125 KHZ PSK
#define CARDTYPE_ORANGE_PACK_ID                            0x8001  //; 13.56 MHz
#define CARDTYPE_PARADOX                                   0xE902  //; 125 kHz FSK
#define CARDTYPE_PIV_UNIQUE_CARD_HOLDER_INDENTIFIER        0x7702  //; 13.56 MHz
#define CARDTYPE_POSTECH   	 							   0xE922 //; 125 kHz FSK
#define CARDTYPE_PSIA_PKOC_BLE                             0x8601  //; Bluetooth
#define CARDTYPE_PYRAMID_FARPOINTE_DATA_PSC1_26_BIT        0xEA12  //; 125 KHZ FSK
#define CARDTYPE_PYRAMID_FARPOINTE_DATA_UID                0xEA11  //; 125 KHZ FSK
#define CARDTYPE_RADIO_KEY_SECURA_KEY_02_RKCX02_RDR6Z8X_COMPATIBLE 0xEB02  //; 125 KHZ ASK
#define CARDTYPE_RDR758X_EQUIVALENT                        0x6F01  //; 13.56 MHZ
#define CARDTYPE_RDR758X_EQUIVALENT_NTWARE                 0x6F01  //; 13.56 MHZ
#define CARDTYPE_RDR_758x_EXPANDED_CSN  	 		       0x6F02  //; 13.56 MHZ
#define CARDTYPE_RDR_758x_EXPANDED_FOR_LEGIC               0x6F03  //; 13.56 MHZ
#define CARDTYPE_READYKEY_PRO_UID_RDR6R8X_COMPATIBLE       0xF004  //; 125 KHZ ASK
#define CARDTYPE_ROSSLARE                                  0xFB21  //; 125 KHZ ASK
#define CARDTYPE_RUSSWIN_UID                               0xF612  //; 125 KHZ ASK
#define CARDTYPE_SAFETRUST                                 0X8401  //; BLUETOOTH
#define CARDTYPE_SECURA_KEY_01_RKCX01                      0xEC01  //; 125 KHZ ASK
#define CARDTYPE_TAGIT_CSN_TEXAS_INSTRUMENTS               0x7E41  //; 13.56 MHZ
#define CARDTYPE_TCOS						               0x7801  //; 13.56 MHZ
#define CARDTYPE_TOPAZ_CSN_NFC1  	 					   0x7F02  //; 13.56 MHZ
#define CARDTYPE_URMET        	 						   0xFC03  //; 125 kHz ASK
#define CARDTYPE_WAVELYNX_MY_PASS_ID_BLE                    0x8701  //; Bluetooth
#define CARDTYPE_WAVELYNX_NFC_WALLET_LEAF                  0x030F  //; 13.56 MHz
#define CARDTYPE_WAVELYNX_NFC_WALLET                       0x0309  //; 13.56 MHz
#define CARDTYPE_OFF                                       0x0000  //; ZERO IS NEVER USED AS A CARD TYPE


#endif

// EOF
