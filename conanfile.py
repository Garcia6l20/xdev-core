from conans import ConanFile, CMake, tools


class XdevBaseConan(ConanFile):
    name = "xdev-core"
    version = "0.1.0"
    license = "MIT"
    author = "Sylvain Garcia <garcia.6l20@gmail.com>"
    url = "https://github.com/Garcia6l20/xdev-base"
    description = "@PACKAGE_DESCRIPTION@"
    topics = ("xdev", "reflexion", "templating", "moderncpp")
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    scm = {
     "type": "git",  # Use "type": "svn", if local repo is managed using SVN
     "url": "auto",
     "revision": "auto"
    }
    requires = (
        'boost/1.71.0',
        'openssl/1.1.1d',
    )
    build_requires = ('gtest/1.10.0')
    options = {"shared": [True, False]}
    default_options = {"shared": False, "boost:shared": False, "OpenSSL:shared": False}
    exports_sources = "include/*"
    no_copy_source = True

    def _cmake(self):
        if hasattr(self, 'cmake'):
            return self.cmake
        self.cmake = CMake(self)
        self.run_tests = tools.get_env("CONAN_RUN_TESTS", False)
        self.cmake.definitions["XDEV_UNIT_TESTING"] = "ON" if self.run_tests else "OFF"
        self.cmake.configure()
        return self.cmake

    def configure(self):
        self.settings.compiler.cppstd = "20"
        tools.check_min_cppstd(self, "20")

    def build(self):
        cmake = self._cmake()
        cmake.build()
        if self.run_tests:
            cmake.test()

    def package(self):
        cmake = self._cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
