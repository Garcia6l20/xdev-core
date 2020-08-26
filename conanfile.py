from conans import ConanFile, CMake, tools


class MeltConanFile(ConanFile):
    name = "xdev-core"
    license = "MIT"
    author = "Garcia Sylvain <garcia.6l20@gmail.com>"
    url = "https://github.com/Garcia6l20/xdev-core"
    description = "The XDev framework aims to ease modern c++ development, providing reflection, templating, serialization, and much more."
    topics = ("conan", "serialization", "reflection", "templating")
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "cmake_find_package"
    scm = {
        "type": "git",  # Use "type": "svn", if local repo is managed using SVN
        "url": "https://github.com/Garcia6l20/xdev-core",
        "revision": "auto"
    }
    requires = "boost/1.72.0", "fmt/6.1.2", "lyra/1.3.0", "spdlog/1.5.0", "pybind11/2.3.0@conan/stable", "ctre/2.8.2", "benchmark/1.5.0"
    build_requires = "catch2/2.11.0"
    options = {"fPIC": [True, False], "shared": [True, False], }
    default_options = {
        "fPIC": True,
        "shared": False,
        "boost:header_only": True
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
        self.cmake.definitions["BUILD_TESTING"] = "ON" if self.run_tests else "OFF"
        self.cmake.configure()
        return self.cmake

    def configure(self):
        tools.check_min_cppstd(self, "20")
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
        self.cpp_info.build_modules.extend(["cmake/xdev.cmake"])
