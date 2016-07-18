//
// Created by xaqq on 6/24/15.
//

#ifndef LIBLOGICALACCESS_RPLETH_FWD_HPP
#define LIBLOGICALACCESS_RPLETH_FWD_HPP

namespace logicalaccess
{
    class RplethReaderCardAdapter;
    class RplethLEDBuzzerDisplay;
    class RplethReaderUnitConfiguration;
    class RplethReaderProvider;

    /**
   * \brief Device code in Rpleth protocol.
   */
    typedef enum
    {
        RPLETH = 0x00,
        HID = 0X01,
        LCD = 0x02
    } Device;

    /**
     * \brief RplethCommand code in Rpleth protocol.
     */
    typedef enum
    {
        STATEDHCP = 0x01,
        DHCP = 0x02,
        MAC = 0x03,
        IP = 0x04,
        SUBNET = 0x05,
        GATEWAY = 0x06,
        PORT = 0x07,
        MESSAGE = 0x08,
        RESET = 0x09,
        PING = 0x0a,
        SET_CONTEXT = 0x0b,
        GET_CONTEXT = 0x0c
    } RplethCommand;

    /**
     * \brief HidCommand code in Rpleth protocol.
     */
    typedef enum
    {
        BEEP = 0x00,
        GREENLED = 0x01,
        REDLED = 0x02,
        NOP = 0x03,
        BADGE = 0x04,
        COM = 0x05,
        WAIT_INSERTION = 0x06,
        WAIT_REMOVAL = 0x07,
        CONNECT = 0x08,
        DISCONNECT = 0x09,
        GET_READERTYPE = 0x0A,
        GET_CSN = 0x0B,
        SET_CARDTYPE = 0x0C,
        SEND_CARDS = 0x0D,
        RECEIVE_UNPRESENTED_CARDS = 0x0E
    } HidCommand;

    /**
     * \brief LcdCommand code in Rpleth protocol.
     */
    typedef enum
    {
        DISPLAY = 0x00,
        DISPLAYT = 0X01,
        BLINK = 0X02,
        SCROLL = 0X03,
        DISPLAYTIME = 0x04
    } LcdCommand;

}

#endif //LIBLOGICALACCESS_RPLETH_FWD_HPP
