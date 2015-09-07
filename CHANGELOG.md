LibLogicalAccess ChangeLog
==========================

1.78.0 | 06/12/2015
-------------------
New Features:
 - String Data Field with charset support
 - Omnikey 5x27 reader support
 - Clang compiler support
 - MAC OSX operating system support
 - StdErr logging
 
Code Improvement:
 - Better generic command result checker
 - Unit tests (/tests)
 
Fixes:
 - Fix custom format Parity Field bits position
 - Fix crash when card plugin for inserted card cannot be found
 - Fix DESFire CMAC communication with large buffer
 - Fix DESFire card storage service on plain and free files access

1.77.3 | 01/21/2015
-------------------
Fixes:
 - Fix TCP Network Issue when send function failed
 
1.77.2 | 01/09/2015
-------------------
Fixes:
 - Fix SAM ISOAuthentication
 
1.77.1 | 12/11/2014
-------------------
New Features:
 - Add SpringCard error code checker
 - Handle Mifare Ultralight C card tracking on SpringCard reader
 - Add Omnitech Key Diversification algorithm
 - Add OMNIKEY 5025 CL Reader Prox compatibility
 - Add Mifare Ultralight-C Generic Mode wrapping command for Omnikey XX21

Code Improvement:
 - Moving read / write function to std::vector buffer
 - Delay load of OpenSSL for Windows (Loaded only when needed)
 - Switch boost::shared_ptr to std::shared_ptr
 - Improve TCP implementation with Boost Asio
 - Add coverity (Static Analysis Tools)
 - Code review => 76,529 Lines of Code with 0.00 Defect Density !

Fixes:
 - Fix a lot of leaks
 - Fix Security issue - Wrong use of OpenSSL
 - Fix Omnikey Ultralight C authentication
 - Fix PC/SC Mifare Ultralight C authentication algo
 - Fix deister waitRemoval
 - Fix DESFire - getApplicationIDs

Dependencies Changes:
 - Remove zlib dependency
 - Update boost to 1.57
 - Move to VS2013 and gcc-4.8

1.76.2 | 09/16/2014
-------------------
 - Add NFC Tags Services on ISO7816 cards & DESFireEV1
 - SerialPort code has been fully rewrited for fixing a problem of speed
 - Fix: Check CardsFormatComposite contains a card type on ReaderUnit
 - Fix: SCIEL reader chip list

1.75.0 | 06/13/2014
-------------------
 - Add NXP AV1 diversification
 - Add NXP AV2 features
 - Add Linux ARM build support
 - Add resource information on Windows
 - Improve CMake scripts (thanks xantares)
 - Improve error code handler
 - Improve serial port readers communication
 - Fix Rpleth stability
 - Fix DESFire write data with length > 24 bytes
 - Fix memory leak on ASCII data field
 - Fix DESFire AES PICC Master Key change
 - Fix ISO Generic Tag type detection
 - Fix code style errors
 - Move old plugins code to liblogicalaccess-oldy repository

1.74.0 | 01/24/2014
-------------------
 - Fix DESFire DES key authenticate
 - Fix DESFire PICC authentication failure behavior when using SAM
 - Fix several Rpleth instabilities
 - Fix key diversification serialization
 - Fix RFIDeas SetConnectProdut / SetDevTypeSrch call
 - Fix MifareCommands ReadSector on large sector (COM layer)
 - Fix DataTransport SendCommands (COM layer)
 - Fix MifareUltraLightCommands ReadPages (COM layer)
 - Fix getter on TagIt, Twic and SAM commands (COM layer)
 - Add method to get Mifare SAB byte array (COM layer)

1.73.0 | 12/18/2013
-------------------
 - Add waitForReaders function to wait for a reader insertion
 - Improve TCP and Rpleth communication
 - Fix Mifare ReadSector command on COM layer
 - Fix some HID iClass page size (0xFE ? 0xFF)
 - Update VC++ runtime dependency to Visual C++ Runtime 2012 Update 4

1.70.0 | 10/31/2013
-------------------
 - Add readBinary/updateBinary on COM layer Mifare Commands
 - Add Omnikey 5121 LAN reader
 - Read config file from /etc/liblogicalaccess.config by default on Linux
 - Fix DESFire AuthenticateKey function in COM layer
 - Fix SAM changeKeyPICC
 - Fix Binary Data Field unserialization

1.69.0 | 10/16/2013
-------------------
 - Add key diversification abstraction and NXP support
 - Add possibility to read trailer block on ReadSector Mifare command
 - Add more exceptions and logs
 - Review PC/SC waitInsertion
 - Support Travis automatic builds
 - Fix GetApplicationIDs and GetFileIDs on COM layer DESFire commands
 - Fix ACR122U reader connect() function
 - Fix DESFire ChangeKeyPICC with SAM
 - Fix Binary and Parity custom fields

1.68.0 | 09/05/2013
-------------------
 - Add Release method on Reader Provider
 - Fix getter/setter on SAM SET struct
 - Fix CardsFormatComposite, Mifare Classic and Mifare Plus serialization/deserialization
 - Fix Read Format on access control service

1.67.0 | 08/26/2013
-------------------
 - Fix LED / PAD getters on all reader units (COM layer)
 - Fix Custom access control format (COM layer)
 - Fix DESFireEV1Commands getter on DESFireEV1Chip
 - Fix Keyboard reader at 32-bit runtime on 64-bit OS and add some logs
 - Add SAM AV1 support
 - Separate Rpleth and OK5553 readers

1.66.0 | 07/23/2013
-------------------
 - CardProvider layer removed from cards
 - New DataTransport layer on readers
 - New reader: Rpleth
 - Fix several readers communication

1.65.0 | 06/13/2013
-------------------
 - Initial open source version with major architectural changes