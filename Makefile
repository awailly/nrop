CC=gcc
#CC=clang --analyze -x c
RM=rm -f
CFLAGS=-DNEED_CPU_H -DCONFIG_USER_ONLY -pthread -W -Wall -Wextra #-O2 -pedantic -Werror -D_FORTIFY_SOURCE=2 
QEMU_ALL=qemu/qobject/qfloat.o qemu/qobject/json-streamer.o qemu/qobject/qdict.o qemu/qobject/json-parser.o qemu/qobject/qjson.o qemu/qobject/qerror.o qemu/qobject/json-lexer.o qemu/qobject/qstring.o qemu/qobject/qlist.o qemu/qobject/qbool.o qemu/qobject/qint.o qemu/block.o qemu/qemu-coroutine.o qemu/trace/control.o qemu/trace/generated-events.o qemu/util/event_notifier-posix.o qemu/util/path.o qemu/util/notify.o qemu/util/envlist.o qemu/util/acl.o qemu/util/unicode.o qemu/util/hbitmap.o qemu/util/bitmap.o qemu/util/bitops.o qemu/util/crc32c.o qemu/util/qemu-sockets.o qemu/util/compatfd.o qemu/util/qemu-option.o qemu/util/qemu-timer-common.o qemu/util/aes.o qemu/util/error.o qemu/util/qemu-error.o qemu/util/uri.o qemu/util/iov.o qemu/util/rfifolock.o qemu/util/osdep.o qemu/util/qemu-config.o qemu/util/getauxval.o qemu/util/qemu-thread-posix.o qemu/util/module.o qemu/util/hexdump.o qemu/util/oslib-posix.o qemu/util/throttle.o qemu/util/readline.o qemu/util/cutils.o qemu/util/qemu-progress.o qemu/blockjob.o qemu/x86_64-linux-user/disas.o qemu/x86_64-linux-user/kvm-stub.o qemu/x86_64-linux-user/translate-all.o qemu/x86_64-linux-user/tcg/optimize.o qemu/x86_64-linux-user/tcg/tcg.o qemu/x86_64-linux-user/linux-user/signal.o qemu/x86_64-linux-user/linux-user/syscall.o qemu/x86_64-linux-user/linux-user/mmap.o qemu/x86_64-linux-user/linux-user/linuxload.o qemu/x86_64-linux-user/linux-user/uaccess.o qemu/x86_64-linux-user/linux-user/uname.o qemu/x86_64-linux-user/linux-user/strace.o qemu/x86_64-linux-user/linux-user/elfload.o qemu/x86_64-linux-user/fpu/softfloat.o qemu/x86_64-linux-user/cpu-exec.o qemu/x86_64-linux-user/exec.o qemu/x86_64-linux-user/gdbstub.o qemu/x86_64-linux-user/user-exec.o qemu/x86_64-linux-user/target-i386/excp_helper.o qemu/x86_64-linux-user/target-i386/seg_helper.o qemu/x86_64-linux-user/target-i386/kvm-stub.o qemu/x86_64-linux-user/target-i386/misc_helper.o qemu/x86_64-linux-user/target-i386/mem_helper.o qemu/x86_64-linux-user/target-i386/helper.o qemu/x86_64-linux-user/target-i386/ioport-user.o qemu/x86_64-linux-user/target-i386/translate.o qemu/x86_64-linux-user/target-i386/cpu.o qemu/x86_64-linux-user/target-i386/smm_helper.o qemu/x86_64-linux-user/target-i386/gdbstub.o qemu/x86_64-linux-user/target-i386/fpu_helper.o qemu/x86_64-linux-user/target-i386/int_helper.o qemu/x86_64-linux-user/target-i386/cc_helper.o qemu/x86_64-linux-user/target-i386/svm_helper.o qemu/x86_64-linux-user/thunk.o qemu/main-loop.o qemu/qemu-io-cmds.o qemu/qapi/qmp-registry.o qemu/qapi/opts-visitor.o qemu/qapi/qmp-input-visitor.o qemu/qapi/qmp-event.o qemu/qapi/qmp-dispatch.o qemu/qapi/qmp-output-visitor.o qemu/qapi/qapi-visit-core.o qemu/qapi/qapi-dealloc-visitor.o qemu/qapi/string-input-visitor.o qemu/qapi/string-output-visitor.o qemu/tcg-runtime.o qemu/qapi-types.o qemu/async.o qemu/qemu-coroutine-io.o qemu/qom/qom-qobject.o qemu/qom/container.o qemu/qom/object_interfaces.o qemu/qom/cpu.o qemu/qom/object.o qemu/hw/core/qdev.o qemu/hw/core/fw-path-provider.o qemu/hw/core/irq.o qemu/hw/core/hotplug.o qemu/hw/core/qdev-properties.o qemu/coroutine-ucontext.o qemu/disas/i386.o qemu/qemu-coroutine-lock.o qemu/qapi-visit.o qemu/qemu-coroutine-sleep.o qemu/aio-posix.o qemu/iohandler.o qemu/block/qcow.o qemu/block/qed-check.o qemu/block/qed.o qemu/block/qcow2-cluster.o qemu/block/sheepdog.o qemu/block/qcow2.o qemu/block/nbd-client.o qemu/block/dmg.o qemu/block/blkdebug.o qemu/block/qed-cluster.o qemu/block/cloop.o qemu/block/nbd.o qemu/nbd.o qemu/block/raw_bsd.o qemu/block/qed-l2-cache.o qemu/block/parallels.o qemu/block/vpc.o qemu/block/blkverify.o qemu/block/vdi.o qemu/block/snapshot.o qemu/block/vmdk.o qemu/block/qed-table.o qemu/block/qapi.o qemu/block/qcow2-snapshot.o qemu/block/bochs.o qemu/block/qed-gencb.o qemu/block/vvfat.o qemu/block/qcow2-refcount.o qemu/block/qcow2-cache.o qemu/qemu-log.o qemu/qemu-timer.o qemu/qapi-event.o qemu/thread-pool.o qemu/stubs/slirp.o qemu/stubs/clock-warp.o qemu/stubs/migr-blocker.o qemu/stubs/mon-printf.o qemu/stubs/vc-init.o qemu/stubs/chr-baum-init.o qemu/stubs/is-daemonized.o qemu/stubs/dump.o qemu/stubs/mon-set-error.o qemu/stubs/runstate-check.o qemu/stubs/reset.o qemu/stubs/cpu-get-clock.o qemu/stubs/mon-is-qmp.o qemu/stubs/kvm.o qemu/stubs/pci-drive-hot-add.o qemu/stubs/fdset-get-fd.o qemu/stubs/get-fd.o qemu/stubs/fdset-find-fd.o qemu/stubs/vm-stop.o qemu/stubs/fdset-remove-fd.o qemu/stubs/monitor-init.o qemu/stubs/fdset-add-fd.o qemu/stubs/qtest.o qemu/stubs/uuid.o qemu/stubs/gdbstub.o qemu/stubs/cpu-get-icount.o qemu/stubs/chr-msmouse.o qemu/stubs/iothread-lock.o qemu/stubs/machine-init-done.o qemu/stubs/get-vm-name.o qemu/stubs/vmstate.o qemu/stubs/cpus.o qemu/stubs/get-next-serial.o qemu/stubs/sysbus.o qemu/x86_64-linux-user/linux-user/main.o qemu/block/block-backend.o qemu/block/accounting.o qemu/util/id.o qemu/qapi/qapi-util.o
#qemu/util/qemu-openpty.o qemu/stubs/set-fd-handler.o qemu/util/fifo8.o qemu/libqemuutil.a 
QEMU_SPE=
LIBS=xed2-intel64/lib/libxed.a -lcapstone -Lz3/build -L/usr/lib -lz -lm -lrt -lglib-2.0 $(QEMU_ALL) `llvm-config --libs --cflags --ldflags` -lz3 -ldl -lncurses # -lpthread
#`llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`
# pkg-config --cflags --libs glib-2.0
#INCLUDES=-Ixed2-intel64/include -I. -Iplugins -Iparsers -Iqemu/tcg/i386 -Iqemu/x86_64-linux-user -Iqemu/target-i386 -Iqemu -Iqemu/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include 
INCLUDES=-Ixed2-intel64/include -I. -Idisassemblers -Iplugins -Iparsers -isystem qemu/tcg/i386 -isystem qemu/x86_64-linux-user -isystem qemu/target-i386 -isystem qemu -isystem qemu/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -Iz3/build -Iz3/src/api

.SUFFIXES:
.SUFFIXES: .c .o
.PHONY: examples

SRCS=packer.c\
     chunk.c\
     enumerator.c\
     linked_list.c\
     utils.c\
     region.c\
	 thpool.c\
     chain.c\
	 map.c\
	 constraints.c\
	 converter.c\
	 disassemblers/disassembler_xed.c\
	 disassemblers/disassembler_capstone.c\
	 disassemblers/disassembler_meta.c\
     parsers/section.c\
     parsers/program_header.c\
     parsers/parser_builder.c\
     parsers/elf_type.c\
     parsers/pe_type.c\
     parsers/parser_elf.c\
     parsers/parser_pe.c\
     plugins/plugin_builder.c\
     plugins/plugin_rop.c

OBJS=$(SRCS:.c=.o)

MAIN=packer

all: $(MAIN)

#$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -I/usr/include  -DNDEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fPIC
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ `llvm-config --cflags`

$(MAIN): $(OBJS)
	g++ $^ $(INCLUDES) -o $@ $(LIBS) $(CFLAGS) 

clean:
	$(RM) *.o *.bak *~ $(EXEC) plugins/*.o plugins/*~ parsers/*.o parsers/*~ disassemblers/*.o disassemblers/*~

realclean:
	$(clean)
	rm packer

examples:
	nasm -f elf64 examples/split.s -o examples/split.o
	ld examples/split.o -o examples/split
	nasm -f elf64 examples/hello.s -o examples/hello.o
	ld examples/hello.o -o examples/hello
	nasm -f elf64 examples/mark.s -o examples/mark.o
	ld examples/mark.o -o examples/mark
	nasm -f elf64 examples/opti.s -o examples/opti.o
	ld examples/opti.o -o examples/opti
	nasm -f elf64 examples/new.s -o examples/new.o
	ld examples/new.o -o examples/new
	nasm -f elf64 examples/pn2.s -o examples/pn2.o
	ld examples/pn2.o -o examples/pn2

depend: $(SRCS)
	makedepend $(INCLUDES) $^
