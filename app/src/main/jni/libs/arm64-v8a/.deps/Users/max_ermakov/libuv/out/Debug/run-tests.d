cmd_/Users/max_ermakov/libuv/out/Debug/run-tests := arm-linux-androideabi-g++ -fPIE -pie  -o /Users/max_ermakov/libuv/out/Debug/run-tests -Wl,--start-group /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/blackhole-server.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/echo-server.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/run-tests.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/runner.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-get-loadavg.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-active.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-async.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-async-null-cb.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-callback-stack.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-callback-order.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-close-fd.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-close-order.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-connection-fail.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-cwd-and-chdir.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-default-loop-close.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-delayed-accept.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-eintr-handling.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-error.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-embed.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-emfile.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-fail-always.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-fs.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-fs-event.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-get-currentexe.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-get-memory.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-get-passwd.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-getaddrinfo.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-getnameinfo.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-getsockname.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-handle-fileno.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-homedir.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-hrtime.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-idle.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-ip6-addr.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-ipc.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-ipc-send-recv.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-loop-handles.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-loop-alive.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-loop-close.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-loop-stop.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-loop-time.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-loop-configure.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-walk-handles.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-watcher-cross-stop.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-multiple-listen.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-osx-select.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pass-always.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-ping-pong.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-bind-error.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-connect-error.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-connect-multiple.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-connect-prepare.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-getsockname.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-pending-instances.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-sendmsg.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-server-close.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-close-stdout-read-stdin.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-pipe-set-non-blocking.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-platform-output.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-poll.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-poll-close.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-poll-close-doesnt-corrupt-stack.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-poll-closesocket.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-process-title.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-queue-foreach-delete.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-ref.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-run-nowait.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-run-once.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-semaphore.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-shutdown-close.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-shutdown-eof.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-shutdown-twice.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-signal.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-signal-multiple-loops.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-socket-buffer-size.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-spawn.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-fs-poll.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-stdio-over-pipes.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-alloc-cb-fail.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-bind-error.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-bind6-error.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-close.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-close-accept.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-close-while-connecting.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-create-socket-early.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-connect-error-after-write.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-shutdown-after-write.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-flags.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-connect-error.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-connect-timeout.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-connect6-error.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-open.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-write-to-half-open-connection.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-write-after-connect.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-writealot.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-write-fail.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-try-write.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-unexpected-read.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-oob.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-read-stop.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tcp-write-queue-order.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-threadpool.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-threadpool-cancel.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-thread-equal.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tmpdir.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-mutexes.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-thread.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-barrier.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-condvar.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-timer-again.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-timer-from-check.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-timer.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-tty.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-alloc-cb-fail.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-bind.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-create-socket-early.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-dgram-too-big.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-ipv6.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-open.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-options.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-send-and-recv.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-send-immediate.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-send-unreachable.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-multicast-join.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-multicast-join6.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-dlerror.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-multicast-ttl.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-ip4-addr.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-multicast-interface.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-multicast-interface6.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/test-udp-try-send.o /Users/max_ermakov/libuv/out/Debug/obj.target/run-tests/test/runner-unix.o /Users/max_ermakov/libuv/out/Debug/obj.target/libuv.a -Wl,--end-group -lm -ldl
