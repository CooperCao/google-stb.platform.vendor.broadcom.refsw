NxClient tests

Each test script starts nxserver, runs a variety of clients, then waits for a clean shutdown of everything.

Requirements:
* run for a finite amount of time (ideally < 3 minutes)
* require no user interaction
* should not fail if settop doesn't have resources (for instance, a dual decode script should still pass on a single decode chip)
* script exits with zero on pass, non-zero on fail
* if any client or the server exits with non-zero, the whole script must exit with non-zero
