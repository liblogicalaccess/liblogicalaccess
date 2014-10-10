/**
 * \file axesstmc13readerunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC13 Reader unit.
 */

#ifndef LOGICALACCESS_AXESSTMC13READERUNIT_HPP
#define LOGICALACCESS_AXESSTMC13READERUNIT_HPP

#include "logicalaccess/readerproviders/readerunit.hpp"
#include "logicalaccess/readerproviders/serialportxml.hpp"
#include "axesstmc13readerunitconfiguration.hpp"

namespace logicalaccess
{
    class Profile;
    class AxessTMC13ReaderCardAdapter;
    class AxessTMC13ReaderProvider;

    /**
     * \brief The AxessTMC13 reader unit class.
     */
    class LIBLOGICALACCESS_API AxessTMC13ReaderUnit : public ReaderUnit
    {
    public:

        /**
         * \brief Constructor.
         */
        AxessTMC13ReaderUnit();

        /**
         * \brief Destructor.
         */
        virtual ~AxessTMC13ReaderUnit();

        /**
         * \brief Get the reader unit name.
         * \return The reader unit name.
         */
        virtual std::string getName() const;

        /**
         * \brief Get the connected reader unit name.
         * \return The connected reader unit name.
         */
        virtual std::string getConnectedName();

        /**
         * \brief Set the card type.
         * \param cardType The card type.
         */
        virtual void setCardType(std::string cardType);

        /**
         * \brief Wait for a card insertion.
         * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
         * \return True if a card was inserted, false otherwise. If a card was inserted, the name of the reader on which the insertion was detected is accessible with getReader().
         * \warning If the card is already connected, then the method always fail.
         */
        virtual bool waitInsertion(unsigned int maxwait);

        /**
         * \brief Wait for a card removal.
         * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
         * \return True if a card was removed, false otherwise. If a card was removed, the name of the reader on which the removal was detected is accessible with getReader().
         */
        virtual bool waitRemoval(unsigned int maxwait);

        /**
         * \brief Create the chip object from card type.
         * \param type The card type.
         * \return The chip.
         */
        virtual boost::shared_ptr<Chip> createChip(std::string type);

        /**
         * \brief Get the first and/or most accurate chip found.
         * \return The single chip.
         */
        virtual boost::shared_ptr<Chip> getSingleChip();

        /**
         * \brief Get chip available in the RFID rang.
         * \return The chip list.
         */
        virtual std::vector<boost::shared_ptr<Chip> > getChipList();

        /**
         * \brief Get the reader ping command.
         * \return The ping command.
         */
        virtual std::vector<unsigned char> getPingCommand() const;

        /**
         * \brief Get the current chip in air.
         * \return The chip in air.
         */
        boost::shared_ptr<Chip> getChipInAir();

        /**
         * \brief Get the default AxessTMC13 reader/card adapter.
         * \return The default AxessTMC13 reader/card adapter.
         */
        virtual boost::shared_ptr<AxessTMC13ReaderCardAdapter> getDefaultAxessTMC13ReaderCardAdapter();

        /**
         * \brief Connect to the card.
         * \return True if the card was connected without error, false otherwise.
         *
         * If the card handle was already connected, connect() first call disconnect(). If you intend to do a reconnection, call reconnect() instead.
         */
        bool connect();

        /**
         * \brief Disconnect from the reader.
         * \see connect
         *
         * Calling this method on a disconnected reader has no effect.
         */
        void disconnect();

        /**
         * \brief Check if the card is connected.
         * \return True if the card is connected, false otherwise.
         */
        virtual bool isConnected();

        /**
         * \brief Connect to the reader. Implicit connection on first command sent.
         * \return True if the connection successed.
         */
        virtual bool connectToReader();

        /**
         * \brief Disconnect from reader.
         */
        virtual void disconnectFromReader();

        /**
         * \brief Get a string hexadecimal representation of the reader serial number
         * \return The reader serial number or an empty string on error.
         */
        virtual std::string getReaderSerialNumber();

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
        virtual void serialize(boost::property_tree::ptree& parentNode);

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
        virtual void unSerialize(boost::property_tree::ptree& node);

        /**
         * \brief Retrieve reader identifier.
         * \return True on success, false otherwise. False means it's not a TMC reader connected to COM port or COM port inaccessible.
         */
        bool retrieveReaderIdentifier();

        /**
         * \brief Get the TMC reader identifier.
         * \return The TMC reader identifier.
         */
        std::vector<unsigned char> getTMCIdentifier();

        /**
         * \brief Get the AxessTMC13 reader unit configuration.
         * \return The AxessTMC13 reader unit configuration.
         */
        boost::shared_ptr<AxessTMC13ReaderUnitConfiguration> getAxessTMC13Configuration() { return boost::dynamic_pointer_cast<AxessTMC13ReaderUnitConfiguration>(getConfiguration()); };

        /**
         * \brief Get the AxessTMC13 reader provider.
         * \return The AxessTMC13 reader provider.
         */
        boost::shared_ptr<AxessTMC13ReaderProvider> getAxessTMC13ReaderProvider() const;

    protected:

        /**
         * \brief The TMC reader identifier.
         */
        std::vector<unsigned char> d_tmcIdentifier;
    };
}

#endif