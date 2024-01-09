#include <logicalaccess/cards/pkcskeystorage.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{

void PKCSKeyStorage::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    node.put("KeyIdentity", BufferHelper::getHex(pkcs_object_id_));
    node.put("SlotId", slot_id_);
    node.put("ProteccioConfDir", env_PROTECCIO_CONF_DIR_);
    node.put("PKCSLibraryPath", pkcs_library_shared_object_path_);

    // We rely on the application not providing a password /clearing
    // the password before serializing if it doesnt want to save
    // the password.
    // This should be improved later.
    node.put("PKCSPassword", pkcs_session_password_);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void PKCSKeyStorage::unSerialize(boost::property_tree::ptree &node)
{
    pkcs_object_id_ = BufferHelper::fromHexString(
        node.get_child("KeyIdentity").get_value<std::string>(""));
    slot_id_ = node.get_child("SlotId").get_value<size_t>();
    env_PROTECCIO_CONF_DIR_ =
        node.get_child("ProteccioConfDir").get_value<std::string>("");
    pkcs_library_shared_object_path_ =
        node.get_child("PKCSLibraryPath").get_value<std::string>("");

    pkcs_session_password_ = node.get_child("PKCSPassword").get_value<std::string>("");
}
}
