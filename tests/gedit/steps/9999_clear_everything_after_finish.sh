#!/bin/bash

ROOT="$(git rev-parse --show-toplevel)"
BUILDER="/home/$(username)/ibus-unikey/Base/Tools/Builder/build"
CURRENT=$(pwd)

source $ROOT/tests/pipeline/libraries/Logcat.sh
source $ROOT/tests/pipeline/libraries/Package.sh
source $ROOT/tests/pipeline/libraries/Console.sh

exec_on_test_machine 'rm -fr ~/\*.py'
