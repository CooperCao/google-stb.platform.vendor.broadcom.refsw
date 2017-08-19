    HELP FOR PEOPLE NEEDING TO ANALYZE A BROKEN BUILD

LOGGING:

Builds are broken into a sequence of steps, each of which is
logged in its own logfile. These logfiles are called "Name-n.log"
where "Name" is the step name and "n" is its sequence number,
e.g. "Preamble-1.log".  For convenience and compatibility these
individual logs are also concatenated into a single file
",release.log". Steps within ,release.log are separated by
"MARK-START: Name" and "MARK-END: Name".

WRITABILITY:

Normally, automated build files are created with a umask of 022,
making them writable only by the build user. This is generally a
feature. However, builds into the USERS area have their umask relaxed
to 002. It's impossible to guarantee all files will be group writable
because a build step could always explicitly "chmod go-w" a file,
but that's not something the build scaffolding can control.

REPLAY SCRIPTS:

Just as a build is a sequence of steps, each step comprises a
sequence of commands ("make", "tar", "cp", etc).  Rather than
executing these directly, the build system generates a temporary
shell script containing the command along with context such as
PATH overrides and current working directory, then executes
the script. These "replay" scripts are removed when the step
succeeds. In a failing build the scripts for the failing step
will remain and it should be possible to reproduce the error
by executing the last one. The benefit here is that the script
handles any required environment setup, and keeps the user from
having to reverse engineer the failing command line.

LOG TIMESTAMPS:

Another optional feature is log timestamping, in which each line
in each logfile is prefixed by the time it arrived. This can be
helpful for pinpointing unexpected delays.

PRESERVATION:

The presence of a file called _DO_NOT_DELETE at the build root will
ask the build system not to clean up that build tree. This does
not guarantee to protect it from a human or an unrelated cleanup
job but the build system, when looking for old builds to prune,
will try to leave it alone.

FILE TIMESTAMP WARNING:

For speed, builds are often done in local storage and then
published to a public NFS area. This may have the unfortunate
side effect of truncating any sub-second resolutions of file
timestamps.  In other words, a file might be given a mod time
of 1355864476.063853682 as created but copies, even when told to
preserve mod time ("cp -p"), might become 1355864476.0. This is
not fixable in the filesystem or any copying utility; the POSIX
API does not provide a system call with sufficient time resolution.

Since the files are already built before this happens it isn't
usually a problem but it may explain why files created within
the same second may seem to be out of order in a time listing.
