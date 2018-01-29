#include <logicalaccess/iks/IslogKeyServer.hpp>
#include <thread>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/plugins/lla-tests/macros.hpp>

using namespace logicalaccess;
using namespace iks;

static void test_big()
{
    IslogKeyServer &srv = IslogKeyServer::fromGlobalSettings();

    std::cout << "Testing with big payload" << std::endl;

    auto bytes = ByteVector{};
    for (int i = 0 ; i < 16*100; ++i)
        bytes.push_back(i);

    auto iv = std::array<uint8_t, 16>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    auto encrypted = srv.aes_encrypt(bytes, "akey", iv);
    std::cout << "Encrypted (big) = " << encrypted << std::endl;
    auto decrypted = srv.aes_decrypt(encrypted, "akey", iv);
    std::cout << "Decrypted (big) = " << decrypted << std::endl;
    assert(bytes == decrypted);
}

int main() {
    iks::IslogKeyServer::configureGlobalInstance("127.0.0.1",
                                                 9876,
                                                 "/home/xaqq/Downloads/demo/certs/MyClient1.pem",
                                                 "/home/xaqq/Downloads/demo/certs/MyClient1.key",
                                                 "/home/xaqq/Downloads/demo/certs/mydomain.crt");
  IslogKeyServer &srv = IslogKeyServer::fromGlobalSettings();

    srv.get_random(17);
    auto bytes = ByteVector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                            11, 12, 13, 14, 15};
    auto iv = std::array<uint8_t, 16>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    auto encrypted = srv.aes_encrypt(bytes, "akey", iv);
    std::cout << "Encrypted = " << encrypted << std::endl;
    auto decrypted = srv.aes_decrypt(encrypted, "akey", iv);
    std::cout << "Decrypted: " << decrypted << std::endl;
    assert(bytes == decrypted);

    test_big();
}
