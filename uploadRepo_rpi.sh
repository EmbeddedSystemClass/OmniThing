#!/bin/bash

remote_host=$1

scp deb/Packages deb/Packages.gz deb/omnithing.deb deb/InRelease deb/KEY.gpg deb/Release deb/Release.gpg $remote_host:~/omni_repositories/rpi
