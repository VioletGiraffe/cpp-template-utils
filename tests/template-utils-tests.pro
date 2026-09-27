TEMPLATE = subdirs

SUBDIRS = tests cpp-template-utils catch2-runner

cpp-template-utils.file = ../cpp-template-utils.pro
catch2-runner.file = ../3rdparty/catch2/catch2.pro
tests.file = test-app/cpp-template-utils-test-app.pro
tests.depends = cpp-template-utils catch2-runner
