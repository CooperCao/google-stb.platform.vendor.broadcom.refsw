"""
Manage marker files to indicate that the process is running.

The Heartbeat object manages a set of files which are automatically
created by the constructor, removed by the destructor, and updated at
specified intervals in between. With this in use we know that:

    - If the file exists and is <=60 seconds old the process is
      probably still running.
    - If the file does not exist the process has exited normally.
    - If the file exists and is >60 seconds old, the process must
      have aborted so badly it was unable to clean up.

Although the pulse rate is nominally overridable it's best to leave
it on the default setting of 60 seconds. This allows external tools
to trust that if the file is more than a minute old it's dead. And
it's probably a good idea to allow some leeway for time skew too.

"""

from __future__ import print_function
from __future__ import unicode_literals

import os
import threading
import time

import lib.util

PULSE = 60
WIP_FILE = '_WARNING_BUILD_IN_PROGRESS'


class Heartbeat(object):

    """Keep a flag file updated as long as the process is running."""

    def __init__(self, paths, message, rate=PULSE):

        def heartbeat(path, message, rate):
            """Write a message to a file every <rate> seconds."""
            while os.path.isfile(path):
                with open(path, 'w') as f:
                    f.write(message.rstrip() + '\n')
                time.sleep(rate)

        self.pulses = {}

        # It's critical that the thread writes to the file only if it exists.
        # Otherwise there could be a race where it gets removed by build
        # cleanup but the daemon thread re-creates it before dying.
        # Responsibility for creating and removing the marker file must
        # live outside the thread.
        for path in paths:
            open(path, 'w').close()
            pulse = threading.Thread(target=heartbeat,
                                     args=(path, message, rate))
            pulse.daemon = True
            pulse.start()
            self.pulses[os.path.abspath(path)] = pulse

    @property
    def paths(self):
        """Return the list of paths to marker files."""
        return self.pulses.keys()

    def shutdown(self):
        """Remove the marker file(s) explicitly."""
        # Don't worry if a file is already gone, but we do need
        # to worry about a potential race where we remove a file
        # here after the heartbeat has determined it exists and
        # thus revives it.
        for path in self.paths:
            try:
                os.unlink(path)
            except OSError:
                pass

            time.sleep(0.1)
            if os.path.exists(path):
                os.unlink(path)

    def __del__(self):
        self.shutdown()


# If the HeartBeat class above is in use we can conclude a build is dead
# if the flag file is more than PULSE seconds old, after allowing some
# leeway for time skew.
def died(wip, timeout=PULSE * 3):
    """Return True if the WIP file appears to be stale (build has died)."""
    return time.time() - os.path.getmtime(wip) > timeout


def wait(path, delay=10, limit=7200, msg=None):
    """
    Block until the marker file goes away.

    Once the marker disappears, return True if a ,succeeded file is
    present and False otherwise. Return None if we gave up waiting.
    This may be used if one build needs to sync with another.
    """
    wip = os.path.join(path, WIP_FILE)
    successfile = os.path.join(os.path.dirname(wip), ',succeeded')
    if msg and os.path.exists(wip):
        lib.util.note(msg, vl=0)
    while limit > 0:
        if os.path.exists(wip):
            if died(wip):
                break
        else:
            return os.path.isfile(successfile)
        time.sleep(delay)
        limit -= delay
    return None

# vim: ts=8:sw=4:tw=80:et:
