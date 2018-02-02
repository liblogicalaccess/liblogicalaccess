/**
 * \file test_appIDs_brutForce_class.h
 * \author Leo Jullerot
 * \brief
 */

#ifndef LOGICALACCESS_APPIDSBRUTFORCE_H
#define LOGICALACCESS_APPIDSBRUTFORCE_H

#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand26format.hpp>
#include <logicalaccess/readerproviders/serialportdatatransport.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/lla-tests/macros.hpp>
#include <logicalaccess/plugins/lla-tests/utils.hpp>
#include <logicalaccess/utils.hpp>

class AppIDsBrutForce {
public:

/**
 * \brief Constructor
 * \param cmd DESFireISO7816Commands
 * \param idApp ID of application
 */
    AppIDsBrutForce(std::shared_ptr<logicalaccess::DESFireISO7816Commands> &cmd, unsigned int idApp);

/**
 * \brief Search all files IDs in selected application
 */
    void brutForceFileIDs();

/**
 * \brief Search all key numbers in selected application
 */
    void brutForceKeyNo();

/**
 * \brief Return Files IDs vector of selected application
 */
    std::vector<unsigned int> getFilesIDs();

/**
 * \brief Return application ID
 */
    int getAppID();

/**
 * \brief Return key numbers vector of selected application
 */
    std::vector<unsigned int> getKeysNo();

/**
 * \brief Return type of key (AES or DES)
 */
    std::string getKeyType();

private:

    unsigned int appID;

    std::vector<unsigned int> fileIDs;

    std::vector<unsigned int> keysNo;

    std::shared_ptr<logicalaccess::DESFireISO7816Commands> &cmd;

    logicalaccess::DESFireKeyType keyType;
};

#endif //LIBLOGICALACCESS_APPIDSBRUTFORCE_H
