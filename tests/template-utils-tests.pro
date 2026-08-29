TEMPLATE = subdirs

SUBDIRS = tests cpp-template-utils

cpp-template-utils.file = ../cpp-template-utils.pro
tests.file = test-app/cpp-template-utils-test-app.pro
tests.depends = cpp-template-utils
