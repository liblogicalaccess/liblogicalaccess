#include <logicalaccess/iks/IslogKeyServer.hpp>
#include <thread>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/iks/RemoteCrypto.hpp>
#include <logicalaccess/utils.hpp>
#include <iomanip>
#include <logicalaccess/bufferhelper.hpp>
#include <mutex>
#include <cassert>
#include <algorithm>
#include <string>
#include <iostream>
#include <logicalaccess/plugins/iks/IKSRPCClient.hpp>

using namespace logicalaccess;
using namespace iks;

std::mutex lock;

struct TestResult
{
    uint64_t nb_itr;
    int op_per_itr;
    std::string name;

    // Record an iteration time and update extreme values.
    void update_extreme_itr(uint64_t t)
    {
        if (!extreme_itr_set)
        {
            slowest_itr_micro = t;
            fastest_itr_micro = t;
            extreme_itr_set   = true;
            return;
        }

        if (t > slowest_itr_micro)
            slowest_itr_micro = t;
        if (t < fastest_itr_micro)
            fastest_itr_micro = t;
    }

    uint64_t total_elapsed_ms{0};

    uint64_t slowest_itr_micro{0};
    uint64_t fastest_itr_micro{0};

    bool extreme_itr_set{false};
};

void print_test_result(const TestResult &r)
{
    static std::mutex m;
    std::unique_lock<decltype(m)> ul(m);

    double average_itr = static_cast<double>(r.total_elapsed_ms) / r.nb_itr;
    // double average_op =      static_cast<double>(r.total_elapsed_ms) / (r.nb_itr *
    // r.op_per_itr);

    std::cout << "Test result for test: " << r.name << std::endl;
    std::cout << std::setprecision(3);
    std::cout << "\t ITERATIONS COUNT: " << r.nb_itr << std::endl
              << "\t OPERATIONS PER ITERATIONS: " << r.op_per_itr << std::endl
              << "\t TOTAL ELAPSED MS: " << r.total_elapsed_ms << std::endl;
    std::cout << "\t AVERAGE ITERATION: " << average_itr << std::endl
              << "\t FASTEST ITERATION: " << ((double)r.fastest_itr_micro) / 1000
              << std::endl
              << "\t SLOWEST ITERATION: " << ((double)r.slowest_itr_micro) / 1000
              << std::endl
              << std::endl;
}

/**
 * Helper class to collect and display benchmark test results.
 */
struct TestResultAgg
{
    explicit TestResultAgg(bool display_agg_only)
        : display_agg_only_(display_agg_only)
    {
    }

    void record_test_result(TestResult tr)
    {
        std::unique_lock<decltype(lock)> ul(lock);
        results.push_back(std::move(tr));
    }

    // Returns the unique list of test names
    std::vector<std::string> test_names() const
    {
        std::vector<std::string> names;
        for (const auto &tr : results)
        {
            if (std::find(names.begin(), names.end(), tr.name) == names.end())
            {
                names.push_back(tr.name);
            }
        }
        return names;
    }

    // Returns all tests result for a given test name.
    std::vector<TestResult> get_test_results_by_name(const std::string &test_name)
    {
        std::vector<TestResult> trs;

        for (const auto &tr : results)
        {
            if (tr.name == test_name)
                trs.push_back(tr);
        }
        return trs;
    }

    // Converts multiple test (regarding the same test) result into one.
    TestResult aggregate(const std::vector<TestResult> test_results)
    {
        assert(!test_results.empty());
        TestResult merged{};

        merged.name       = test_results[0].name;
        merged.op_per_itr = test_results[0].op_per_itr;
        bool first        = true;
        for (const auto &tr : test_results)
        {
            assert(tr.name == merged.name);
            merged.nb_itr += tr.nb_itr;
            merged.total_elapsed_ms += tr.total_elapsed_ms;

            if (first || tr.fastest_itr_micro < merged.fastest_itr_micro)
                merged.fastest_itr_micro = tr.fastest_itr_micro;
            if (first || tr.slowest_itr_micro > merged.slowest_itr_micro)
                merged.slowest_itr_micro = tr.slowest_itr_micro;
            if (first)
                first = false;
        }
        return merged;
    }

    void display()
    {
        std::unique_lock<decltype(lock)> ul(lock);

        for (const auto &test_name : test_names())
        {
            std::cout << "--------------------------------------" << std::endl;
            std::cout << "--- " << test_name << std::endl;
            std::cout << "--------------------------------------" << std::endl;

            if (!display_agg_only_)
            {
                std::cout << "Individual (per threads) results..." << std::endl;
                for (const auto &tr : get_test_results_by_name(test_name))
                {
                    print_test_result(tr);
                }
            }

            std::cout << "Aggregated tests results for all thread" << std::endl;
            print_test_result(aggregate(get_test_results_by_name(test_name)));
        }
    }

  private:
    std::vector<TestResult> results;
    std::mutex lock;

    // Only show aggregated test results, not each threads.
    bool display_agg_only_;
};

TestResultAgg *test_result_aggregator;

static void test_aes_key(size_t payload_size, size_t iterations,
                         const std::string &key_uuid, bool with_signature)
{
    TestResult test_result{.nb_itr = iterations, .op_per_itr = 2};

    std::string test_name = "test_aes" + std::to_string(payload_size);
    if (with_signature)
        test_name += "_with_signature";
    test_result.name = test_name;

    iks::IKSRPCClient rpc(iks::IslogKeyServer::get_global_config());

    // Build payload
    auto payload = ByteVector{};
    for (int i = 0; i < payload_size; ++i)
        payload.push_back(i);
    auto iv = ByteVector{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    ElapsedTimeCounter etc;
    SignatureResult sr;
    SignatureResult *sr_ptr = with_signature ? &sr : nullptr;
    for (uint64_t count = 0; count < test_result.nb_itr; ++count)
    {
        ElapsedTimeCounter itr_etc;

        auto encrypted = rpc.aes_encrypt(payload, key_uuid, iv);
        auto decrypted = rpc.aes_decrypt(encrypted, key_uuid, iv, sr_ptr);
        if (payload != decrypted)
        {
            std::cout << "Failed at operation " << count << std::endl;
            assert(payload == decrypted);
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(15));
        test_result.update_extreme_itr(itr_etc.elapsed_micro());
    }
    test_result.total_elapsed_ms = etc.elapsed();
    test_result_aggregator->record_test_result(test_result);
}

void test_desfire_auth(size_t iterations, const std::string &key_uuid)
{
    TestResult test_result{.nb_itr = iterations, .op_per_itr = 2};
    test_result.name = "desfire_auth";

    using namespace logicalaccess;
    iks::IKSRPCClient rpc(iks::IslogKeyServer::get_global_config());

    ElapsedTimeCounter etc;
    SignatureResult sr;
    for (uint64_t count = 0; count < test_result.nb_itr; ++count)
    {
        ElapsedTimeCounter itr_etc;

        CMSG_DesfireAESAuth_Step1 req;
        req.set_key_uuid(key_uuid);
        req.set_encrypted_random_picc(BufferHelper::getStdString(
            BufferHelper::fromHexString("01020304010203040102030401020304")));
        auto rep = rpc.desfire_auth_aes_step1(req);

        CMSG_DesfireAuth_Step2 req2;
        req2.set_auth_context_id(rep.auth_context_id());
        req2.set_key_uuid(key_uuid);
        req2.set_picc_cryptogram(BufferHelper::getStdString(
            BufferHelper::fromHexString("01020304010203040102030401020304")));

        auto rep2 = rpc.desfire_auth_aes_step2(req2);
        test_result.update_extreme_itr(itr_etc.elapsed_micro());
    }
    test_result.total_elapsed_ms = etc.elapsed();
    test_result_aggregator->record_test_result(test_result);
}

int main(int ac, char **av)
{
    const int itr_count  = 1000;
    const int NB_THREADS = 10;
    /*
        iks::IslogKeyServer::configureGlobalInstance(
            "iksf", 6565, "/home/xaqq/Documents/iks/crypto/arnaud.pem",
            "/home/xaqq/Documents/iks/crypto/arnaud.key",
            "/tmp/iks-crypto/iks-server-intermediate-ca.pem");
    */

    iks::IslogKeyServer::configureGlobalInstance(
        "iksf", 6565, "/home/xaqq/Documents/iks/crypto/certs/my-client-1.pem",
        "/home/xaqq/Documents/iks/crypto/certs/my-client-1.key",
        "/home/xaqq/Documents/iks/crypto/certs/iks-server-intermediate-ca.pem");

    if (ac == 2 && std::string(av[1]) == "--agg")
        test_result_aggregator = new TestResultAgg(true);
    else
        test_result_aggregator = new TestResultAgg(false);


    std::vector<std::thread> threads;
    for (int n = 0; n < NB_THREADS; ++n)
    {
        std::thread t([]() {
            std::string key = "e8c0e771-3db8-4f53-9209-98ba4209ca59";
            // std::string key = "ae6b9177-4ea0-49a0-a91d-ca72a0ab8955";
            // test_aes_key(1024, itr_count, key, false);
            //   test_aes_key(1024, itr_count, key, true);
            test_aes_key(16, itr_count, key, false);
            // test_aes_key(16, itr_count, key, true);
            // test_aes_key(1024 * 10, itr_count, key, false);
            // test_aes_key(1024 * 10, itr_count, key, true);


            // Now with key stored in IKSD
            key = "ae6b9177-4ea0-49a0-a91d-ca72a0ab8955";
            // test_desfire_auth(itr_count, key);
            //           test_aes_key(1024, itr_count, key, false);
            //         test_aes_key(1024, itr_count, key, true);
            //       test_aes_key(16, itr_count, key, false);
            //     test_aes_key(16, itr_count, key, true);
            //          test_aes_key(1024 * 10, itr_count, key, false);
            // test_aes_key(1024 * 10, itr_count, key, true);
        });

        threads.push_back(std::move(t));
    }
    for (auto &t : threads)
        t.join();

    test_result_aggregator->display();
}
