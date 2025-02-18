#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This fixes a problem with times when switching in the same file system from
# Windows to Linux (clock skew).
#
# You must run it from the root of the mrv2 project.
#
find mrv2 -exec touch {} +
find tlRender -exec touch {} +
