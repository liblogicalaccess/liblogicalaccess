#include <logicalaccess/plugins/readers/iso7816/commands/nedapmobileaccessiso7816commands.hpp>

#include <cstddef>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
namespace
{
constexpr size_t MAX_SHORT_APDU_DATA_LENGTH = 0xff;
}

NedapMobileAccessISO7816Commands::NedapMobileAccessISO7816Commands()
{
    setCmdType(CMD_NEDAP_MOBILE_ACCESS_ISO7816);
}

std::shared_ptr<ISO7816Commands>
NedapMobileAccessISO7816Commands::getISO7816Commands() const
{
    auto commands = std::make_shared<ISO7816ISO7816Commands>();
    commands->setChip(getChip());
    commands->setReaderCardAdapter(getReaderCardAdapter());
    return commands;
}

ByteVector
NedapMobileAccessISO7816Commands::selectApplication(const ByteVector &dfName)
{
    EXCEPTION_ASSERT_WITH_LOG(!dfName.empty() &&
                                  dfName.size() <= MAX_SHORT_APDU_DATA_LENGTH,
                              LibLogicalAccessException,
                              "Invalid Nedap Mobile Access application name length.");

    auto commands =
        std::dynamic_pointer_cast<ISO7816ISO7816Commands>(getISO7816Commands());
    EXCEPTION_ASSERT_WITH_LOG(commands, LibLogicalAccessException,
                              "Generic ISO7816-4 commands are unavailable.");

    auto response =
        commands->selectFileAndGetResponse(P1_SELECT_BY_DFNAME, P2_RETURN_FCI, dfName);
    clearAuthenticationState();
    return response;
}

ISO7816Response NedapMobileAccessISO7816Commands::sendNativeCommand(
    unsigned char instruction, const ByteVector &data) const
{
    EXCEPTION_ASSERT_WITH_LOG(data.size() <= MAX_SHORT_APDU_DATA_LENGTH,
                              LibLogicalAccessException,
                              "Nedap Mobile Access command data exceeds short APDU limit.");

    auto adapter =
        std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(getReaderCardAdapter());
    EXCEPTION_ASSERT_WITH_LOG(adapter, LibLogicalAccessException,
                              "Nedap Mobile Access requires an ISO7816 reader adapter.");

    return adapter->sendAPDUCommand(
        0x90, instruction, 0x00, 0x00, static_cast<unsigned char>(data.size()), data,
        0x00);
}
}
