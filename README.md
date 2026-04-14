# MyProject

Project code for C6678 IFD.

## Stable Baseline (2026-04-14)

The following settings were verified to remove frequent instruction-fetch crashes:

1. In `IFD.cfg`:
	- `Task.defaultStackSize = 0x8000`
	- `Program.stack = 0x10000`
2. Disable SYS/BIOS Load module usage (no `Load` module import or idle update usage).
3. Keep protocol parameter sanitization enabled in `protocol.c`.
4. Keep FFT/MAD split logic in `adaptive_filter.c`:
	- `fft_time_window == 2`: FFT path uses down-sampled updates.
	- MAD low-pass path always uses original 4096-point raw data.

## Release Regression Checklist

Run these checks before release:

1. Clean and full rebuild.
2. Reflash and reboot all cores.
3. Run `param set -> first enable` at least 50 cycles.
4. Run `enable/stop` at least 500 cycles.
5. Long-run test for 1 hour with mixed commands.
6. Confirm no exception logs (`pc=0x0`, instruction fetch exception).

## If Crash Reappears

Collect and attach:

1. Last 20 log lines before crash.
2. Exception registers (`IRP`, `NRP`, `sp`, `IERR`).
3. Current map file (`Debug/IFD_0410_RAM.map`).
4. The exact parameter packet used before `CMD_ENABLE`.
