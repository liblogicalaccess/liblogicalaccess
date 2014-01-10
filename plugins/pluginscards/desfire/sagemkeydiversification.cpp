#include "desfirekey.hpp"
#include "logicalaccess/logs.hpp"
#include "sagemkeydiversification.hpp"
#include "boost/shared_ptr.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "desfirecrypto.hpp"
#include <vector>

namespace logicalaccess
{
	void SagemKeyDiversification::initDiversification(std::vector<unsigned char> identifier, int /*AID*/, boost::shared_ptr<Key> /*key*/, std::vector<unsigned char>& diversify)
	{
		if (identifier.size() > 0)
		{
			diversify.resize(2 * identifier.size() + 2);
			diversify[0] = 0xFF;
			memcpy(&diversify[1], &identifier[0], identifier.size());
			diversify[8] = 0x00;
			memcpy(&diversify[9], &identifier[0], identifier.size());
			for (unsigned char i = 0; i < 7; ++i)
			{
				diversify[9 + i] ^= 0xFF;
			}
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Chip identifier is required for key diversification.");
		}
	}

	std::vector<unsigned char> SagemKeyDiversification::getDiversifiedKey(boost::shared_ptr<Key> key, std::vector<unsigned char> diversify)
	{
		INFO_("Using key diversification Sagem with div : %s", BufferHelper::getHex(diversify).c_str());
		std::vector<unsigned char> keydiv;

		boost::shared_ptr<DESFireKey> desfirekey = boost::dynamic_pointer_cast<DESFireKey>(key);

		// Sagem diversification algo. Should be an option with SAM diversification soon...
		std::vector<unsigned char> iv;
		// Two time, to have ECB and not CBC mode (laazzyyy to create new function :))
		std::vector<unsigned char> vkeydata;
		if (desfirekey->isEmpty())
		{
			vkeydata.resize(desfirekey->getLength(), 0x00);
		}
		else
		{
			vkeydata.insert(vkeydata.end(), desfirekey->getData(), desfirekey->getData() + desfirekey->getLength());
		}

		std::vector<unsigned char> r = DESFireCrypto::sam_CBC_send(vkeydata, iv, std::vector<unsigned char>(diversify.begin(), diversify.begin() + 8));
		keydiv.insert(keydiv.end(), r.begin(), r.end());
		std::vector<unsigned char> r2 = DESFireCrypto::sam_CBC_send(vkeydata, iv, std::vector<unsigned char>(diversify.begin() + 8, diversify.begin() + 16));
		keydiv.insert(keydiv.end(), r2.begin(), r2.end());
		return keydiv;
	}

	void SagemKeyDiversification::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SagemKeyDiversification::unSerialize(boost::property_tree::ptree& /*node*/)
	{
	}
}
