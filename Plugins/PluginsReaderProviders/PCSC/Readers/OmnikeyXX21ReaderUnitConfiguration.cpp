/**
 * \file OmnikeyXX21ReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX21 reader unit configuration.
 */

#include "../Readers/OmnikeyXX21ReaderUnitConfiguration.h"


namespace LOGICALACCESS
{
	OmnikeyXX21ReaderUnitConfiguration::OmnikeyXX21ReaderUnitConfiguration()
		: PCSCReaderUnitConfiguration()
	{
		resetConfiguration();	
	}

	OmnikeyXX21ReaderUnitConfiguration::~OmnikeyXX21ReaderUnitConfiguration()
	{
	}

	void OmnikeyXX21ReaderUnitConfiguration::resetConfiguration()
	{
		d_useSecureMode = true;
		d_secureReadKey.reset(new TripleDESKey());
		d_secureReadKey->fromString("");
		d_secureWriteKey.reset(new TripleDESKey());
		d_secureWriteKey->fromString("");
		d_encryptionMode = HID_AUTO;
	}

	bool OmnikeyXX21ReaderUnitConfiguration::getUseSecureMode()
	{
		return d_useSecureMode;
	}

	void OmnikeyXX21ReaderUnitConfiguration::setUseSecureMode(bool use_sm)
	{
		d_useSecureMode = use_sm;
	}

	HIDEncryptionMode OmnikeyXX21ReaderUnitConfiguration::getEncryptionMode()
	{
		return d_encryptionMode;
	}

	void OmnikeyXX21ReaderUnitConfiguration::setEncryptionMode(HIDEncryptionMode encryption_mode)
	{
		d_encryptionMode = encryption_mode;
	}

	boost::shared_ptr<TripleDESKey> OmnikeyXX21ReaderUnitConfiguration::getSecureReadKey()
	{
		return d_secureReadKey;
	}

	void OmnikeyXX21ReaderUnitConfiguration::setSecureReadKey(boost::shared_ptr<TripleDESKey> key)
	{
		d_secureReadKey = key;
	}

	boost::shared_ptr<TripleDESKey> OmnikeyXX21ReaderUnitConfiguration::getSecureWriteKey()
	{
		return d_secureWriteKey;
	}

	void OmnikeyXX21ReaderUnitConfiguration::setSecureWriteKey(boost::shared_ptr<TripleDESKey> key)
	{
		d_secureWriteKey = key;
	}

	void OmnikeyXX21ReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		PCSCReaderUnitConfiguration::serialize(node);
		node.put("UseSecureMode", d_useSecureMode);
		boost::property_tree::ptree rknode;
		d_secureReadKey->serialize(rknode);
		node.add_child("SecureReadKey", rknode);
		boost::property_tree::ptree wknode;
		d_secureWriteKey->serialize(wknode);
		node.add_child("SecureWriteKey", wknode);
		node.put("EncryptionMode", d_encryptionMode);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void OmnikeyXX21ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		PCSCReaderUnitConfiguration::unSerialize(node.get_child(PCSCReaderUnitConfiguration::getDefaultXmlNodeName()));
		d_useSecureMode = node.get_child("UseSecureMode").get_value<bool>();
		boost::dynamic_pointer_cast<XmlSerializable>(d_secureReadKey)->unSerialize(node.get_child("SecureReadKey"), "");
		boost::dynamic_pointer_cast<XmlSerializable>(d_secureWriteKey)->unSerialize(node.get_child("SecureWriteKey"), "");
		d_encryptionMode = static_cast<HIDEncryptionMode>(node.get_child("EncryptionMode").get_value<unsigned int>());
	}

	std::string OmnikeyXX21ReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "OmnikeyXX21ReaderUnitConfiguration";
	}
}
