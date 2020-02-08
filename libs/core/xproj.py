from pathlib import Path
from pyxdev.project import XDevProject

here = Path(__file__).parent


def glob(pattern):
    return [p.relative_to(here) for p in here.glob(pattern)]


class XDevCore(XDevProject):
    name = "xdev-core"
    version = "0.1.0"
    license = "MIT"
    author = "Garcia Sylvain <garcia dot 6l20 at gmail dot com>"
    url = "https://github.com/Garcia6l20/xdev"
    description = "XDev framework"
    topics = ("networking", "http", "boost")
    requires = ("boost/1.71.0@conan/stable", "openssl/1.0.2s", "zlib/1.2.11")
    default_options = {"shared": False, "boost:shared": False, "OpenSSL:shared": False}
    exports_sources = "include/*"
    # no_copy_source = True

    libraries = {
        name: {
            'sources': glob('include/*.h*') + glob('src/*.cpp') + glob('inline/*.inl')
        }
    }
