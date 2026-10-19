# src/meson.build

# Platform independent library source code
sources = files('cimba.c',
                'cmb_assert.c',
                'cmb_buffer.c',
                'cmb_condition.c',
                'cmb_dataset.c',
                'cmb_datasummary.c',
                'cmb_event.c',
                'cmb_logger.c',
                'cmb_objectqueue.c',
                'cmb_priorityqueue.c',
                'cmb_process.c',
                'cmb_random.c',
                'cmb_resource.c',
                'cmb_resourceguard.c',
                'cmb_resourcepool.c',
                'cmb_timeseries.c',
                'cmb_wtdsummary.c',
                'cmi_coroutine.c',
                'cmi_hashheap.c',
                'cmi_holdable.c',
                'cmi_mempool.c',
                'cmi_memregistry.c',
                'cmi_resourcebase.c',
                'cmi_thread.c'
)

inc_int = include_directories('.')

# Internal headers installed alongside the public API.
# Every header reachable by #include from a public header must
# appear here. The linux-install CI job enforces this by compiling each public
# header standalone against the installed tree only (see test/tools/verify_install.sh).
install_headers(
    'cmi_config.h',
    'cmi_coroutine.h',
    'cmi_dlist.h',
    'cmi_hashheap.h',
    'cmi_holdable.h',
    'cmi_slist.h',
    'cmi_memregistry.h',
    'cmi_memutils.h',
    'cmi_resourcebase.h',
    'cmi_process.h',
    'cmi_thread.h'
)

# Platform and operating system dependent source code
subdir('port')

# Link-time optimisation of the library, by default in release builds only
# Not available on Windows due to the usual DLL issues
cimba_lto = []
cimba_lto_args = []
lto_feature = get_option('lto')
if host_machine.system() == 'windows'
    cimba_lto = ['b_lto=false']
    if lto_feature.enabled()
        warning('-Dlto=enabled is ignored on Windows: LTO breaks MinGW DLL exports')
    endif
elif lto_feature.enabled() or (lto_feature.auto() and get_option('buildtype').startswith('release'))
    # A static library is linked into consumers that are not themselves built
    # with LTO, so it needs fat objects. Apple clang accepts
    # -ffat-lto-objects on its own and then rejects it once -flto is set
    # ("unsupported option '-ffat-lto-objects'"), so probe the pair.
    if get_option('default_library') == 'shared'
        cimba_lto = ['b_lto=true']
    elif cc.has_multi_arguments('-flto', '-ffat-lto-objects')
        cimba_lto = ['b_lto=true']
        cimba_lto_args = ['-ffat-lto-objects']
    elif lto_feature.enabled()
        warning('-Dlto=enabled is ignored: this compiler cannot emit fat LTO objects for a static library')
    endif
endif

# Print a configuration summary
summary({'LTO (libcimba)' : cimba_lto.contains('b_lto=true')}, section : 'Optimisation')

cimba_lib = library('cimba',
                    sources,
                    include_directories : [inc_api, inc_int],
                    dependencies : [project_deps, codegen_deps],
                    install : true,
                    version : meson.project_version(),
                    soversion : '1',
                    c_args : cimba_lto_args,
                    override_options : cimba_lto
)

my_lib_dep = declare_dependency(
                    link_with : cimba_lib,
                    include_directories : [inc_api, inc_int]
)
