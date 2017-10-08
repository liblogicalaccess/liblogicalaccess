/**
 * \file pcscreaderunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC Reader unit.
 */

#ifndef LOGICALACCESS_PCSCREADERUNIT_HPP
#define LOGICALACCESS_PCSCREADERUNIT_HPP

#include <logicalaccess/utils.hpp>
#include "pcscreaderunitconfiguration.hpp"
#include "../iso7816/iso7816readerunit.hpp"
#include "pcsc_fwd.hpp"
#include "logicalaccess/lla_fwd.hpp"
#include "pcsc_connection.hpp"

namespace logicalaccess
{
    /**
     * \brief The PC/SC reader unit class.
     */
    class LIBLOGICALACCESS_API PCSCReaderUnit : public ISO7816ReaderUnit
    {
    public:

        /**
         * \brief Constructor.
         */
	    explicit PCSCReaderUnit(const std::string& name);

        /**
         * \brief Destructor.
         */
        virtual ~PCSCReaderUnit();

        /**
         * \brief Get the reader unit name.
         * \return The reader unit name.
         */
	    std::string getName() const override;

        /**
         * \brief Set the reader unit name.
         * \param name The reader unit name.
         */
        void setName(const std::string& name);

        /**
         * \brief Get the connected reader unit name.
         * \return The connected reader unit name.
         */
	    std::string getConnectedName() override;

        /**
         * \brief Get the PC/SC reader unit type.
         * \return The PC/SC reader unit type.
         */
        virtual PCSCReaderUnitType getPCSCType() const;

        /**
         * \brief Set the card type.
         * \param cardType The card type.
         */
	    void setCardType(std::string cardType) override { d_card_type = cardType; }

        /**
         * \brief Wait for a card insertion.
         * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
         * \return True if a card was inserted, false otherwise. If a card was inserted, the name of the reader on which the insertion was detected is accessible with getReader().
         * \warning If the card is already connected, then the method always fail.
         */
	    bool waitInsertion(unsigned int maxwait) override final;

        /**
         * \brief Wait for a card removal.
         * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
         * \return True if a card was removed, false otherwise. If a card was removed, the name of the reader on which the removal was detected is accessible with getReader().
         */
	    bool waitRemoval(unsigned int maxwait) override;

        /**
         * \brief Connect to the card.
         * \return True if the card was connected without error, false otherwise.
         *
         * If the card handle was already connected, connect() first call disconnect(). If you intend to do a reconnection, call reconnect() instead.
         */
	    bool connect() override;

         /**
         * \brief Connect to the card.
         * \param reader The reader name. If an empty or invalid reader name is specified, connect will fail.
         * \param share_mode The share mode.
         * \return True if the card was connected without error, false otherwise.
         *
         * If the card handle was already connected, connect() first call disconnect(). If you intend to do a reconnection, call reconnect() instead.
         */
        virtual bool connect(PCSCShareMode share_mode);

        /**
         * \brief Reconnect to the card with the currently active share mode on the same reader.
         * \param control the behavior of the reconnect.
         * \return True if the card was reconnected without error, false otherwise.
         */
	    bool reconnect(int action = 0) override;

        /**
         * \brief Disconnect from the card.
         */
	    void disconnect() override;

        /**
         * \brief Check if the handle is associated to a card (aka. "connected").
         * \return True if the handle is associated to a card, false otherwise.
         */
	    bool isConnected() override;

        /**
         * \brief Connect to the reader. Implicit connection on first command sent.
         * \return True if the connection successed.
         */
	    bool connectToReader() override;

        /**
         * \brief Disconnect from reader.
         */
	    void disconnectFromReader() override;

        /**
         * \brief Get the low-level SCARDHANDLE.
         * \return The SCARDHANDLE.
         */
        const SCARDHANDLE& getHandle() const;

        /**
        * \brief Get the active protocol for the connected card.
        * \return The active protocol.
        */
        unsigned long getActiveProtocol() const;

        /**
         * \brief Get the currently active share mode.
         * \return The currently active share mode.
         *
         * If the card is not connected, requesting the share mode value is meaningless.
         */
        PCSCShareMode getShareMode() const;

        /**
         * \brief Create the chip object from card type.
         * \param type The card type.
         * \return The chip.
         */
	    std::shared_ptr<Chip> createChip(std::string type) override;

        /**
         * \brief Get the first and/or most accurate chip found.
         * \return The single chip.
         */
	    std::shared_ptr<Chip> getSingleChip() override;

        /**
         * \brief Get chip available in the RFID rang.
         * \return The chip list.
         */
	    std::vector<std::shared_ptr<Chip> > getChipList() override;

        /**
         * \brief Get the card serial number.
         * \return The card serial number.
         *
         * If the card handle is not connected to the card, the call fails.
         */
        virtual ByteVector getCardSerialNumber();

        /**
         * \brief Get a string hexadecimal representation of the reader serial number
         * \return The reader serial number or an empty string on error.
         */
	    std::string getReaderSerialNumber() override;

        /**
         * \brief Get the card ATR.
         * \param atr The array that will contains the ATR data.
         * \param atrLength The ATR array length.
         * \return The ATR length.
         */
	const std::vector<uint8_t>& getATR() const;

        /**
         * \brief Get the default PC/SC reader/card adapter.
         * \return The default PC/SC reader/card adapter.
         */
        virtual std::shared_ptr<PCSCReaderCardAdapter> getDefaultPCSCReaderCardAdapter();

        /**
         * \brief Get the PC/SC reader/card adapter for a card type.
         * \param type The card type.
         * \return The PC/SC reader/card adapter.
         */
        virtual std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter(std::string type);

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
	    void serialize(boost::property_tree::ptree& parentNode) override;

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
	    void unSerialize(boost::property_tree::ptree& node) override;

        /**
         * \brief Get the reader unit configuration.
         * \return The reader unit configuration.
         */
	    std::shared_ptr<ReaderUnitConfiguration> getConfiguration() override;

        /**
         * \brief Set the reader unit configuration.
         * \param config The reader unit configuration.
         */
	    void setConfiguration(std::shared_ptr<ReaderUnitConfiguration> config) override;

        /**
         * \brief Change a key in reader memory.
         * \param keystorage The key storage information.
         * \param key The key value.
         */
        virtual void changeReaderKey(std::shared_ptr<ReaderMemoryKeyStorage> keystorage, const ByteVector& key);

        /**
         * \brief Get the PC/SC reader unit configuration.
         * \return The PC/SC reader unit configuration.
         */
        std::shared_ptr<PCSCReaderUnitConfiguration> getPCSCConfiguration() { return std::dynamic_pointer_cast<PCSCReaderUnitConfiguration>(getConfiguration()); }

        std::shared_ptr<PCSCReaderProvider> getPCSCReaderProvider() const;

        static std::shared_ptr<PCSCReaderUnit> createPCSCReaderUnit(const std::string& readerName);

        /**
         * \brief Make the reader unit as a proxy to another. Use when listening on all PC/SC reader unit.
         * \param readerUnit The referenced reader unit.
         * \param readerUnitConfig The referenced reader unit configuration.
         */
        void makeProxy(std::shared_ptr<PCSCReaderUnit> readerUnit, std::shared_ptr<PCSCReaderUnitConfiguration> readerUnitConfig);

        /**
         * \brief Get if the inserted card is from ISO 14443-4 A or B.
         * \param isTypeA Will be set to true if the card is from ISO 14443-4 A, false if not or unknown.
         * \param isTypeB Will be set to true if the card is from ISO 14443-4 B, false if not or unknown.
         */
        virtual void getT_CL_ISOType(bool& isTypeA, bool& isTypeB);

        /**
         * This method is used to notify the (proxyfied) implementation
         * that a card was connected
         */
        virtual void cardConnected() {}

        /**
         * Returns the proxy implementation reader unit, or null.
         */
        std::shared_ptr<PCSCReaderUnit> getProxyReaderUnit() const;


	    void setCardTechnologies(const TechnoBitset &bitset) override;

	    TechnoBitset getCardTechnologies() override;

	    std::shared_ptr<LCDDisplay> getLCDDisplay() override;

	    void setLCDDisplay(std::shared_ptr<LCDDisplay> d) override;

	    std::shared_ptr<LEDBuzzerDisplay>
        getLEDBuzzerDisplay() override;

	    void
        setLEDBuzzerDisplay(std::shared_ptr<LEDBuzzerDisplay> lbd) override;

	    ReaderServicePtr getService(const ReaderServiceType &type) override;

    protected:

        /**
         * Perform adjustment regarding a Chip.
         *
         * For example, if the Chip type is desfire, we check that it
         * is not in fact a desfire ev1.
         * Similarly, we check to see if a desfire has random UID enabled or not.
         *
         * This function may return a new Chip object, that should be used.
         */
        std::shared_ptr<Chip> adjustChip(std::shared_ptr<Chip> c);

	    std::shared_ptr<ResultChecker> createDefaultResultChecker() const override;

	    std::shared_ptr<CardProbe> createCardProbe() override;

        void configure_mifareplus_chip(std::shared_ptr<Chip> c,
                                       std::shared_ptr<Commands> &commands,
                                       std::shared_ptr<ResultChecker> &resultChecker) const;

        /**
         * Attempt to detect the security level by sending command to the card
         * once its' been connected.
         *
         * If we find something, we'll override the commands objects and reconfigure
         * the chip.
         */
        void detect_mifareplus_security_level(std::shared_ptr<Chip> c);

        /**
         * \brief The reader unit name.
         */
        std::string d_name;

        /**
         * \brief The reader unit connected name.
         */
        std::string d_connectedName;

        /**
         * \brief Set the first and/or most accurate chip found.
         * \param chip The single chip.
         */
        void setSingleChip(std::shared_ptr<Chip> chip);

        /**
        * \brief Get The SAM Chip
        */
	    std::shared_ptr<SAMChip> getSAMChip() override;

        /**
        * \brief Set the SAM Chip
        */
	    void setSAMChip(std::shared_ptr<SAMChip> t) override;

        /**
        * \brief Get The SAM ReaderUnit
        */
	    std::shared_ptr<ISO7816ReaderUnit> getSAMReaderUnit() override;

        /**
        * \brief Set the SAM ReaderUnit
        */
	    void setSAMReaderUnit(std::shared_ptr<ISO7816ReaderUnit> t) override;

	    // Internal helper for waitInsertion
        using SPtrStringVector = std::vector<std::shared_ptr<std::string>>;
        using ReaderStateVector = std::vector<SCARD_READERSTATE>;

        /**
         * Prepare the parameters for the call to SCardGetStatusChange
         * invoked by waitInsertion()
         */
        std::tuple<SPtrStringVector, ReaderStateVector> prepare_poll_parameters() const;

        /**
         * Create the proxy reader based on which reader detected a card
         * during waitInsertion.
         */
        void waitInsertion_create_proxy(const std::string &reader_name);

        /**
         * Give a chance to concrete reader implementation to do something just
         * after a insertion has been detected.
         *
         * This default implemented MUST be called explicitly if you override this
         * method.
         */
        virtual bool process_insertion(const std::string &cardType,
                                       int maxwait,
                                       const ElapsedTimeCounter &elapsed);

      public:
        /**
         * Direct means we established without requiring
         * that a card be near the card reader.
         */
        void setup_pcsc_connection(PCSCShareMode share_mode);

        /**
         * Terminate the PCSC connection.
         * This is a noop if the connection is already terminated.
         */
        void teardown_pcsc_connection();

      protected:
		/**
		 * A PCSC connection object.
		 */
		std::unique_ptr<PCSCConnection> connection_;

        /**
         * The current ATR
         */
        std::vector<uint8_t> atr_;

        /**
         * \brief The proxy reader unit.
         */
        std::shared_ptr<PCSCReaderUnit> d_proxyReaderUnit;
    };

}

#endif
