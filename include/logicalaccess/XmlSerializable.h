/**
 * \file XmlSerializable.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Xml Serializable.
 */

#ifndef LOGICALACCESS_XMLSERIALIZABLE_H
#define LOGICALACCESS_XMLSERIALIZABLE_H

// Bug number 621653 reported on Microsoft Connect
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4005)
#include <intsafe.h>
#include <stdint.h>
#pragma warning (push)
#pragma warning (default : 4005)
#endif

#ifdef _MSC_VER
#ifdef __cplusplus
#include <WinSock2.h>
#endif
#include <windows.h>
///* replace stdint.h type for MS Windows*/
//typedef __int8 int8_t;
//typedef unsigned __int8 uint8_t;
//typedef __int16 int16_t;
//typedef unsigned __int16 uint16_t;
//typedef __int32 int32_t;
//typedef unsigned __int32 uint32_t;
//typedef __int64 int64_t;
//typedef unsigned __int64 uint64_t;
//typedef int ssize_t;
#define strdup _strdup
//#else
//#include <stdint.h>
#endif

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#ifdef _MSC_VER
#include "msliblogicalaccess.h"
#else
#ifndef LIBLOGICALACCESS_API
#define LIBLOGICALACCESS_API
#endif
#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this macro, to avoid MSVC specific warnings pragma */
#endif
#endif

#include "logicalaccess/logs.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>


namespace LOGICALACCESS
{
	/**
	 * \brief A Xml Serializable base class. Add Xml serialization to a class.
	 */
	class LIBLOGICALACCESS_API XmlSerializable
	{
		public:
			/**
			 * \brief Serialize object to Xml Node string.
			 * \return The serialized object.
			 */
			virtual std::string serialize();

			/**
			 * \brief Serialize object to Xml Node.
			 * \param os The XML stream.
			 */
			virtual void serialize(boost::property_tree::ptree& parentNode) = 0;			

			/**
			 * \brief Serialize object to a Xml file.
			 * \param filename The Xml file.
			 * \return True on success, false otherwise.
			 */
			virtual bool serializeToFile(const std::string& filename);

			/**
			 * \brief UnSerialize object from a Xml Node.
			 * \param node The Xml node.
			 */
			virtual void unSerialize(boost::property_tree::ptree& node) = 0;

			/**
			 * \brief UnSerialize object from a Xml Node string.
			 * \param xmlstring The Xml Node string.
			 * \param rootNode The root node.
			 * \return True on success, false otherwise.
			 */
			virtual bool unSerialize(const std::string& xmlstring, const std::string& rootNode);

			/**
			 * \brief UnSerialize object from a Xml node.
			 * \param is The Xml stream.
			 * \param rootNode The root node.
			 * \return True on success, false otherwise.
			 */
			virtual bool unSerialize(std::istream& is, const std::string& rootNode);

			/**
			 * \brief UnSerialize object from a Xml node.
			 * \param node The Xml node.
			 * \param rootNode The root node.
			 * \return True on success, false otherwise.
			 */
			virtual bool unSerialize(boost::property_tree::ptree& node, const std::string& rootNode);

			/**
			 * \brief UnSerialize object from a Xml file.
			 * \param filename The Xml file.
			 * \return True on success, false otherwise.
			 */
			virtual bool unSerializeFromFile(const std::string& filename);

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const = 0;

			/**
			 * \brief Remove Xml declaration from a xml string.
			 * \param xmlstring Xml string containing xml declaration.
			 * \return Xml string without xml declaration
			 * \remarks To remove after a good xml library !!!
			 */
			static std::string removeXmlDeclaration(const std::string& xmlstring);

			/**
			 * \brief Format hex string to hex string with space.
			 * \param hexstr The hex string without space.
			 * \return The hex buffer with space.
			 */
			static std::vector<unsigned char> formatHexString(std::string hexstr);

	protected:

			unsigned long long atoull(const std::string& str);
	};	
}

#endif /* LOGICALACCESS_XMLSERIALIZABLE_H */
