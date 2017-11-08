//
// Created by xaqq on 6/16/15.
//

#ifndef LIBLOGICALACCESS_MIFAREPLUSISO7816RESULTCHECKER_H
#define LIBLOGICALACCESS_MIFAREPLUSISO7816RESULTCHECKER_H

#include <logicalaccess/resultchecker.hpp>
#include <cstdint>

namespace logicalaccess
{
/**
 * Result checker when talking to a MifarePlus in T=CL mode.
 */
class LIBLOGICALACCESS_API MifarePlusISO7816ResultChecker : public ResultChecker
{
  public:
    MifarePlusISO7816ResultChecker();

    virtual ~MifarePlusISO7816ResultChecker() = default;

    /**
     * Check the buffer result and throw exception if needed.
     *
     * This method needs to be reimplemented because the result code doesn't follow
     * the format SW1 SW2.
     *
     * @param data The buffer.
     * @param size_t The buffer length.
     */
    void CheckResult(const void *data, size_t datalen) override;

  private:
    static bool checkByte(uint8_t);
};
}

#endif // LIBLOGICALACCESS_MIFAREPLUSISO7816RESULTCHECKER_H
