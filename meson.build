# main meson.build

project('cimba', ['c'],
        version : '3.0.0',
        meson_version : '>= 0.64',
        default_options : [
            'warning_level=3',
            'buildtype=release',
            'c_std=c2x',
            'cuda_std=c++17',
        ]
)

# Check for compiler flags availability
# DNDEBUG removes debug asserts
# DNLOGINFO removes all internal Cimba logging
# DNASSERT removes all debug and release asserts (including argument checking)
# DNMXCSR turns off saving and restoring the MXCSR floating point
#         state / control register in all context switches.
cc = meson.get_compiler('c')
# Flags common to all build types
common_flags = [
    '-Wno-pedantic',
    '-fno-semantic-interposition',
    '-ffp-contract=off',
]

if host_machine.system() == 'linux'
    common_flags += '-D_POSIX_C_SOURCE=200809L'
elif host_machine.system() == 'darwin'
    # Keep declarations for Darwin extensions such as pthread_get_stackaddr_np
    # and arc4random_buf visible while building the native runtime.
    common_flags += '-D_DARWIN_C_SOURCE'
endif

if cc.has_argument('-std=c23')
    common_flags += '-std=c23'
elif cc.has_argument('-std=c2x')
    common_flags += '-std=c2x'
else
    warning('Compiler does not support C23/C2x standard')
endif

# Flags per build type
# 'debug'    : full asserts and logging, no optimization
# 'release'  : standard optimized build, release asserts only
buildtype_flags = {
    'debug'   : ['-fno-omit-frame-pointer', '-Og'],
    'release' : ['-DNDEBUG', '-DNLOGINFO', '-O3'],
    'plain'   : [],
}

# No-holds-barred optimized build: meson setup --buildtype=release -Dpgo=use
pgo_flags = [
    '-DNDEBUG',
    '-DNLOGINFO',
    '-DNASSERT',
    '-DNMXCSR',
    '-fprofile-use',
    '-fprofile-correction',
]

# Need to generate profile before using it
pgo_gen_flags = [
    '-DNDEBUG',
    '-DNLOGINFO',
    '-DNASSERT',
    '-DNMXCSR',
    '-fprofile-generate',
    '-fno-omit-frame-pointer',
]

buildtype = get_option('buildtype')
desired_flags = common_flags + buildtype_flags.get(buildtype, [])

if cc.get_id() == 'gcc'
    # Suppress spurious gcc warnings related to its own glibc macros
     desired_flags += ['-Wno-clobbered']
endif

if get_option('pgo') == 'generate'
    desired_flags += pgo_gen_flags
elif get_option('pgo') == 'use'
    desired_flags += pgo_flags
endif

# Matches CMB_THREAD_LOCAL in cmi_config.h: initial-exec on x86-64 Linux only
if host_machine.system() == 'linux' and host_machine.cpu_family() == 'x86_64'
    desired_flags += ['-ftls-model=initial-exec' ]
endif

if host_machine.system() == 'windows'
    desired_flags += ['-fno-plt',
                      '-fcf-protection=none',
                      '-fno-stack-protector']
endif

supported_flags = cc.get_supported_arguments(desired_flags)
add_project_arguments(supported_flags, language : 'c')
if '-fprofile-generate' in supported_flags
    add_project_link_arguments('-fprofile-generate', language : 'c')
endif

nasm_args = []
if '-DNMXCSR' in supported_flags
    nasm_args += '-DNMXCSR'
endif

# Need to link with the math library, linker flag -lm
math_dep = cc.find_library('m', required : false)
project_deps = []
if math_dep.found()
    project_deps += math_dep
endif

if get_option('external_context_switch_wrapper')
    add_project_arguments(
        '-DCMI_EXTERNAL_CONTEXT_SWITCH_WRAPPER',
        language : 'c',
    )
endif

# Print a configuration summary
summary({'PGO'             : get_option('pgo'),
         'Release asserts' : get_option('pgo') == 'none',
         'MXCSR isolation' : get_option('pgo') == 'none'},
        section : 'Optimisation', bool_yn : true)

# Build Cimba
subdir('include')
subdir('codegen')
subdir('src')
if get_option('enable_extras')
    cuda_dep = disabler()
    if get_option('enable_cuda_tutorials')
        add_languages('cuda', required : false)
        cuda_dep = dependency('cuda', required : false, modules : ['cudart'])
        if cuda_dep.found()
            add_project_arguments(['-arch=sm_86', '-ccbin=/usr/bin/g++'], language : 'cuda')
        endif
    endif
    subdir('test')
    subdir('tutorial')
    subdir('benchmark')
endif

pkg = import('pkgconfig')
pkg.generate(
    cimba_lib,
    name : 'cimba',
    description : 'Multithreaded discrete event simulation',
    url : 'https://github.com/ambonvik/cimba'
)

if get_option('enable_docs')
    subdir('docs')
endif
