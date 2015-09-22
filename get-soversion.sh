#!/bin/sh

set -e

distro=$(lsb_release -c -s)

case "$distro" in
    vivd)
        echo 3
        ;;
    *)
        echo 4
        ;;
esac
