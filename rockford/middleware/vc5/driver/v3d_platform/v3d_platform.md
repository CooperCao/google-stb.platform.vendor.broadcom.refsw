# What's a "V3D platform"?

Either an interface to a real chip, or a simulator. If it's a simulator, it
might be Simpenrose (which just simulates the functionality at a relatively
high level), or a lower level simulator of the RTL such as Carbon.

# What are the differences between the platforms?

Simulation platforms have to provide the actual simulation obviously. They
also (currently) use a different scheduler, although all simulation platforms
use the same one.

They may use a different "shared" memory implementation (i.e. implementation of
gmem). On some simulation platforms, such as Carbon, the memory isn't actually
shared, but gmem uses the sync lists to copy memory between two different
areas.

There are a few more interfaces to the platform that aren't via the scheduler
or gmem, and these are defined in `v3d_platform.h`.

# What about middleware/khronos/egl/platform?

An EGL platform has more to do with the OS you're running on than the
underlying hardware.

In theory you could have any V3D platform with any EGL platform-- for example
Simpenrose simulating V3DV3 running under Android on a V3DV2 (we actually did
do this).

In practice you're likely to use the simulation V3D platforms with EGL
platforms like X11, Null (no windows at all) and Win32; and hardware V3D
platforms with the sorts of OSes commonly seen on devices such as Android.

The hardware V3D platform for Android is called "linux" rather than "android"
because the interface between the userspace component and the scheduler is a
Linux driver.

# Organization

## ./android

Userspace interface to the Linux kernel scheduler for Android.

## ./simulation

Contains the simulation scheduler and simulation gmem (used for all simulation
platforms), the different simulators that scheduler can use (carbon and
simpenrose), and the interfaces to those simulators.
