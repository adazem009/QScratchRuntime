#!/bin/bash

.ci/common/build.sh macos

macdeployqt QScratchRuntime.app -dmg
