# Links catch2_runner, which catch2.pro builds into the parent repository's bin directory
# Requires OUTPUT_DIR, the configuration's directory name under bin

INCLUDEPATH += $$PWD/../..

CATCH2_RUNNER_DIR = $$clean_path($$PWD/../../../bin/$${OUTPUT_DIR})
LIBS += -L$${CATCH2_RUNNER_DIR} -lcatch2_runner

mac*|linux*|freebsd {
	PRE_TARGETDEPS += $${CATCH2_RUNNER_DIR}/libcatch2_runner.a
}
