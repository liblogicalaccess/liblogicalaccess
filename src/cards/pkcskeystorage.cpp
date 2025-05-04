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
    boost::property_tree::ptree pnode;
    for (const std::pair<const std::string, std::string>& n : pkcs_properties_)
    {
        pnode.put(n.first, n.second);
    }
    node.add_child("PKCSProperties", pnode);
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
    pkcs_properties_.clear();
    boost::optional<boost::property_tree::ptree &> pprop = node.get_child_optional("PKCSProperties");
    if (pprop)
    {
        for (boost::property_tree::ptree::iterator p = pprop->begin(); p != pprop->end(); ++p)
        {
            std::string pname = p->first;
            pkcs_properties_.insert(std::pair<std::string, std::string>(pname, p->second.get_value<std::string>()));
        }
    }
    pkcs_library_shared_object_path_ =
        node.get_child("PKCSLibraryPath").get_value<std::string>("");

    pkcs_session_password_ = node.get_child("PKCSPassword").get_value<std::string>("");
}
}
