#!/bin/sh

set -e

[ -n "$SERIES" ] || SERIES=$(lsb_release -c -s)

case "$SERIES" in
    vivid)
        echo 3
        ;;
    *)
        echo 4
        ;;
esac
