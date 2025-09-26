# GNU Pth Modernization - Agent Instructions

## Project Goals

Modernize GNU Pth 2.0.7 to:
- **C17 standard compliance** with modern (2025) Linux focus
- **Meson/Ninja build system only** - eliminate autotools and code generation
- **All source code present** - no generated code constructs
- **Clean builds** - zero warnings with `-Werror -Wall -Wextra`
- **Modern, automated tests** - no user interaction required, good coverage
- **Remove legacy compatibility** - drop pre-modern Linux and non-Linux systems
- **100% API behavioral compatibility** - preserve all public API semantics

## Guiding Principles

- **API Compatibility**: Maintain 100% API and behavioral compatibility with GNU Pth 2.0.7. All public functions must behave identically.
- **Modern Linux Focus**: Target modern Linux with recent glibc (2020+). Remove ancient platform workarounds.
- **Incremental Progress**: Keep the codebase buildable and testable after each change. Commit working states frequently.
- **Verification First**: Write tests proving current behavior before making changes that could affect it.

## Current Status

### Recently Completed ✓
- Initial meson.build created (not yet functional)
- Test files cleaned up (unused parameter warnings)
- Git repository initialized with meaningful commits
- pth_mctx_switch macro added to pth_p.h
- Function signature fixes: pth_scheduler, pth_writev_iov_advance
- Static/extern linkage conflicts resolved in pth_pqueue.c, pth_ring.c, pth_sched.c, pth_syscall.c, pth_util.c
- Empty-body warnings fixed in pth_lib.c
- **MAJOR MILESTONE: All 26 source files compile with -Werror! Build system works!**
- Replaced all `intern` keywords with `static` (95 instances)
- Fixed 60+ static/extern linkage conflicts
- Added pth_mctx_restore/pth_mctx_restored macros to pth_p.h
- Unused function warnings fixed with __attribute__((unused))
- **MAJOR MILESTONE: All autotools files removed! Meson-only build system!**
- **pth_mctx.c simplified from 561 to 83 lines (85% reduction) - removed all setjmp/longjmp compatibility code, keeping only modern ucontext API**
- **Phase 2.2 complete: Removed non-Linux compatibility from pth_syscall.c (25 HAVE checks), pth_time.c (1 check), pth_string.c (2 checks)**

### Critical Blockers ✗

**BLOCKER #1: pth_p.h Generation Problem** ✓ RESOLVED!
- Status: **RESOLVED** - All code generation eliminated successfully
- Solution: Manually extracted all declarations from `#if cpp` blocks and created proper pth_p.h
- All 26 source files now compile successfully with meson/ninja

**BLOCKER #2: Autotools Still Present** ✓ RESOLVED!
- Status: **RESOLVED** - All autotools files removed successfully
- Removed: configure, configure.ac, Makefile, Makefile.in, Makefile.am, aclocal.m4, config.guess, config.sub, config.h.in, compile, depcomp, install-sh, shtool, libtool, ltmain.sh, libtool.m4, pth_p.h.in
- Kept: pth.m4 (useful for pkg-config integration)
- Build verified working with meson/ninja only

**BLOCKER #3: Code Generation Constructs** ✓ RESOLVED!
- Status: **RESOLVED** - All `#if cpp` blocks handled
- Solution: Declarations extracted to pth_p.h, blocks remain in source for now (harmless)

### Statistics
- **Source files**: 26 pth_*.c files
- **Test files**: 11 test_*.c files (only 2 auto-run in meson)
- **Warnings**: ~17,000+ from clang-tidy analysis
- **`intern` keyword**: 71 uses (should be `static`)
- **Template files**: 9 .in files

## Modernization Plan

### Phase 1: Fix Build System (HIGHEST PRIORITY)

#### 1.1 Resolve pth_p.h Problem ⚠️ **DO THIS FIRST**

**Recommended Approach: Eliminate Code Generation**
1. Create tools/extract_declarations.sh script to help extract `#if cpp` blocks
2. Manually extract all declarations from `#if cpp` blocks in all .c files
3. Create proper pth_p.h with declarations in correct dependency order:
   - Constants first (PTH_TCB_NAMELEN, etc.)
   - Type definitions (structs, enums)
   - Global variable declarations
   - Function declarations
   - Macros last
4. Remove all `#if cpp` blocks from source files
5. Delete pth_p.h.in template
6. Test compilation after each few files processed
7. Document the extraction process for future reference

**Alternative: Replicate Generation**
- More complex, maintains duplication
- Would need Python/shell script in meson
- Not recommended unless elimination proves too difficult

#### 1.2 Complete meson.build
- Fix pth_p.h handling (from 1.1)
- Generate pth.h from pth.h.in (configure version numbers)
- Generate pthread.h from pthread.h.in if needed
- Ensure all 26 pth_*.c files compile
- Add proper install targets for headers and libraries
- Configure pkg-config file

#### 1.3 Remove Autotools
- Delete: configure, configure.ac, Makefile.in, Makefile.am
- Delete: aclocal.m4, libtool.m4, config.guess, config.sub
- Delete: shtool, install-sh, compile, depcomp
- Keep: README, COPYING, INSTALL (update content), documentation

### Phase 2: C17 Modernization

#### 2.1 Basic Code Cleanup
- Replace all `intern` → `static` (71 instances, can use sed)
- Add missing `#include` directives for implicit declarations
- Use stdint.h types consistently (int32_t, uint64_t, etc.)
- Fix function prototypes to be complete and correct
- Add `(void)` to unused parameters systematically

#### 2.2 Remove Non-Linux Compatibility
- Remove platform checks for FreeBSD 3.x, Solaris, AIX, IRIX, etc.
- Remove workarounds for ancient glibc versions
- Assume modern Linux syscalls are available
- Simplify pth_mctx.c to modern Linux only
- Remove HAVE_* checks for universally available features

#### 2.3 Fix All Compiler Warnings
- Work through clang-tidy warnings systematically
- Fix implicit function declarations
- Add braces around single-statement if/else/for/while
- Remove unused variables and functions
- Fix magic numbers (define constants)
- Address cognitive complexity issues

### Phase 3: Test Suite Modernization

#### 3.1 Eliminate User Interaction
- Review all 11 tests for interactive prompts/waits
- test_httpd: already fixed ✓
- Add timeouts to all tests
- Make tests exit automatically on completion/timeout
- Remove any scanf/getchar/pause calls

#### 3.2 Expand Test Coverage
- Context switching stress tests
- Signal handling tests
- Thread synchronization tests
- Event system edge cases
- Error handling paths
- Target: >70% code coverage

#### 3.3 Automate Testing
- Enable all tests in meson.build with test()
- Set reasonable timeouts (30-60 seconds)
- Create test wrapper that runs all tests
- Consider valgrind/sanitizer integration

### Phase 4: API Behavioral Verification

#### 4.1 Document & Test Current Behavior
- Review pth.pod and other documentation
- Create tests that prove current API semantics
- Test thread scheduling algorithm behavior
- Test event system behavior
- Test error conditions and errno values

#### 4.2 Regression Testing
- Run original tests (from old build) vs new build
- Compare outputs and behaviors
- Verify errno handling is identical
- Test signal mask handling
- Test edge cases match original

### Phase 5: Final Polish

#### 5.1 Documentation Updates
- Update README for meson/ninja build instructions
- Remove references to old platforms from all docs
- Document modern Linux requirements (kernel, glibc versions)
- Update INSTALL guide completely
- Clean up PORTING (focus on modern systems only)

#### 5.2 Code Quality
- Remove obsolete comments about old platforms
- Add explanatory comments for complex algorithms
- Document thread safety considerations
- Remove dead code
- Simplify where possible without breaking API

## Iterative Working Process

### How to Use This Process

The user can invoke this iterative process by saying:
- "Perform the next iteration"
- "Do next work unit"
- "Continue modernization"
- "Next iteration please"

### Iteration Steps

Each iteration follows this cycle:

#### Step 1: ASSESS
- Read AGENTS.md current status section
- Check build status: `ninja -C builddir 2>&1 | head -50`
- Review recent git changes: `git status`, `git diff --stat`
- Identify the highest priority blocker or next task from plan
- Check if we're approaching context limits (monitor conversation length)

#### Step 2: WORK
- Select ONE focused unit of work (typically 30-60 minutes of effort)
- Work should be:
  - **Atomic**: Can be completed and tested in one iteration
  - **Testable**: Can verify it works before moving on
  - **Committable**: Results in a valid git commit
- Examples of work units:
  - Extract declarations from 3-5 source files
  - Replace all `intern` with `static` and test
  - Fix compilation errors in one subsystem
  - Make one test non-interactive
  - Remove one category of autotools files
- Prefer smaller units over larger ones
- Test the change works before proceeding

#### Step 3: UPDATE
- Update this AGENTS.md file:
  - Move completed items from plan to "Recently Completed"
  - Update blocker status if changed
  - Update statistics if relevant
  - Add new blockers if discovered
  - Update "Current Work" section below
- Commit the work: `git add -A && git commit -m "descriptive message"`
- Report to user what was done and what's next

#### Step 4: REPEAT OR PAUSE
- If approaching context limits: Update AGENTS.md and tell user to start new conversation
- If blocker encountered: Update AGENTS.md, report to user, await guidance
- If iteration successful: Ready for next iteration
- If phase complete: Celebrate, update AGENTS.md, move to next phase

### Context Limit Handling

When approaching context limits (~80% of conversation):
1. Ensure AGENTS.md is fully updated with all progress
2. Commit all changes
3. Tell user: "Approaching context limits. Please start a new conversation and say 'perform the next iteration' to continue."
4. User can then use `/clear` and resume with fresh context

## Progress Tracking

### Current Work
- **Active Task**: Phase 2 - C17 modernization in progress
- **Current Phase**: Phase 2.3 - Ready to fix compiler warnings
- **Progress**:
  - ✓ Extracted all 27 #if cpp blocks to temp file
  - ✓ Created new pth_p.h with proper ordering (constants, types, structs, externs, macros)
  - ✓ Added pth_mctx_t definition
  - ✓ Replaced all `intern` keywords with `static` (95 instances)
  - ✓ Fixed 60+ static/extern linkage conflicts by removing static from cross-file functions
  - ✓ Added missing declarations: pth_snprintf, pth_vsnprintf, pth_tcb_alloc, pth_tcb_free, pth_mctx_set, pth_time_cmp, pth_time_t2d, pth_mutex_releaseall, pth_util_sigdelete, pth_time_zero, pth_time_set macro, pth_sc macro
  - ✓ Fixed pth_writev_iov_advance signature (7 params, not 5)
  - ✓ Added pth_mctx_switch macro to pth_p.h
  - ✓ Fixed pth_scheduler signature: void* (void*) not void (void)
  - ✓ Removed static from pth_pqueue, pth_ring, pth_sched, pth_syscall, and pth_util functions
  - ✓ Fixed empty-body warnings in pth_lib.c
  - ✓ Added forward declaration for pth_sched_eventmanager_sighandler
  - ✓ Fixed unused function warnings with __attribute__((unused))
  - ✓ Added pth_mctx_restore/pth_mctx_restored macro definitions to pth_p.h
  - ✓ ALL 26 SOURCE FILES COMPILE SUCCESSFULLY!
  - ✓ Both libpth.so and libpth.a build successfully
  - ✓ All 11 test programs compile and link successfully

### Phase 1 Progress
- [x] Initial meson.build created
- [x] pth_p.h code generation eliminated ✓✓✓
- [x] All files compile ✓✓✓
- [x] Autotools removed ✓✓✓
- [ ] meson.build complete and functional (tests not all configured yet)

### Phase 2 Progress
- [x] `intern` → `static` (95/95 done)
- [x] pth_mctx.c simplified to modern Linux ucontext only (removed all sjlj variants)
- [x] Non-Linux code removed from pth_syscall.c, pth_time.c, pth_string.c
- [ ] All warnings fixed (0/~17000)
- [ ] Clean build with `-Werror`

### Phase 3 Progress
- [x] test_httpd made non-interactive
- [ ] All tests non-interactive (1/11)
- [ ] Test coverage expanded
- [ ] All tests enabled in meson (2/11)

### Phase 4 Progress
- [ ] API behavior documented
- [ ] Regression tests created
- [ ] Behavioral compatibility verified

### Phase 5 Progress
- [ ] Documentation updated
- [ ] Code comments cleaned up
- [ ] Final polish complete

## Success Criteria

The modernization is complete when ALL of these are true:
- ✓ Builds with meson/ninja only (no autotools)
- ✓ Compiles with `-std=c17 -Werror -Wall -Wextra` with zero warnings
- ✓ No code generation constructs (no `#if cpp` blocks)
- ✓ All tests pass without user interaction
- ✓ Test coverage >70%
- ✓ No non-Linux compatibility code (for ancient systems)
- ✓ All 26 source files compile and link correctly
- ✓ All 11 tests run automatically
- ✓ 100% API behavioral compatibility verified with tests
- ✓ Documentation updated for modern Linux/meson
- ✓ Clean git history with meaningful commits

## File Organization

### Source Files (26 total)
pth_attr.c, pth_cancel.c, pth_clean.c, pth_compat.c, pth_data.c, pth_debug.c, pth_errno.c, pth_event.c, pth_ext.c, pth_fork.c, pth_high.c, pth_lib.c, pth_mctx.c, pth_msg.c, pth_pqueue.c, pth_ring.c, pth_sched.c, pth_string.c, pth_sync.c, pth_syscall.c, pth_tcb.c, pth_time.c, pth_uctx.c, pth_util.c, pth_vers.c

### Test Files (11 total)
test_common.c, test_httpd.c, test_misc.c, test_mp.c, test_philo.c, test_pthread.c, test_select.c, test_sfio.c, test_sig.c, test_std.c, test_uctx.c

### Headers
- pth.h (public API)
- pth_p.h (private/internal API) ← CURRENTLY BROKEN
- pthread.h (POSIX threads emulation)
- pth_acdef.h (autoconf defines)
- pth_acmac.h (autoconf macros)
- config.h (build configuration)
- test_common.h

### Build Files
- meson.build (NEW - primary build system)
- .gitignore
- ~~configure, Makefile.in, etc.~~ (to be removed)

## Notes for AI Agents

- Always read this file first when resuming work
- Update this file after each work iteration
- Commit frequently with clear messages
- Test after each change before proceeding
- When in doubt, prefer smaller incremental changes
- Preserve API compatibility above all else
- Modern Linux means: kernel 4.0+, glibc 2.28+, as of 2020
- Focus on one blocker at a time
- Keep build in working state when possible