#pragma once

#include <iosfwd>
#include <vector>
#include <cstdint>
#include <memory>
#include "logicalaccess/msliblogicalaccess.h"

LIBLOGICALACCESS_API std::ostream &operator<<(std::ostream &o, std::vector<uint8_t> &v);


namespace logicalaccess
{
    class ReaderUnit;
    class PCSCReaderUnit;
    class Chip;
    class ReaderProvider;
}

using ReaderUnitPtr     = std::shared_ptr<logicalaccess::ReaderUnit>;
using ChipPtr           = std::shared_ptr<logicalaccess::Chip>;
using ReaderProviderPtr = std::shared_ptr<logicalaccess::ReaderProvider>;
using PCSCReaderUnitPtr = std::shared_ptr<logicalaccess::PCSCReaderUnit>;

/**
 * PCSC tests initialization routine.
 *
 * This function handles the calls necessary to create a reader unit
 * and get a chip from it. This code is re-usable for PCSC tests.
 */
LIBLOGICALACCESS_API std::tuple<ReaderProviderPtr, ReaderUnitPtr, ChipPtr> pcsc_test_init();

/**
 * Take care of disconnecting the reader and the card.
 */
LIBLOGICALACCESS_API void pcsc_test_shutdown(ReaderUnitPtr);

/**
 * Honor the environment variable LLA_TEST_WAIT and wait for a given duration before
 * starting.
 *
 * This helps when running the tests with CTest, as it gives the user some time to remove
 * the card from the previous test.
 */
LIBLOGICALACCESS_API void prologue();

/**
 * Use compile time ifdef to return the operating system name.
 */
LIBLOGICALACCESS_API std::string get_os_name();

/**
 * Returns a "short name" (OK5321, OK5327) for PCSC reader.
 */
LIBLOGICALACCESS_API std::string pcsc_reader_unit_name(ReaderUnitPtr ru);
