/**
 * \file osdpcommands.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief OSDP commands. 
 */

#ifndef LOGICALACCESS_OSDPCOMMANDS_HPP
#define LOGICALACCESS_OSDPCOMMANDS_HPP

#include "logicalaccess/cards/commands.hpp"
#include "osdpchannel.hpp"

namespace logicalaccess
{	
#define CMD_ODSP "ODSP"

	enum class TemporaryControleCode : unsigned char {
		NOP = 0x00,
		CancelTemporaryOperation = 0x01,
		SetTemporaryState = 0x02
	};

	enum class PermanentControlCode : unsigned char {
		NotPermanentState = 0x00,
		SetPermanentState = 0x01
	};

	enum class OSDPColor : unsigned char {
		Black = 0x00,
		Red = 0x01,
		Green = 0x02,
		Amber = 0x03,
		Blue = 0x04
	};

	typedef struct	t_led_cmd {
		unsigned char reader;
		unsigned char ledNumber;
		TemporaryControleCode tempControlCode;
		unsigned char tempOnTime;
		unsigned char tempOffTime;
		OSDPColor tempOnColor;
		OSDPColor tempOffColor;
		unsigned char timerLSB;
		unsigned char timerMSB;
		PermanentControlCode permControlCode;
		unsigned char permOnTime;
		unsigned char permOffTime;
		OSDPColor permOnColor;
		OSDPColor permOffColor;
	}				s_led_cmd;

	typedef struct	t_buz_cmd {
		unsigned char reader;
		unsigned char toneCode;
		unsigned char onTime;
		unsigned char offTime;
		unsigned char count;
	}				s_buz_cmd;

	/**
	 * \brief OSDP Commands class.
	 */
	class LIBLOGICALACCESS_API OSDPCommands : public Commands
	{
	public:	
		OSDPCommands() : Commands(CMD_ODSP) { initCommands(); }

	    explicit OSDPCommands(std::string ct) : Commands(ct) { initCommands(); }

		~OSDPCommands() {}

		void initCommands(unsigned char address = 0);

		std::shared_ptr<OSDPChannel> poll() const;

		std::shared_ptr<OSDPChannel> challenge() const;

		std::shared_ptr<OSDPChannel> sCrypt() const;

		std::shared_ptr<OSDPChannel> led(s_led_cmd& led) const;

		std::shared_ptr<OSDPChannel> buz(s_buz_cmd& led) const;

		std::shared_ptr<OSDPChannel> setProfile(unsigned char profile) const;

		std::shared_ptr<OSDPChannel> getProfile() const;

		std::shared_ptr<OSDPChannel> disconnectFromSmartcard() const;

		void setSecureChannel(std::shared_ptr<OSDPChannel> channel) { m_channel = channel; }

		std::shared_ptr<OSDPChannel> getChannel() const { return m_channel; }

		std::shared_ptr<OSDPChannel> transmit() const;

	private:

		std::shared_ptr<OSDPChannel> m_channel;
	};
}

#endif /* LOGICALACCESS_OSDPCOMMANDS_HPP */

