/**
 * \file keyboardreaderprovider.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Keyboard card reader provider.
 */

#ifndef LOGICALACCESS_READERKEYBOARD_PROVIDER_HPP
#define LOGICALACCESS_READERKEYBOARD_PROVIDER_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/keyboard/keyboardreaderunit.hpp>

#include <string>
#include <vector>
#include <map>

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/readers/keyboard/lla_readers_private_keyboard_api.hpp>

namespace logicalaccess
{
#define READER_KEYBOARD "Keyboard"

#ifdef _WINDOWS
#define SessionHookMap std::map<DWORD, DWORD>

DWORD WINAPI WatchThread(LPVOID lpThreadParameter);
#endif

/**
 * \brief Keyboard Reader Provider class.
 */
class LLA_READERS_PRIVATE_KEYBOARD_API KeyboardReaderProvider : public ReaderProvider
{
  protected:
    /**
     * \brief Constructor.
     */
    KeyboardReaderProvider();

  public:
    /**
     * \brief Destructor.
     */
    ~KeyboardReaderProvider();

    /**
     * \brief Release the provider resources.
     */
    void release() override;

    /**
     * \brief Get the keyboard reader provider instance.
     * \return The Keyboard reader provider instance.
     */
    static std::shared_ptr<KeyboardReaderProvider> getSingletonInstance();

    /**
     * \brief Get the reader provider type.
     * \return The reader provider type.
     */
    std::string getRPType() const override
    {
        return READER_KEYBOARD;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "Keyboard";
    }

    /**
     * \brief List all readers of the system.
     * \return True if the list was updated, false otherwise.
     */
    bool refreshReaderList() override;

    /**
     * \brief Get reader list for this reader provider.
     * \return The reader list.
     */
    const ReaderList &getReaderList() override
    {
        return d_readers;
    }

    /**
     * \brief Create a new reader unit for the reader provider.
     * \return A reader unit.
     */
    std::shared_ptr<ReaderUnit> createReaderUnit() override;

#ifdef _WINDOWS
    DWORD launchHook(HANDLE hUserTokenDup = nullptr) const;

    DWORD launchHookIntoDifferentSession(DWORD destSessionId) const;

  protected:
    void generateSharedGuid();

    long createKbdFileMapping();

    void freeKbdFileMapping();

    static void fillSecurityDescriptor(LPSECURITY_ATTRIBUTES sa, PACL pACL);

    long createKbdEvent();

    void freeKbdEvent();

    void startAndWatchOnActiveConsole();

    void stopWatchingActiveConsole();

    static HANDLE
    retrieveWinlogonUserToken(const DWORD destSessionId,
                              const SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
                              const TOKEN_TYPE TokenType);

    static std::string getHookPath();

    std::string getHookArguments() const;

    static bool is64BitWindows();

    static bool terminateProcess(DWORD dwProcessId, UINT uExitCode);
#endif
  protected:
    /**
 * \brief The reader list.
 */
    ReaderList d_readers;

#ifdef _WINDOWS
    HANDLE shKeyboard;

    HANDLE hWatchThrd;

    std::string sharedGuid;

  public:
    KeyboardSharedStruct *sKeyboard;

    HANDLE hKbdEvent;

    HANDLE hKbdEventProcessed;

    HANDLE hHostEvent;

    HANDLE hStillAliveEvent;

    SessionHookMap processHookedSessions;

    bool watchSessions;
#endif
};
}

#endif /* LOGICALACCESS_READERKEYBOARD_PROVIDER_HPP */