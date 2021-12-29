#!/bin/bash

.ci/common/build.sh macos

cd QScratchRuntime.app
zip -r ../QScratchRuntime-mac.zip .
