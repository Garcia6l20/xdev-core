from pathlib import Path
from conans import ConanFile, CMakeSelf

here = Path(__file__).parent


def glob(pattern):
    return [p.relative_to(here) for p in here.glob(pattern)]


class XDevCore(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    name = "xdev-core"
    version = "0.1.0"
    license = "MIT"
    author = "Garcia Sylvain <garcia dot 6l20 at gmail dot com>"
    url = "https://github.com/Garcia6l20/xdev"
    description = "XDev framework"
    topics = ("networking", "http", "boost")
    requires = ("boost/1.71.0@conan/stable", "openssl/1.0.2s", "zlib/1.2.11")
    scm = {
        "type": "git",  # Use "type": "svn", if local repo is managed using SVN
        "url": "auto",
        "revision": "auto"
    }
    options = {"shared": [True, False]}
    default_options = {"shared": False, "boost:shared": False, "OpenSSL:shared": False}
    exports_sources = "include/*"
    no_copy_source = True
    generators = "cmake_self"

    fetch_content = {
        'ctti': {
            'repository': 'git@github.com:Manu343726/ctti.git',
            'tag': 'master'
        },
    }
    libraries = {
        name: {
            'sources': glob('include/*.h*') + glob('src/*.cpp') + glob('inline/*.inl'),
            'deps': {
                'boost',
                'ctti',
                'dl',
            }
        },
    }

    def configure(self):
        self.settings.compiler.cppstd = 20
        if self.settings.os == "Windows":
            self.fetch_content.update({
                'dlfcn-win32': {
                    'repository': 'https://github.com/dlfcn-win32/dlfcn-win32.git',
                    'tag': '974b39c7a481e2b2cf767090ebd6c104ad16c4b3'
                }
            })

    def build(self):
        cmake = CMakeSelf(self)
        cmake.configure()
        cmake.build()
