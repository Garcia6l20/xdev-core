from conans import ConanFile, CMake


class XdevCoreTestConan(ConanFile):
    name = 'xdev-core-test-package'
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    requires = "xdev-core/0.1.0"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")

    def test(self):
        self.run("./bin/xdev-core-test-package")
