from conans import ConanFile, CMake, tools

class LLAConan(ConanFile):
    name = "LogicalAccess"
    version = "3.0.0"
    license = "https://github.com/liblogicalaccess/liblogicalaccess/blob/develop/LICENSE"
    url = "https://github.com/liblogicalaccess/liblogicalaccess"
    description = "LLA RFID library"
    settings = "os", "compiler", "build_type", "arch"
    requires = 'boost/1.83.0', 'openssl/1.1.1w', 'nlohmann_json/3.11.3', 'zlib/1.3'
    generators = "cmake"
    options = {'LLA_BUILD_PKCS': [True, False],
               'LLA_BUILD_UNITTEST': [True, False],
               'LLA_BUILD_LIBUSB': [True, False]}
    revision_mode = "scm"
    exports_sources = "plugins*", "src*", "include*", "CMakeLists.txt", "cmake*", "liblogicalaccess.config", "tests*", "samples*"
    
    if tools.os_info.is_windows:
        default_options = '''
        openssl:shared=True
        openssl:no_asm=True
        boost:shared=False
        gtest:shared=True
        LLA_BUILD_PKCS=True
        LLA_BUILD_UNITTEST=False
        LLA_BUILD_LIBUSB=False'''
    else:
        default_options = '''
        openssl:shared=True
        boost:shared=True
        gtest:shared=True
        LLA_BUILD_PKCS=True
        LLA_BUILD_UNITTEST=False
        LLA_BUILD_LIBUSB=False'''
       
    def requirements(self):
        if self.options.LLA_BUILD_UNITTEST:
            self.requires('gtest/1.14.0')
        if self.options.LLA_BUILD_PKCS:
            self.requires('cppkcs11/1.1')
        if self.options.LLA_BUILD_LIBUSB:
            self.requires('libusb/1.0.26')

    def imports(self):
        if tools.os_info.is_windows:
            self.copy("*.dll", "bin", "bin")

    def configure_cmake(self):
        cmake = CMake(self, build_type=self.settings.build_type)
        if self.settings.os == 'Android':
            # Workaround to avoid conan passing -stdlib=libc++
            # to compiler. See https://github.com/conan-io/conan/issues/2856
            cmake.definitions['CONAN_LIBCXX'] = ''
            cmake.definitions['LLA_BOOST_ASIO_HAS_STD_STRING_VIEW'] = 1

        if self.options.LLA_BUILD_PKCS:
            cmake.definitions['LLA_BUILD_PKCS'] = True
        else:
            cmake.definitions['LLA_BUILD_PKCS'] = False

        if self.options.LLA_BUILD_UNITTEST:
            cmake.definitions['LLA_BUILD_UNITTEST'] = True
        else:
            cmake.definitions['LLA_BUILD_UNITTEST'] = False
            
        if self.options.LLA_BUILD_LIBUSB:
            cmake.definitions['LLA_BUILD_LIBUSB'] = True
        else:
            cmake.definitions['LLA_BUILD_LIBUSB'] = False

        cmake.definitions['LIBLOGICALACCESS_VERSION_STRING'] = self.version
        cmake.definitions['LIBLOGICALACCESS_WINDOWS_VERSION'] = self.version.replace('.', ',') + ',0'
        cmake.definitions['TARGET_ARCH'] = self.settings.arch		

        if tools.os_info.is_windows:
            # For MSVC we need to restrict configuration type to avoid issues.
            cmake.definitions['CMAKE_CONFIGURATION_TYPES'] = self.settings.build_type

        cmake.configure()
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()

    def package_info(self):
        if self.settings.os == 'Android':
            # For Android we only package a subsets of libraries.
            self.cpp_info.libs.append('logicalaccess')
            self.cpp_info.libs.append('llacommon')
            self.cpp_info.libs.append('logicalaccess-cryptolib')
            self.cpp_info.libs.append('iso7816readers')
            self.cpp_info.libs.append('desfirecards')
            self.cpp_info.libs.append('mifarecards')
            self.cpp_info.libs.append('iso7816cards')
            self.cpp_info.libs.append('samavcards')
            self.cpp_info.libs.append('epasscards')
            self.cpp_info.libs.append('seoscards')
            return

        if self.settings.os == 'Windows':
            # Those are some windows specific stuff.
            self.cpp_info.libs.append('keyboardreaders')
            if self.settings.arch == 'x86_64':
                self.cpp_info.libs.append('islogkbdhooklib64')
            else:
                self.cpp_info.libs.append('islogkbdhooklib32')
        
        # Linux / Windows common plugins.
        self.cpp_info.libs.append('llacommon')
        self.cpp_info.libs.append('logicalaccess-cryptolib')
        self.cpp_info.libs.append('cps3cards')
        self.cpp_info.libs.append('deisterreaders')
        self.cpp_info.libs.append('desfirecards')
        self.cpp_info.libs.append('elatecreaders')
        self.cpp_info.libs.append('em4102cards')
        self.cpp_info.libs.append('em4135cards')
        self.cpp_info.libs.append('felicacards')
        self.cpp_info.libs.append('generictagcards')
        self.cpp_info.libs.append('gunneboreaders')
        self.cpp_info.libs.append('icode1cards')
        self.cpp_info.libs.append('icode2cards')
        self.cpp_info.libs.append('indalacards')
        self.cpp_info.libs.append('infineonmydcards')
        self.cpp_info.libs.append('iso15693cards')
        self.cpp_info.libs.append('iso7816cards')
        self.cpp_info.libs.append('iso7816readers')
        self.cpp_info.libs.append('legicprimecards')
        if self.options.LLA_BUILD_LIBUSB:
            self.cpp_info.libs.append('libusbreaders')
        self.cpp_info.libs.append('logicalaccess')
        self.cpp_info.libs.append('mifarecards')
        self.cpp_info.libs.append('mifarepluscards')
        self.cpp_info.libs.append('mifareultralightcards')
        self.cpp_info.libs.append('ok5553readers')
        self.cpp_info.libs.append('osdpreaders')
        self.cpp_info.libs.append('pcscreaders')
        self.cpp_info.libs.append('proxcards')
        self.cpp_info.libs.append('proxlitecards')
        self.cpp_info.libs.append('samavcards')
        self.cpp_info.libs.append('seoscards')
        self.cpp_info.libs.append('smartframecards')
        self.cpp_info.libs.append('stidstrreaders')
        self.cpp_info.libs.append('stmlri512cards')
        self.cpp_info.libs.append('tagitcards')
        self.cpp_info.libs.append('topazcards')
        self.cpp_info.libs.append('twiccards')
        self.cpp_info.libs.append('epasscards')
        self.cpp_info.libs.append('yubikeycards')

    def package_id(self):
        self.info.requires["boost"].full_package_mode()
        self.info.requires["openssl"].full_package_mode()
