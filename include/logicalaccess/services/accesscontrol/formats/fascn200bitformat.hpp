/**
 * \file fascn200bitformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FASCN 200-Bit format.
 */

#ifndef LOGICALACCESS_FASCN200BITFORMAT_HPP
#define LOGICALACCESS_FASCN200BITFORMAT_HPP

#include <string>
#include <vector>
#include <iostream>

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

namespace logicalaccess
{
    /**
     * \brief FASC-N Organizational category
     */
    typedef enum {
        OC_FEDERAL_GOVERNMENT_AGENCY = 0x01,
        OC_STATE_GOVERNMENT_AGENCY = 0x02,
        OC_COMMERCIAL_ENTERPRISE = 0x03,
        OC_FOREIGN_GOVERNMENT = 0x04
    } FASCNOrganizationalCategory;

    /**
     * \brief FASC-N Person/Organization association category
     */
    typedef enum {
        POA_EMPLOYEE = 0x01,
        POA_CIVIL = 0x02,
        POA_EXECUTIVE_STAFF = 0x03,
        POA_UNIFORMED_SERVICE = 0x04
    } FASCNPOAssociationCategory;

    /**
     * \brief A FASC-N 200 bit format class.
     */
    class LIBLOGICALACCESS_API FASCN200BitFormat : public StaticFormat
    {
    public:
        /**
         * \brief Constructor.
         *
         * Create a FASC-N 200 bit Format.
         */
        FASCN200BitFormat();

        /**
         * \brief Destructor.
         *
         * Release the FASC-N 200 bit Format.
         */
        virtual ~FASCN200BitFormat();

        static const unsigned char FASCN_SS; /**< \brief The Start Sentinel value. */
        static const unsigned char FASCN_FS; /**< \brief The Field Separator value. */
        static const unsigned char FASCN_ES; /**< \brief The End Sentinel value. */

        /**
         * \brief Get the agency code.
         */
        unsigned short getAgencyCode() const;

        /**
         * \brief Set the agency code.
         */
        void setAgencyCode(unsigned short agencyCode);

        /**
         * \brief Get the system code.
         */
        unsigned short getSystemCode() const;

        /**
         * \brief Set the system code.
         */
        void setSystemCode(unsigned short systemCode);

        /**
         * \brief Get the serie code.
         */
        unsigned char getSerieCode() const;

        /**
         * \brief Set the serie code.
         */
        void setSerieCode(unsigned char serieCode);

        /**
         * \brief Get the credential code.
         */
        unsigned char getCredentialCode() const;

        /**
         * \brief Set the credential code.
         */
        void setCredentialCode(unsigned char credentialCode);

        /**
         * \brief Get the person identifier.
         */
        unsigned long long getPersonIdentifier() const;

        /**
         * \brief Set the person identifier.
         */
        void setPersonIdentifier(unsigned long long personIdentifier);

        /**
         * \brief Get the organizational category.
         */
        FASCNOrganizationalCategory getOrganizationalCategory() const;

        /**
         * \brief Set the organizational category.
         */
        void setOrganizationalCategory(FASCNOrganizationalCategory organizationalCategory);

        /**
         * \brief Get the organizational identifier.
         */
        unsigned short getOrganizationalIdentifier() const;

        /**
         * \brief Set the organizational identifier.
         */
        void setOrganizationalIdentifier(unsigned short organizationalIdentifier);

        /**
         * \brief Get the person/organization association category.
         */
        FASCNPOAssociationCategory getPOACategory() const;

        /**
         * \brief Set the person/organization association category.
         */
        void setPOACategory(FASCNPOAssociationCategory organizationalCategory);

        /**
         * \brief Get the format length in bits.
         */
	    unsigned int getDataLength() const override;

        /**
         * \brief Get the format name.
         * \return The format name.
         */
		std::string getName() const override;

        /**
         * \brief Get the format type.
         * \return The format type.
         */
	    FormatType getType() const override;

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
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
		std::string getDefaultXmlNodeName() const override;

        /**
         * \brief Get linear data.
         * \param data Where to put data
         * \param dataLengthBytes Length in byte of data
         */
	    void getLinearData(void* data, size_t dataLengthBytes) const override;

        /**
         * \brief Set linear data
         * \param data Where to get data
         * \param dataLengthBytes Length of data in bytes
         */
	    void setLinearData(const void* data, size_t dataLengthBytes) override;

        /**
         * \brief Get the format linear data in bytes.
         * \param data The data buffer
         * \param dataLengthBytes The data buffer length
         * \return The format linear data length.
         */
	    size_t getFormatLinearData(void* data, size_t dataLengthBytes) const override;

        /**
         * \brief Set the format linear data in bytes.
         * \param data The data buffer
         * \param indexByte The data offset
         */
	    void setFormatLinearData(const void* data, size_t* indexByte) override;

        /**
         * \brief Check the current format skeleton with another format.
         * \param format The format to check.
         * \return True on success, false otherwise.
         */
	    bool checkSkeleton(std::shared_ptr<Format> format) const override;

        /**
         * \brief Calculate the Longitudinal Redundancy Check for a buffer.
         * \param data The buffer.
         * \param datalenBits The buffer length (in bits).
         * \return The LRC value.
         */
        unsigned char calculateLRC(const void* data, unsigned int datalenBits) const;

    protected:

        struct {
            /**
             * \brief The agency code. Identifies the government agency issuing the credential.
             */
            unsigned short d_agencyCode;

            /**
             * \brief The system code. Identifies the system the card is enrolled in and is unique for each site.
             */
            unsigned short d_systemCode;

            /**
             * \brief The credential series. Field is available to reflect major system changes.
             */
            unsigned char d_serieCode;

            /**
             * \brief The individual credential issue. Initially encoded as '1', will be incremented if a card is replaced due to loss or damage.
             */
            unsigned char d_credentialCode;

            /**
             * \brief The person identifier. Numeric code used by the identity source to uniquely identify the token carrier.
             */
            unsigned long long d_personIdentifier;

            /**
             * \brief The organizational category.
             */
            FASCNOrganizationalCategory d_orgCategory;

            /**
             * \brief The organizational identifier. OC=1 – FIPS 95-2 Agency Code. OC=2 – State Code. OC=3 – Company Code. OC=4 – Numeric Country Code.
             */
            unsigned short d_orgIdentifier;

            /**
             * \brief The person/organization association category.
             */
            FASCNPOAssociationCategory d_poaCategory;
        } d_formatLinear;
    };
}

#endif /* LOGICALACCESS_FASCN200BITFORMAT_HPP */