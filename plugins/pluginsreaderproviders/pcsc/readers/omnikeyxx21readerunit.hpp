/**
 * \file omnikeyxx21readerunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX21 Reader unit.
 */

#ifndef LOGICALACCESS_OMNIKEYXX21READERUNIT_HPP
#define LOGICALACCESS_OMNIKEYXX21READERUNIT_HPP

#include "omnikeyreaderunit.hpp"
#include "omnikeyxx21readerunitconfiguration.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Omnikey XX21 reader unit class.
     */
    class LIBLOGICALACCESS_API OmnikeyXX21ReaderUnit : public OmnikeyReaderUnit
    {
    public:

        /**
         * This represents the status of the secure mode.
         * This struct is implicitly castable to bool: it will be true when secure
         * mode is not in DISABLED state.
         */
        struct SecureModeStatus
        {
            SecureModeStatus(int st)
            {
                assert(st == READ || st == WRITE || st == DISABLED);
                value_ = st;
            }

            enum
            {
                READ,
                WRITE,
                DISABLED
            };

            operator bool() const
            {
                return value_ != DISABLED;
            }

            operator int() const
            {
                return value_;
            }

			LIBLOGICALACCESS_API friend std::ostream &operator<<(std::ostream &os, const SecureModeStatus &s);
        private:
            int value_;

        };
        /**
         * \brief Constructor.
         */
        OmnikeyXX21ReaderUnit(const std::string& name);

        /**
         * \brief Destructor.
         */
        virtual ~OmnikeyXX21ReaderUnit();

        /**
         * Retrieve the current status of the secure mode.
         */
        SecureModeStatus getSecureConnectionStatus();

        /**
         * Set the current status of the secure mode.
         */
        void setSecureConnectionStatus(SecureModeStatus st);

        /**
         * Set the current status of the secure mode.
         * Integer overload for less typing. Assert on invalid value.
         */
        void setSecureConnectionStatus(int v);


        /**
         * \brief Get the PC/SC reader unit type.
         * \return The PC/SC reader unit type.
         */
        virtual PCSCReaderUnitType getPCSCType() const;

        /**
         * \brief Wait for a card removal.
         * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
         * \return True if a card was removed, false otherwise. If a card was removed, the name of the reader on which the removal was detected is accessible with getReader().
         */
        virtual bool waitRemoval(unsigned int maxwait);

        /**
         * \brief Get the PC/SC reader/card adapter for a card type.
         * \param type The card type.
         * \return The PC/SC reader/card adapter.
         */
        virtual std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter(std::string type);

        /**
         * \brief Get the Omnikey XX21 reader unit configuration.
         * \return The Omnikey XX21 reader unit configuration.
         */
        std::shared_ptr<OmnikeyXX21ReaderUnitConfiguration> getOmnikeyXX21Configuration() { return std::dynamic_pointer_cast<OmnikeyXX21ReaderUnitConfiguration>(getPCSCConfiguration()); };

        /**
         * \brief Change a key in reader memory.
         * \param keystorage The key storage information.
         * \param key The key value.
         */
        virtual void changeReaderKey(std::shared_ptr<ReaderMemoryKeyStorage> keystorage, const std::vector<unsigned char>& key);

        /**
         * \brief Get if the inserted card is from ISO 14443-4 A or B.
         * \param isTypeA Will be set to true if the card is from ISO 14443-4 A, false if not or unknown.
         * \param isTypeB Will be set to true if the card is from ISO 14443-4 B, false if not or unknown.
         */
        virtual void getT_CL_ISOType(bool& isTypeA, bool& isTypeB);

		/**
		* Request that the reader enable or disable the various card technologies
		* as described in the bitset.
		* The default implementation just does nothing.
		*/
		virtual void setCardTechnologies(const TechnoBitset& bitset);

		/**
		* Return a bitset describing which cards technology are enabled.
		*
		* The default implementation return a bitset with all flags set to false.
		*/
		virtual TechnoBitset getCardTechnologies();

    protected:
         /**
         * \brief Is secure connection mode ?
         * \remarks We must store it in static memory because the connection mode is global for all connection to the reader
         */
        static std::map<std::string, SecureModeStatus> secure_connection_status_;

    private:
        /**
         * Search a line matching the configuration for a given technology.
         *
         * @param lines is a vector of all lines in /etc/omnikey.ini
         * @param techno the technology string we are looking for.
         *
         * @return true if the techno is found and enabled, false otherwise.
         */
        bool fetchCardTechoLinux(const std::vector<std::string> &lines, const std::string &techno);

        /**
         * Write the updated configuration for enabled technology on disk.
         */
        bool replaceCardTechnoLinux(std::vector<std::string> &lines,
                                    const std::map<std::string, bool> technos);
    };
}

#endif