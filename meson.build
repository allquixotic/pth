project('gnu-pth', 'c',
  version: '2.0.8',
  license: 'LGPL-2.1-or-later',
  default_options: [
    'c_std=c17',
    'warning_level=2',
    'werror=true',
    'optimization=2',
  ],
  meson_version: '>= 0.56.0'
)

cc = meson.get_compiler('c')

# Project metadata
project_description = 'GNU Portable Threads - A portable threading library for UNIX'
project_url = 'https://www.gnu.org/software/pth/'

# Version information
version_array = meson.project_version().split('.')
version_major = version_array[0].to_int()
version_minor = version_array[1].to_int()
version_patch = version_array[2].to_int()
version_hex = '0x@0@@1@@2@07'.format(
  version_major.to_string().substring(0, 1),
  version_minor.to_string().substring(0, 1),
  version_patch.to_string().substring(0, 1)
)

# Configuration data
conf_data = configuration_data()
conf_data.set('PACKAGE_NAME', 'gnu-pth')
conf_data.set('PACKAGE_VERSION', meson.project_version())
conf_data.set('PACKAGE_STRING', 'gnu-pth @0@'.format(meson.project_version()))
conf_data.set('PACKAGE_BUGREPORT', 'bug-pth@gnu.org')
conf_data.set('PACKAGE_URL', project_url)
conf_data.set('PTH_VERSION_STR', meson.project_version())
conf_data.set('PTH_VERSION', version_hex)

# Check for required headers
required_headers = [
  'stdio.h',
  'stdlib.h',
  'stdarg.h',
  'string.h',
  'setjmp.h',
  'signal.h',
  'unistd.h',
  'fcntl.h',
  'errno.h',
  'ctype.h',
  'sys/types.h',
  'sys/time.h',
  'sys/wait.h',
  'sys/stat.h',
  'sys/socket.h',
  'time.h',
  'stdint.h',
]

foreach h : required_headers
  if cc.has_header(h)
    conf_data.set('HAVE_@0@'.format(h.underscorify().to_upper()), 1)
  endif
endforeach

# Check for optional headers
optional_headers = [
  'sys/resource.h',
  'dlfcn.h',
  'paths.h',
  'poll.h',
  'sys/uio.h',
  'sys/select.h',
  'ucontext.h',
  'inttypes.h',
]

foreach h : optional_headers
  if cc.has_header(h)
    conf_data.set('HAVE_@0@'.format(h.underscorify().to_upper()), 1)
  endif
endforeach

# Check for required functions
required_functions = [
  'gettimeofday',
  'memset',
  'select',
  'socket',
  'malloc',
]

foreach f : required_functions
  if cc.has_function(f)
    conf_data.set('HAVE_@0@'.format(f.to_upper()), 1)
  endif
endforeach

# Check for context switching functions
have_mcsc = false
if cc.has_header('ucontext.h')
  if cc.has_function('getcontext') and cc.has_function('makecontext') and cc.has_function('swapcontext')
    conf_data.set('HAVE_MCSC', 1)
    conf_data.set('HAVE_GETCONTEXT', 1)
    conf_data.set('HAVE_MAKECONTEXT', 1)
    conf_data.set('HAVE_SWAPCONTEXT', 1)
    have_mcsc = true
  endif
endif

if not have_mcsc
  conf_data.set('HAVE_SJLJ', 1)
endif

# Check for optional functions
optional_functions = [
  'dlopen',
  'dlclose',
  'dlsym',
]

foreach f : optional_functions
  if cc.has_function(f, dependencies: cc.find_library('dl', required: false))
    conf_data.set('HAVE_@0@'.format(f.to_upper()), 1)
  endif
endforeach

# Additional defines
conf_data.set('STDC_HEADERS', 1)
conf_data.set('_GNU_SOURCE', 1)

# Generate config.h
configure_file(
  output: 'config.h',
  configuration: conf_data
)

# Generate pth_acdef.h
acdef_data = configuration_data()
foreach key : conf_data.keys()
  if key.startswith('HAVE_')
    acdef_data.set(key, conf_data.get(key))
  endif
endforeach

configure_file(
  input: 'pth_acdef.h.in',
  output: 'pth_acdef.h',
  configuration: acdef_data
)

# Include directories
inc_dirs = include_directories('.')

# Compiler flags for C17
add_project_arguments(
  '-D_POSIX_C_SOURCE=200809L',
  '-D_XOPEN_SOURCE=700',
  language: 'c'
)

# Note: pth_p.h is pre-generated and checked into source control
# It contains extracted #if cpp blocks from all source files

# Library sources
pth_sources = files(
  'pth_attr.c',
  'pth_cancel.c',
  'pth_clean.c',
  'pth_compat.c',
  'pth_data.c',
  'pth_debug.c',
  'pth_errno.c',
  'pth_event.c',
  'pth_ext.c',
  'pth_fork.c',
  'pth_high.c',
  'pth_lib.c',
  'pth_mctx.c',
  'pth_mctx_swap.S',
  'pth_msg.c',
  'pth_pqueue.c',
  'pth_ring.c',
  'pth_sched.c',
  'pth_string.c',
  'pth_sync.c',
  'pth_syscall.c',
  'pth_tcb.c',
  'pth_time.c',
  'pth_uctx.c',
  'pth_util.c',
  'pth_vers.c',
)

# Build shared library
pth_lib = shared_library(
  'pth',
  pth_sources,
  version: meson.project_version(),
  soversion: '7',
  include_directories: inc_dirs,
  install: true,
)

# Build static library
pth_static_lib = static_library(
  'pth',
  pth_sources,
  include_directories: inc_dirs,
  install: true,
)

# Declare dependency
pth_dep = declare_dependency(
  link_with: pth_lib,
  include_directories: inc_dirs,
)

# Test programs
test_common_src = files('test_common.c')

tests = {
  'test_std': ['test_std.c', test_common_src],
  'test_mp': ['test_mp.c', test_common_src],
  'test_misc': ['test_misc.c'],
  'test_philo': ['test_philo.c', test_common_src],
  'test_sig': ['test_sig.c'],
  'test_select': ['test_select.c'],
  'test_httpd': ['test_httpd.c', test_common_src],
  'test_sfio': ['test_sfio.c'],
  'test_uctx': ['test_uctx.c'],
  'test_pthread': ['test_pthread.c'],
  'test_coverage': ['test_coverage.c'],
  'test_io': ['test_io.c'],
  'test_semantics': ['test_semantics.c'],
  'test_string': ['test_string.c'],
  'test_events_advanced': ['test_events_advanced.c'],
  'test_io_ev': ['test_io_ev.c'],
  'test_fork': ['test_fork.c'],
  'test_ring': ['test_ring.c'],
}

foreach test_name, test_sources : tests
  test_exe = executable(
    test_name,
    test_sources,
    dependencies: pth_dep,
    include_directories: inc_dirs,
    install: false,
  )

  # Add all tests to the suite with PTH_AUTOTEST environment variable
  # PTH_HTTPD_SECS sets runtime for test_httpd (default 60s is too long)
  test(test_name, test_exe, timeout: 30, env: ['PTH_AUTOTEST=1', 'PTH_HTTPD_SECS=5'])
endforeach

# Generate pkg-config file
pkg = import('pkgconfig')
pkg.generate(
  pth_lib,
  name: 'pth',
  description: project_description,
  url: project_url,
  version: meson.project_version(),
)

# Summary
summary({
  'Machine context method': have_mcsc ? 'MCSC (makecontext/swapcontext)' : 'SJLJ (setjmp/longjmp)',
  'C standard': 'C17',
  'Prefix': get_option('prefix'),
  'Library directory': get_option('libdir'),
}, section: 'Configuration')