#!/bin/bash

set -e

app_ver=${1}
plist_dir=${2}
commit_id=$(git rev-parse --short=8 HEAD)

echo "replace app bundle version: ${app_ver}.${commit_id}"
plutil -replace CFBundleVersion -string "${app_ver}.${commit_id}" "${plist_dir}/Info.plist"
plutil -replace CFBundleShortVersionString -string "${app_ver}.${commit_id}" "${plist_dir}/Info.plist"
