CC=gcc
#CC=clang --analyze -x c
RM=rm -f
CFLAGS=-g -DNEED_CPU_H -DCONFIG_USER_ONLY -pthread -W -Wall -Wextra #-O2 -pedantic -Werror -D_FORTIFY_SOURCE=2 
QEMU_ALL=qemu/qobject/qfloat.o qemu/qobject/json-streamer.o qemu/qobject/qdict.o qemu/qobject/json-parser.o qemu/qobject/qjson.o qemu/qobject/qerror.o qemu/qobject/json-lexer.o qemu/qobject/qstring.o qemu/qobject/qlist.o qemu/qobject/qbool.o qemu/qobject/qint.o qemu/block.o qemu/qemu-coroutine.o qemu/trace/control.o qemu/trace/generated-events.o qemu/util/event_notifier-posix.o qemu/util/path.o qemu/util/notify.o qemu/util/envlist.o qemu/util/acl.o qemu/util/unicode.o qemu/util/hbitmap.o qemu/util/bitmap.o qemu/util/bitops.o qemu/util/crc32c.o qemu/util/qemu-sockets.o qemu/util/compatfd.o qemu/util/qemu-option.o qemu/util/qemu-timer-common.o qemu/util/aes.o qemu/util/error.o qemu/util/qemu-error.o qemu/util/uri.o qemu/util/iov.o qemu/util/rfifolock.o qemu/util/osdep.o qemu/util/qemu-config.o qemu/util/getauxval.o qemu/util/qemu-thread-posix.o qemu/util/module.o qemu/util/hexdump.o qemu/util/oslib-posix.o qemu/util/throttle.o qemu/util/readline.o qemu/util/cutils.o qemu/util/qemu-progress.o qemu/blockjob.o qemu/x86_64-linux-user/disas.o qemu/x86_64-linux-user/kvm-stub.o qemu/x86_64-linux-user/translate-all.o qemu/x86_64-linux-user/tcg/optimize.o qemu/x86_64-linux-user/tcg/tcg.o qemu/x86_64-linux-user/linux-user/signal.o qemu/x86_64-linux-user/linux-user/syscall.o qemu/x86_64-linux-user/linux-user/mmap.o qemu/x86_64-linux-user/linux-user/linuxload.o qemu/x86_64-linux-user/linux-user/uaccess.o qemu/x86_64-linux-user/linux-user/uname.o qemu/x86_64-linux-user/linux-user/strace.o qemu/x86_64-linux-user/linux-user/elfload.o qemu/x86_64-linux-user/fpu/softfloat.o qemu/x86_64-linux-user/cpu-exec.o qemu/x86_64-linux-user/exec.o qemu/x86_64-linux-user/gdbstub.o qemu/x86_64-linux-user/user-exec.o qemu/x86_64-linux-user/target-i386/excp_helper.o qemu/x86_64-linux-user/target-i386/seg_helper.o qemu/x86_64-linux-user/target-i386/kvm-stub.o qemu/x86_64-linux-user/target-i386/misc_helper.o qemu/x86_64-linux-user/target-i386/mem_helper.o qemu/x86_64-linux-user/target-i386/helper.o qemu/x86_64-linux-user/target-i386/ioport-user.o qemu/x86_64-linux-user/target-i386/translate.o qemu/x86_64-linux-user/target-i386/cpu.o qemu/x86_64-linux-user/target-i386/smm_helper.o qemu/x86_64-linux-user/target-i386/gdbstub.o qemu/x86_64-linux-user/target-i386/fpu_helper.o qemu/x86_64-linux-user/target-i386/int_helper.o qemu/x86_64-linux-user/target-i386/cc_helper.o qemu/x86_64-linux-user/target-i386/svm_helper.o qemu/x86_64-linux-user/thunk.o qemu/main-loop.o qemu/qemu-io-cmds.o qemu/qapi/qmp-registry.o qemu/qapi/opts-visitor.o qemu/qapi/qmp-input-visitor.o qemu/qapi/qmp-event.o qemu/qapi/qmp-dispatch.o qemu/qapi/qmp-output-visitor.o qemu/qapi/qapi-visit-core.o qemu/qapi/qapi-dealloc-visitor.o qemu/qapi/string-input-visitor.o qemu/qapi/string-output-visitor.o qemu/tcg-runtime.o qemu/qapi-types.o qemu/async.o qemu/qemu-coroutine-io.o qemu/qom/qom-qobject.o qemu/qom/container.o qemu/qom/object_interfaces.o qemu/qom/cpu.o qemu/qom/object.o qemu/hw/core/qdev.o qemu/hw/core/fw-path-provider.o qemu/hw/core/irq.o qemu/hw/core/hotplug.o qemu/hw/core/qdev-properties.o qemu/coroutine-ucontext.o qemu/disas/i386.o qemu/qemu-coroutine-lock.o qemu/qapi-visit.o qemu/qemu-coroutine-sleep.o qemu/aio-posix.o qemu/iohandler.o qemu/block/qcow.o qemu/block/qed-check.o qemu/block/qed.o qemu/block/qcow2-cluster.o qemu/block/sheepdog.o qemu/block/qcow2.o qemu/block/nbd-client.o qemu/block/dmg.o qemu/block/blkdebug.o qemu/block/qed-cluster.o qemu/block/cloop.o qemu/block/nbd.o qemu/nbd.o qemu/block/raw_bsd.o qemu/block/qed-l2-cache.o qemu/block/parallels.o qemu/block/vpc.o qemu/block/blkverify.o qemu/block/vdi.o qemu/block/snapshot.o qemu/block/vmdk.o qemu/block/qed-table.o qemu/block/qapi.o qemu/block/qcow2-snapshot.o qemu/block/bochs.o qemu/block/qed-gencb.o qemu/block/vvfat.o qemu/block/qcow2-refcount.o qemu/block/qcow2-cache.o qemu/qemu-log.o qemu/qemu-timer.o qemu/qapi-event.o qemu/thread-pool.o qemu/stubs/slirp.o qemu/stubs/clock-warp.o qemu/stubs/migr-blocker.o qemu/stubs/mon-printf.o qemu/stubs/vc-init.o qemu/stubs/chr-baum-init.o qemu/stubs/is-daemonized.o qemu/stubs/dump.o qemu/stubs/mon-set-error.o qemu/stubs/runstate-check.o qemu/stubs/reset.o qemu/stubs/cpu-get-clock.o qemu/stubs/mon-is-qmp.o qemu/stubs/kvm.o qemu/stubs/pci-drive-hot-add.o qemu/stubs/fdset-get-fd.o qemu/stubs/get-fd.o qemu/stubs/fdset-find-fd.o qemu/stubs/vm-stop.o qemu/stubs/fdset-remove-fd.o qemu/stubs/monitor-init.o qemu/stubs/fdset-add-fd.o qemu/stubs/qtest.o qemu/stubs/uuid.o qemu/stubs/gdbstub.o qemu/stubs/cpu-get-icount.o qemu/stubs/chr-msmouse.o qemu/stubs/iothread-lock.o qemu/stubs/machine-init-done.o qemu/stubs/get-vm-name.o qemu/stubs/vmstate.o qemu/stubs/cpus.o qemu/stubs/get-next-serial.o qemu/stubs/sysbus.o qemu/x86_64-linux-user/linux-user/main.o qemu/block/block-backend.o qemu/block/accounting.o qemu/util/id.o qemu/qapi/qapi-util.o
#qemu/util/qemu-openpty.o qemu/stubs/set-fd-handler.o qemu/util/fifo8.o qemu/libqemuutil.a 
QEMU_SPE=
LIBS=xed2-intel64/lib/libxed.a -L/usr/lib -lz -lm -lrt -lglib-2.0 $(QEMU_ALL) `llvm-config --libs --cflags --ldflags` -lz3 -ldl -lncurses # -lpthread
#`llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`
# pkg-config --cflags --libs glib-2.0
#INCLUDES=-Ixed2-intel64/include -I. -Iplugins -Iparsers -Iqemu/tcg/i386 -Iqemu/x86_64-linux-user -Iqemu/target-i386 -Iqemu -Iqemu/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include 
INCLUDES=-Ixed2-intel64/include -I. -Iplugins -Iparsers -isystem qemu/tcg/i386 -isystem qemu/x86_64-linux-user -isystem qemu/target-i386 -isystem qemu -isystem qemu/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include 

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

chain.o: chain.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ `llvm-config --cflags`

converter.o: converter.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ `llvm-config --cflags`

map.o: map.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ `llvm-config --cflags`

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ `llvm-config --cflags`

$(MAIN): $(OBJS)
	g++ $^ $(INCLUDES) -o $@ $(LIBS) $(CFLAGS) 

clean:
	$(RM) *.o *.bak *~ $(EXEC) plugins/*.o parsers/*.o

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
# DO NOT DELETE

packer.o: packer.h /usr/include/stdio.h /usr/include/features.h
packer.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
packer.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
packer.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
packer.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
packer.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
packer.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
packer.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
packer.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
packer.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
packer.o: /usr/include/time.h /usr/include/sys/select.h
packer.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
packer.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
packer.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
packer.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
packer.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
packer.o: /usr/include/bits/confname.h /usr/include/getopt.h utils.h
packer.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
packer.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
packer.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
packer.o: /usr/include/bits/stat.h /usr/include/string.h
packer.o: /usr/include/xlocale.h /usr/include/inttypes.h
packer.o: /usr/include/stdint.h /usr/include/bits/wchar.h
packer.o: /usr/include/ctype.h /usr/include/sys/stat.h region.h chunk.h
packer.o: code.h linked_list.h ./enumerator.h constraints.h
packer.o: xed2-intel64/include/xed-interface.h
packer.o: xed2-intel64/include/xed-common-hdrs.h
packer.o: xed2-intel64/include/xed-types.h
packer.o: xed2-intel64/include/xed-operand-enum.h
packer.o: xed2-intel64/include/xed-init.h xed2-intel64/include/xed-decode.h
packer.o: xed2-intel64/include/xed-decoded-inst.h
packer.o: xed2-intel64/include/xed-common-defs.h
packer.o: xed2-intel64/include/xed-portability.h
packer.o: xed2-intel64/include/xed-util.h
packer.o: xed2-intel64/include/xed-operand-values-interface.h
packer.o: xed2-intel64/include/xed-state.h
packer.o: xed2-intel64/include/xed-address-width-enum.h
packer.o: xed2-intel64/include/xed-machine-mode-enum.h
packer.o: xed2-intel64/include/xed-operand-storage.h
packer.o: xed2-intel64/include/xed-reg-enum.h
packer.o: xed2-intel64/include/xed-iclass-enum.h
packer.o: xed2-intel64/include/xed-inst.h
packer.o: xed2-intel64/include/xed-category-enum.h
packer.o: xed2-intel64/include/xed-extension-enum.h
packer.o: xed2-intel64/include/xed-operand-visibility-enum.h
packer.o: xed2-intel64/include/xed-operand-action-enum.h
packer.o: xed2-intel64/include/xed-operand-convert-enum.h
packer.o: xed2-intel64/include/xed-operand-type-enum.h
packer.o: xed2-intel64/include/xed-nonterminal-enum.h
packer.o: xed2-intel64/include/xed-operand-width-enum.h
packer.o: xed2-intel64/include/xed-operand-element-xtype-enum.h
packer.o: xed2-intel64/include/xed-attribute-enum.h
packer.o: xed2-intel64/include/xed-exception-enum.h
packer.o: xed2-intel64/include/xed-iform-enum.h
packer.o: xed2-intel64/include/xed-iform-map.h
packer.o: xed2-intel64/include/xed-isa-set-enum.h
packer.o: xed2-intel64/include/xed-attributes.h
packer.o: xed2-intel64/include/xed-gen-table-defs.h
packer.o: xed2-intel64/include/xed-flags.h
packer.o: xed2-intel64/include/xed-flag-enum.h
packer.o: xed2-intel64/include/xed-flag-action-enum.h
packer.o: xed2-intel64/include/xed-encoder-gen-defs.h
packer.o: xed2-intel64/include/xed-chip-enum.h
packer.o: xed2-intel64/include/xed-operand-element-type-enum.h
packer.o: xed2-intel64/include/xed-error-enum.h
packer.o: xed2-intel64/include/xed-decode-cache.h
packer.o: xed2-intel64/include/xed-syntax-enum.h
packer.o: xed2-intel64/include/xed-reg-class-enum.h
packer.o: xed2-intel64/include/xed-reg-class.h
packer.o: xed2-intel64/include/xed-inst-printer.h
packer.o: xed2-intel64/include/xed-encode.h
packer.o: xed2-intel64/include/xed-encoder-iforms.h
packer.o: xed2-intel64/include/xed-encoder-hl.h
packer.o: xed2-intel64/include/xed-operand-action.h
packer.o: xed2-intel64/include/xed-version.h xed2-intel64/include/xed-disas.h
packer.o: xed2-intel64/include/xed-format-options.h
packer.o: xed2-intel64/include/xed-iformfl-enum.h
packer.o: xed2-intel64/include/xed-agen.h parsers/parser_builder.h
packer.o: parsers/parser.h plugins/plugin_builder.h plugins/plugin.h
packer.o: parsers/parser_elf.h /usr/include/elf.h parsers/elf_type.h
packer.o: parsers/section.h parsers/program_header.h parsers/parser_pe.h
packer.o: parsers/pe_type.h plugins/plugin_rop.h chain.h map.h
chunk.o: /usr/include/stdio.h /usr/include/features.h
chunk.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
chunk.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
chunk.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
chunk.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
chunk.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
chunk.o: /usr/include/sys/stat.h /usr/include/time.h /usr/include/bits/stat.h
chunk.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
chunk.o: /usr/include/bits/environments.h /usr/include/bits/confname.h
chunk.o: /usr/include/getopt.h /usr/include/errno.h /usr/include/bits/errno.h
chunk.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
chunk.o: /usr/include/asm-generic/errno.h
chunk.o: /usr/include/asm-generic/errno-base.h chunk.h /usr/include/string.h
chunk.o: /usr/include/xlocale.h /usr/include/sys/types.h
chunk.o: /usr/include/endian.h /usr/include/bits/endian.h
chunk.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
chunk.o: /usr/include/sys/select.h /usr/include/bits/select.h
chunk.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
chunk.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
chunk.o: utils.h /usr/include/stdlib.h /usr/include/bits/waitflags.h
chunk.o: /usr/include/bits/waitstatus.h /usr/include/alloca.h
chunk.o: /usr/include/bits/stdlib-float.h /usr/include/sys/mman.h
chunk.o: /usr/include/bits/mman.h /usr/include/bits/mman-linux.h
chunk.o: /usr/include/fcntl.h /usr/include/bits/fcntl.h
chunk.o: /usr/include/bits/fcntl-linux.h /usr/include/inttypes.h
chunk.o: /usr/include/stdint.h /usr/include/bits/wchar.h /usr/include/ctype.h
enumerator.o: ./enumerator.h utils.h /usr/include/stdio.h
enumerator.o: /usr/include/features.h /usr/include/stdc-predef.h
enumerator.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
enumerator.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
enumerator.o: /usr/include/bits/typesizes.h /usr/include/libio.h
enumerator.o: /usr/include/_G_config.h /usr/include/wchar.h
enumerator.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
enumerator.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
enumerator.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
enumerator.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
enumerator.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
enumerator.o: /usr/include/time.h /usr/include/sys/select.h
enumerator.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
enumerator.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
enumerator.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
enumerator.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
enumerator.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
enumerator.o: /usr/include/bits/confname.h /usr/include/getopt.h
enumerator.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
enumerator.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
enumerator.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
enumerator.o: /usr/include/bits/stat.h /usr/include/string.h
enumerator.o: /usr/include/xlocale.h /usr/include/inttypes.h
enumerator.o: /usr/include/stdint.h /usr/include/bits/wchar.h
enumerator.o: /usr/include/ctype.h /usr/include/sys/stat.h
enumerator.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
enumerator.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
enumerator.o: /usr/include/bits/posix2_lim.h /usr/include/dirent.h
enumerator.o: /usr/include/bits/dirent.h /usr/include/errno.h
enumerator.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
enumerator.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
enumerator.o: /usr/include/asm-generic/errno-base.h
linked_list.o: /usr/include/stdlib.h /usr/include/features.h
linked_list.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
linked_list.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
linked_list.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
linked_list.o: /usr/include/endian.h /usr/include/bits/endian.h
linked_list.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
linked_list.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
linked_list.o: /usr/include/sys/types.h /usr/include/time.h
linked_list.o: /usr/include/sys/select.h /usr/include/bits/select.h
linked_list.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
linked_list.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
linked_list.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h chain.h
linked_list.o: utils.h /usr/include/stdio.h /usr/include/libio.h
linked_list.o: /usr/include/_G_config.h /usr/include/wchar.h
linked_list.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
linked_list.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
linked_list.o: /usr/include/bits/environments.h /usr/include/bits/confname.h
linked_list.o: /usr/include/getopt.h /usr/include/sys/mman.h
linked_list.o: /usr/include/bits/mman.h /usr/include/bits/mman-linux.h
linked_list.o: /usr/include/fcntl.h /usr/include/bits/fcntl.h
linked_list.o: /usr/include/bits/fcntl-linux.h /usr/include/bits/stat.h
linked_list.o: /usr/include/string.h /usr/include/xlocale.h
linked_list.o: /usr/include/inttypes.h /usr/include/stdint.h
linked_list.o: /usr/include/bits/wchar.h /usr/include/ctype.h
linked_list.o: /usr/include/sys/stat.h chunk.h map.h
linked_list.o: xed2-intel64/include/xed-interface.h
linked_list.o: xed2-intel64/include/xed-common-hdrs.h
linked_list.o: xed2-intel64/include/xed-types.h
linked_list.o: xed2-intel64/include/xed-operand-enum.h
linked_list.o: xed2-intel64/include/xed-init.h
linked_list.o: xed2-intel64/include/xed-decode.h
linked_list.o: xed2-intel64/include/xed-decoded-inst.h
linked_list.o: xed2-intel64/include/xed-common-defs.h
linked_list.o: xed2-intel64/include/xed-portability.h
linked_list.o: xed2-intel64/include/xed-util.h
linked_list.o: xed2-intel64/include/xed-operand-values-interface.h
linked_list.o: xed2-intel64/include/xed-state.h
linked_list.o: xed2-intel64/include/xed-address-width-enum.h
linked_list.o: xed2-intel64/include/xed-machine-mode-enum.h
linked_list.o: xed2-intel64/include/xed-operand-storage.h
linked_list.o: xed2-intel64/include/xed-reg-enum.h
linked_list.o: xed2-intel64/include/xed-iclass-enum.h
linked_list.o: xed2-intel64/include/xed-inst.h
linked_list.o: xed2-intel64/include/xed-category-enum.h
linked_list.o: xed2-intel64/include/xed-extension-enum.h
linked_list.o: xed2-intel64/include/xed-operand-visibility-enum.h
linked_list.o: xed2-intel64/include/xed-operand-action-enum.h
linked_list.o: xed2-intel64/include/xed-operand-convert-enum.h
linked_list.o: xed2-intel64/include/xed-operand-type-enum.h
linked_list.o: xed2-intel64/include/xed-nonterminal-enum.h
linked_list.o: xed2-intel64/include/xed-operand-width-enum.h
linked_list.o: xed2-intel64/include/xed-operand-element-xtype-enum.h
linked_list.o: xed2-intel64/include/xed-attribute-enum.h
linked_list.o: xed2-intel64/include/xed-exception-enum.h
linked_list.o: xed2-intel64/include/xed-iform-enum.h
linked_list.o: xed2-intel64/include/xed-iform-map.h
linked_list.o: xed2-intel64/include/xed-isa-set-enum.h
linked_list.o: xed2-intel64/include/xed-attributes.h
linked_list.o: xed2-intel64/include/xed-gen-table-defs.h
linked_list.o: xed2-intel64/include/xed-flags.h
linked_list.o: xed2-intel64/include/xed-flag-enum.h
linked_list.o: xed2-intel64/include/xed-flag-action-enum.h
linked_list.o: xed2-intel64/include/xed-encoder-gen-defs.h
linked_list.o: xed2-intel64/include/xed-chip-enum.h
linked_list.o: xed2-intel64/include/xed-operand-element-type-enum.h
linked_list.o: xed2-intel64/include/xed-error-enum.h
linked_list.o: xed2-intel64/include/xed-decode-cache.h
linked_list.o: xed2-intel64/include/xed-syntax-enum.h
linked_list.o: xed2-intel64/include/xed-reg-class-enum.h
linked_list.o: xed2-intel64/include/xed-reg-class.h
linked_list.o: xed2-intel64/include/xed-inst-printer.h
linked_list.o: xed2-intel64/include/xed-encode.h
linked_list.o: xed2-intel64/include/xed-encoder-iforms.h
linked_list.o: xed2-intel64/include/xed-encoder-hl.h
linked_list.o: xed2-intel64/include/xed-operand-action.h
linked_list.o: xed2-intel64/include/xed-version.h
linked_list.o: xed2-intel64/include/xed-disas.h
linked_list.o: xed2-intel64/include/xed-format-options.h
linked_list.o: xed2-intel64/include/xed-iformfl-enum.h
linked_list.o: xed2-intel64/include/xed-agen.h linked_list.h ./enumerator.h
utils.o: utils.h /usr/include/stdio.h /usr/include/features.h
utils.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
utils.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
utils.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
utils.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
utils.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
utils.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
utils.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
utils.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
utils.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
utils.o: /usr/include/time.h /usr/include/sys/select.h
utils.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
utils.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
utils.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
utils.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
utils.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
utils.o: /usr/include/bits/confname.h /usr/include/getopt.h
utils.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
utils.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
utils.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
utils.o: /usr/include/bits/stat.h /usr/include/string.h
utils.o: /usr/include/xlocale.h /usr/include/inttypes.h /usr/include/stdint.h
utils.o: /usr/include/bits/wchar.h /usr/include/ctype.h
utils.o: /usr/include/sys/stat.h
region.o: region.h utils.h /usr/include/stdio.h /usr/include/features.h
region.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
region.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
region.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
region.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
region.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
region.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
region.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
region.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
region.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
region.o: /usr/include/time.h /usr/include/sys/select.h
region.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
region.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
region.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
region.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
region.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
region.o: /usr/include/bits/confname.h /usr/include/getopt.h
region.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
region.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
region.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
region.o: /usr/include/bits/stat.h /usr/include/string.h
region.o: /usr/include/xlocale.h /usr/include/inttypes.h
region.o: /usr/include/stdint.h /usr/include/bits/wchar.h
region.o: /usr/include/ctype.h /usr/include/sys/stat.h chunk.h
chain.o: chain.h utils.h /usr/include/stdio.h /usr/include/features.h
chain.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
chain.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
chain.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
chain.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
chain.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
chain.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
chain.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
chain.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
chain.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
chain.o: /usr/include/time.h /usr/include/sys/select.h
chain.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
chain.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
chain.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
chain.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
chain.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
chain.o: /usr/include/bits/confname.h /usr/include/getopt.h
chain.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
chain.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
chain.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
chain.o: /usr/include/bits/stat.h /usr/include/string.h
chain.o: /usr/include/xlocale.h /usr/include/inttypes.h /usr/include/stdint.h
chain.o: /usr/include/bits/wchar.h /usr/include/ctype.h
chain.o: /usr/include/sys/stat.h chunk.h map.h
chain.o: xed2-intel64/include/xed-interface.h
chain.o: xed2-intel64/include/xed-common-hdrs.h
chain.o: xed2-intel64/include/xed-types.h
chain.o: xed2-intel64/include/xed-operand-enum.h
chain.o: xed2-intel64/include/xed-init.h xed2-intel64/include/xed-decode.h
chain.o: xed2-intel64/include/xed-decoded-inst.h
chain.o: xed2-intel64/include/xed-common-defs.h
chain.o: xed2-intel64/include/xed-portability.h
chain.o: xed2-intel64/include/xed-util.h
chain.o: xed2-intel64/include/xed-operand-values-interface.h
chain.o: xed2-intel64/include/xed-state.h
chain.o: xed2-intel64/include/xed-address-width-enum.h
chain.o: xed2-intel64/include/xed-machine-mode-enum.h
chain.o: xed2-intel64/include/xed-operand-storage.h
chain.o: xed2-intel64/include/xed-reg-enum.h
chain.o: xed2-intel64/include/xed-iclass-enum.h
chain.o: xed2-intel64/include/xed-inst.h
chain.o: xed2-intel64/include/xed-category-enum.h
chain.o: xed2-intel64/include/xed-extension-enum.h
chain.o: xed2-intel64/include/xed-operand-visibility-enum.h
chain.o: xed2-intel64/include/xed-operand-action-enum.h
chain.o: xed2-intel64/include/xed-operand-convert-enum.h
chain.o: xed2-intel64/include/xed-operand-type-enum.h
chain.o: xed2-intel64/include/xed-nonterminal-enum.h
chain.o: xed2-intel64/include/xed-operand-width-enum.h
chain.o: xed2-intel64/include/xed-operand-element-xtype-enum.h
chain.o: xed2-intel64/include/xed-attribute-enum.h
chain.o: xed2-intel64/include/xed-exception-enum.h
chain.o: xed2-intel64/include/xed-iform-enum.h
chain.o: xed2-intel64/include/xed-iform-map.h
chain.o: xed2-intel64/include/xed-isa-set-enum.h
chain.o: xed2-intel64/include/xed-attributes.h
chain.o: xed2-intel64/include/xed-gen-table-defs.h
chain.o: xed2-intel64/include/xed-flags.h
chain.o: xed2-intel64/include/xed-flag-enum.h
chain.o: xed2-intel64/include/xed-flag-action-enum.h
chain.o: xed2-intel64/include/xed-encoder-gen-defs.h
chain.o: xed2-intel64/include/xed-chip-enum.h
chain.o: xed2-intel64/include/xed-operand-element-type-enum.h
chain.o: xed2-intel64/include/xed-error-enum.h
chain.o: xed2-intel64/include/xed-decode-cache.h
chain.o: xed2-intel64/include/xed-syntax-enum.h
chain.o: xed2-intel64/include/xed-reg-class-enum.h
chain.o: xed2-intel64/include/xed-reg-class.h
chain.o: xed2-intel64/include/xed-inst-printer.h
chain.o: xed2-intel64/include/xed-encode.h
chain.o: xed2-intel64/include/xed-encoder-iforms.h
chain.o: xed2-intel64/include/xed-encoder-hl.h
chain.o: xed2-intel64/include/xed-operand-action.h
chain.o: xed2-intel64/include/xed-version.h xed2-intel64/include/xed-disas.h
chain.o: xed2-intel64/include/xed-format-options.h
chain.o: xed2-intel64/include/xed-iformfl-enum.h
chain.o: xed2-intel64/include/xed-agen.h linked_list.h ./enumerator.h
chain.o: /usr/include/setjmp.h /usr/include/bits/setjmp.h
chain.o: qemu/include/exec/user/abitypes.h qemu/target-i386/cpu.h
chain.o: qemu/include/config.h qemu/config-host.h
chain.o: qemu/x86_64-linux-user/config-target.h qemu/include/qemu-common.h
chain.o: qemu/include/qemu/compiler.h qemu/include/qemu/typedefs.h
chain.o: /usr/include/strings.h /usr/include/limits.h
chain.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
chain.o: /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h
chain.o: /usr/include/errno.h /usr/include/bits/errno.h
chain.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
chain.o: /usr/include/asm-generic/errno.h
chain.o: /usr/include/asm-generic/errno-base.h /usr/include/sys/time.h
chain.o: /usr/include/assert.h /usr/include/signal.h
chain.o: /usr/include/bits/signum.h /usr/include/bits/siginfo.h
chain.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
chain.o: /usr/include/bits/sigstack.h /usr/include/sys/ucontext.h
chain.o: /usr/include/bits/sigthread.h qemu/include/glib-compat.h
chain.o: /usr/include/glib-2.0/glib.h /usr/include/glib-2.0/glib/galloca.h
chain.o: /usr/include/glib-2.0/glib/gtypes.h
chain.o: /usr/lib/glib-2.0/include/glibconfig.h
chain.o: /usr/include/glib-2.0/glib/gmacros.h
chain.o: /usr/include/glib-2.0/glib/gversionmacros.h
chain.o: /usr/include/glib-2.0/glib/garray.h
chain.o: /usr/include/glib-2.0/glib/gasyncqueue.h
chain.o: /usr/include/glib-2.0/glib/gthread.h
chain.o: /usr/include/glib-2.0/glib/gatomic.h
chain.o: /usr/include/glib-2.0/glib/gerror.h
chain.o: /usr/include/glib-2.0/glib/gquark.h
chain.o: /usr/include/glib-2.0/glib/gbacktrace.h
chain.o: /usr/include/glib-2.0/glib/gbase64.h
chain.o: /usr/include/glib-2.0/glib/gbitlock.h
chain.o: /usr/include/glib-2.0/glib/gbookmarkfile.h
chain.o: /usr/include/glib-2.0/glib/gbytes.h
chain.o: /usr/include/glib-2.0/glib/gcharset.h
chain.o: /usr/include/glib-2.0/glib/gchecksum.h
chain.o: /usr/include/glib-2.0/glib/gconvert.h
chain.o: /usr/include/glib-2.0/glib/gdataset.h
chain.o: /usr/include/glib-2.0/glib/gdate.h
chain.o: /usr/include/glib-2.0/glib/gdatetime.h
chain.o: /usr/include/glib-2.0/glib/gtimezone.h
chain.o: /usr/include/glib-2.0/glib/gdir.h /usr/include/dirent.h
chain.o: /usr/include/bits/dirent.h /usr/include/glib-2.0/glib/genviron.h
chain.o: /usr/include/glib-2.0/glib/gfileutils.h
chain.o: /usr/include/glib-2.0/glib/ggettext.h
chain.o: /usr/include/glib-2.0/glib/ghash.h
chain.o: /usr/include/glib-2.0/glib/glist.h /usr/include/glib-2.0/glib/gmem.h
chain.o: /usr/include/glib-2.0/glib/gnode.h
chain.o: /usr/include/glib-2.0/glib/ghmac.h
chain.o: /usr/include/glib-2.0/glib/gchecksum.h
chain.o: /usr/include/glib-2.0/glib/ghook.h
chain.o: /usr/include/glib-2.0/glib/ghostutils.h
chain.o: /usr/include/glib-2.0/glib/giochannel.h
chain.o: /usr/include/glib-2.0/glib/gmain.h
chain.o: /usr/include/glib-2.0/glib/gpoll.h
chain.o: /usr/include/glib-2.0/glib/gslist.h
chain.o: /usr/include/glib-2.0/glib/gstring.h
chain.o: /usr/include/glib-2.0/glib/gunicode.h
chain.o: /usr/include/glib-2.0/glib/gutils.h
chain.o: /usr/include/glib-2.0/glib/gkeyfile.h
chain.o: /usr/include/glib-2.0/glib/gmappedfile.h
chain.o: /usr/include/glib-2.0/glib/gmarkup.h
chain.o: /usr/include/glib-2.0/glib/gmessages.h
chain.o: /usr/include/glib-2.0/glib/goption.h
chain.o: /usr/include/glib-2.0/glib/gpattern.h
chain.o: /usr/include/glib-2.0/glib/gprimes.h
chain.o: /usr/include/glib-2.0/glib/gqsort.h
chain.o: /usr/include/glib-2.0/glib/gqueue.h
chain.o: /usr/include/glib-2.0/glib/grand.h
chain.o: /usr/include/glib-2.0/glib/gregex.h
chain.o: /usr/include/glib-2.0/glib/gscanner.h
chain.o: /usr/include/glib-2.0/glib/gsequence.h
chain.o: /usr/include/glib-2.0/glib/gshell.h
chain.o: /usr/include/glib-2.0/glib/gslice.h
chain.o: /usr/include/glib-2.0/glib/gspawn.h
chain.o: /usr/include/glib-2.0/glib/gstrfuncs.h
chain.o: /usr/include/glib-2.0/glib/gstringchunk.h
chain.o: /usr/include/glib-2.0/glib/gtestutils.h
chain.o: /usr/include/glib-2.0/glib/gthreadpool.h
chain.o: /usr/include/glib-2.0/glib/gtimer.h
chain.o: /usr/include/glib-2.0/glib/gtrashstack.h
chain.o: /usr/include/glib-2.0/glib/gtree.h
chain.o: /usr/include/glib-2.0/glib/gurifuncs.h
chain.o: /usr/include/glib-2.0/glib/gvarianttype.h
chain.o: /usr/include/glib-2.0/glib/gvariant.h
chain.o: /usr/include/glib-2.0/glib/gversion.h
chain.o: /usr/include/glib-2.0/glib/deprecated/gallocator.h
chain.o: /usr/include/glib-2.0/glib/deprecated/gcache.h
chain.o: /usr/include/glib-2.0/glib/deprecated/gcompletion.h
chain.o: /usr/include/glib-2.0/glib/deprecated/gmain.h
chain.o: /usr/include/glib-2.0/glib/deprecated/grel.h
chain.o: /usr/include/glib-2.0/glib/deprecated/gthread.h
chain.o: /usr/include/pthread.h /usr/include/sched.h
chain.o: /usr/include/bits/sched.h qemu/include/sysemu/os-posix.h
chain.o: qemu/include/qemu/osdep.h /usr/include/sys/wait.h
chain.o: /usr/include/sys/uio.h /usr/include/bits/uio.h
chain.o: qemu/include/qemu/bswap.h qemu/include/fpu/softfloat.h
chain.o: /usr/include/byteswap.h qemu/include/qemu/module.h
chain.o: qemu/include/exec/cpu-defs.h qemu/include/qemu/queue.h
chain.o: qemu/include/qemu/atomic.h qemu/target-i386/cpu-qom.h
chain.o: qemu/include/qom/cpu.h qemu/include/hw/qdev-core.h
chain.o: qemu/include/qemu/option.h qemu/include/qapi/error.h
chain.o: qemu/qapi-types.h qemu/include/qapi/qmp/qdict.h
chain.o: qemu/include/qapi/qmp/qobject.h qemu/include/qapi/qmp/qlist.h
chain.o: qemu/include/qemu/bitmap.h qemu/include/qemu/bitops.h
chain.o: qemu/include/qemu/host-utils.h qemu/include/qom/object.h
chain.o: qemu/include/hw/irq.h qemu/include/hw/hotplug.h
chain.o: qemu/include/exec/hwaddr.h qemu/include/qemu/thread.h
chain.o: qemu/include/qemu/thread-posix.h /usr/include/semaphore.h
chain.o: /usr/include/bits/semaphore.h qemu/include/qemu/tls.h
chain.o: qemu/include/exec/cpu-all.h qemu/include/exec/cpu-common.h
chain.o: qemu/include/exec/poison.h qemu/include/exec/memory.h
chain.o: qemu/include/exec/user/abitypes.h qemu/target-i386/svm.h
chain.o: qemu/include/exec/exec-all.h qemu/include/qemu/log.h
chain.o: qemu/include/exec/spinlock.h qemu/include/exec/exec-all.h
map.o: map.h utils.h /usr/include/stdio.h /usr/include/features.h
map.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
map.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
map.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
map.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
map.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
map.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
map.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
map.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
map.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
map.o: /usr/include/time.h /usr/include/sys/select.h
map.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
map.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
map.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
map.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
map.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
map.o: /usr/include/bits/confname.h /usr/include/getopt.h
map.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
map.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
map.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
map.o: /usr/include/bits/stat.h /usr/include/string.h /usr/include/xlocale.h
map.o: /usr/include/inttypes.h /usr/include/stdint.h
map.o: /usr/include/bits/wchar.h /usr/include/ctype.h /usr/include/sys/stat.h
map.o: xed2-intel64/include/xed-interface.h
map.o: xed2-intel64/include/xed-common-hdrs.h
map.o: xed2-intel64/include/xed-types.h
map.o: xed2-intel64/include/xed-operand-enum.h
map.o: xed2-intel64/include/xed-init.h xed2-intel64/include/xed-decode.h
map.o: xed2-intel64/include/xed-decoded-inst.h
map.o: xed2-intel64/include/xed-common-defs.h
map.o: xed2-intel64/include/xed-portability.h xed2-intel64/include/xed-util.h
map.o: xed2-intel64/include/xed-operand-values-interface.h
map.o: xed2-intel64/include/xed-state.h
map.o: xed2-intel64/include/xed-address-width-enum.h
map.o: xed2-intel64/include/xed-machine-mode-enum.h
map.o: xed2-intel64/include/xed-operand-storage.h
map.o: xed2-intel64/include/xed-reg-enum.h
map.o: xed2-intel64/include/xed-iclass-enum.h xed2-intel64/include/xed-inst.h
map.o: xed2-intel64/include/xed-category-enum.h
map.o: xed2-intel64/include/xed-extension-enum.h
map.o: xed2-intel64/include/xed-operand-visibility-enum.h
map.o: xed2-intel64/include/xed-operand-action-enum.h
map.o: xed2-intel64/include/xed-operand-convert-enum.h
map.o: xed2-intel64/include/xed-operand-type-enum.h
map.o: xed2-intel64/include/xed-nonterminal-enum.h
map.o: xed2-intel64/include/xed-operand-width-enum.h
map.o: xed2-intel64/include/xed-operand-element-xtype-enum.h
map.o: xed2-intel64/include/xed-attribute-enum.h
map.o: xed2-intel64/include/xed-exception-enum.h
map.o: xed2-intel64/include/xed-iform-enum.h
map.o: xed2-intel64/include/xed-iform-map.h
map.o: xed2-intel64/include/xed-isa-set-enum.h
map.o: xed2-intel64/include/xed-attributes.h
map.o: xed2-intel64/include/xed-gen-table-defs.h
map.o: xed2-intel64/include/xed-flags.h xed2-intel64/include/xed-flag-enum.h
map.o: xed2-intel64/include/xed-flag-action-enum.h
map.o: xed2-intel64/include/xed-encoder-gen-defs.h
map.o: xed2-intel64/include/xed-chip-enum.h
map.o: xed2-intel64/include/xed-operand-element-type-enum.h
map.o: xed2-intel64/include/xed-error-enum.h
map.o: xed2-intel64/include/xed-decode-cache.h
map.o: xed2-intel64/include/xed-syntax-enum.h
map.o: xed2-intel64/include/xed-reg-class-enum.h
map.o: xed2-intel64/include/xed-reg-class.h
map.o: xed2-intel64/include/xed-inst-printer.h
map.o: xed2-intel64/include/xed-encode.h
map.o: xed2-intel64/include/xed-encoder-iforms.h
map.o: xed2-intel64/include/xed-encoder-hl.h
map.o: xed2-intel64/include/xed-operand-action.h
map.o: xed2-intel64/include/xed-version.h xed2-intel64/include/xed-disas.h
map.o: xed2-intel64/include/xed-format-options.h
map.o: xed2-intel64/include/xed-iformfl-enum.h
map.o: xed2-intel64/include/xed-agen.h linked_list.h ./enumerator.h
constraints.o: constraints.h utils.h /usr/include/stdio.h
constraints.o: /usr/include/features.h /usr/include/stdc-predef.h
constraints.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
constraints.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
constraints.o: /usr/include/bits/typesizes.h /usr/include/libio.h
constraints.o: /usr/include/_G_config.h /usr/include/wchar.h
constraints.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
constraints.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
constraints.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
constraints.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
constraints.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
constraints.o: /usr/include/time.h /usr/include/sys/select.h
constraints.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
constraints.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
constraints.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
constraints.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
constraints.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
constraints.o: /usr/include/bits/confname.h /usr/include/getopt.h
constraints.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
constraints.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
constraints.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
constraints.o: /usr/include/bits/stat.h /usr/include/string.h
constraints.o: /usr/include/xlocale.h /usr/include/inttypes.h
constraints.o: /usr/include/stdint.h /usr/include/bits/wchar.h
constraints.o: /usr/include/ctype.h /usr/include/sys/stat.h
constraints.o: xed2-intel64/include/xed-interface.h
constraints.o: xed2-intel64/include/xed-common-hdrs.h
constraints.o: xed2-intel64/include/xed-types.h
constraints.o: xed2-intel64/include/xed-operand-enum.h
constraints.o: xed2-intel64/include/xed-init.h
constraints.o: xed2-intel64/include/xed-decode.h
constraints.o: xed2-intel64/include/xed-decoded-inst.h
constraints.o: xed2-intel64/include/xed-common-defs.h
constraints.o: xed2-intel64/include/xed-portability.h
constraints.o: xed2-intel64/include/xed-util.h
constraints.o: xed2-intel64/include/xed-operand-values-interface.h
constraints.o: xed2-intel64/include/xed-state.h
constraints.o: xed2-intel64/include/xed-address-width-enum.h
constraints.o: xed2-intel64/include/xed-machine-mode-enum.h
constraints.o: xed2-intel64/include/xed-operand-storage.h
constraints.o: xed2-intel64/include/xed-reg-enum.h
constraints.o: xed2-intel64/include/xed-iclass-enum.h
constraints.o: xed2-intel64/include/xed-inst.h
constraints.o: xed2-intel64/include/xed-category-enum.h
constraints.o: xed2-intel64/include/xed-extension-enum.h
constraints.o: xed2-intel64/include/xed-operand-visibility-enum.h
constraints.o: xed2-intel64/include/xed-operand-action-enum.h
constraints.o: xed2-intel64/include/xed-operand-convert-enum.h
constraints.o: xed2-intel64/include/xed-operand-type-enum.h
constraints.o: xed2-intel64/include/xed-nonterminal-enum.h
constraints.o: xed2-intel64/include/xed-operand-width-enum.h
constraints.o: xed2-intel64/include/xed-operand-element-xtype-enum.h
constraints.o: xed2-intel64/include/xed-attribute-enum.h
constraints.o: xed2-intel64/include/xed-exception-enum.h
constraints.o: xed2-intel64/include/xed-iform-enum.h
constraints.o: xed2-intel64/include/xed-iform-map.h
constraints.o: xed2-intel64/include/xed-isa-set-enum.h
constraints.o: xed2-intel64/include/xed-attributes.h
constraints.o: xed2-intel64/include/xed-gen-table-defs.h
constraints.o: xed2-intel64/include/xed-flags.h
constraints.o: xed2-intel64/include/xed-flag-enum.h
constraints.o: xed2-intel64/include/xed-flag-action-enum.h
constraints.o: xed2-intel64/include/xed-encoder-gen-defs.h
constraints.o: xed2-intel64/include/xed-chip-enum.h
constraints.o: xed2-intel64/include/xed-operand-element-type-enum.h
constraints.o: xed2-intel64/include/xed-error-enum.h
constraints.o: xed2-intel64/include/xed-decode-cache.h
constraints.o: xed2-intel64/include/xed-syntax-enum.h
constraints.o: xed2-intel64/include/xed-reg-class-enum.h
constraints.o: xed2-intel64/include/xed-reg-class.h
constraints.o: xed2-intel64/include/xed-inst-printer.h
constraints.o: xed2-intel64/include/xed-encode.h
constraints.o: xed2-intel64/include/xed-encoder-iforms.h
constraints.o: xed2-intel64/include/xed-encoder-hl.h
constraints.o: xed2-intel64/include/xed-operand-action.h
constraints.o: xed2-intel64/include/xed-version.h
constraints.o: xed2-intel64/include/xed-disas.h
constraints.o: xed2-intel64/include/xed-format-options.h
constraints.o: xed2-intel64/include/xed-iformfl-enum.h
constraints.o: xed2-intel64/include/xed-agen.h linked_list.h ./enumerator.h
parsers/section.o: parsers/section.h utils.h /usr/include/stdio.h
parsers/section.o: /usr/include/features.h /usr/include/stdc-predef.h
parsers/section.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
parsers/section.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
parsers/section.o: /usr/include/bits/typesizes.h /usr/include/libio.h
parsers/section.o: /usr/include/_G_config.h /usr/include/wchar.h
parsers/section.o: /usr/include/bits/stdio_lim.h
parsers/section.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
parsers/section.o: /usr/include/bits/waitflags.h
parsers/section.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
parsers/section.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
parsers/section.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
parsers/section.o: /usr/include/time.h /usr/include/sys/select.h
parsers/section.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
parsers/section.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
parsers/section.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
parsers/section.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
parsers/section.o: /usr/include/bits/posix_opt.h
parsers/section.o: /usr/include/bits/environments.h
parsers/section.o: /usr/include/bits/confname.h /usr/include/getopt.h
parsers/section.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
parsers/section.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
parsers/section.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
parsers/section.o: /usr/include/bits/stat.h /usr/include/string.h
parsers/section.o: /usr/include/xlocale.h /usr/include/inttypes.h
parsers/section.o: /usr/include/stdint.h /usr/include/bits/wchar.h
parsers/section.o: /usr/include/ctype.h /usr/include/sys/stat.h chunk.h
parsers/section.o: linked_list.h ./enumerator.h /usr/include/elf.h
parsers/program_header.o: parsers/program_header.h utils.h
parsers/program_header.o: /usr/include/stdio.h /usr/include/features.h
parsers/program_header.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
parsers/program_header.o: /usr/include/bits/wordsize.h
parsers/program_header.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
parsers/program_header.o: /usr/include/bits/typesizes.h /usr/include/libio.h
parsers/program_header.o: /usr/include/_G_config.h /usr/include/wchar.h
parsers/program_header.o: /usr/include/bits/stdio_lim.h
parsers/program_header.o: /usr/include/bits/sys_errlist.h
parsers/program_header.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
parsers/program_header.o: /usr/include/bits/waitstatus.h
parsers/program_header.o: /usr/include/endian.h /usr/include/bits/endian.h
parsers/program_header.o: /usr/include/bits/byteswap.h
parsers/program_header.o: /usr/include/bits/byteswap-16.h
parsers/program_header.o: /usr/include/sys/types.h /usr/include/time.h
parsers/program_header.o: /usr/include/sys/select.h
parsers/program_header.o: /usr/include/bits/select.h
parsers/program_header.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
parsers/program_header.o: /usr/include/sys/sysmacros.h
parsers/program_header.o: /usr/include/bits/pthreadtypes.h
parsers/program_header.o: /usr/include/alloca.h
parsers/program_header.o: /usr/include/bits/stdlib-float.h
parsers/program_header.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
parsers/program_header.o: /usr/include/bits/environments.h
parsers/program_header.o: /usr/include/bits/confname.h /usr/include/getopt.h
parsers/program_header.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
parsers/program_header.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
parsers/program_header.o: /usr/include/bits/fcntl.h
parsers/program_header.o: /usr/include/bits/fcntl-linux.h
parsers/program_header.o: /usr/include/bits/stat.h /usr/include/string.h
parsers/program_header.o: /usr/include/xlocale.h /usr/include/inttypes.h
parsers/program_header.o: /usr/include/stdint.h /usr/include/bits/wchar.h
parsers/program_header.o: /usr/include/ctype.h /usr/include/sys/stat.h
parsers/program_header.o: chunk.h /usr/include/elf.h
parsers/parser_builder.o: parsers/parser_builder.h utils.h
parsers/parser_builder.o: /usr/include/stdio.h /usr/include/features.h
parsers/parser_builder.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
parsers/parser_builder.o: /usr/include/bits/wordsize.h
parsers/parser_builder.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
parsers/parser_builder.o: /usr/include/bits/typesizes.h /usr/include/libio.h
parsers/parser_builder.o: /usr/include/_G_config.h /usr/include/wchar.h
parsers/parser_builder.o: /usr/include/bits/stdio_lim.h
parsers/parser_builder.o: /usr/include/bits/sys_errlist.h
parsers/parser_builder.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
parsers/parser_builder.o: /usr/include/bits/waitstatus.h
parsers/parser_builder.o: /usr/include/endian.h /usr/include/bits/endian.h
parsers/parser_builder.o: /usr/include/bits/byteswap.h
parsers/parser_builder.o: /usr/include/bits/byteswap-16.h
parsers/parser_builder.o: /usr/include/sys/types.h /usr/include/time.h
parsers/parser_builder.o: /usr/include/sys/select.h
parsers/parser_builder.o: /usr/include/bits/select.h
parsers/parser_builder.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
parsers/parser_builder.o: /usr/include/sys/sysmacros.h
parsers/parser_builder.o: /usr/include/bits/pthreadtypes.h
parsers/parser_builder.o: /usr/include/alloca.h
parsers/parser_builder.o: /usr/include/bits/stdlib-float.h
parsers/parser_builder.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
parsers/parser_builder.o: /usr/include/bits/environments.h
parsers/parser_builder.o: /usr/include/bits/confname.h /usr/include/getopt.h
parsers/parser_builder.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
parsers/parser_builder.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
parsers/parser_builder.o: /usr/include/bits/fcntl.h
parsers/parser_builder.o: /usr/include/bits/fcntl-linux.h
parsers/parser_builder.o: /usr/include/bits/stat.h /usr/include/string.h
parsers/parser_builder.o: /usr/include/xlocale.h /usr/include/inttypes.h
parsers/parser_builder.o: /usr/include/stdint.h /usr/include/bits/wchar.h
parsers/parser_builder.o: /usr/include/ctype.h /usr/include/sys/stat.h
parsers/parser_builder.o: parsers/parser.h region.h chunk.h code.h
parsers/parser_builder.o: linked_list.h ./enumerator.h
parsers/elf_type.o: parsers/elf_type.h code.h utils.h /usr/include/stdio.h
parsers/elf_type.o: /usr/include/features.h /usr/include/stdc-predef.h
parsers/elf_type.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
parsers/elf_type.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
parsers/elf_type.o: /usr/include/bits/typesizes.h /usr/include/libio.h
parsers/elf_type.o: /usr/include/_G_config.h /usr/include/wchar.h
parsers/elf_type.o: /usr/include/bits/stdio_lim.h
parsers/elf_type.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
parsers/elf_type.o: /usr/include/bits/waitflags.h
parsers/elf_type.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
parsers/elf_type.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
parsers/elf_type.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
parsers/elf_type.o: /usr/include/time.h /usr/include/sys/select.h
parsers/elf_type.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
parsers/elf_type.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
parsers/elf_type.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
parsers/elf_type.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
parsers/elf_type.o: /usr/include/bits/posix_opt.h
parsers/elf_type.o: /usr/include/bits/environments.h
parsers/elf_type.o: /usr/include/bits/confname.h /usr/include/getopt.h
parsers/elf_type.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
parsers/elf_type.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
parsers/elf_type.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
parsers/elf_type.o: /usr/include/bits/stat.h /usr/include/string.h
parsers/elf_type.o: /usr/include/xlocale.h /usr/include/inttypes.h
parsers/elf_type.o: /usr/include/stdint.h /usr/include/bits/wchar.h
parsers/elf_type.o: /usr/include/ctype.h /usr/include/sys/stat.h region.h
parsers/elf_type.o: chunk.h linked_list.h ./enumerator.h parsers/section.h
parsers/elf_type.o: /usr/include/elf.h parsers/program_header.h
parsers/elf_type.o: xed2-intel64/include/xed-portability.h
parsers/elf_type.o: xed2-intel64/include/xed-types.h
parsers/elf_type.o: xed2-intel64/include/xed-common-hdrs.h
parsers/pe_type.o: parsers/pe_type.h code.h utils.h /usr/include/stdio.h
parsers/pe_type.o: /usr/include/features.h /usr/include/stdc-predef.h
parsers/pe_type.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
parsers/pe_type.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
parsers/pe_type.o: /usr/include/bits/typesizes.h /usr/include/libio.h
parsers/pe_type.o: /usr/include/_G_config.h /usr/include/wchar.h
parsers/pe_type.o: /usr/include/bits/stdio_lim.h
parsers/pe_type.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
parsers/pe_type.o: /usr/include/bits/waitflags.h
parsers/pe_type.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
parsers/pe_type.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
parsers/pe_type.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
parsers/pe_type.o: /usr/include/time.h /usr/include/sys/select.h
parsers/pe_type.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
parsers/pe_type.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
parsers/pe_type.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
parsers/pe_type.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
parsers/pe_type.o: /usr/include/bits/posix_opt.h
parsers/pe_type.o: /usr/include/bits/environments.h
parsers/pe_type.o: /usr/include/bits/confname.h /usr/include/getopt.h
parsers/pe_type.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
parsers/pe_type.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
parsers/pe_type.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
parsers/pe_type.o: /usr/include/bits/stat.h /usr/include/string.h
parsers/pe_type.o: /usr/include/xlocale.h /usr/include/inttypes.h
parsers/pe_type.o: /usr/include/stdint.h /usr/include/bits/wchar.h
parsers/pe_type.o: /usr/include/ctype.h /usr/include/sys/stat.h region.h
parsers/pe_type.o: chunk.h linked_list.h ./enumerator.h parsers/section.h
parsers/pe_type.o: /usr/include/elf.h parsers/program_header.h
parsers/pe_type.o: xed2-intel64/include/xed-portability.h
parsers/pe_type.o: xed2-intel64/include/xed-types.h
parsers/pe_type.o: xed2-intel64/include/xed-common-hdrs.h
parsers/parser_elf.o: parsers/parser_elf.h parsers/parser.h utils.h
parsers/parser_elf.o: /usr/include/stdio.h /usr/include/features.h
parsers/parser_elf.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
parsers/parser_elf.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
parsers/parser_elf.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
parsers/parser_elf.o: /usr/include/libio.h /usr/include/_G_config.h
parsers/parser_elf.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
parsers/parser_elf.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
parsers/parser_elf.o: /usr/include/bits/waitflags.h
parsers/parser_elf.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
parsers/parser_elf.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
parsers/parser_elf.o: /usr/include/bits/byteswap-16.h
parsers/parser_elf.o: /usr/include/sys/types.h /usr/include/time.h
parsers/parser_elf.o: /usr/include/sys/select.h /usr/include/bits/select.h
parsers/parser_elf.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
parsers/parser_elf.o: /usr/include/sys/sysmacros.h
parsers/parser_elf.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
parsers/parser_elf.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
parsers/parser_elf.o: /usr/include/bits/posix_opt.h
parsers/parser_elf.o: /usr/include/bits/environments.h
parsers/parser_elf.o: /usr/include/bits/confname.h /usr/include/getopt.h
parsers/parser_elf.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
parsers/parser_elf.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
parsers/parser_elf.o: /usr/include/bits/fcntl.h
parsers/parser_elf.o: /usr/include/bits/fcntl-linux.h
parsers/parser_elf.o: /usr/include/bits/stat.h /usr/include/string.h
parsers/parser_elf.o: /usr/include/xlocale.h /usr/include/inttypes.h
parsers/parser_elf.o: /usr/include/stdint.h /usr/include/bits/wchar.h
parsers/parser_elf.o: /usr/include/ctype.h /usr/include/sys/stat.h region.h
parsers/parser_elf.o: chunk.h code.h linked_list.h ./enumerator.h
parsers/parser_elf.o: /usr/include/elf.h parsers/elf_type.h parsers/section.h
parsers/parser_elf.o: parsers/program_header.h
parsers/parser_elf.o: xed2-intel64/include/xed-portability.h
parsers/parser_elf.o: xed2-intel64/include/xed-types.h
parsers/parser_elf.o: xed2-intel64/include/xed-common-hdrs.h
parsers/parser_pe.o: parsers/parser_pe.h parsers/parser.h utils.h
parsers/parser_pe.o: /usr/include/stdio.h /usr/include/features.h
parsers/parser_pe.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
parsers/parser_pe.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
parsers/parser_pe.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
parsers/parser_pe.o: /usr/include/libio.h /usr/include/_G_config.h
parsers/parser_pe.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
parsers/parser_pe.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
parsers/parser_pe.o: /usr/include/bits/waitflags.h
parsers/parser_pe.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
parsers/parser_pe.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
parsers/parser_pe.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
parsers/parser_pe.o: /usr/include/time.h /usr/include/sys/select.h
parsers/parser_pe.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
parsers/parser_pe.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
parsers/parser_pe.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
parsers/parser_pe.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
parsers/parser_pe.o: /usr/include/bits/posix_opt.h
parsers/parser_pe.o: /usr/include/bits/environments.h
parsers/parser_pe.o: /usr/include/bits/confname.h /usr/include/getopt.h
parsers/parser_pe.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
parsers/parser_pe.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
parsers/parser_pe.o: /usr/include/bits/fcntl.h
parsers/parser_pe.o: /usr/include/bits/fcntl-linux.h /usr/include/bits/stat.h
parsers/parser_pe.o: /usr/include/string.h /usr/include/xlocale.h
parsers/parser_pe.o: /usr/include/inttypes.h /usr/include/stdint.h
parsers/parser_pe.o: /usr/include/bits/wchar.h /usr/include/ctype.h
parsers/parser_pe.o: /usr/include/sys/stat.h region.h chunk.h code.h
parsers/parser_pe.o: linked_list.h ./enumerator.h parsers/pe_type.h
parsers/parser_pe.o: parsers/section.h /usr/include/elf.h
parsers/parser_pe.o: parsers/program_header.h
parsers/parser_pe.o: xed2-intel64/include/xed-portability.h
parsers/parser_pe.o: xed2-intel64/include/xed-types.h
parsers/parser_pe.o: xed2-intel64/include/xed-common-hdrs.h
plugins/plugin_builder.o: plugins/plugin_builder.h utils.h
plugins/plugin_builder.o: /usr/include/stdio.h /usr/include/features.h
plugins/plugin_builder.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
plugins/plugin_builder.o: /usr/include/bits/wordsize.h
plugins/plugin_builder.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
plugins/plugin_builder.o: /usr/include/bits/typesizes.h /usr/include/libio.h
plugins/plugin_builder.o: /usr/include/_G_config.h /usr/include/wchar.h
plugins/plugin_builder.o: /usr/include/bits/stdio_lim.h
plugins/plugin_builder.o: /usr/include/bits/sys_errlist.h
plugins/plugin_builder.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
plugins/plugin_builder.o: /usr/include/bits/waitstatus.h
plugins/plugin_builder.o: /usr/include/endian.h /usr/include/bits/endian.h
plugins/plugin_builder.o: /usr/include/bits/byteswap.h
plugins/plugin_builder.o: /usr/include/bits/byteswap-16.h
plugins/plugin_builder.o: /usr/include/sys/types.h /usr/include/time.h
plugins/plugin_builder.o: /usr/include/sys/select.h
plugins/plugin_builder.o: /usr/include/bits/select.h
plugins/plugin_builder.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
plugins/plugin_builder.o: /usr/include/sys/sysmacros.h
plugins/plugin_builder.o: /usr/include/bits/pthreadtypes.h
plugins/plugin_builder.o: /usr/include/alloca.h
plugins/plugin_builder.o: /usr/include/bits/stdlib-float.h
plugins/plugin_builder.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
plugins/plugin_builder.o: /usr/include/bits/environments.h
plugins/plugin_builder.o: /usr/include/bits/confname.h /usr/include/getopt.h
plugins/plugin_builder.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
plugins/plugin_builder.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
plugins/plugin_builder.o: /usr/include/bits/fcntl.h
plugins/plugin_builder.o: /usr/include/bits/fcntl-linux.h
plugins/plugin_builder.o: /usr/include/bits/stat.h /usr/include/string.h
plugins/plugin_builder.o: /usr/include/xlocale.h /usr/include/inttypes.h
plugins/plugin_builder.o: /usr/include/stdint.h /usr/include/bits/wchar.h
plugins/plugin_builder.o: /usr/include/ctype.h /usr/include/sys/stat.h
plugins/plugin_builder.o: plugins/plugin.h code.h region.h chunk.h
plugins/plugin_builder.o: linked_list.h ./enumerator.h
plugins/plugin_rop.o: plugins/plugin_rop.h plugins/plugin.h code.h utils.h
plugins/plugin_rop.o: /usr/include/stdio.h /usr/include/features.h
plugins/plugin_rop.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
plugins/plugin_rop.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
plugins/plugin_rop.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
plugins/plugin_rop.o: /usr/include/libio.h /usr/include/_G_config.h
plugins/plugin_rop.o: /usr/include/wchar.h /usr/include/bits/stdio_lim.h
plugins/plugin_rop.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
plugins/plugin_rop.o: /usr/include/bits/waitflags.h
plugins/plugin_rop.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
plugins/plugin_rop.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
plugins/plugin_rop.o: /usr/include/bits/byteswap-16.h
plugins/plugin_rop.o: /usr/include/sys/types.h /usr/include/time.h
plugins/plugin_rop.o: /usr/include/sys/select.h /usr/include/bits/select.h
plugins/plugin_rop.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
plugins/plugin_rop.o: /usr/include/sys/sysmacros.h
plugins/plugin_rop.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
plugins/plugin_rop.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
plugins/plugin_rop.o: /usr/include/bits/posix_opt.h
plugins/plugin_rop.o: /usr/include/bits/environments.h
plugins/plugin_rop.o: /usr/include/bits/confname.h /usr/include/getopt.h
plugins/plugin_rop.o: /usr/include/sys/mman.h /usr/include/bits/mman.h
plugins/plugin_rop.o: /usr/include/bits/mman-linux.h /usr/include/fcntl.h
plugins/plugin_rop.o: /usr/include/bits/fcntl.h
plugins/plugin_rop.o: /usr/include/bits/fcntl-linux.h
plugins/plugin_rop.o: /usr/include/bits/stat.h /usr/include/string.h
plugins/plugin_rop.o: /usr/include/xlocale.h /usr/include/inttypes.h
plugins/plugin_rop.o: /usr/include/stdint.h /usr/include/bits/wchar.h
plugins/plugin_rop.o: /usr/include/ctype.h /usr/include/sys/stat.h region.h
plugins/plugin_rop.o: chunk.h linked_list.h ./enumerator.h
plugins/plugin_rop.o: xed2-intel64/include/xed-interface.h
plugins/plugin_rop.o: xed2-intel64/include/xed-common-hdrs.h
plugins/plugin_rop.o: xed2-intel64/include/xed-types.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-init.h
plugins/plugin_rop.o: xed2-intel64/include/xed-decode.h
plugins/plugin_rop.o: xed2-intel64/include/xed-decoded-inst.h
plugins/plugin_rop.o: xed2-intel64/include/xed-common-defs.h
plugins/plugin_rop.o: xed2-intel64/include/xed-portability.h
plugins/plugin_rop.o: xed2-intel64/include/xed-util.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-values-interface.h
plugins/plugin_rop.o: xed2-intel64/include/xed-state.h
plugins/plugin_rop.o: xed2-intel64/include/xed-address-width-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-machine-mode-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-storage.h
plugins/plugin_rop.o: xed2-intel64/include/xed-reg-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-iclass-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-inst.h
plugins/plugin_rop.o: xed2-intel64/include/xed-category-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-extension-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-visibility-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-action-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-convert-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-type-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-nonterminal-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-width-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-element-xtype-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-attribute-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-exception-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-iform-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-iform-map.h
plugins/plugin_rop.o: xed2-intel64/include/xed-isa-set-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-attributes.h
plugins/plugin_rop.o: xed2-intel64/include/xed-gen-table-defs.h
plugins/plugin_rop.o: xed2-intel64/include/xed-flags.h
plugins/plugin_rop.o: xed2-intel64/include/xed-flag-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-flag-action-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-encoder-gen-defs.h
plugins/plugin_rop.o: xed2-intel64/include/xed-chip-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-element-type-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-error-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-decode-cache.h
plugins/plugin_rop.o: xed2-intel64/include/xed-syntax-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-reg-class-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-reg-class.h
plugins/plugin_rop.o: xed2-intel64/include/xed-inst-printer.h
plugins/plugin_rop.o: xed2-intel64/include/xed-encode.h
plugins/plugin_rop.o: xed2-intel64/include/xed-encoder-iforms.h
plugins/plugin_rop.o: xed2-intel64/include/xed-encoder-hl.h
plugins/plugin_rop.o: xed2-intel64/include/xed-operand-action.h
plugins/plugin_rop.o: xed2-intel64/include/xed-version.h
plugins/plugin_rop.o: xed2-intel64/include/xed-disas.h
plugins/plugin_rop.o: xed2-intel64/include/xed-format-options.h
plugins/plugin_rop.o: xed2-intel64/include/xed-iformfl-enum.h
plugins/plugin_rop.o: xed2-intel64/include/xed-agen.h /usr/include/elf.h
plugins/plugin_rop.o: parsers/elf_type.h parsers/section.h
plugins/plugin_rop.o: parsers/program_header.h constraints.h chain.h map.h
