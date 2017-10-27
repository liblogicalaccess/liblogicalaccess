/**
 * \file resultchecker.hpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief Base result checker.
 */

#ifndef LOGICALACCESS_RESULTCHECKER_HPP
#define LOGICALACCESS_RESULTCHECKER_HPP

#include "logicalaccess_api.hpp"
#include "myexception.hpp"
#include <iostream>
#include <map>
#include <tuple>

namespace logicalaccess
{
/**
 * \brief A result checker base class.
 */
class LIBLOGICALACCESS_API ResultChecker
{
  public:
    /**
     * \brief Constructor.
     */
    ResultChecker();

    /**
     * \brief Destructor.
     */
    virtual ~ResultChecker();

    /**
     * \brief Check the buffer result and throw exception if needed.
     * \param data The buffer.
     * \param size_t The buffer length.
     */
    virtual void CheckResult(const void *data, size_t datalen);

    /**
     * Whether or not an empty result is a valid response
     * or not.
     * Default to false.
     */
    virtual bool AllowEmptyResult() const;

  protected:
    /**
     * \brief Add a new check. Maps already existing are overwritten.
     * \param SW1 The SW1 error code.
     * \param SW2 The SW2 error code.
     * \param msg The string message related to the error codes.
             * \param error The (liblogicalacccess-level) error code corresponding to the
     * error.
     * \param throwException Define if an exception should be throwed for the error code.
     */
    void AddCheck(unsigned char SW1, unsigned char SW2, const std::string &msg,
                  CardException::ErrorType error = CardException::DEFAULT,
                  bool throwException            = true);

    /**
     * \brief Remove a check.
     * \param SW1 The SW1 error code.
     * \param SW2 The SW2 error code.
     */
    void RemoveCheck(unsigned char SW1, unsigned char SW2);

    /**
     * \brief Clear all checks.
     */
    void ClearChecks();

    /**
     * \brief Add a new single shot check. Single shots are always cleared after the check
     * result.
     * \param SW1 The SW1 error code.
     * \param SW2 The SW2 error code.
     * \param msg The string message related to the error codes.
     */
    void AddSingleShotCheck(unsigned char SW1, unsigned char SW2, const std::string &msg);

  private:
    /**
     * \brief Result checker map keys.
     */
    typedef std::pair<unsigned char, unsigned char> ResultCheckerKeys;

    /**
     * \brief Result checker map values.
     */
    typedef std::tuple<std::string, CardException::ErrorType, bool> ResultCheckerValues;
    // typedef std::pair<std::string, bool> ResultCheckerValues;

    /**
     * \brief Primary map checks.
     */
    std::map<ResultCheckerKeys, ResultCheckerValues> mapPrimary;

    /**
     * \brief Single shot checks.
     */
    std::map<ResultCheckerKeys, ResultCheckerValues> mapSingleShot;
};
}

#endif /* LOGICALACCESS_RESULTCHECKER_HPP */