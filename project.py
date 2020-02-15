#!/usr/bin/env python3
import platform
from pathlib import Path

from cpppm import Project, main
from cpppm.library import Library
from cpppm.target import Target
from git import Repo

win32 = platform.system() == 'Windows'


def get_dependency(path, url):
    path = Path(path)
    if not path.exists():
        repo = Repo.clone_from(url, to_path=path)
    else:
        repo = Repo(path)
    repo.remotes.origin.pull()
    return path, repo


ctti, _ = get_dependency('.deps/ctti', 'https://github.com/Manu343726/ctti.git')

project = Project('xdev-core')
project.requires = 'boost/1.71.0@conan/stable', 'fmt/6.1.2', 'gtest/1.10.0'
project.requires_options.update({
    'boost:header_only': True
})


def basic_target_setup(target: Target):
    target.sources.glob('include/**/*.hpp')
    target.sources.glob('inline/**/*.inl')
    target.sources.glob('src/*.cpp')
    target.include_dirs = 'include'
    if target.source_path.joinpath('inline').exists():
        target.include_dirs = 'inline'
    if isinstance(target, Library):
        target.export_header = f'xdev/{target.name}-export.hpp'


def add_test(parent: Target, name):
    target = project.executable(f'{parent.name}-test-{name}', root=f'{parent.source_path}/tests')
    target.sources = 'test-events.cpp'
    target.link_libraries = parent, 'gtest'
    return target


core = project.main_library(root='libs/core')
core.static = True
basic_target_setup(core)
core.include_dirs = ctti.absolute() / 'include'
core.link_libraries = 'fmt', 'boost', 'dl'
core.subdirs = ctti.absolute()

if win32:
    core.compile_options = '/std:c++latest'
    dlfcn, _ = get_dependency('.deps/dlfcn', 'https://github.com/dlfcn-win32/dlfcn-win32.git')
    core.subdirs = dlfcn.absolute()
    core.include_dirs = dlfcn.absolute()
else:
    core.compile_options = '-std=c++2a', '-fconcepts'
    core.compile_options = '-fPIC'

template = project.library('xdev-template', root='libs/template')
basic_target_setup(template)
template.link_libraries = core

project.default_executable = add_test(core, 'test-events')

main()
