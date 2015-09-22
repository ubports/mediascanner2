#!/bin/sh

set -e

distro=$(lsb_release -c -s)

case "$distro" in
    vivid)
        echo 3
        ;;
    *)
        echo 4
        ;;
esac
