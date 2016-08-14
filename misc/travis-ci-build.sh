#!/bin/bash
PLATFORM=$(uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
failed=0

(make -k lean release release-${PLATFORM}-${ARCH}.zip) || failed=1

if [[ $failed -eq 1 ]]; then
	echo "Build failure."
else
	echo "Build successful."
fi

exit $failed

