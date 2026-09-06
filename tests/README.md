# ReceiveData contract check

Run `python tests/check_receive_data.py` with `cl` in a Windows developer shell,
or with `g++`/`CXX` on another system. The test extracts the **current production
ReceiveData method** and compiles it against a minimal queue/message fixture.
It does not contain a duplicate receive implementation and needs no Steam login
or private SDK checkout.

The cases cover application/system messages, null-data size probes, insufficient
and oversized buffers, exact-size receives, zero-length payloads, peek/retry,
sender/recipient filters, empty queues and invalid pointers. Queue retention and
bytes beyond the returned payload are checked. Before the size-output fix, the
first probe returns BUFFERTOOSMALL while leaving the input capacity unchanged;
the test fails. With the fix all cases pass.

This checks the method's data-copy/API contract. It does not exercise Steam
transport, callbacks, threading, authentication, or native DirectPlay system
message pointer reconstruction. The minimal packed SData layout is explicitly
checked as nine bytes and must be kept consistent with Messages.h.
