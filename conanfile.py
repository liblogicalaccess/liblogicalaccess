from conans import ConanFile, CMake, tools

class LLAConan(ConanFile):
    name = "LogicalAccess"
    version = "2.1.0"
    license = "https://github.com/islog/liblogicalaccess/blob/master/LICENSE.LibLogicalAccess.txt"
    url = "https://github.com/islog/liblogicalaccess"
    description = "ISLOG RFID library"
    settings = "os", "compiler", "build_type", "arch"
    requires = 'boost/1.68.0@conan/stable', 'OpenSSL/1.0.2o@conan/stable'
    generators = "cmake"
    options = {'LLA_BUILD_IKS': [True, False],
               'LLA_BUILD_PKCS': [True, False],
               'LLA_BUILD_UNITTEST': [True, False],
               'LLA_BUILD_RFIDEAS': [True, False]}
    exports_sources = "plugins*", "src*", "include*", "CMakeLists.txt", "cmake*", "liblogicalaccess.config", "tests*"
    
    if tools.os_info.is_windows:
        default_options = '''
        OpenSSL:shared=True
        boost:shared=False
        gtest:shared=True
        LLA_BUILD_IKS=True
        LLA_BUILD_PKCS=True
        LLA_BUILD_RFIDEAS=True
        LLA_BUILD_UNITTEST=False'''
    else:
        default_options = '''
        OpenSSL:shared=True
        boost:shared=True
        gtest:shared=True
        LLA_BUILD_IKS=True
        LLA_BUILD_PKCS=True
        LLA_BUILD_UNITTEST=False'''

    def configure(self):
        if self.settings.os != 'Windows':
            # This options is not used on Linux
            del self.options.LLA_BUILD_RFIDEAS
       
    def requirements(self):
        if tools.os_info.is_windows and self.options.LLA_BUILD_RFIDEAS:
            self.requires('rfideas/7.1.5@islog/stable')
        if self.options.LLA_BUILD_IKS:
            self.requires('grpc/1.14.1@inexorgame/stable')
        if self.options.LLA_BUILD_UNITTEST:
            self.requires('gtest/1.8.1@bincrafters/stable')
        if self.options.LLA_BUILD_PKCS:
            self.requires('cppkcs11/1.0@islog/master')

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
        
        if self.options.LLA_BUILD_IKS:
            cmake.definitions['LLA_BUILD_IKS'] = True
        else:
            cmake.definitions['LLA_BUILD_IKS'] = False

        if self.options.LLA_BUILD_PKCS:
            cmake.definitions['LLA_BUILD_PKCS'] = True
        else:
            cmake.definitions['LLA_BUILD_PKCS'] = False

        if self.options.LLA_BUILD_UNITTEST:
            cmake.definitions['LLA_BUILD_UNITTEST'] = True
        else:
            cmake.definitions['LLA_BUILD_UNITTEST'] = False
            
        if 'LLA_BUILD_RFIDEAS' in self.options and self.options.LLA_BUILD_RFIDEAS:
            cmake.definitions['LLA_BUILD_RFIDEAS'] = True
        else:
            cmake.definitions['LLA_BUILD_RFIDEAS'] = False

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
            self.cpp_info.libs.append('samav2cards')
            self.cpp_info.libs.append('epasscards')
            self.cpp_info.libs.append('seoscards')
            return

        if self.settings.os == 'Windows':
            # Those are some windows specific stuff.
            if self.options.LLA_BUILD_RFIDEAS:
                self.cpp_info.libs.append('rfideasreaders')
                self.cpp_info.libs.append('keyboardreaders')
            if self.settings.arch == 'x86_64':
                self.cpp_info.libs.append('islogkbdhooklib64')
            else:
                self.cpp_info.libs.append('islogkbdhooklib32')
        
        # Linux / Windows common plugins.
        self.cpp_info.libs.append('llacommon')
        self.cpp_info.libs.append('logicalaccess-cryptolib')
        self.cpp_info.libs.append('a3mlgm5600readers')
        self.cpp_info.libs.append('admittoreaders')
        self.cpp_info.libs.append('axesstmc13readers')
        self.cpp_info.libs.append('axesstmclegicreaders')
        self.cpp_info.libs.append('cps3cards')
        self.cpp_info.libs.append('deisterreaders')
        self.cpp_info.libs.append('desfirecards')
        self.cpp_info.libs.append('elatecreaders')
        self.cpp_info.libs.append('em4102cards')
        self.cpp_info.libs.append('em4135cards')
        self.cpp_info.libs.append('felicacards')
        self.cpp_info.libs.append('generictagcards')
        self.cpp_info.libs.append('gigatmsreaders')
        self.cpp_info.libs.append('gunneboreaders')
        self.cpp_info.libs.append('icode1cards')
        self.cpp_info.libs.append('icode2cards')
        self.cpp_info.libs.append('idondemandreaders')
        self.cpp_info.libs.append('indalacards')
        self.cpp_info.libs.append('infineonmydcards')
        self.cpp_info.libs.append('iso15693cards')
        self.cpp_info.libs.append('iso7816cards')
        self.cpp_info.libs.append('iso7816readers')
        self.cpp_info.libs.append('legicprimecards')
        self.cpp_info.libs.append('logicalaccess')
        self.cpp_info.libs.append('mifarecards')
        self.cpp_info.libs.append('mifarepluscards')
        self.cpp_info.libs.append('mifareultralightcards')
        self.cpp_info.libs.append('ok5553readers')
        self.cpp_info.libs.append('osdpreaders')
        self.cpp_info.libs.append('pcscreaders')
        self.cpp_info.libs.append('promagreaders')
        self.cpp_info.libs.append('proxcards')
        self.cpp_info.libs.append('proxlitecards')
        self.cpp_info.libs.append('rplethreaders')
        self.cpp_info.libs.append('samav2cards')
        self.cpp_info.libs.append('scielreaders')
        self.cpp_info.libs.append('seoscards')
        self.cpp_info.libs.append('smartframecards')
        self.cpp_info.libs.append('smartidreaders')
        self.cpp_info.libs.append('stidprgreaders')
        self.cpp_info.libs.append('stidstrreaders')
        self.cpp_info.libs.append('stmlri512cards')
        self.cpp_info.libs.append('tagitcards')
        self.cpp_info.libs.append('topazcards')
        self.cpp_info.libs.append('twiccards')
        self.cpp_info.libs.append('epasscards')

    def package_id(self):
        self.info.requires["boost"].full_package_mode()
        self.info.requires["OpenSSL"].full_package_mode()
