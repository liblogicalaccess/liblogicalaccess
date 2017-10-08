#pragma once

#include <chrono>
#include <logicalaccess/cards/accessinfo.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/services/cardservice.hpp>

namespace logicalaccess
{
class LIBLOGICALACCESS_API IdentityCardService : public CardService
{
  public:
    enum class MetaData
    {
        /**
         * The document number. It is retrieved as a std::string
         */
        DOC_NO,

        /**
        * Name of the person whose identity information
        * is stored on the chip.
        */
        NAME,

        /**
         * Nationality of the card holder. For EPassport this is a
         * 3 letter country code.
         */
        NATIONALITY,

        /**
         * Binary data representing a facial picture of the card holder.
         *
         * The format of this image is not specified at this API level and is
         * dependant on the type of the underlying card.
         */
        PICTURE,

        /**
         * Date of birth, as a std::chrono::system_clock::time_point
         */
        BIRTHDATE,
        /**
         * Expiration date of the document, as a
         * std::chrono::system_clock::time_point
         */
        EXPIRATION
    };

	explicit IdentityCardService(std::shared_ptr<Chip> chip);

    virtual ~IdentityCardService() = default;

    /**
     * Method used to retrieve identity data that fits
     */
	virtual std::string getString(MetaData what) = 0;

    /**
     * Used to retrieve binary data (picture).
     */
	virtual ByteVector getData(MetaData what) = 0;

    /**
     * To retrieve date entry.
     */
	virtual std::chrono::system_clock::time_point getTime(MetaData what) = 0;

    /**
     * Provide an AccessInfo that will be used by underlying implementation
     * to authenticate against the chip.
     *
     * As an example, the EPassport Identity Service expect to find an
     * EPassAccessInfo and will extract MRZ information from it.
     */
    void setAccessInfo(std::shared_ptr<AccessInfo> ai);

  protected:
    /**
     * AccessInfo that may be used by underlying implementation
     * to gather whatever data they may need to perform various operation.
     */
    std::shared_ptr<AccessInfo> access_info_;
};
}
