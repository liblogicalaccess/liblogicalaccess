/**
 * \file keyboardreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Keyboard card reader provider.
 */

#ifndef LOGICALACCESS_READERKEYBOARD_PROVIDER_HPP
#define LOGICALACCESS_READERKEYBOARD_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "keyboardreaderunit.hpp"

#include <string>
#include <vector>
#include <map>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
#define READER_KEYBOARD		"Keyboard"

#ifdef _WINDOWS
#define SessionHookMap std::map<DWORD, DWORD>

    DWORD WINAPI WatchThread(LPVOID lpThreadParameter);
#endif

    /**
     * \brief Keyboard Reader Provider class.
     */
    class LIBLOGICALACCESS_API KeyboardReaderProvider : public ReaderProvider
    {
    public:

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
        virtual void release();

        /**
         * \brief Get the keyboard reader provider instance.
         * \return The Keyboard reader provider instance.
         */
        static boost::shared_ptr<KeyboardReaderProvider> getSingletonInstance();

        /**
         * \brief Get the reader provider type.
         * \return The reader provider type.
         */
        virtual std::string getRPType() const { return READER_KEYBOARD; };

        /**
         * \brief Get the reader provider name.
         * \return The reader provider name.
         */
        virtual std::string getRPName() const { return "Keyboard"; };

        /**
         * \brief List all readers of the system.
         * \return True if the list was updated, false otherwise.
         */
        virtual bool refreshReaderList();

        /**
         * \brief Get reader list for this reader provider.
         * \return The reader list.
         */
        virtual const ReaderList& getReaderList() { return d_readers; };

        /**
         * \brief Create a new reader unit for the reader provider.
         * \return A reader unit.
         */
        virtual boost::shared_ptr<ReaderUnit> createReaderUnit();

#ifdef _WINDOWS
        DWORD launchHook(HANDLE hUserTokenDup = NULL);

        DWORD launchHookIntoDifferentSession(DWORD destSessionId);

    protected:

        void generateSharedGuid();

        long createKbdFileMapping();

        void freeKbdFileMapping();

        void fillSecurityDescriptor(LPSECURITY_ATTRIBUTES sa, PACL pACL);

        long createKbdEvent();

        void freeKbdEvent();

        void startAndWatchOnActiveConsole();

        void stopWatchingActiveConsole();

        HANDLE retrieveWinlogonUserToken(const DWORD destSessionId, const SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, const TOKEN_TYPE TokenType);

        std::string getHookPath() const;

        std::string getHookArguments() const;

        bool is64BitWindows() const;

        bool terminateProcess(DWORD dwProcessId, UINT uExitCode);
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

        KeyboardSharedStruct* sKeyboard;

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