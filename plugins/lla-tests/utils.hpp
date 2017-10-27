#pragma once

#include <iosfwd>
#include <vector>
#include <cstdint>
#include <memory>
#include "logicalaccess/liblogicalaccess_export.hpp"
#include "logicalaccess/lla_fwd.hpp"

namespace detail
{
extern LIBLOGICALACCESS_API bool prologue_has_run;
enum ReaderType
{
    PCSC,
    NFC
};
extern LIBLOGICALACCESS_API enum ReaderType reader_type;
}

/**
 * PCSC tests initialization routine.
 *
 * This function handles the calls necessary to create a reader unit
 * and get a chip from it. This code is re-usable for PCSC tests.
 */
LIBLOGICALACCESS_API
std::tuple<logicalaccess::ReaderProviderPtr, logicalaccess::ReaderUnitPtr,
           logicalaccess::ChipPtr>
pcsc_test_init(const std::string &card_type = "");

/**
 * NFC tests initialization routine.
 */
LIBLOGICALACCESS_API std::tuple<logicalaccess::ReaderProviderPtr,
                                logicalaccess::ReaderUnitPtr, logicalaccess::ChipPtr>
nfc_test_init();

/**
 * Generic test initialization routine.
 *
 * This function will use the static variable set by prologue() to chose between
 * NFC or PCSC reader unit type.
 *
 * It defaults to PCSC.
 */
LIBLOGICALACCESS_API std::tuple<logicalaccess::ReaderProviderPtr,
                                logicalaccess::ReaderUnitPtr, logicalaccess::ChipPtr>
lla_test_init(const std::string &card_type = "");

/**
 * Take care of disconnecting the reader and the card.
 */
LIBLOGICALACCESS_API void pcsc_test_shutdown(logicalaccess::ReaderUnitPtr);

/**
 * Honor the environment variable LLA_TEST_WAIT and wait for a given duration before
 * starting.
 *
 * It will parse the program's arguments to determine what kind of reader unit
 * shall be created.
 */
LIBLOGICALACCESS_API void prologue(int ac, char **av);

/**
 * Use compile time ifdef to return the operating system name.
 */
LIBLOGICALACCESS_API std::string get_os_name();

/**
 * Returns a "short name" (OK5321, OK5327) for PCSC reader.
 */
LIBLOGICALACCESS_API std::string pcsc_reader_unit_name(logicalaccess::ReaderUnitPtr ru);
