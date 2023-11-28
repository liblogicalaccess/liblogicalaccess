/**
 * \file iso7816ok5553readercardadapter.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ISO7816 OK5553 reader/card adapter.
 */

#ifndef LOGICALACCESS_ISO7816OK5553READERCARDADAPTER_HPP
#define LOGICALACCESS_ISO7816OK5553READERCARDADAPTER_HPP

#include <logicalaccess/plugins/readers/ok5553/readercardadapters/ok5553readercardadapter.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>

#include <string>
#include <vector>

namespace logicalaccess
{
/**
 * \brief A ISO7816 OK5553 reader/card adapter class.
 */
class LLA_READERS_OK5553_API ISO7816OK5553ReaderCardAdapter
    : public ISO7816ReaderCardAdapter
{
  public:
    /**
     * \brief Constructor.
     */
    ISO7816OK5553ReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~ISO7816OK5553ReaderCardAdapter();

#ifndef SWIG
    using ISO7816ReaderCardAdapter::sendAPDUCommand;
#endif

    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char lc, const unsigned char *data,
                                       size_t datalen, unsigned char le);

    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char lc, const unsigned char *data,
                                       size_t datalen);

    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       const unsigned char *data, size_t datalen);
    /**
     * \brief Send an APDU command to the reader without data and result.
     */
    ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                    unsigned char p1,
                               unsigned char p2, unsigned char le) override;

    /**
     * \brief Send an APDU command to the reader without data.
     */
    ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                    unsigned char p1,
                               unsigned char p2, unsigned char lc,
                               unsigned char le) override;

    /**
     * \brief Send an APDU command to the reader without data.
     */
    ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                    unsigned char p1,
                               unsigned char p2) override;

    /**
     * \brief Adapt the command to send to the reader.
     * \param command The command to send.
     * \return The adapted command to send.
     */
    ByteVector adaptCommand(const ByteVector &command) override;

    /**
     * \brief Adapt the asnwer received from the reader.
     * \param answer The answer received.
     * \return The adapted answer received.
     */
    ByteVector adaptAnswer(const ByteVector &answer) override;

  protected:
    /**
     * \brief Reader card adapter used to send command..
     */
    std::shared_ptr<OK5553ReaderCardAdapter> d_ok5553_reader_card_adapter;

    /**
     * \brief Handle the asnwer received from the reader.
     * \param answer The answer received.
     * \return The handled answer.
     */
    static ByteVector handleAnswer(const ByteVector &answer);

    /**
     * \brief Reverse the asnwer received from the reader.
     * \param answer The answer received.
     * \return The reversed answer.
     */
    static ByteVector answerReverse(const ByteVector &answer);

    /**
     * \brief Represent stat of prefix in desfire command. (0x02 of 0x03).
     */
    bool d_prefix;
};
}

#endif /* LOGICALACCESS_ISO7816OK5553READERCARDADAPTER_HPP */