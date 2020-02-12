from conans import ConanFile, CMake, tools


class XdevBaseConan(ConanFile):
    name = "xdev-core"
    license = "MIT"
    author = "Sylvain Garcia <garcia.6l20@gmail.com>"
    url = "https://github.com/Garcia6l20/xdev-core"
    description = "@PACKAGE_DESCRIPTION@"
    topics = ("xdev", "reflexion", "templating", "moderncpp")
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    scm = {
        "type": "git",  # Use "type": "svn", if local repo is managed using SVN
        "url": "https://github.com/Garcia6l20/xdev-core.git",
        "revision": "auto"
    }
    requires = 'boost/1.71.0@conan/stable', 'fmt/6.1.2', 'openssl/1.1.1d', 'zlib/1.2.11', 'bzip2/1.0.8'
    build_requires = 'gtest/1.10.0'
    options = {"fPIC": [True, False], "shared": [True, False]}
    default_options = {"fPIC": True,
                       "shared": False,
                       "OpenSSL:shared": False,
                       "boost:error_code_header_only": True,
                       "boost:without_chrono": True,
                       "boost:without_coroutine": True,
                       "boost:without_filesystem": True,
                       "boost:without_graph": True,
                       "boost:without_graph_parallel": True,
                       "boost:without_iostreams": True,
                       "boost:without_locale": True,
                       "boost:without_log": True,
                       "boost:without_math": True,
                       "boost:without_mpi": True,
                       "boost:without_program_options": True,
                       "boost:without_python": True,
                       "boost:without_random": True,
                       "boost:without_regex": True,
                       "boost:without_thread": True,
                       "boost:without_timer": True,
                       "boost:without_type_erasure": True,
                       "boost:without_wave": True,
                       }
    no_copy_source = True

    def set_version(self):
        git = tools.Git(folder=self.recipe_folder)
        self.version = str(git.get_tag())[1:] if git.get_tag() is not None else git.get_revision()

    def deploy(self):
        self.copy("*", dst="bin", src="bin")

    def _cmake(self):
        if hasattr(self, 'cmake'):
            return self.cmake
        self.cmake = CMake(self)
        self.run_tests = tools.get_env("CONAN_RUN_TESTS", False)
        self.cmake.definitions["XDEV_UNIT_TESTING"] = "ON" if self.run_tests else "OFF"
        self.cmake.configure()
        return self.cmake

    def configure(self):
        # tools.check_min_cppstd(self, "20")
        if self.settings.os == "Windows":
            del self.options.fPIC

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
        self.cpp_info.bindirs = ['bin']
        self.cpp_info.build_modules.append('cmake/xdev.cmake')
